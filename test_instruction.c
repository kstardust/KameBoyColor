#include "memory.h"
#include <assert.h>

static uint8_t _flat_mem[0xffff]; // 64KB

static uint8_t 
_mem_write(void *udata, uint16_t addr, uint8_t data)
{
    printf("Writing to memory at address %x [%x]\n", addr, data);
    _flat_mem[addr] = data;
    return data;
}

static uint8_t 
_mem_read(void *udata, uint16_t addr)
{
    printf("Reading from memory at address %x\n", addr);
    return _flat_mem[addr];
}

static void 
test_inc_r16(gbc_cpu_t *cpu)
{   
    cpu_register_t *reg = &(cpu->regs);
    WRITE_R16(reg, REG_BC, 0x1234);
    uint8_t code[] = {0x03}; // INC BC
    instruction_t *inst = decode(code);
    inst->func(cpu, inst);
    assert(READ_R16(reg, REG_BC) == 0x1235);

    WRITE_R16(reg, REG_BC, 0xffff);    
    inst->func(cpu, inst);
    assert(READ_R16(reg, REG_BC) == 0x0000);
}

static void 
test_dec_r16(gbc_cpu_t *cpu)
{
    cpu_register_t *reg = &(cpu->regs);
    WRITE_R16(reg, REG_BC, 0x1234);
    uint8_t code[] = {0x0b}; // DEC BC
    instruction_t *inst = decode(code);
    inst->func(cpu, inst);
    assert(READ_R16(reg, REG_BC) == 0x1233);

    WRITE_R16(reg, REG_BC, 0x0000);
    inst->func(cpu, inst);
    assert(READ_R16(reg, REG_BC) == 0xffff);    
}

static void
test_inc_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *reg = &(cpu->regs);
    WRITE_R8(reg, REG_B, 0x34);
    uint8_t code[] = {0x04}; // INC B

    instruction_t *inst = decode(code);
    inst->func(cpu, inst);

    assert(READ_R8(reg, REG_B) == 0x35);
    assert(READ_R_FLAG(reg, FLAG_Z) == 0);
    assert(READ_R_FLAG(reg, FLAG_N) == 0);
    assert(READ_R_FLAG(reg, FLAG_H) == 0);

    WRITE_R8(reg, REG_B, 0xff);

    uint8_t h = READ_R8(reg, REG_C);

    inst = decode(code);
    inst->func(cpu, inst);
    
    assert(READ_R8(reg, REG_B) == 0);
    assert(READ_R8(reg, REG_C) == h);
    assert(READ_R_FLAG(reg, FLAG_Z) == 1);
    assert(READ_R_FLAG(reg, FLAG_N) == 0);
    assert(READ_R_FLAG(reg, FLAG_H) == 1);
}

static void
test_dec_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *reg = &(cpu->regs);
    WRITE_R8(reg, REG_B, 0x01);
    uint8_t code[] = {0x05}; // DEC B

    instruction_t *inst = decode(code);
    inst->func(cpu, inst);

    assert(READ_R8(reg, REG_B) == 0);
    assert(READ_R_FLAG(reg, FLAG_Z) == 1);
    assert(READ_R_FLAG(reg, FLAG_N) == 1);
    assert(READ_R_FLAG(reg, FLAG_H) == 0);

    WRITE_R8(reg, REG_B, 0x00);

    uint8_t h = READ_R8(reg, REG_C);

    inst = decode(code);
    inst->func(cpu, inst);
    
    assert(READ_R8(reg, REG_B) == 0xff);
    assert(READ_R8(reg, REG_C) == h);
    assert(READ_R_FLAG(reg, FLAG_Z) == 0);
    assert(READ_R_FLAG(reg, FLAG_N) == 1);
    assert(READ_R_FLAG(reg, FLAG_H) == 1);
}

static void 
test_inc_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *reg = &(cpu->regs);
    const uint16_t addr = 0x34;
    WRITE_R16(reg, REG_HL, addr);
    cpu->mem_write(cpu->mem_data, addr, 0x34);
    uint8_t code[] = {0x34}; // INC HL

    instruction_t *inst = decode(code);
    inst->func(cpu, inst);

    assert(cpu->mem_read(cpu->mem_data, addr) == 0x35);
    assert(READ_R_FLAG(reg, FLAG_Z) == 0);
    assert(READ_R_FLAG(reg, FLAG_N) == 0);
    assert(READ_R_FLAG(reg, FLAG_H) == 0);

    cpu->mem_write(cpu->mem_data, addr, 0xff);
    uint8_t h = READ_R8(reg, REG_C);

    inst = decode(code);
    inst->func(cpu, inst);
    
    assert(cpu->mem_read(cpu->mem_data, addr) == 0);        
    assert(READ_R_FLAG(reg, FLAG_Z) == 1);
    assert(READ_R_FLAG(reg, FLAG_N) == 0);
    assert(READ_R_FLAG(reg, FLAG_H) == 1);
}

static void 
test_dec_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *reg = &(cpu->regs);
    const uint16_t addr = 0x34;
    WRITE_R16(reg, REG_HL, addr);
    cpu->mem_write(cpu->mem_data, addr, 0x01);
    uint8_t code[] = {0x35}; // DEC (HL)

    instruction_t *inst = decode(code);
    inst->func(cpu, inst);

    assert(cpu->mem_read(cpu->mem_data, addr) == 0);
    assert(READ_R_FLAG(reg, FLAG_Z) == 1);
    assert(READ_R_FLAG(reg, FLAG_N) == 1);
    assert(READ_R_FLAG(reg, FLAG_H) == 0);

    cpu->mem_write(cpu->mem_data, addr, 0x0);
    uint8_t h = READ_R8(reg, REG_C);

    inst = decode(code);
    inst->func(cpu, inst);
    
    assert(cpu->mem_read(cpu->mem_data, addr) == 0xff);
    assert(READ_R_FLAG(reg, FLAG_Z) == 0);
    assert(READ_R_FLAG(reg, FLAG_N) == 1);
    assert(READ_R_FLAG(reg, FLAG_H) == 1);
}

static void 
test_rlca(gbc_cpu_t *cpu)
{    
    cpu_register_t *reg = &(cpu->regs);
    WRITE_R8(reg, REG_A, 0x85);
    uint8_t code[] = {0x07}; // RLCA

    instruction_t *inst = decode(code);
    inst->func(cpu, inst);

    assert(READ_R8(reg, REG_A) == 0x0b);
    assert(READ_R_FLAG(reg, FLAG_C) == 1);
    assert(READ_R_FLAG(reg, FLAG_Z) == 0);
    assert(READ_R_FLAG(reg, FLAG_N) == 0);
    assert(READ_R_FLAG(reg, FLAG_H) == 0);

    WRITE_R8(reg, REG_A, 0x1);

    inst = decode(code);
    inst->func(cpu, inst);

    assert(READ_R8(reg, REG_A) == 0x2);
    assert(READ_R_FLAG(reg, FLAG_C) == 0);
    assert(READ_R_FLAG(reg, FLAG_Z) == 0);
    assert(READ_R_FLAG(reg, FLAG_N) == 0);
    assert(READ_R_FLAG(reg, FLAG_H) == 0);
}

static void 
test_rla(gbc_cpu_t *cpu)
{    
    cpu_register_t *reg = &(cpu->regs);
    WRITE_R8(reg, REG_A, 0x02);
    uint8_t code[] = {0x17}; // RLA

    SET_R_FLAG(reg, FLAG_C);

    instruction_t *inst = decode(code);
    inst->func(cpu, inst);

    assert(READ_R8(reg, REG_A) == 0x05);
    assert(READ_R_FLAG(reg, FLAG_C) == 0);
    assert(READ_R_FLAG(reg, FLAG_Z) == 0);
    assert(READ_R_FLAG(reg, FLAG_N) == 0);
    assert(READ_R_FLAG(reg, FLAG_H) == 0);

    WRITE_R8(reg, REG_A, 0xff);
    inst = decode(code);
    inst->func(cpu, inst);
   
    assert(READ_R8(reg, REG_A) == 0xfe);
    assert(READ_R_FLAG(reg, FLAG_C) == 1);
    assert(READ_R_FLAG(reg, FLAG_Z) == 0);
    assert(READ_R_FLAG(reg, FLAG_N) == 0);
    assert(READ_R_FLAG(reg, FLAG_H) == 0);    
}

