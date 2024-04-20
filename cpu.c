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
}

void
gbc_cpu_connect(gbc_cpu_t *cpu, gbc_memory_t *mem)
{
    cpu->mem_data = mem;
    cpu->mem_read = mem->read;
    cpu->mem_write = mem->write;    
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

    /* the max length of an GBC instruction is 3 bytes */
    uint8_t code[3];
    code[0] = cpu->mem_read(cpu->mem_data, pc);
    code[1] = cpu->mem_read(cpu->mem_data, pc+1);
    code[2] = cpu->mem_read(cpu->mem_data, pc+2);

    instruction_t *ins = decode_mem(cpu->mem_read, pc, cpu->mem_data);
    if (!ins->func) {
        LOG_ERROR("Unknown instruction [0x%x]\n", ins->opcode);
        abort();
    }

    ins->func(cpu, ins);

    cpu->ins_cycles = ins->cycles - 1;
    WRITE_R16(cpu, REG_PC, pc + ins->size);
}