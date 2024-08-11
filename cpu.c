#include <string.h>
#include "cpu.h"
#include "instruction_set.h"

void
gbc_cpu_init(gbc_cpu_t *cpu)
{
    memset(cpu, 0, sizeof(gbc_cpu_t));

    /* https://gbdev.io/pandocs/Power_Up_Sequence.html#cpu-registers */
    WRITE_R16(cpu, REG_PC, 0x0100);
    WRITE_R16(cpu, REG_SP, 0xFFFE);
    cpu->ime = 0;
}

static uint8_t
ier_read(void *udata, uint16_t addr)
{
    LOG_DEBUG("[CPU] Reading from IE register\n");
    return ((gbc_cpu_t*)udata)->ier;
}

static uint8_t
ier_write(void *udata, uint16_t addr, uint8_t val)
{
    LOG_DEBUG("[CPU] Writing to IE register [%x]\n", val);    
    ((gbc_cpu_t*)udata)->ier = val;
    return val;
}

void
gbc_cpu_connect(gbc_cpu_t *cpu, gbc_memory_t *mem)
{
    cpu->mem_data = mem;
    cpu->mem_read = mem->read;
    cpu->mem_write = mem->write;    

    memory_map_entry_t entry;
    entry.read = ier_read;
    entry.write = ier_write;

    entry.udata = cpu;
    entry.id = IE_REGISTER_ID;
    entry.addr_begin = IE_REGISTER_BEGIN;
    entry.addr_end = IE_REGISTER_END;

    register_memory_map(mem, &entry);
    cpu->ifp = connect_io_port(mem, IO_PORT_IF);
}

uint8_t
gbc_cpu_interrupt(gbc_cpu_t *cpu)
{   
    if (cpu->ime == 0)
        return 0;

    /* only the least significant 5 bits are used */
    uint8_t ints = (cpu->ier & *cpu->ifp) & INTERRUPT_MASK;
    if (ints) {
        /* disable interrupts */
        cpu->ime = 0;
        /* according to interrupt priorities */        
        uint16_t pc = 0;
        if (ints & INTERRUPT_VBLANK) {
            *cpu->ifp &= ~INTERRUPT_VBLANK;
            pc = INT_HANDLER_VBLANK;
        } else if (ints & INTERRUPT_LCD_STAT) {
            *cpu->ifp &= ~INTERRUPT_LCD_STAT;
            pc = INT_HANDLER_LCD_STAT;
        } else if (ints & INTERRUPT_TIMER) {
            *cpu->ifp &= ~INTERRUPT_TIMER;
            pc = INT_HANDLER_TIMER;
        } else if (ints & INTERRUPT_SERIAL) {
            *cpu->ifp &= ~INTERRUPT_SERIAL;
            pc = INT_HANDLER_SERIAL;
        } else if (ints & INTERRUPT_JOYPAD) {
            *cpu->ifp &= ~INTERRUPT_JOYPAD;
            pc = INT_HANDLER_JOYPAD;
        }

        assert(pc != 0);
        int_call_i16(cpu, pc);
        return 1;
    }

    return 0;
}

static void
print_cpu_stat(gbc_cpu_t *cpu)
{
    cpu_register_t *r = &cpu->regs;

    printf("{PC: 0x%x, SP: 0x%x, AF: 0x%x, BC: 0x%x, DE: 0x%x, HL: 0x%x, C: %d, Z: %d, N: %d, H: %d, IME: %x, IE: %x, IF: %x}: M-Cycles: %d\n",            
           READ_R16(r, REG_PC), READ_R16(r, REG_SP), READ_R16(r, REG_AF),
           READ_R16(r, REG_BC), READ_R16(r, REG_DE), READ_R16(r, REG_HL),
           READ_R_FLAG(r, FLAG_C), READ_R_FLAG(r, FLAG_Z), READ_R_FLAG(r, FLAG_N), READ_R_FLAG(r, FLAG_H),
              cpu->ime, cpu->ier, *cpu->ifp, cpu->cycles / 4
           );
}

void
gbc_cpu_cycle(gbc_cpu_t *cpu)
{    
    cpu->cycles++;
    
    if (cpu->ins_cycles) {
        /* Waiting for instruction to run up its cycles,
           the operation is already completed 
        */
        cpu->ins_cycles--;
        return;
    }

    /* di instruction will enable ime AFTER the next instruction */    
    if (cpu->ime_insts) {
        cpu->ime_insts--;
        if (cpu->ime_insts == 0)
            cpu->ime = 1;
    }

    /* if there are pending interrupts, the cpu is awaken */
    if ((cpu->ier & *cpu->ifp) & INTERRUPT_MASK) {
        cpu->halt = 0;
    }

    if (gbc_cpu_interrupt(cpu)) {
        #if LOGLEVEL == LOG_LEVEL_DEBUG
        print_cpu_stat(cpu);
        #endif
        return;
    }

    if (cpu->halt) {
        return;
    }

    uint16_t pc = READ_R16(cpu, REG_PC);
    instruction_t *ins = decode_mem(cpu->mem_read, pc, cpu->mem_data);

    if (!ins->func) {
        LOG_ERROR("Unknown instruction [0x%x]\n", ins->opcode);
        abort();
    }

    WRITE_R16(cpu, REG_PC, pc + ins->size);
    ins->func(cpu, ins);

    /* https://forums.nesdev.org/viewtopic.php?t=12815 
        The lower 4 bits of the F register are always 0    
        Blargg test cpu_instrs/01-special
    */
    WRITE_R8(cpu, REG_F, READ_R8(cpu, REG_F) & 0xF0);

    cpu->ins_cycles = ins->r_cycles - 1;

    #if LOGLEVEL == LOG_LEVEL_DEBUG
    print_cpu_stat(cpu);
    #endif    
}