static void 
test_rra(gbc_cpu_t *cpu)
{    
    cpu_register_t *reg = &(cpu->regs);
    WRITE_R8(reg, REG_A, 0x02);
    uint8_t code[] = {0x1f}; // RRA

    SET_R_FLAG(reg, FLAG_C);
    instruction_t *inst = decode(code);
    inst->func(cpu, inst);

    assert(READ_R8(reg, REG_A) == 0x81);
    assert(READ_R_FLAG(reg, FLAG_C) == 0);
    assert(READ_R_FLAG(reg, FLAG_Z) == 0);
    assert(READ_R_FLAG(reg, FLAG_N) == 0);
    assert(READ_R_FLAG(reg, FLAG_H) == 0);

    WRITE_R8(reg, REG_A, 0x1);
    inst = decode(code);
    inst->func(cpu, inst);

    assert(READ_R8(reg, REG_A) == 0);
    assert(READ_R_FLAG(reg, FLAG_C) == 1);
    assert(READ_R_FLAG(reg, FLAG_Z) == 0);
    assert(READ_R_FLAG(reg, FLAG_N) == 0);
    assert(READ_R_FLAG(reg, FLAG_H) == 0);    
}

static void 
test_rrca(gbc_cpu_t *cpu)
{
    cpu_register_t *reg = &(cpu->regs);
    WRITE_R8(reg, REG_A, 0x03);
    uint8_t code[] = {0x0f}; // RRCA

    instruction_t *inst = decode(code);
    inst->func(cpu, inst);

    assert(READ_R8(reg, REG_A) == 0x81);
    assert(READ_R_FLAG(reg, FLAG_C) == 1);
    assert(READ_R_FLAG(reg, FLAG_Z) == 0);
    assert(READ_R_FLAG(reg, FLAG_N) == 0);
    assert(READ_R_FLAG(reg, FLAG_H) == 0);

    WRITE_R8(reg, REG_A, 0x2);
    inst = decode(code);
    inst->func(cpu, inst);

    assert(READ_R8(reg, REG_A) == 0x1);
    assert(READ_R_FLAG(reg, FLAG_C) == 0);
    assert(READ_R_FLAG(reg, FLAG_Z) == 0);
    assert(READ_R_FLAG(reg, FLAG_N) == 0);
    assert(READ_R_FLAG(reg, FLAG_H) == 0);    
}

static void 
test_daa(gbc_cpu_t *cpu)
{
    cpu_register_t *reg = &(cpu->regs);
    WRITE_R8(reg, REG_A, 0x1f);
    uint8_t code[] = {0x27}; // DAA

    SET_R_FLAG(reg, FLAG_H);
    CLEAR_R_FLAG(reg, FLAG_C);
    SET_R_FLAG(reg, FLAG_N);

    instruction_t *inst = decode(code);
    inst->func(cpu, inst);
    
    assert(READ_R8(reg, REG_A) == 0x19);
    assert(READ_R_FLAG(reg, FLAG_C) == 0);
    assert(READ_R_FLAG(reg, FLAG_Z) == 0);
    assert(READ_R_FLAG(reg, FLAG_N) == 1);
    assert(READ_R_FLAG(reg, FLAG_H) == 0);

    WRITE_R8(reg, REG_A, 0x41);
    CLEAR_R_FLAG(reg, FLAG_N);
    SET_R_FLAG(reg, FLAG_H);
    CLEAR_R_FLAG(reg, FLAG_C);

    inst = decode(code);
    inst->func(cpu, inst);

    assert(READ_R8(reg, REG_A) == 0x47);
    assert(READ_R_FLAG(reg, FLAG_C) == 0);
    assert(READ_R_FLAG(reg, FLAG_Z) == 0);
    assert(READ_R_FLAG(reg, FLAG_N) == 0);
    assert(READ_R_FLAG(reg, FLAG_H) == 0);
}

static void
test_jr(gbc_cpu_t *cpu)
{
    cpu_register_t *reg = &(cpu->regs);
    WRITE_R16(reg, REG_PC, 0x1000);
    uint8_t code[] = {0x18, 0x02}; // JR 0x02

    instruction_t *inst = decode(code);
    inst->func(cpu, inst);

    assert(READ_R16(reg, REG_PC) == 0x1002);

    WRITE_R16(reg, REG_PC, 0x1000);
    code[1] = 0xfe; // JR -2

    inst = decode(code);
    inst->func(cpu, inst);

    assert(READ_R16(reg, REG_PC) == 0x0ffe);
}

static void
test_ld_r16_i16(gbc_cpu_t *cpu)
{
    cpu_register_t *reg = &(cpu->regs);    
    uint8_t code[] = {0x01, 0x34, 0x12}; // LD BC, 0x1234    
    instruction_t *inst = decode(code);
    inst->func(cpu, inst);

    assert(READ_R16(reg, REG_BC) == 0x1234);
}

static void
test_ld_sp_hl(gbc_cpu_t *cpu)
{
    cpu_register_t *reg = &(cpu->regs);
    WRITE_R16(reg, REG_HL, 0x1234);
    WRITE_R16(reg, REG_SP, 0x0000);
    uint8_t code[] = {0xf9}; // LD SP, HL
    instruction_t *inst = decode(code);
    inst->func(cpu, inst);

    assert(READ_R16(reg, REG_SP) == 0x1234);
}

static void 
test_ld_hl_sp_i8(gbc_cpu_t *cpu)
{
    cpu_register_t *reg = &(cpu->regs);
    WRITE_R16(reg, REG_SP, 0x1001);
    uint8_t code[] = {0xf8, 0x02}; // LD HL, SP+e8
    instruction_t *inst = decode(code);
    inst->func(cpu, inst);

    assert(READ_R16(reg, REG_HL) == 0x1003);
    assert(READ_R_FLAG(reg, FLAG_Z) == 0);
    assert(READ_R_FLAG(reg, FLAG_N) == 0);
    assert(READ_R_FLAG(reg, FLAG_H) == 0);
    assert(READ_R_FLAG(reg, FLAG_C) == 0);

    WRITE_R16(reg, REG_SP, 0x00ff);
    code[1] = 0x2;
    inst = decode(code);
    inst->func(cpu, inst);

    assert(READ_R16(reg, REG_HL) == 0x0101);
    assert(READ_R_FLAG(reg, FLAG_Z) == 0);
    assert(READ_R_FLAG(reg, FLAG_N) == 0);
    assert(READ_R_FLAG(reg, FLAG_H) == 1);
    assert(READ_R_FLAG(reg, FLAG_C) == 1);

    WRITE_R16(reg, REG_SP, 0x1000);
    code[1] = 0xfe; // LD HL, SP-e8

    inst = decode(code);
    inst->func(cpu, inst);

    assert(READ_R16(reg, REG_HL) == 0x0ffe);
    assert(READ_R_FLAG(reg, FLAG_Z) == 0);
    assert(READ_R_FLAG(reg, FLAG_N) == 0);
    assert(READ_R_FLAG(reg, FLAG_H) == 0);
    assert(READ_R_FLAG(reg, FLAG_C) == 0);
}

static void 
test_ld_r8_i8(gbc_cpu_t *cpu)
{
    cpu_register_t *reg = &(cpu->regs);
    WRITE_R8(reg, REG_B, 0x00);
    uint8_t code[] = {0x06, 0x34}; // LD B, 0x34
    instruction_t *inst = decode(code);
    inst->func(cpu, inst);

    assert(READ_R8(reg, REG_B) == 0x34);
}

