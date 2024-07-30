#include <string.h>
#include "cpu.h"
#include "instruction_set.h"

void
gbc_cpu_init(gbc_cpu_t *cpu)
{
    memset(cpu, 0, sizeof(gbc_cpu_t));

    /* https://gbdev.io/pandocs/Power_Up_Sequence.html#cpu-registers */
    //WRITE_R16(cpu, REG_PC, 0x0100);
    WRITE_R16(cpu, REG_SP, 0xFFFE);
    cpu->ier = 0;
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

    cpu->ins_cycles = ins->cycles - 1;
}