#include "instruction_set.h"
#include "common.h"
#include "cpu.h"

void 
stop(gbc_cpu_t *cpu, void *op1, void *op2)
{
    printf("STOP\n");
}

void 
inc_r8(gbc_cpu_t *cpu, void *op1, void *op2)
{
    printf("INC r8\n");
}

void 
inc_r16(gbc_cpu_t *cpu, void *op1, void *op2)
{
    printf("INC r16\n");
}

void 
inc_m16(gbc_cpu_t *cpu, void *op1, void *op2)
{
    printf("INC m16\n");
}

void 
dec_r8(gbc_cpu_t *cpu, void *op1, void *op2)
{
    printf("DEC r8\n");
}

void 
dec_r16(gbc_cpu_t *cpu, void *op1, void *op2)
{
    printf("DEC r16\n");
}

void 
dec_m16(gbc_cpu_t *cpu, void *op1, void *op2)
{
    printf("DEC m16\n");
}   

void
rlca(gbc_cpu_t *cpu, void *op1, void *op2)
{
    printf("RLCA\n");
}

void
rla(gbc_cpu_t *cpu, void *op1, void *op2)
{
    printf("RLA\n");
}

void
daa(gbc_cpu_t *cpu, void *op1, void *op2)
{
    printf("DAA\n");
}

void
scf(gbc_cpu_t *cpu, void *op1, void *op2)
{
    printf("SCF\n");
}

void
jr(gbc_cpu_t *cpu, void *op1, void *op2)
{
    printf("JR\n");
}

void
jr_cc(gbc_cpu_t *cpu, void *op1, void *op2)
{
    printf("JR cc\n");
}

void 
nop(gbc_cpu_t *cpu, void *op1, void *op2)
{
    printf("NOP\n");
}

void
ld_r16_i16(gbc_cpu_t *cpu, void *op1, void *op2)
{
    printf("LD r16, i16\n");
}

void
ld_r8_i8(gbc_cpu_t *cpu, void *op1, void *op2)
{
    printf("LD r8, i8\n");
}

void 
ld_m16_a(gbc_cpu_t *cpu, void *op1, void *op2)
{
    printf("LD m16, a\n");
}

void 
ld_r16_m16(gbc_cpu_t *cpu, void *op1, void *op2)
{
    printf("LD r16, m16\n");    
}

static instruction_t instruction_set[INSTRUCTIONS_SET_SIZE] = {
    INSTRUCTION_ADD(0x00, 1, nop, REG_AF, REG_B, "NOP"),
    INSTRUCTION_ADD(0xfe, 1, nop, NULL, NULL, "T0xfe"),
};

static instruction_t prefixed_instruction_set[INSTRUCTIONS_SET_SIZE] = {
    INSTRUCTION_ADD(0x10, 1, nop, NULL, NULL, "T0x10"),    
};

void 
init_instruction_set()
{    
    int idx;
    instruction_t t;
    for (int i = 0; i < INSTRUCTIONS_SET_SIZE; i++) {
        
        if (instruction_set[i].func) {
            idx = instruction_set[i].opcode;            
            if (idx != i) {
                t = instruction_set[idx];
                instruction_set[idx] = instruction_set[i];      
                instruction_set[i] = t;            
            }
        }

        if (prefixed_instruction_set[i].func) {            
            idx = prefixed_instruction_set[i].opcode;
            if (idx != i) {
                t = instruction_set[idx];
                prefixed_instruction_set[idx] = prefixed_instruction_set[i];
                instruction_set[i] = t;
            }
        }
    }       
    printf("%s\n", instruction_set[0xfe].name);
    printf("%s\n", prefixed_instruction_set[0x10].name);
}

instruction_t 
decode(gbc_cpu_t *cpu, uint8_t *data)
{
    uint8_t opcode = data[0];    
    instruction_t *inst_set = instruction_set;

    if (opcode == PREFIX_CB) {
        // TODO prefixed instructions
        inst_set = prefixed_instruction_set;
    }
    
    instruction_t inst = inst_set[opcode];
    inst.opcode = opcode;
    if (inst.size == 2) {
        inst.opcode_ext.a8 = READ_8(*(data + 1));
    } else if (inst.size == 3) {
        /* immediate value is little-endian */
        inst.opcode_ext.n16 = READ_16(*(uint16_t*)(data + 1));
    } else {
        fprintf(stderr, "Invalid instruction, imme size [%d]", inst.size);
        abort();
    }

    return inst;
}