static void
test_ldi_r8_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *reg = &(cpu->regs);
    WRITE_R16(reg, REG_HL, 0x1000);    
    cpu->mem_write(cpu->mem_data, 0x1000, 0x34);
    uint8_t code[] = {0x2a}; // LDI A, (HL)
    instruction_t *inst = decode(code);
    inst->func(cpu, inst);

    assert(READ_R8(reg, REG_A) == 0x34);
    assert(READ_R16(reg, REG_HL) == 0x1001);
}

static void
test_ldi_m16_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *reg = &(cpu->regs);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x00);
    WRITE_R16(reg, REG_HL, 0x1000);
    WRITE_R8(reg, REG_A, 0xff);
    uint8_t code[] = {0x22}; // LDI (HL), A
    instruction_t *inst = decode(code);
    inst->func(cpu, inst);

    assert(cpu->mem_read(cpu->mem_data, 0x1000) == 0xff);
    assert(READ_R16(reg, REG_HL) == 0x1001);
}

static void
test_ldd_r8_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *reg = &(cpu->regs);
    WRITE_R16(reg, REG_HL, 0x1000);    
    WRITE_R8(reg, REG_A, 0x00);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x34);
    uint8_t code[] = {0x3a}; // LDD A, (HL)
    instruction_t *inst = decode(code);
    inst->func(cpu, inst);

    assert(READ_R8(reg, REG_A) == 0x34);
    assert(READ_R16(reg, REG_HL) == 0x0fff);
}

static void
test_ldd_m16_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *reg = &(cpu->regs);
    WRITE_R16(reg, REG_HL, 0x1000);
    WRITE_R8(reg, REG_A, 0xff);
    uint8_t code[] = {0x32}; // LDD (HL), A
    instruction_t *inst = decode(code);
    inst->func(cpu, inst);

    assert(cpu->mem_read(cpu->mem_data, 0x1000) == 0xff);
    assert(READ_R16(reg, REG_HL) == 0x0fff);
}

static void 
test_ld_m16_i8(gbc_cpu_t *cpu)
{
    cpu_register_t *reg = &(cpu->regs);
    cpu->mem_write(cpu->mem_data, 0x1234, 0x00);
    WRITE_R16(reg, REG_HL, 0x1234);
    uint8_t code[] = {0x36, 0x34}; // LD (HL), 0x34
    instruction_t *inst = decode(code);
    inst->func(cpu, inst);

    assert(cpu->mem_read(cpu->mem_data, 0x1234) == 0x34);    
}

static void
test_ld_r8_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *reg = &(cpu->regs);
    WRITE_R16(reg, REG_BC, 0x1000);
    WRITE_R8(reg, REG_A, 0x00);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x34);
    uint8_t code[] = {0x0a}; // LD A, (BC)
    instruction_t *inst = decode(code);
    inst->func(cpu, inst);

    assert(READ_R8(reg, REG_A) == 0x34);
}

static void
test_ld_m16_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);    
    WRITE_R16(regs, REG_BC, 0x1000);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x0);
    WRITE_R8(regs, REG_A, 0x10);    
    uint8_t code[] = {0x02}; // LD (BC), A
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(cpu->mem_read(cpu->mem_data, 0x1000) == 0x10);
}

static void 
test_ld_im16_r16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    uint8_t code[] = {0x08, 0x34, 0x12}; // LD (0x1234), SP
    WRITE_R16(regs, REG_SP, 0x1001);
    cpu->mem_write(cpu->mem_data, 0x1234, 0x00);
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(cpu->mem_read(cpu->mem_data, 0x1234) == 0x01);
    assert(cpu->mem_read(cpu->mem_data, 0x1235) == 0x10);
}

static void
test_ld_im16_r8(gbc_cpu_t *cpu)
{
    WRITE_R8(&(cpu->regs), REG_A, 0xff);
    cpu->mem_write(cpu->mem_data, 0x1234, 0x0);
    uint8_t code[] = {0xea, 0x34, 0x12}; // LD (0x1234), A
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(cpu->mem_read(cpu->mem_data, 0x1234) == 0xff);
}

static void
test_ld_r8_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_A, 0x00);
    WRITE_R8(regs, REG_B, 0xfe);
    uint8_t code[] = {0x78}; // LD A, B
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0xfe);
}

static void 
test_jr_nz_i8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R16(regs, REG_PC, 0x1000);

    CLEAR_R_FLAG(regs, FLAG_Z);
    uint8_t code[] = {0x20, 0x02}; // JR NZ, 0x02
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R16(regs, REG_PC) == 0x1002);

    assert(ins->r_cycles == ins->cycles2);
    
    code[1] = 0xff;
    ins = decode(code);
    SET_R_FLAG(regs, FLAG_Z);
    ins->func(cpu, ins);
    assert(READ_R16(regs, REG_PC) == 0x1002);
    assert(ins->r_cycles != ins->cycles2);
}

static void 
test_add_r16_r16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R16(regs, REG_BC, 0x1234);
    WRITE_R16(regs, REG_HL, 0x5f78);
    SET_R_FLAG(regs, FLAG_Z);
    uint8_t code[] = {0x09}; // ADD HL, BC
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R16(regs, REG_HL) == 0x71ac);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    WRITE_R16(regs, REG_BC, 0x0001);    
    WRITE_R16(regs, REG_HL, 0xffff);
    code[0] = 0x09;
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R16(regs, REG_HL) == 0x0000);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
}

static void 
test_add_r16_i8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R16(regs, REG_SP, 0x10ff);
    uint8_t code[] = {0xe8, 0x02}; // ADD SP, 0x02
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R16(regs, REG_SP) == 0x1101);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);

    WRITE_R16(regs, REG_SP, 0x1001);
    code[1] = 0xef; // ADD SP, -2
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R16(regs, REG_SP) == 0x0ff0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);    
}

static void 
test_add_r8_i8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_A, 0x01);    
    uint8_t code[] = {0xc6, 0x01}; // ADD A, 0x01
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x02);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    WRITE_R8(regs, REG_A, 0xff);
    code[1] = 0x01;
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x00);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
}

static void 
test_add_r8_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_A, 0x01);
    WRITE_R8(regs, REG_B, 0x02);
    uint8_t code[] = {0x80}; // ADD A, B
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x03);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    WRITE_R8(regs, REG_A, 0xff);
    WRITE_R8(regs, REG_B, 0x01);
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x00);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
}

static void
test_add_r8_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_A, 0x01);
    WRITE_R16(regs, REG_HL, 0x1000);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x02);    
    uint8_t code[] = {0x86}; // ADD A, (HL)
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x03);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    WRITE_R8(regs, REG_A, 0xff);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x01);    
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x00);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);    
}

static void 
test_adc_r8_i8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_A, 0x01);    
    CLEAR_R_FLAG(regs, FLAG_C);
    uint8_t code[] = {0xce, 0x01}; // ADD A, 0x01
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x02);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    SET_R_FLAG(regs, FLAG_C);
    WRITE_R8(regs, REG_A, 0xfe);
    code[1] = 0x01;
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x00);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);    
}

static void 
test_adc_r8_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_A, 0x0f);    
    CLEAR_R_FLAG(regs, FLAG_C);
    uint8_t code[] = {0x8f}; // ADD A, A
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x1e);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    WRITE_R8(regs, REG_A, 0xfe);
    WRITE_R8(regs, REG_B, 0x01);
    SET_R_FLAG(regs, FLAG_C);
    code[0] = 0x88; // ADC A, B
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
}

static void
test_adc_r8_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_A, 0x01);
    WRITE_R16(regs, REG_HL, 0x1000);
    CLEAR_R_FLAG(regs, FLAG_C);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x02);    
    uint8_t code[] = {0x8e}; // ADD A, (HL)
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x03);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    WRITE_R8(regs, REG_A, 0xfe);
    SET_R_FLAG_VALUE(regs, FLAG_C, 1);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x01);    
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x00);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);    
}

static void
test_sub_r8_i8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    CLEAR_R_FLAG(regs, FLAG_Z);
    WRITE_R8(regs, REG_A, 0x01);    
    uint8_t code[] = {0xd6, 0x01}; // SUB A, 0x01
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x00);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 1);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    WRITE_R8(regs, REG_A, 0x01);
    code[1] = 0x02;
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0xff);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 1);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
}

static void
test_sub_r8_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    CLEAR_R_FLAG(regs, FLAG_Z);    
    WRITE_R8(regs, REG_A, 0x01);
    WRITE_R8(regs, REG_B, 0x01);
    uint8_t code[] = {0x90}; // SUB A, B
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x00);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 1);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    WRITE_R8(regs, REG_A, 0x01);
    WRITE_R8(regs, REG_B, 0x02);
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0xff);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 1);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);    
}

static void
test_sub_r8_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_A, 0x01);
    WRITE_R16(regs, REG_HL, 0x1000);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x01);
    uint8_t code[] = {0x96}; // SUB A, (HL)
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x00);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 1);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    WRITE_R8(regs, REG_A, 0x01);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x02);
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0xff);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 1);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);    
}

static void 
test_subc_r8_i8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_A, 0x01);
    CLEAR_R_FLAG(regs, FLAG_C);
    uint8_t code[] = {0xde, 0x01}; // SBC A, 0x01
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 1);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    SET_R_FLAG(regs, FLAG_C);
    WRITE_R8(regs, REG_A, 0x01);
    code[1] = 0x01;
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0xff);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 1);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);    
}

static void 
test_subc_r8_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_A, 0x0f);    
    WRITE_R8(regs, REG_B, 0x0f);
    CLEAR_R_FLAG(regs, FLAG_C);
    uint8_t code[] = {0x98}; // SBC A, B
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x00);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 1);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    WRITE_R8(regs, REG_A, 0x01);
    WRITE_R8(regs, REG_B, 0x01);
    SET_R_FLAG(regs, FLAG_C);    
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0xff);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 1);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
}

static void
test_subc_r8_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_A, 0x01);
    WRITE_R16(regs, REG_HL, 0x1000);
    CLEAR_R_FLAG(regs, FLAG_C);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x01);
    uint8_t code[] = {0x9e}; // SBC A, (HL)
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x00);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 1);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    WRITE_R8(regs, REG_A, 0xff);
    SET_R_FLAG_VALUE(regs, FLAG_C, 1);
    cpu->mem_write(cpu->mem_data, 0x1000, 0xff);
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0xff);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 1);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);    
}

static void 
test_and_r8_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_A, 0x0f);    
    WRITE_R8(regs, REG_B, 0xf1);
    uint8_t code[] = {0xa0}; // AND A, B
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x01);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    WRITE_R8(regs, REG_A, 0x0f);
    WRITE_R8(regs, REG_B, 0xf0);
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x00);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);    
}

static void 
test_and_r8_i8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_A, 0x0f);        
    uint8_t code[] = {0xe6, 0xf1}; // AND A, 0xf1
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x01);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    WRITE_R8(regs, REG_A, 0x0f);
    code[1] = 0xf0;    
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x00);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);
}

static void 
test_and_r8_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_A, 0x0f);        
    WRITE_R16(regs, REG_HL, 0x1000);
    cpu->mem_write(cpu->mem_data, 0x1000, 0xf1);
    uint8_t code[] = {0xa6}; // AND A, (HL)
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x01);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    WRITE_R8(regs, REG_A, 0x0f);
    cpu->mem_write(cpu->mem_data, 0x1000, 0xf0);
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x00);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);
}

static void 
test_or_r8_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_A, 0x0f);    
    WRITE_R8(regs, REG_B, 0xf1);
    uint8_t code[] = {0xb0}; // OR A, B
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0xff);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    WRITE_R8(regs, REG_A, 0x00);
    WRITE_R8(regs, REG_B, 0x00);
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x00);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);
}

static void 
test_or_r8_i8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_A, 0x0f);        
    uint8_t code[] = {0xf6, 0xf1}; // OR A, 0xf1
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0xff);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    WRITE_R8(regs, REG_A, 0x00);
    code[1] = 0x0;
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x00);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);    
}

static void 
test_or_r8_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_A, 0x0f);        
    WRITE_R16(regs, REG_HL, 0x1000);
    cpu->mem_write(cpu->mem_data, 0x1000, 0xf1);
    uint8_t code[] = {0xb6}; // OR A, (HL)
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0xff);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    WRITE_R8(regs, REG_A, 0x00);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x00);
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x00);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);    
}

static void 
test_xor_r8_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_A, 0x0f);    
    WRITE_R8(regs, REG_B, 0xf1);
    uint8_t code[] = {0xa8}; // XOR A, B
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0xfe);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    WRITE_R8(regs, REG_A, 0xff);
    WRITE_R8(regs, REG_B, 0xff);
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x00);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);    
}

static void 
test_xor_r8_i8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_A, 0x0f);        
    uint8_t code[] = {0xee, 0xf1}; // XOR A, 0xf1
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0xfe);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    WRITE_R8(regs, REG_A, 0xff);
    code[1] = 0xff;
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x00);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);    
}

static void 
test_xor_r8_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_A, 0x0f);        
    WRITE_R16(regs, REG_HL, 0x1000);
    cpu->mem_write(cpu->mem_data, 0x1000, 0xf1);
    uint8_t code[] = {0xae}; // XOR A, (HL)
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0xfe);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    WRITE_R8(regs, REG_A, 0xff);
    cpu->mem_write(cpu->mem_data, 0x1000, 0xff);
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x00);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);    
}

static void
test_cp_r8_i8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    CLEAR_R_FLAG(regs, FLAG_Z);
    WRITE_R8(regs, REG_A, 0x01);    
    uint8_t code[] = {0xfe, 0x01}; // CP A, 0x01
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x01);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 1);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    WRITE_R8(regs, REG_A, 0x01);
    code[1] = 0x02;
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x01);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 1);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
}

static void
test_cp_r8_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    CLEAR_R_FLAG(regs, FLAG_Z);    
    WRITE_R8(regs, REG_A, 0x01);
    WRITE_R8(regs, REG_B, 0x01);
    uint8_t code[] = {0xb8}; // CP A, B
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x01);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 1);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    WRITE_R8(regs, REG_A, 0x01);
    WRITE_R8(regs, REG_B, 0x02);
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x01);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 1);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);    
}

static void
test_cp_r8_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_A, 0x01);
    WRITE_R16(regs, REG_HL, 0x1000);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x01);
    uint8_t code[] = {0xbe}; // CP A, (HL)
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x01);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 1);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);

    WRITE_R8(regs, REG_A, 0x01);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x02);
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x01);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 1);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);    
}

static void 
test_push_pop_r16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R16(regs, REG_BC, 0x1234);

    WRITE_R16(regs, REG_SP, 0x1000);
    uint8_t code[] = {0xc5}; // PUSH BC
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);

    assert(cpu->mem_read(cpu->mem_data, 0x0fff) == 0x12);
    assert(cpu->mem_read(cpu->mem_data, 0x0ffe) == 0x34);
    assert(READ_R16(regs, REG_SP) == 0x0ffe);

    WRITE_R16(regs, REG_BC, 0x0000);
    code[0] = 0xc1; // POP BC
    ins = decode(code);
    ins->func(cpu, ins);

    assert(READ_R16(regs, REG_BC) == 0x1234);
    assert(READ_R16(regs, REG_SP) == 0x1000);
    assert(READ_R8(regs, REG_C) == 0x34);
    assert(READ_R8(regs, REG_B) == 0x12);
}

static void 
test_push_pop_af(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R16(regs, REG_AF, 0x1100);

    WRITE_R16(regs, REG_SP, 0x1000);
    SET_R_FLAG(regs, FLAG_C);
    CLEAR_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    CLEAR_R_FLAG(regs, FLAG_N);

    uint8_t code[] = {0xf5}; // PUSH AF
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);

    assert(READ_R16(regs, REG_SP) == 0x0ffe);

    WRITE_R16(regs, REG_AF, 0x0000);
    code[0] = 0xf1; // POP AF
    ins = decode(code);
    ins->func(cpu, ins);

    assert(READ_R8(regs, REG_A) == 0x11);
    assert(READ_R_FLAG(regs, FLAG_C) == 1); 
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);    
}

static void 
test_ret_nz(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R16(regs, REG_BC, 0x1234);    
    WRITE_R16(regs, REG_SP, 0x1000);
    WRITE_R16(regs, REG_PC, 0x0);
        
    uint8_t code[] = {0xc5}; // PUSH BC
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);

    SET_R_FLAG(regs, FLAG_Z);
    code[0] = 0xc0; // RET NZ
    ins = decode(code);
    ins->func(cpu, ins);

    assert(READ_R16(regs, REG_PC) == 0x0);
    assert(READ_R16(regs, REG_SP) == 0x0ffe);

    CLEAR_R_FLAG(regs, FLAG_Z);
    ins = decode(code);
    ins->func(cpu, ins);

    assert(READ_R16(regs, REG_PC) == 0x1234);
    assert(READ_R16(regs, REG_SP) == 0x1000);
}

static void 
test_ret_z(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R16(regs, REG_BC, 0x1234);    
    WRITE_R16(regs, REG_SP, 0x1000);
    WRITE_R16(regs, REG_PC, 0x0);
        
    uint8_t code[] = {0xc5}; // PUSH BC
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);

    CLEAR_R_FLAG(regs, FLAG_Z);
    code[0] = 0xc8; // RET Z
    ins = decode(code);
    ins->func(cpu, ins);

    assert(READ_R16(regs, REG_PC) == 0x0);
    assert(READ_R16(regs, REG_SP) == 0x0ffe);

    SET_R_FLAG(regs, FLAG_Z);
    ins = decode(code);
    ins->func(cpu, ins);

    assert(READ_R16(regs, REG_PC) == 0x1234);
    assert(READ_R16(regs, REG_SP) == 0x1000);    
}

static void 
test_ret_nc(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R16(regs, REG_BC, 0x1234);    
    WRITE_R16(regs, REG_SP, 0x1000);
    WRITE_R16(regs, REG_PC, 0x0);
        
    uint8_t code[] = {0xc5}; // PUSH BC
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);

    SET_R_FLAG(regs, FLAG_C);
    code[0] = 0xd0; // RET NC
    ins = decode(code);
    ins->func(cpu, ins);

    assert(READ_R16(regs, REG_PC) == 0x0);
    assert(READ_R16(regs, REG_SP) == 0x0ffe);

    CLEAR_R_FLAG(regs, FLAG_C);
    ins = decode(code);
    ins->func(cpu, ins);

    assert(READ_R16(regs, REG_PC) == 0x1234);
    assert(READ_R16(regs, REG_SP) == 0x1000);    
}

static void 
test_ret_c(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R16(regs, REG_BC, 0x1234);    
    WRITE_R16(regs, REG_SP, 0x1000);
    WRITE_R16(regs, REG_PC, 0x0);
        
    uint8_t code[] = {0xc5}; // PUSH BC
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);

    CLEAR_R_FLAG(regs, FLAG_C);
    code[0] = 0xd8; // RET C
    ins = decode(code);
    ins->func(cpu, ins);

    assert(READ_R16(regs, REG_PC) == 0x0);
    assert(READ_R16(regs, REG_SP) == 0x0ffe);

    SET_R_FLAG(regs, FLAG_C);
    ins = decode(code);
    ins->func(cpu, ins);

    assert(READ_R16(regs, REG_PC) == 0x1234);
    assert(READ_R16(regs, REG_SP) == 0x1000);
}

static void
test_ret(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R16(regs, REG_BC, 0x1234);    
    WRITE_R16(regs, REG_SP, 0x1000);
    WRITE_R16(regs, REG_PC, 0x0);

    uint8_t code[] = {0xc5}; // PUSH BC
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);

    code[0] = 0xc9; // RET
    ins = decode(code);
    ins->func(cpu, ins);

    assert(READ_R16(regs, REG_PC) == 0x1234);
    assert(READ_R16(regs, REG_SP) == 0x1000);
}

static void
test_jp_r16(gbc_cpu_t *cpu)
{   
    WRITE_R16(&(cpu->regs), REG_HL, 0x1234);
    WRITE_R16(&(cpu->regs), REG_PC, 0x0);
    uint8_t code[] = {0xe9}; // JP (HL)
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R16(&(cpu->regs), REG_PC) == 0x1234);
}

static void
test_jp_i16(gbc_cpu_t *cpu)
{
    WRITE_R16(&(cpu->regs), REG_PC, 0x0);
    uint8_t code[] = {0xc3, 0xff, 0xee}; // JP 0xeeff
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R16(&(cpu->regs), REG_PC) == 0xeeff);    
}

static void 
test_jp_nz_i16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R16(regs, REG_PC, 0x0);
    CLEAR_R_FLAG(regs, FLAG_Z);
    uint8_t code[] = {0xc2, 0xff, 0xee}; // JP NZ, 0xeeff
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R16(regs, REG_PC) == 0xeeff);

    WRITE_R16(regs, REG_PC, 0x0);
    SET_R_FLAG(regs, FLAG_Z);
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R16(regs, REG_PC) == 0x0);
}

static void 
test_jp_nc_i16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R16(regs, REG_PC, 0x0);
    CLEAR_R_FLAG(regs, FLAG_C);
    uint8_t code[] = {0xd2, 0xff, 0xee}; // JP NC, 0xeeff
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R16(regs, REG_PC) == 0xeeff);

    WRITE_R16(regs, REG_PC, 0x0);
    SET_R_FLAG(regs, FLAG_C);
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R16(regs, REG_PC) == 0x0);    
}

static void 
test_jp_c_i16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R16(regs, REG_PC, 0x0);    
    SET_R_FLAG(regs, FLAG_C);
    uint8_t code[] = {0xda, 0xff, 0xee}; // JP C, 0xeeff
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R16(regs, REG_PC) == 0xeeff);

    WRITE_R16(regs, REG_PC, 0x0);
    CLEAR_R_FLAG(regs, FLAG_C);    
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R16(regs, REG_PC) == 0x0);        
}    

static void 
test_jp_z_i16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R16(regs, REG_PC, 0x0);    
    SET_R_FLAG(regs, FLAG_Z);
    uint8_t code[] = {0xca, 0xff, 0xee}; // JP Z, 0xeeff
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R16(regs, REG_PC) == 0xeeff);

    WRITE_R16(regs, REG_PC, 0x0);
    CLEAR_R_FLAG(regs, FLAG_Z);    
    ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R16(regs, REG_PC) == 0x0);    
}

static void 
test_call_i16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R16(regs, REG_PC, 0x1234);
    WRITE_R16(regs, REG_SP, 0x1000);
    uint8_t code[] = {0xcd, 0xff, 0xee}; // CALL 0xeeff
    instruction_t *ins = decode(code);
    WRITE_R16(regs, REG_PC, 0x1234 + ins->size);
    ins->func(cpu, ins);
    assert(READ_R16(regs, REG_PC) == 0xeeff);
    assert(READ_R16(regs, REG_SP) == 0x0ffe);
    printf("%x %x\n", cpu->mem_read(cpu->mem_data, 0x0ffe), cpu->mem_read(cpu->mem_data, 0x0ffd));
    assert(cpu->mem_read(cpu->mem_data, 0x0ffe) == 0x37);
    assert(cpu->mem_read(cpu->mem_data, 0x0fff) == 0x12);
}

static void
test_call_ret(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    cpu->mem_write(cpu->mem_data, 0x0ffe, 0xc9); // RET

    // CALL 0x0ffe
    // STOP
    cpu->mem_write(cpu->mem_data, 0x0ef0, 0xcd);
    cpu->mem_write(cpu->mem_data, 0x0ef1, 0xfe); 
    cpu->mem_write(cpu->mem_data, 0x0ef2, 0x0f);
    cpu->mem_write(cpu->mem_data, 0x0ef3, 0x10); 

    WRITE_R16(regs, REG_SP, 0x2000);
    WRITE_R16(regs, REG_PC, 0x0ef0);
    
    uint8_t code[4];
    uint16_t pc = READ_R16(regs, REG_PC);
    code[0] = cpu->mem_read(cpu->mem_data, pc);
    code[1] = cpu->mem_read(cpu->mem_data, pc+1);
    code[2] = cpu->mem_read(cpu->mem_data, pc+2);
    code[3] = cpu->mem_read(cpu->mem_data, pc+3);

    instruction_t *ins = decode(code);
    WRITE_R16(regs, REG_PC, READ_R16(regs, REG_PC) + ins->size);
    ins->func(cpu, ins);    
    assert(READ_R16(regs, REG_PC) == 0x0ffe);
    assert(READ_R16(regs, REG_SP) == 0x1ffe);

    code[0] = cpu->mem_read(cpu->mem_data, READ_R16(regs, REG_PC));
    ins = decode(code);
    WRITE_R16(regs, REG_PC, READ_R16(regs, REG_PC) + ins->size);
    ins->func(cpu, ins);
    assert(READ_R16(regs, REG_PC) == 0x0ef3);
    assert(READ_R16(regs, REG_SP) == 0x2000);
}

static void
test_ldh_r8_im8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    uint8_t code[] = {0xe0, 0x12}; // LDH (0x12), A
    WRITE_R8(regs, REG_A, 0x34);
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(cpu->mem_read(cpu->mem_data, 0xff12) == 0x34);    
}

static void
test_ldh_im8_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    uint8_t code[] = {0xf0, 0x12}; // LDH A, (0x12)
    cpu->mem_write(cpu->mem_data, 0xff12, 0x33);
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x33);    
}

static void
test_ldh_r8_c(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    uint8_t code[] = {0xf2}; // LDH A, (C)
    WRITE_R8(regs, REG_C, 0x12);
    WRITE_R8(regs, REG_A, 0x00);
    cpu->mem_write(cpu->mem_data, 0xff12, 0x44);
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0x44);
}

static void
test_ldh_c_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    uint8_t code[] = {0xe2}; // LDH (C), A
    cpu->mem_write(cpu->mem_data, 0xff13, 0x00);
    WRITE_R8(regs, REG_A, 0x14);
    WRITE_R8(regs, REG_C, 0x13);
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(cpu->mem_read(cpu->mem_data, 0xff13) == 0x14);
}

static void 
test_ccf(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    uint8_t code[] = {0x3f}; // CCF
    CLEAR_R_FLAG(regs, FLAG_C);
    uint8_t z = READ_R_FLAG(regs, FLAG_Z);
    instruction_t *ins = decode(code);    
    ins->func(cpu, ins);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == z);

    ins->func(cpu, ins);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == z);
}

static void
test_cpl(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    uint8_t code[] = {0x2f}; // CPL
    WRITE_R8(regs, REG_A, 0x0f);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    uint8_t z = READ_R_FLAG(regs, FLAG_Z);
    uint8_t c = READ_R_FLAG(regs, FLAG_C);

    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    assert(READ_R8(regs, REG_A) == 0xf0);
    assert(READ_R_FLAG(regs, FLAG_N) == 1);
    assert(READ_R_FLAG(regs, FLAG_H) == 1); 
    assert(READ_R_FLAG(regs, FLAG_Z) == z);
    assert(READ_R_FLAG(regs, FLAG_C) == c);
}

static void
test_cb_rlc_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_B, 0x01);
    SET_R_FLAG(regs, FLAG_C);
    SET_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);

    uint8_t code[] = {0xcb, 0x00}; // RLC B    
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    
    assert(READ_R8(regs, REG_B) == 0x02);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);

    WRITE_R8(regs, REG_B, 0xfe);
    CLEAR_R_FLAG(regs, FLAG_C);
    CLEAR_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);
        
    ins = decode(code);
    ins->func(cpu, ins);
    
    assert(READ_R8(regs, REG_B) == 0xfd);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);    
}

static void
test_cb_rlc_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);

    WRITE_R16(regs, REG_HL, 0x1000);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x01);
    SET_R_FLAG(regs, FLAG_C);
    SET_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);

    uint8_t code[] = {0xcb, 0x06}; // RLC (HL)
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    
    assert(cpu->mem_read(cpu->mem_data, 0x1000) == 0x02);    
    assert(READ_R_FLAG(regs, FLAG_C) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);

    cpu->mem_write(cpu->mem_data, 0x1000, 0xfe);
    CLEAR_R_FLAG(regs, FLAG_C);
    CLEAR_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);
    
    ins = decode(code);
    ins->func(cpu, ins);
    
    assert(cpu->mem_read(cpu->mem_data, 0x1000) == 0xfd);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
}

static void
test_cb_rrc_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_B, 0x01);
    SET_R_FLAG(regs, FLAG_C);
    SET_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);

    uint8_t code[] = {0xcb, 0x08}; // RRC B
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    
    assert(READ_R8(regs, REG_B) == 0x80);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);

    WRITE_R8(regs, REG_B, 0xfe);
    CLEAR_R_FLAG(regs, FLAG_C);
    CLEAR_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);
        
    ins = decode(code);
    ins->func(cpu, ins);
    
    assert(READ_R8(regs, REG_B) == 0x7f);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);    
}

static void
test_cb_rrc_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);

    WRITE_R16(regs, REG_HL, 0x1000);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x01);
    SET_R_FLAG(regs, FLAG_C);
    SET_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);

    uint8_t code[] = {0xcb, 0x0e}; // RRC (HL)
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    
    assert(cpu->mem_read(cpu->mem_data, 0x1000) == 0x80);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);

    cpu->mem_write(cpu->mem_data, 0x1000, 0xfe);
    SET_R_FLAG(regs, FLAG_C);
    SET_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);
    
    ins = decode(code);
    ins->func(cpu, ins);
    
    assert(cpu->mem_read(cpu->mem_data, 0x1000) == 0x7f);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);    
}

static void
test_cb_rl_r8(gbc_cpu_t *cpu)
{    
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_B, 0x01);
    SET_R_FLAG(regs, FLAG_C);
    SET_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);

    uint8_t code[] = {0xcb, 0x10}; // RL B
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    
    assert(READ_R8(regs, REG_B) == 0x03);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);

    WRITE_R8(regs, REG_B, 0xfe);
    CLEAR_R_FLAG(regs, FLAG_C);
    CLEAR_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);
        
    ins = decode(code);
    ins->func(cpu, ins);
    
    assert(READ_R8(regs, REG_B) == 0xfc);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
}

static void
test_cb_rl_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);

    WRITE_R16(regs, REG_HL, 0x1000);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x01);
    SET_R_FLAG(regs, FLAG_C);
    SET_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);

    uint8_t code[] = {0xcb, 0x16}; // RL (HL)
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    
    assert(cpu->mem_read(cpu->mem_data, 0x1000) == 0x03);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);

    cpu->mem_write(cpu->mem_data, 0x1000, 0xfe);
    CLEAR_R_FLAG(regs, FLAG_C);
    CLEAR_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);
    
    ins = decode(code);
    ins->func(cpu, ins);
    
    assert(cpu->mem_read(cpu->mem_data, 0x1000) == 0xfc);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);    
}

static void
test_cb_rr_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_B, 0x01);
    SET_R_FLAG(regs, FLAG_C);
    SET_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);

    uint8_t code[] = {0xcb, 0x18}; // RR B
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    
    assert(READ_R8(regs, REG_B) == 0x80);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);

    WRITE_R8(regs, REG_B, 0xff);
    CLEAR_R_FLAG(regs, FLAG_C);
    CLEAR_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);
        
    ins = decode(code);
    ins->func(cpu, ins);
    
    assert(READ_R8(regs, REG_B) == 0x7f);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);    
}

static void
test_cb_rr_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);

    WRITE_R16(regs, REG_HL, 0x1000);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x01);
    SET_R_FLAG(regs, FLAG_C);
    SET_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);

    uint8_t code[] = {0xcb, 0x1e}; // RR (HL)
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    
    assert(cpu->mem_read(cpu->mem_data, 0x1000) == 0x80);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);

    cpu->mem_write(cpu->mem_data, 0x1000, 0xff);
    CLEAR_R_FLAG(regs, FLAG_C);
    SET_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);
    
    ins = decode(code);
    ins->func(cpu, ins);
    
    assert(cpu->mem_read(cpu->mem_data, 0x1000) == 0x7f);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);    
}

static void
test_cb_sra_r8(gbc_cpu_t *cpu)
{   
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_B, 0x01);
    SET_R_FLAG(regs, FLAG_C);
    SET_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);

    uint8_t code[] = {0xcb, 0x28}; // SRA B
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    
    assert(READ_R8(regs, REG_B) == 0x00);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);

    WRITE_R8(regs, REG_B, 0xfe);
    CLEAR_R_FLAG(regs, FLAG_C);
    CLEAR_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);
        
    ins = decode(code);
    ins->func(cpu, ins);
    
    assert(READ_R8(regs, REG_B) == 0xff);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);        
}

static void
test_cb_sra_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);

    WRITE_R16(regs, REG_HL, 0x1000);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x01);
    SET_R_FLAG(regs, FLAG_C);
    SET_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);

    uint8_t code[] = {0xcb, 0x2e}; // SRA (HL)
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    
    assert(cpu->mem_read(cpu->mem_data, 0x1000) == 0x00);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);

    cpu->mem_write(cpu->mem_data, 0x1000, 0xfe);
    CLEAR_R_FLAG(regs, FLAG_C);
    CLEAR_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);
    
    ins = decode(code);
    ins->func(cpu, ins);
    
    assert(cpu->mem_read(cpu->mem_data, 0x1000) == 0xff);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);    
}

static void
test_cb_sla_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_B, 0x01);
    SET_R_FLAG(regs, FLAG_C);
    SET_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);

    uint8_t code[] = {0xcb, 0x20}; // SLA B
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    
    assert(READ_R8(regs, REG_B) == 0x02);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);

    WRITE_R8(regs, REG_B, 0xfe);
    CLEAR_R_FLAG(regs, FLAG_C);
    CLEAR_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);
        
    ins = decode(code);
    ins->func(cpu, ins);
    
    assert(READ_R8(regs, REG_B) == 0xfc);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
}

static void
test_cb_sla_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);

    WRITE_R16(regs, REG_HL, 0x1000);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x01);
    SET_R_FLAG(regs, FLAG_C);
    SET_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);

    uint8_t code[] = {0xcb, 0x26}; // SLA (HL)
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    
    assert(cpu->mem_read(cpu->mem_data, 0x1000) == 0x02);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);

    cpu->mem_write(cpu->mem_data, 0x1000, 0xfe);
    CLEAR_R_FLAG(regs, FLAG_C);
    CLEAR_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);
    
    ins = decode(code);
    ins->func(cpu, ins);
    
    assert(cpu->mem_read(cpu->mem_data, 0x1000) == 0xfc);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
}

static void 
test_cb_swap_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);

    WRITE_R16(regs, REG_HL, 0x1000);
    cpu->mem_write(cpu->mem_data, 0x1000, 0xfe);
    SET_R_FLAG(regs, FLAG_C);
    SET_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);

    uint8_t code[] = {0xcb, 0x36}; // SWAP (HL)
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    
    assert(cpu->mem_read(cpu->mem_data, 0x1000) == 0xef);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
}

static void 
test_cb_swap_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_B, 0xfe);
    SET_R_FLAG(regs, FLAG_C);
    SET_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);

    uint8_t code[] = {0xcb, 0x30}; // SWAP B
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    
    assert(READ_R8(regs, REG_B) == 0xef);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
}

static void
test_cb_srl_r8(gbc_cpu_t *cpu)
{   
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_B, 0x01);
    SET_R_FLAG(regs, FLAG_C);
    SET_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);

    uint8_t code[] = {0xcb, 0x38}; // SRL B
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    
    assert(READ_R8(regs, REG_B) == 0x00);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);

    WRITE_R8(regs, REG_B, 0xfe);
    CLEAR_R_FLAG(regs, FLAG_C);
    CLEAR_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);
        
    ins = decode(code);
    ins->func(cpu, ins);
    
    assert(READ_R8(regs, REG_B) == 0x7f);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);        
}

static void
test_cb_srl_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);

    WRITE_R16(regs, REG_HL, 0x1000);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x01);
    SET_R_FLAG(regs, FLAG_C);
    SET_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);

    uint8_t code[] = {0xcb, 0x3e}; // SRL (HL)
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    
    assert(cpu->mem_read(cpu->mem_data, 0x1000) == 0x00);
    assert(READ_R_FLAG(regs, FLAG_C) == 1);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);

    cpu->mem_write(cpu->mem_data, 0x1000, 0xfe);
    CLEAR_R_FLAG(regs, FLAG_C);
    CLEAR_R_FLAG(regs, FLAG_Z);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);
    
    ins = decode(code);
    ins->func(cpu, ins);
    
    assert(cpu->mem_read(cpu->mem_data, 0x1000) == 0x7f);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);    
}

static void
test_cb_bit_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);
    WRITE_R8(regs, REG_B, 0x41);
    CLEAR_R_FLAG(regs, FLAG_C);
    CLEAR_R_FLAG(regs, FLAG_Z);
    CLEAR_R_FLAG(regs, FLAG_H);
    CLEAR_R_FLAG(regs, FLAG_N);

    uint8_t code[] = {0xcb, 0x48}; // BIT 1, B
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    
    assert(READ_R8(regs, REG_B) == 0x41);
    assert(READ_R_FLAG(regs, FLAG_C) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);

    CLEAR_R_FLAG(regs, FLAG_C);
    CLEAR_R_FLAG(regs, FLAG_Z);
    CLEAR_R_FLAG(regs, FLAG_H);
    CLEAR_R_FLAG(regs, FLAG_N);

    code[1] = 0x70; // BIT 6, B
    ins = decode(code);
    ins->func(cpu, ins);    

    assert(READ_R_FLAG(regs, FLAG_C) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);
}

static void
test_cb_bit_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);    
    WRITE_R16(regs, REG_HL, 0x1000);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x41);    
    
    CLEAR_R_FLAG(regs, FLAG_C);
    CLEAR_R_FLAG(regs, FLAG_Z);
    CLEAR_R_FLAG(regs, FLAG_H);
    CLEAR_R_FLAG(regs, FLAG_N);

    uint8_t code[] = {0xcb, 0x4e}; // BIT 0, (HL)
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);
    
    assert(READ_R_FLAG(regs, FLAG_C) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);

    CLEAR_R_FLAG(regs, FLAG_C);
    CLEAR_R_FLAG(regs, FLAG_Z);
    CLEAR_R_FLAG(regs, FLAG_H);
    CLEAR_R_FLAG(regs, FLAG_N);

    code[1] = 0x76; // BIT 6, (HL)
    ins = decode(code);
    ins->func(cpu, ins);    

    assert(READ_R_FLAG(regs, FLAG_C) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 0);
    assert(READ_R_FLAG(regs, FLAG_N) == 0);
    assert(READ_R_FLAG(regs, FLAG_H) == 1);    

    assert(cpu->mem_read(cpu->mem_data, 0x1000) == 0x41);
}

static void
test_cb_res_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);

    CLEAR_R_FLAG(regs, FLAG_C);
    SET_R_FLAG(regs, FLAG_Z);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);

    WRITE_R8(regs, REG_B, 0x42);
    uint8_t code[] = {0xcb, 0x80}; // RES 0, B
    
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);

    assert(READ_R8(regs, REG_B) == 0x42);
    code[1] = 0xb0; // RES 0, B

    ins = decode(code);
    ins->func(cpu, ins);

    assert(READ_R8(regs, REG_B) == 0x02);

    assert(READ_R_FLAG(regs, FLAG_C) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 1);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
}

static void
test_cb_res_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);    
    WRITE_R16(regs, REG_HL, 0x1000);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x42);
    
    CLEAR_R_FLAG(regs, FLAG_C);
    SET_R_FLAG(regs, FLAG_Z);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);

    uint8_t code[] = {0xcb, 0x86}; // RES 0, (HL)
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);

    assert(cpu->mem_read(cpu->mem_data, 0x1000) == 0x42);

    code[1] = 0xb6; // RES 6, (HL)
    ins = decode(code);

    ins->func(cpu, ins);
    assert(cpu->mem_read(cpu->mem_data, 0x1000) == 0x02);

    assert(READ_R_FLAG(regs, FLAG_C) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 1);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
}

static void
test_cb_set_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);

    CLEAR_R_FLAG(regs, FLAG_C);
    SET_R_FLAG(regs, FLAG_Z);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);

    WRITE_R8(regs, REG_B, 0x42);
    uint8_t code[] = {0xcb, 0xc0}; // SET 0, B
    
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);

    assert(READ_R8(regs, REG_B) == 0x43);
    
    code[1] = 0xf0; // SET 6, B

    ins = decode(code);
    ins->func(cpu, ins);

    assert(READ_R8(regs, REG_B) == 0x43);

    assert(READ_R_FLAG(regs, FLAG_C) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 1);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
}

static void
test_cb_set_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *regs = &(cpu->regs);    
    WRITE_R16(regs, REG_HL, 0x1000);
    cpu->mem_write(cpu->mem_data, 0x1000, 0x42);
    
    CLEAR_R_FLAG(regs, FLAG_C);
    SET_R_FLAG(regs, FLAG_Z);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG(regs, FLAG_N);

    uint8_t code[] = {0xcb, 0xc6}; // SET 0, (HL)
    instruction_t *ins = decode(code);
    ins->func(cpu, ins);

    assert(cpu->mem_read(cpu->mem_data, 0x1000) == 0x43);

    code[1] = 0xf6; // SET 6, (HL)
    ins = decode(code);

    ins->func(cpu, ins);
    assert(cpu->mem_read(cpu->mem_data, 0x1000) == 0x43);

    assert(READ_R_FLAG(regs, FLAG_C) == 0);
    assert(READ_R_FLAG(regs, FLAG_Z) == 1);
    assert(READ_R_FLAG(regs, FLAG_N) == 1);
    assert(READ_R_FLAG(regs, FLAG_H) == 0);
}

void
test_instructions() 
{
    gbc_memory_t mem;
    gbc_mem_init(&mem);
    mem.read = _mem_read;
    mem.write = _mem_write;

    gbc_cpu_t cpu;
    gbc_cpu_init(&cpu);
    gbc_cpu_connect(&cpu, &mem);

    test_inc_r16(&cpu);
    test_inc_r8(&cpu);
    test_inc_m16(&cpu);

    test_dec_r16(&cpu);
    test_dec_r8(&cpu);
    test_dec_m16(&cpu);

    test_jr_nz_i8(&cpu);
    test_rlca(&cpu);
    test_rla(&cpu);

    test_rrca(&cpu);
    test_rra(&cpu);    
    test_daa(&cpu);
    test_jr(&cpu);

    test_ld_r16_i16(&cpu);
    test_ld_r8_i8(&cpu);

    test_ld_sp_hl(&cpu);
    test_ld_hl_sp_i8(&cpu);    
    test_ldi_r8_m16(&cpu);
    test_ldi_m16_r8(&cpu);
    test_ldd_m16_r8(&cpu);
    test_ldd_r8_m16(&cpu);
    test_ld_r8_m16(&cpu);
    test_ld_m16_i8(&cpu);
    test_ld_m16_r8(&cpu);
    test_ld_im16_r16(&cpu);
    test_ld_im16_r8(&cpu);
    test_ld_r8_r8(&cpu);

    test_add_r16_i8(&cpu);
    test_add_r16_r16(&cpu);
    test_add_r8_i8(&cpu);
    test_add_r8_r8(&cpu);
    test_add_r8_m16(&cpu);

    test_adc_r8_i8(&cpu);
    test_adc_r8_r8(&cpu);
    test_adc_r8_m16(&cpu);

    test_sub_r8_i8(&cpu);
    test_sub_r8_r8(&cpu);
    test_sub_r8_m16(&cpu);

    test_subc_r8_i8(&cpu);
    test_subc_r8_r8(&cpu);
    test_subc_r8_m16(&cpu);

    test_and_r8_i8(&cpu);
    test_and_r8_r8(&cpu);
    test_and_r8_m16(&cpu);

    test_or_r8_i8(&cpu);
    test_or_r8_r8(&cpu);
    test_or_r8_m16(&cpu);

    test_xor_r8_i8(&cpu);
    test_xor_r8_r8(&cpu);
    test_xor_r8_m16(&cpu);    

    test_cp_r8_i8(&cpu);
    test_cp_r8_r8(&cpu);
    test_cp_r8_m16(&cpu);

    test_ret_nz(&cpu);
    test_ret_z(&cpu);
    test_ret_nc(&cpu);
    test_ret_c(&cpu);
    test_ret(&cpu);

    test_push_pop_r16(&cpu);    
    test_push_pop_af(&cpu);

    test_jp_r16(&cpu);
    test_jp_i16(&cpu);
    test_jp_nz_i16(&cpu);
    test_jp_nc_i16(&cpu);
    test_jp_c_i16(&cpu);
    test_jp_z_i16(&cpu);

    test_call_i16(&cpu);
    test_call_ret(&cpu);

    test_ldh_c_r8(&cpu);
    test_ldh_r8_c(&cpu);
    test_ldh_r8_im8(&cpu);
    test_ldh_im8_r8(&cpu);

    test_ccf(&cpu);
    test_cpl(&cpu);

    test_cb_rlc_r8(&cpu);
    test_cb_rlc_m16(&cpu);

    test_cb_rrc_r8(&cpu);
    test_cb_rrc_m16(&cpu);

    test_cb_rl_r8(&cpu);
    test_cb_rl_m16(&cpu);

    test_cb_rr_r8(&cpu);
    test_cb_rr_m16(&cpu);    

    test_cb_sla_m16(&cpu);
    test_cb_sla_r8(&cpu);

    test_cb_sra_m16(&cpu);
    test_cb_sra_r8(&cpu);    
    
    test_cb_swap_m16(&cpu);
    test_cb_swap_r8(&cpu);    

    test_cb_srl_m16(&cpu);
    test_cb_srl_r8(&cpu);

    test_cb_bit_r8(&cpu);
    test_cb_bit_m16(&cpu);

    test_cb_res_r8(&cpu);
    test_cb_res_m16(&cpu);

    test_cb_set_r8(&cpu);
    test_cb_set_m16(&cpu);        
}