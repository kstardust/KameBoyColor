#include "instruction_set.h"
#include "common.h"
#include "cpu.h"

static void
stop(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("STOP: %s\n", ins->name);
    gbc_memory_t *mem = (gbc_memory_t*)cpu->mem_data;
    uint8_t key1 = IO_PORT_READ(mem, IO_PORT_KEY1);
    if (key1 & KEY1_CPU_SWITCH_ARMED) {
        cpu->dspeed = !cpu->dspeed;
        if (cpu->dspeed)
            key1 |= KEY1_CPU_CURRENT_MODE;
        else
            key1 &= ~KEY1_CPU_CURRENT_MODE;

        key1 &= ~KEY1_CPU_SWITCH_ARMED;
        IO_PORT_WRITE(mem, IO_PORT_KEY1, key1);
    }

    LOG_INFO("[CPU]Speed switch %s -> %s\n",
        (cpu->dspeed ? "DOUBLE" : "NORMAL"),
        (cpu->dspeed ? "NORMAL" : "DOUBLE"));
}

static void
inc_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("INC r8: %s\n", ins->name);

    size_t reg_offset = (size_t)ins->op1;
    cpu_register_t *regs = &(cpu->regs);
    uint16_t v = READ_R8(regs, reg_offset);
    uint8_t hc = HALF_CARRY_ADD(v, 1);
    v++;
    v &= UINT8_MASK;
    WRITE_R8(regs, reg_offset, (uint8_t)v);

    SET_R_FLAG_VALUE(regs, FLAG_Z, v == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
}

static void
inc_r16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("INC r16: %s\n", ins->name);

    size_t reg_offset = (size_t)ins->op1;
    cpu_register_t *regs = &(cpu->regs);
    uint16_t v = READ_R16(regs, reg_offset);
    v++;
    WRITE_R16(regs, reg_offset, v);
}

static void
inc_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("INC m16: %s\n", ins->name);

    size_t reg_offset = (size_t)ins->op1;
    cpu_register_t *regs = &(cpu->regs);
    uint16_t addr = READ_R16(regs, reg_offset);
    uint16_t v = cpu->mem_read(cpu->mem_data, addr);
    uint8_t hc = HALF_CARRY_ADD(v, 1);

    v++;
    v &= UINT8_MASK;
    cpu->mem_write(cpu->mem_data, addr, (uint8_t)v);

    SET_R_FLAG_VALUE(regs, FLAG_Z, v == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
}

static void
dec_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("DEC r8: %s\n", ins->name);

    size_t reg_offset = (size_t)ins->op1;
    cpu_register_t *regs = &(cpu->regs);
    uint16_t v = READ_R8(regs, reg_offset);
    uint8_t hc = HALF_CARRY_SUB(v, 1);
    v--;
    v &= UINT8_MASK;
    WRITE_R8(regs, reg_offset, (uint8_t)v);

    SET_R_FLAG_VALUE(regs, FLAG_Z, v == 0);
    SET_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
}

static void
dec_r16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("DEC r16: %s\n", ins->name);

    size_t reg_offset = (size_t)ins->op1;
    cpu_register_t *regs = &(cpu->regs);
    uint16_t v = READ_R16(regs, reg_offset);
    v--;
    WRITE_R16(regs, reg_offset, v);
}

static void
dec_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("DEC m16: %s\n", ins->name);

    size_t reg_offset = (size_t)ins->op1;
    cpu_register_t *regs = &(cpu->regs);
    uint16_t addr = READ_R16(regs, reg_offset);
    uint16_t v = cpu->mem_read(cpu->mem_data, addr);
    uint8_t hc = HALF_CARRY_SUB(v, 1);

    v--;
    v &= UINT8_MASK;
    cpu->mem_write(cpu->mem_data, addr, (uint8_t)v);

    SET_R_FLAG_VALUE(regs, FLAG_Z, v == 0);
    SET_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
}

static void
rlca(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("RLCA: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    uint8_t v = READ_R8(regs, REG_A);
    uint8_t carry = v >> 7;
    v = (v << 1) | carry;
    WRITE_R8(regs, REG_A, v);

    CLEAR_R_FLAG(regs, FLAG_Z);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
}

static void
rla(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("RLA: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    uint8_t v = READ_R8(regs, REG_A);
    uint8_t carry = v >> 7;
    v = (v << 1) | READ_R_FLAG(regs, FLAG_C);
    WRITE_R8(regs, REG_A, v);

    CLEAR_R_FLAG(regs, FLAG_Z);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
}

static void
rrca(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("RRCA: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    uint8_t v = READ_R8(regs, REG_A);
    uint8_t carry = v & 0x1;
    v = (v >> 1) | (carry << 7);
    WRITE_R8(regs, REG_A, v);

    CLEAR_R_FLAG(regs, FLAG_Z);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
}

static void
rra(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("RRA: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    uint8_t v = READ_R8(regs, REG_A);
    uint8_t carry = v & 0x1;
    v = (v >> 1) | (READ_R_FLAG(regs, FLAG_C) << 7);
    WRITE_R8(regs, REG_A, v);

    CLEAR_R_FLAG(regs, FLAG_Z);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
}

static void
daa(gbc_cpu_t *cpu, instruction_t *ins)
{
    /* https://ehaskins.com/2018-01-30%20Z80%20DAA/ */
    LOG_DEBUG("DAA: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    uint8_t v = READ_R8(regs, REG_A);

    uint8_t n = READ_R_FLAG(regs, FLAG_N);
    uint8_t h = READ_R_FLAG(regs, FLAG_H);
    uint8_t c = READ_R_FLAG(regs, FLAG_C);

    uint8_t correction = 0;

    if (h || (!n && (v & 0xf) > 9)) {
        correction |= 0x6;
    }

    if (c || (!n && v > 0x99)) {
        correction |= 0x60;
        SET_R_FLAG(regs, FLAG_C);
    }

    v += n ? -correction : correction;

    WRITE_R8(regs, REG_A, v);
    SET_R_FLAG_VALUE(regs, FLAG_Z, v == 0);
    CLEAR_R_FLAG(regs, FLAG_H);
}

static void
scf(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("SCF: %s\n", ins->name);
    cpu_register_t *regs = &(cpu->regs);
    SET_R_FLAG(regs, FLAG_C);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
}

static void
_jr_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("\n_JR: %s\n", ins->name);
    cpu_register_t *regs = &(cpu->regs);
    int8_t offset = (int8_t)ins->opcode_ext.i8;
    uint16_t pc = READ_R16(regs, REG_PC);
    pc += offset;
    WRITE_R16(regs, REG_PC, pc);
}

static void
jr_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("JR: %s\n", ins->name);
    _jr_i8(cpu, ins);
}

static void
jr_nz_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("JR NZ: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    if (!READ_R_FLAG(regs, FLAG_Z)) {
        ins->r_cycles = ins->cycles2;
        _jr_i8(cpu, ins);
    }
}

static void
jr_nc_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("JR NC: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    if (!READ_R_FLAG(regs, FLAG_C)) {
        ins->r_cycles = ins->cycles2;
        _jr_i8(cpu, ins);
    }
}

static void
jr_z_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("JR NZ: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    if (READ_R_FLAG(regs, FLAG_Z)) {
        ins->r_cycles = ins->cycles2;
        _jr_i8(cpu, ins);
    }
}

static void
jr_c_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("JR C: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    if (READ_R_FLAG(regs, FLAG_C)) {
        ins->r_cycles = ins->cycles2;
        _jr_i8(cpu, ins);
    }
}

static void
nop(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("NOP: %s\n", ins->name);
}

static void
ld_r16_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("LD r16, i16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint16_t value = ins->opcode_ext.i16;
    WRITE_R16(regs, reg_offset, value);
}

static void
ld_sp_hl(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("LD SP, HL: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    uint16_t hl = READ_R16(regs, REG_HL);
    WRITE_R16(regs, REG_SP, hl);
}

static void
ld_hl_sp_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("LD HL, SP + i8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    int8_t offset = ins->opcode_ext.i8;
    uint16_t sp = READ_R16(regs, REG_SP);

    uint8_t carry = ((sp & UINT8_MASK) + (uint8_t)offset) > UINT8_MASK;
    uint8_t halfc = HALF_CARRY_ADD(sp, offset);

    uint16_t hl = sp + offset;
    WRITE_R16(regs, REG_HL, hl);

    CLEAR_R_FLAG(regs, FLAG_Z);
    CLEAR_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, halfc);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
}

static void
ld_r8_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("LD r8, i8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint8_t value = ins->opcode_ext.i8;
    WRITE_R8(regs, reg_offset, value);
}

static void
ldi_r8_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("LDI r8, m16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    size_t reg2_offset = (size_t)ins->op2;
    uint16_t addr = READ_R16(regs, reg2_offset);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    WRITE_R8(regs, reg_offset, value);
    addr++;
    WRITE_R16(regs, reg2_offset, addr);
}

static void
ldi_m16_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("LDI m16, r8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    size_t reg2_offset = (size_t)ins->op2;
    uint16_t addr = READ_R16(regs, reg_offset);
    uint8_t value = READ_R8(regs, reg2_offset);
    cpu->mem_write(cpu->mem_data, addr, value);
    addr++;
    WRITE_R16(regs, reg_offset, addr);
}

static void
ldd_r8_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("LDD r8, m16: %s\n", ins->name);
    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    size_t reg2_offset = (size_t)ins->op2;
    uint16_t addr = READ_R16(regs, reg2_offset);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    WRITE_R8(regs, reg_offset, value);
    addr--;
    WRITE_R16(regs, reg2_offset, addr);
}

static void
ldd_m16_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("LDD m16, r8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    size_t reg2_offset = (size_t)ins->op2;
    uint16_t addr = READ_R16(regs, reg_offset);
    uint8_t value = READ_R8(regs, reg2_offset);
    cpu->mem_write(cpu->mem_data, addr, value);
    addr--;
    WRITE_R16(regs, reg_offset, addr);
}

static void
ld_m16_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("LD m16, 8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint16_t addr = READ_R16(regs, reg_offset);
    uint8_t value = ins->opcode_ext.i8;
    cpu->mem_write(cpu->mem_data, addr, value);
}

static void
ld_r8_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("LD r8, m16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    size_t reg2_offset = (size_t)ins->op2;
    uint16_t addr = READ_R16(regs, reg2_offset);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    WRITE_R8(regs, reg_offset, value);
}

static void
ld_r8_im16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("LD r8, im16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    uint16_t addr = ins->opcode_ext.i16;
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    WRITE_R8(regs, REG_A, value);
}

static void
ld_m16_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("LD m16, r8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    size_t reg2_offset = (size_t)ins->op2;

    uint16_t addr = READ_R16(regs, reg_offset);
    uint8_t value = READ_R8(regs, reg2_offset);
    cpu->mem_write(cpu->mem_data, addr, value);
}

static void
ld_im16_r16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("LD im16, r16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op2;
    uint16_t addr = ins->opcode_ext.i16;
    uint16_t value = READ_R16(regs, reg_offset);
    cpu->mem_write(cpu->mem_data, addr, value & UINT8_MASK);
    cpu->mem_write(cpu->mem_data, addr + 1, value >> 8);
}

static void
ld_im16_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("LD im16, r8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op2;
    uint16_t addr = ins->opcode_ext.i16;
    uint8_t value = READ_R8(regs, reg_offset);
    cpu->mem_write(cpu->mem_data, addr, value);
}

static void
ld_r8_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("LD r8, r8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    size_t reg2_offset = (size_t)ins->op2;
    uint8_t value = READ_R8(regs, reg2_offset);
    WRITE_R8(regs, reg_offset, value);
}

static void
add_r16_r16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("ADD r16, r16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    size_t reg2_offset = (size_t)ins->op2;
    uint16_t v1 = READ_R16(regs, reg_offset);
    uint16_t v2 = READ_R16(regs, reg2_offset);
    uint32_t result = v1 + v2;

    uint8_t hc = HALF_CARRY_ADD_16(v1, v2);
    uint8_t carry = result > UINT16_MASK;

    result &= UINT16_MASK;
    WRITE_R16(regs, reg_offset, result);

    CLEAR_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
}

static void
add_r16_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("ADD r16, i8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    int8_t value = ins->opcode_ext.i8;
    uint16_t v = READ_R16(regs, reg_offset);
    uint8_t hc = HALF_CARRY_ADD(v, value);
    uint8_t carry = ((v & UINT8_MASK) + (uint8_t)value) > UINT8_MASK;
    v += value;

    WRITE_R16(regs, reg_offset, v);

    CLEAR_R_FLAG(regs, FLAG_Z);
    CLEAR_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
}

static void
add_r8_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("ADD r8, r8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    size_t reg2_offset = (size_t)ins->op2;
    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = READ_R8(regs, reg2_offset);
    uint8_t hc = HALF_CARRY_ADD(v1, v2);
    uint8_t carry = (v1 + v2) > UINT8_MASK;
    uint8_t result = v1 + v2;
    WRITE_R8(regs, reg_offset, result);

    CLEAR_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
add_r8_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("ADD r8, i8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = ins->opcode_ext.i8;
    uint8_t hc = HALF_CARRY_ADD(v1, v2);
    uint8_t carry = (v1 + v2) > UINT8_MASK;
    uint8_t result = v1 + v2;
    WRITE_R8(regs, reg_offset, result);

    CLEAR_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}


static void
add_r8_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("ADD r8, m16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint16_t addr = READ_R16(regs, (size_t)ins->op2);

    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = cpu->mem_read(cpu->mem_data, addr);
    uint8_t hc = HALF_CARRY_ADD(v1, v2);
    uint8_t carry = (v1 + v2) > UINT8_MASK;
    uint8_t result = v1 + v2;
    WRITE_R8(regs, reg_offset, result);

    CLEAR_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
adc_r8_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("ADC r8, r8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    size_t reg2_offset = (size_t)ins->op2;
    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = READ_R8(regs, reg2_offset);
    uint8_t carry = READ_R_FLAG(regs, FLAG_C);
    uint8_t hc = HALF_CARRY_ADC(v1, v2, carry);

    uint8_t result = v1 + v2 + carry;
    carry = (v1 + v2 + carry) > UINT8_MASK;

    WRITE_R8(regs, reg_offset, result);

    CLEAR_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
adc_r8_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("ADC r8, i8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = ins->opcode_ext.i8;
    uint8_t carry = READ_R_FLAG(regs, FLAG_C);
    uint8_t hc = HALF_CARRY_ADC(v1, v2, carry);

    uint8_t result = v1 + v2 + carry;
    carry = (v1 + v2 + carry) > UINT8_MASK;

    WRITE_R8(regs, reg_offset, result);

    CLEAR_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
adc_r8_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("ADC r8, m16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint16_t addr = READ_R16(regs, (size_t)ins->op2);

    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = cpu->mem_read(cpu->mem_data, addr);
    uint8_t carry = READ_R_FLAG(regs, FLAG_C);

    uint8_t hc = HALF_CARRY_ADC(v1, v2, carry);

    uint8_t result = v1 + v2 + carry;
    carry = (v1 + v2 + carry) > UINT8_MASK;

    WRITE_R8(regs, reg_offset, result);

    CLEAR_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
sub_r8_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("SUB r8, r8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    size_t reg2_offset = (size_t)ins->op2;
    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = READ_R8(regs, reg2_offset);
    uint8_t hc = HALF_CARRY_SUB(v1, v2);
    uint8_t carry = v1 < v2;
    uint8_t result = v1 - v2;
    WRITE_R8(regs, reg_offset, result);

    SET_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
sub_r8_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("SUB r8, i8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = ins->opcode_ext.i8;
    uint8_t hc = HALF_CARRY_SUB(v1, v2);
    uint8_t carry = v1 < v2;
    uint8_t result = v1 - v2;
    WRITE_R8(regs, reg_offset, result);

    SET_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
sub_r8_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("SUB r8, m16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint16_t addr = READ_R16(regs, (size_t)ins->op2);

    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = cpu->mem_read(cpu->mem_data, addr);
    uint8_t hc = HALF_CARRY_SUB(v1, v2);
    uint8_t carry = v1 < v2;
    uint8_t result = v1 - v2;
    WRITE_R8(regs, reg_offset, result);

    SET_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
subc_r8_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("SUBC r8, r8: %s\n", ins->name);
    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    size_t reg2_offset = (size_t)ins->op2;
    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = READ_R8(regs, reg2_offset);
    uint8_t carry = READ_R_FLAG(regs, FLAG_C);
    uint8_t hc = HALF_CARRY_SBC(v1, v2, carry);

    uint8_t result = v1 - v2 - carry;
    carry = v1 < (v2 + carry);

    WRITE_R8(regs, reg_offset, result);

    SET_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
subc_r8_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("SUB r8, i8: %s\n", ins->name);
    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = ins->opcode_ext.i8;
    uint8_t carry = READ_R_FLAG(regs, FLAG_C);
    uint8_t hc = HALF_CARRY_SBC(v1, v2, carry);

    uint8_t result = v1 - v2 - carry;
    carry = v1 < (v2 + carry);

    WRITE_R8(regs, reg_offset, result);

    SET_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
subc_r8_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("SUB r8, m16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint16_t addr = READ_R16(regs, (size_t)ins->op2);

    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = cpu->mem_read(cpu->mem_data, addr);
    uint8_t carry = READ_R_FLAG(regs, FLAG_C);

    uint8_t hc = HALF_CARRY_SBC(v1, v2, carry);
    uint8_t result = v1 - v2 - carry;
    carry = v1 < (v2 + carry);

    WRITE_R8(regs, reg_offset, result);

    SET_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
and_r8_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("AND r8, r8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    size_t reg2_offset = (size_t)ins->op2;
    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = READ_R8(regs, reg2_offset);

    uint8_t result = v1 & v2;
    WRITE_R8(regs, reg_offset, result);

    CLEAR_R_FLAG(regs, FLAG_N);
    SET_R_FLAG(regs, FLAG_H);
    CLEAR_R_FLAG(regs, FLAG_C);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
and_r8_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("AND r8, m16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint16_t addr = READ_R16(regs, (size_t)ins->op2);
    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = cpu->mem_read(cpu->mem_data, addr);

    uint8_t result = v1 & v2;
    WRITE_R8(regs, reg_offset, result);

    CLEAR_R_FLAG(regs, FLAG_N);
    SET_R_FLAG(regs, FLAG_H);
    CLEAR_R_FLAG(regs, FLAG_C);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
and_r8_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("AND r8, i8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = ins->opcode_ext.i8;

    uint8_t result = v1 & v2;
    WRITE_R8(regs, reg_offset, result);

    CLEAR_R_FLAG(regs, FLAG_N);
    SET_R_FLAG(regs, FLAG_H);
    CLEAR_R_FLAG(regs, FLAG_C);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
or_r8_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("OR r8, r8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    size_t reg2_offset = (size_t)ins->op2;
    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = READ_R8(regs, reg2_offset);

    uint8_t result = v1 | v2;
    WRITE_R8(regs, reg_offset, result);

    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    CLEAR_R_FLAG(regs, FLAG_C);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
or_r8_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("OR r8, i8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = ins->opcode_ext.i8;

    uint8_t result = v1 | v2;
    WRITE_R8(regs, reg_offset, result);

    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    CLEAR_R_FLAG(regs, FLAG_C);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
or_r8_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("OR r8, m16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint16_t addr = READ_R16(regs, (size_t)ins->op2);
    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = cpu->mem_read(cpu->mem_data, addr);

    uint8_t result = v1 | v2;
    WRITE_R8(regs, reg_offset, result);

    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    CLEAR_R_FLAG(regs, FLAG_C);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
xor_r8_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("XOR r8, r8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    size_t reg2_offset = (size_t)ins->op2;
    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = READ_R8(regs, reg2_offset);

    uint8_t result = v1 ^ v2;
    WRITE_R8(regs, reg_offset, result);

    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    CLEAR_R_FLAG(regs, FLAG_C);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
xor_r8_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("XOR r8, i8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = ins->opcode_ext.i8;

    uint8_t result = v1 ^ v2;
    WRITE_R8(regs, reg_offset, result);

    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    CLEAR_R_FLAG(regs, FLAG_C);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
xor_r8_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("XOR r8, m16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint16_t addr = READ_R16(regs, (size_t)ins->op2);
    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = cpu->mem_read(cpu->mem_data, addr);

    uint8_t result = v1 ^ v2;
    WRITE_R8(regs, reg_offset, result);

    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    CLEAR_R_FLAG(regs, FLAG_C);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
cp_r8_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("CP r8, r8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    size_t reg2_offset = (size_t)ins->op2;
    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = READ_R8(regs, reg2_offset);
    uint8_t hc = HALF_CARRY_SUB(v1, v2);
    uint8_t carry = v1 < v2;
    uint8_t result = v1 - v2;

    SET_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
cp_r8_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("CP r8, i8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = ins->opcode_ext.i8;
    uint8_t hc = HALF_CARRY_SUB(v1, v2);
    uint8_t carry = v1 < v2;
    uint8_t result = v1 - v2;

    SET_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
cp_r8_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("CP r8, m16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint16_t addr = READ_R16(regs, (size_t)ins->op2);

    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = cpu->mem_read(cpu->mem_data, addr);
    uint8_t hc = HALF_CARRY_SUB(v1, v2);
    uint8_t carry = v1 < v2;
    uint8_t result = v1 - v2;

    SET_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

/* This function is equivolent to POP r16, where r16 is PC */
static void
_ret(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("\t_RET: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = REG_PC;
    uint16_t sp = READ_R16(regs, REG_SP);

    uint8_t lo = cpu->mem_read(cpu->mem_data, sp);
    uint8_t hi = cpu->mem_read(cpu->mem_data, sp + 1);

    WRITE_R16(regs, reg_offset, (hi << 8) | lo);
    WRITE_R16(regs, REG_SP, sp + 2);
}

static void
ret_nz(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("RET NZ: %s\n", ins->name);

    if (!READ_R_FLAG(&(cpu->regs), FLAG_Z)) {
        ins->r_cycles = ins->cycles2;
        _ret(cpu, ins);
    }
}

static void
ret_nc(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("RET NC: %s\n", ins->name);

    if (!READ_R_FLAG(&(cpu->regs), FLAG_C)) {
        ins->r_cycles = ins->cycles2;
        _ret(cpu, ins);
    }
}

static void
ret_z(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("RET Z: %s\n", ins->name);

    if (READ_R_FLAG(&(cpu->regs), FLAG_Z)) {
        ins->r_cycles = ins->cycles2;
        _ret(cpu, ins);
    }
}

static void
ret_c(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("RET C: %s\n", ins->name);

    if (READ_R_FLAG(&(cpu->regs), FLAG_C)) {
        ins->r_cycles = ins->cycles2;
        _ret(cpu, ins);
    }
}

static void
ret(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("RET: %s\n", ins->name);
    _ret(cpu, ins);
}

static void
reti(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("RETI: %s\n", ins->name);
    _ret(cpu, ins);
    cpu->ime = 1;
}

static void
_jp_addr16(gbc_cpu_t *cpu, instruction_t *ins, uint16_t addr)
{
    LOG_DEBUG("\t_JP ADDR16: %s %x\n", ins->name, addr);

    cpu_register_t *regs = &(cpu->regs);
    WRITE_R16(regs, REG_PC, addr);
}

static void
_jp_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("\t_JP I16: %s\n", ins->name);
    cpu_register_t *regs = &(cpu->regs);
    uint16_t addr = ins->opcode_ext.i16;
    _jp_addr16(cpu, ins, addr);
}

static void
jp_nz_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("JP NZ: %s\n", ins->name);

    if (!READ_R_FLAG(&(cpu->regs), FLAG_Z)) {
        ins->r_cycles = ins->cycles2;
        _jp_i16(cpu, ins);
    }
}

static void
jp_nc_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("JP NC: %s\n", ins->name);

    if (!READ_R_FLAG(&(cpu->regs), FLAG_C)) {
        ins->r_cycles = ins->cycles2;
        _jp_i16(cpu, ins);
    }
}

static void
jp_c_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("JP: %s\n", ins->name);

    if (READ_R_FLAG(&(cpu->regs), FLAG_C)) {
        ins->r_cycles = ins->cycles2;
        _jp_i16(cpu, ins);
    }
}

static void
jp_z_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("JP Z: %s\n", ins->name);

    if (READ_R_FLAG(&(cpu->regs), FLAG_Z)) {
        ins->r_cycles = ins->cycles2;
        _jp_i16(cpu, ins);
    }
}

static void
jp_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("JP I16: %s\n", ins->name);
    _jp_i16(cpu, ins);
}

static void
jp_r16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("JP R16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint16_t addr = READ_R16(regs, reg_offset);

    _jp_addr16(cpu, ins, addr);
}

static void
pop_r16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("POP r16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint16_t sp = READ_R16(regs, REG_SP);

    uint8_t lo = cpu->mem_read(cpu->mem_data, sp);
    uint8_t hi = cpu->mem_read(cpu->mem_data, sp + 1);

    WRITE_R16(regs, reg_offset, (hi << 8) | lo);
    WRITE_R16(regs, REG_SP, sp + 2);
}

static void
push_r16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("PUSH r16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;

    uint16_t value = READ_R16(regs, reg_offset);
    uint16_t sp = READ_R16(regs, REG_SP);

    cpu->mem_write(cpu->mem_data, sp - 1, value >> 8);
    cpu->mem_write(cpu->mem_data, sp - 2, value & UINT8_MASK);

    WRITE_R16(regs, REG_SP, sp - 2);
}

static void
ldh_im8_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("LDH m8, r8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op2;
    uint16_t addr = 0xFF00 + ins->opcode_ext.i8;
    cpu->mem_write(cpu->mem_data, addr, READ_R8(regs, reg_offset));
}

static void
ldh_r8_im8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("LDH r8, m8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint16_t addr = 0xFF00 + ins->opcode_ext.i8;
    WRITE_R8(regs, reg_offset, cpu->mem_read(cpu->mem_data, addr));
}

static void
ldh_r8_m8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("LDH r8, C: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    size_t reg_offset2 = (size_t)ins->op2;

    uint16_t addr = 0xFF00 + READ_R8(regs, reg_offset2);
    WRITE_R8(regs, reg_offset, cpu->mem_read(cpu->mem_data, addr));
}

static void
ldh_m8_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("LDH C, r8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    size_t reg_offset2 = (size_t)ins->op2;

    uint16_t addr = 0xFF00 + READ_R8(regs, reg_offset);
    cpu->mem_write(cpu->mem_data, addr, READ_R8(regs, reg_offset2));
}

static void
_call_addr(gbc_cpu_t *cpu, instruction_t *ins, uint16_t addr)
{
    LOG_DEBUG("\t_CALL ADDR: %x\n", addr);

    cpu_register_t *regs = &(cpu->regs);
    uint16_t pc = READ_R16(regs, REG_PC);
    uint16_t sp = READ_R16(regs, REG_SP);

    cpu->mem_write(cpu->mem_data, sp - 1, pc >> 8);
    cpu->mem_write(cpu->mem_data, sp - 2, pc & UINT8_MASK);

    WRITE_R16(regs, REG_SP, sp - 2);
    _jp_addr16(cpu, ins, addr);
}

static void
rst(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("RST: %s\n", ins->name);
    uint16_t addr = (uint16_t)(uintptr_t)ins->op1;
    _call_addr(cpu, ins, addr);
}

static void
_call_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("\t_CALL I16: %s\n", ins->name);

    uint16_t addr = ins->opcode_ext.i16;
    _call_addr(cpu, ins, addr);
}

void
int_call_i16(gbc_cpu_t *cpu, uint16_t addr)
{
    LOG_DEBUG("INT CALL I16\n");
    cpu_register_t *regs = &(cpu->regs);

    uint16_t pc = READ_R16(regs, REG_PC);
    uint16_t sp = READ_R16(regs, REG_SP);

    cpu->mem_write(cpu->mem_data, sp - 1, pc >> 8);
    cpu->mem_write(cpu->mem_data, sp - 2, pc & UINT8_MASK);

    WRITE_R16(regs, REG_SP, sp - 2);
    WRITE_R16(regs, REG_PC, addr);
    /* costs 20cycles (5 M-cycles), this function counts as 1 cycle */
    cpu->ins_cycles = 19;
}

static void
call_nz_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("CALL NZ: %s\n", ins->name);
    if (!READ_R_FLAG(&(cpu->regs), FLAG_Z)) {
        ins->r_cycles = ins->cycles2;
        _call_i16(cpu, ins);
    }
}

static void
call_nc_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("CALL NC: %s\n", ins->name);
    if (!READ_R_FLAG(&(cpu->regs), FLAG_C)) {
        ins->r_cycles = ins->cycles2;
        _call_i16(cpu, ins);
    }
}

static void
call_z_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("CALL Z: %s\n", ins->name);
    if (READ_R_FLAG(&(cpu->regs), FLAG_Z)) {
        ins->r_cycles = ins->cycles2;
        _call_i16(cpu, ins);
    }
}

static void
call_c_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("CALL C: %s\n", ins->name);
    if (READ_R_FLAG(&(cpu->regs), FLAG_C)) {
        ins->r_cycles = ins->cycles2;
        _call_i16(cpu, ins);
    }
}

static void
call_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("CALL: %s\n", ins->name);
    _call_i16(cpu, ins);
}

static void
cpl(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("CPL: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    uint8_t v = READ_R8(regs, REG_A);
    WRITE_R8(regs, REG_A, ~v);

    SET_R_FLAG(regs, FLAG_N);
    SET_R_FLAG(regs, FLAG_H);
}

static void
ccf(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("CCF: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    uint8_t carry = READ_R_FLAG(regs, FLAG_C);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_C, !carry);
}

static void
di(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("DI: %s\n", ins->name);
    cpu->ime = 0;
}

static void
ei(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("EI: %s\n", ins->name);
    /* EI itself and the next instruction */
    cpu->ime_insts = 2;
}

static void
halt(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("HALT: %s\n", ins->name);
    cpu->halt = 1;
}

static void
cb_rlc_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("RLC: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint8_t v = READ_R8(regs, reg_offset);

    uint8_t carry = v >> 7;
    uint8_t result = (v << 1) | carry;

    WRITE_R8(regs, reg_offset, result);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
}

static void
cb_rlc_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("RLC: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    uint16_t addr = READ_R16(regs, (size_t)ins->op1);
    uint8_t v = cpu->mem_read(cpu->mem_data, addr);

    uint8_t carry = v >> 7;
    uint8_t result = (v << 1) | carry;

    cpu->mem_write(cpu->mem_data, addr, result);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
}

static void
cb_rrc_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("RRC: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;

    uint8_t value = READ_R8(regs, reg_offset);
    uint8_t carry = value & 0x1;
    uint8_t result = (value >> 1) | (carry << 7);

    WRITE_R8(regs, reg_offset, result);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
}

static void
cb_rrc_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("RRC: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    uint16_t addr = READ_R16(regs, (size_t)ins->op1);

    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    uint8_t carry = value & 0x1;
    uint8_t result = (value >> 1) | (carry << 7);

    cpu->mem_write(cpu->mem_data, addr, result);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
}

static void
cb_rl_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("RL: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint8_t value = READ_R8(regs, reg_offset);
    uint8_t carry = READ_R_FLAG(regs, FLAG_C);
    uint8_t result = (value << 1) | carry;
    carry = value >> 7;

    WRITE_R8(regs, reg_offset, result);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
}

static void
cb_rl_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("RL: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    uint16_t addr = READ_R16(regs, (size_t)ins->op1);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    uint8_t carry = READ_R_FLAG(regs, FLAG_C);
    uint8_t result = (value << 1) | carry;
    carry = value >> 7;

    cpu->mem_write(cpu->mem_data, addr, result);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
}

static void
cb_rr_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("RR: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint8_t value = READ_R8(regs, reg_offset);
    uint8_t carry = READ_R_FLAG(regs, FLAG_C);
    uint8_t result = (value >> 1) | (carry << 7);
    carry = value & 0x1;

    WRITE_R8(regs, reg_offset, result);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
}

static void
cb_rr_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("RR: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    uint16_t addr = READ_R16(regs, (size_t)ins->op1);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    uint8_t carry = READ_R_FLAG(regs, FLAG_C);
    uint8_t result = (value >> 1) | (carry << 7);
    carry = value & 0x1;

    cpu->mem_write(cpu->mem_data, addr, result);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
}

static void
cb_sla_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("SLA: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint8_t value = READ_R8(regs, reg_offset);

    uint8_t carry = value >> 7;
    uint8_t result = value << 1;

    WRITE_R8(regs, reg_offset, result);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
}

static void
cb_sla_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("SLA: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    uint16_t addr = READ_R16(regs, (size_t)ins->op1);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);

    uint8_t carry = value >> 7;
    uint8_t result = value << 1;

    cpu->mem_write(cpu->mem_data, addr, result);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
}

static void
cb_sra_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("SRA: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint8_t value = READ_R8(regs, reg_offset);

    uint8_t carry = value & 0x1;
    uint8_t result = (value >> 1) | (value & 0x80);

    WRITE_R8(regs, reg_offset, result);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
}

static void
cb_sra_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("SRA: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    uint16_t addr = READ_R16(regs, (size_t)ins->op1);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);

    uint8_t carry = value & 0x1;
    uint8_t result = (value >> 1) | (value & 0x80);

    cpu->mem_write(cpu->mem_data, addr, result);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
}

static void
cb_swap_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("SWAP: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint8_t value = READ_R8(regs, reg_offset);

    uint8_t result = (value >> 4) | ((value << 4) & 0xf0);

    WRITE_R8(regs, reg_offset, result);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    CLEAR_R_FLAG(regs, FLAG_C);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
cb_swap_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("SWAP: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint16_t addr = READ_R16(regs, reg_offset);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);

    uint8_t result = (value >> 4) | ((value << 4) & 0xf0);

    cpu->mem_write(cpu->mem_data, addr, result);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    CLEAR_R_FLAG(regs, FLAG_C);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
cb_srl_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("SRL: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    size_t reg_offset = (size_t)ins->op1;
    uint8_t value = READ_R8(regs, reg_offset);

    uint8_t carry = value & 0x1;
    uint8_t result = value >> 1;

    WRITE_R8(regs, reg_offset, result);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
}

static void
cb_srl_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("SRL: %s\n", ins->name);
    cpu_register_t *regs = &(cpu->regs);
    uint16_t addr = READ_R16(regs, (size_t)ins->op1);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);

    uint8_t carry = value & 0x1;
    uint8_t result = value >> 1;

    cpu->mem_write(cpu->mem_data, addr, result);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
}

static void
cb_bit_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("BIT: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    uint8_t bit = (uint8_t)(uintptr_t)ins->op1;
    size_t reg_offset = (size_t)ins->op2;

    uint8_t value = READ_R8(regs, reg_offset);
    uint8_t result = value & (1 << bit);

    CLEAR_R_FLAG(regs, FLAG_N);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
cb_bit_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("BIT: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    uint8_t bit = (uint8_t)(uintptr_t)ins->op1;
    size_t reg_offset = (size_t)ins->op2;
    uint16_t addr = READ_R16(regs, reg_offset);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);

    uint8_t result = value & (1 << bit);

    CLEAR_R_FLAG(regs, FLAG_N);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}

static void
cb_res_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("RES: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    uint8_t bit = (uint8_t)(uintptr_t)ins->op1;
    size_t reg_offset = (size_t)ins->op2;

    uint8_t value = READ_R8(regs, reg_offset);
    uint8_t result = value & ~(1 << bit);

    WRITE_R8(regs, reg_offset, result);
}

static void
cb_res_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("RES: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    uint8_t bit = (uint8_t)(uintptr_t)ins->op1;
    size_t reg_offset = (size_t)ins->op2;
    uint16_t addr = READ_R16(regs, reg_offset);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);

    uint8_t result = value & ~(1 << bit);

    cpu->mem_write(cpu->mem_data, addr, result);
}

static void
cb_set_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("SET: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    uint8_t bit = (uint8_t)(uintptr_t)ins->op1;
    size_t reg_offset = (size_t)ins->op2;

    uint8_t value = READ_R8(regs, reg_offset);
    uint8_t result = value | (1 << bit);

    WRITE_R8(regs, reg_offset, result);
}

static void
cb_set_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_DEBUG("SET: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->regs);
    uint8_t bit = (uint8_t)(uintptr_t)ins->op1;
    size_t reg_offset = (size_t)ins->op2;
    uint16_t addr = READ_R16(regs, reg_offset);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);

    uint8_t result = value | (1 << bit);

    cpu->mem_write(cpu->mem_data, addr, result);
}

/* The following fields in instruction will be modified during the execution:
    r_cycles, op1, op2, opcode_ext
 */
static instruction_t instruction_set[INSTRUCTIONS_SET_SIZE] = {
    /* 0x00 */
    INSTRUCTION_ADD(0x00, 1, nop, NULL, NULL, 4, 4, "NOP"),
    INSTRUCTION_ADD(0x01, 3, ld_r16_i16, REG_BC, NULL, 12, 12, "LD BC, n16"),
    INSTRUCTION_ADD(0x02, 1, ld_m16_r8, REG_BC, REG_A, 8, 8, "LD (BC), A"),
    INSTRUCTION_ADD(0x03, 1, inc_r16, REG_BC, NULL, 8, 8, "INC BC"),
    INSTRUCTION_ADD(0x04, 1, inc_r8, REG_B, NULL, 4, 4, "INC B"),
    INSTRUCTION_ADD(0x05, 1, dec_r8, REG_B, NULL, 4, 4, "DEC B"),
    INSTRUCTION_ADD(0x06, 2, ld_r8_i8, REG_B, NULL,  8, 8, "LD B, n8"),
    INSTRUCTION_ADD(0x07, 1, rlca, NULL, NULL, 4, 4, "RLCA"),
    INSTRUCTION_ADD(0x08, 3, ld_im16_r16, NULL, REG_SP, 20, 20, "LD (n16), SP"),
    INSTRUCTION_ADD(0x09, 1, add_r16_r16, REG_HL, REG_BC, 8, 8, "ADD HL, BC"),
    INSTRUCTION_ADD(0x0a, 1, ld_r8_m16, REG_A, REG_BC, 8, 8, "LD A, (BC)"),
    INSTRUCTION_ADD(0x0b, 1, dec_r16, REG_BC, NULL, 8, 8, "DEC BC"),
    INSTRUCTION_ADD(0x0c, 1, inc_r8, REG_C, NULL, 4, 4, "INC C"),
    INSTRUCTION_ADD(0x0d, 1, dec_r8, REG_C, NULL, 4, 4, "DEC C"),
    INSTRUCTION_ADD(0x0e, 2, ld_r8_i8, REG_C, NULL, 8, 8, "LD C, n8"),
    INSTRUCTION_ADD(0x0f, 1, rrca, NULL, NULL, 4, 4, "RRCA"),

    /* 0x10 */
    INSTRUCTION_ADD(0x10, 2, stop, NULL, NULL, 4, 4, "STOP"),
    INSTRUCTION_ADD(0x11, 3, ld_r16_i16, REG_DE, NULL, 12, 12, "LD DE, n16"),
    INSTRUCTION_ADD(0x12, 1, ld_m16_r8, REG_DE, REG_A, 8, 8, "LD (DE), A"),
    INSTRUCTION_ADD(0x13, 1, inc_r16, REG_DE, NULL, 8, 8, "INC DE"),
    INSTRUCTION_ADD(0x14, 1, inc_r8, REG_D, NULL, 4, 4, "INC D"),
    INSTRUCTION_ADD(0x15, 1, dec_r8, REG_D, NULL, 4, 4, "DEC D"),
    INSTRUCTION_ADD(0x16, 2, ld_r8_i8, REG_D, NULL, 8, 8, "LD D, n8"),
    INSTRUCTION_ADD(0x17, 1, rla, NULL, NULL, 4, 4, "RLA"),
    INSTRUCTION_ADD(0x18, 2, jr_i8, NULL, NULL, 12, 12, "JR e8"),
    INSTRUCTION_ADD(0x19, 1, add_r16_r16, REG_HL, REG_DE, 8, 8, "ADD HL, DE"),
    INSTRUCTION_ADD(0x1a, 1, ld_r8_m16, REG_A, REG_DE, 8, 8, "LD A, (DE)"),
    INSTRUCTION_ADD(0x1b, 1, dec_r16, REG_DE, NULL, 8, 8, "DEC DE"),
    INSTRUCTION_ADD(0x1c, 1, inc_r8, REG_E, NULL, 4, 4, "INC E"),
    INSTRUCTION_ADD(0x1d, 1, dec_r8, REG_E, NULL, 4, 4, "DEC E"),
    INSTRUCTION_ADD(0x1e, 2, ld_r8_i8, REG_E, NULL, 8, 8, "LD E, n8"),
    INSTRUCTION_ADD(0x1f, 1, rra, NULL, NULL, 4, 4, "RRA"),

    /* 0x20 */
    INSTRUCTION_ADD(0x20, 2, jr_nz_i8, NULL, NULL, 8, 12, "JR NZ, e8"),
    INSTRUCTION_ADD(0x21, 3, ld_r16_i16, REG_HL, NULL, 12, 12, "LD HL, n16"),
    INSTRUCTION_ADD(0x22, 1, ldi_m16_r8, REG_HL, REG_A, 8, 8, "LDI (HL), A"),
    INSTRUCTION_ADD(0x23, 1, inc_r16, REG_HL, NULL, 8, 8, "INC HL"),
    INSTRUCTION_ADD(0x24, 1, inc_r8, REG_H, NULL, 4, 4, "INC H"),
    INSTRUCTION_ADD(0x25, 1, dec_r8, REG_H, NULL, 4, 4, "DEC H"),
    INSTRUCTION_ADD(0x26, 2, ld_r8_i8, REG_H, NULL, 8, 8, "LD H, n8"),
    INSTRUCTION_ADD(0x27, 1, daa, NULL, NULL, 4, 4, "DAA"),
    INSTRUCTION_ADD(0x28, 2, jr_z_i8, NULL, NULL, 8, 12, "JR Z, e8"),
    INSTRUCTION_ADD(0x29, 1, add_r16_r16, REG_HL, REG_HL, 8, 8, "ADD HL, HL"),
    INSTRUCTION_ADD(0x2a, 1, ldi_r8_m16, REG_A, REG_HL, 8, 8, "LDI A, (HL)"),
    INSTRUCTION_ADD(0x2b, 1, dec_r16, REG_HL, NULL, 8, 8, "DEC HL"),
    INSTRUCTION_ADD(0x2c, 1, inc_r8, REG_L, NULL, 4, 4, "INC L"),
    INSTRUCTION_ADD(0x2d, 1, dec_r8, REG_L, NULL, 4, 4, "DEC L"),
    INSTRUCTION_ADD(0x2e, 2, ld_r8_i8, REG_L, NULL, 8, 8, "LD L, n8"),
    INSTRUCTION_ADD(0x2f, 1, cpl, NULL, NULL, 4, 4, "CPL"),

    /* 0x30 */
    INSTRUCTION_ADD(0x30, 2, jr_nc_i8, NULL, NULL, 8, 12, "JR NC, e8"),
    INSTRUCTION_ADD(0x31, 3, ld_r16_i16, REG_SP, NULL, 12, 12, "LD SP, n16"),
    INSTRUCTION_ADD(0x32, 1, ldd_m16_r8, REG_HL, REG_A, 8, 8, "LDD (HL), A"),
    INSTRUCTION_ADD(0x33, 1, inc_r16, REG_SP, NULL, 8, 8, "INC SP"),
    INSTRUCTION_ADD(0x34, 1, inc_m16, REG_HL, NULL, 12, 12, "INC (HL)"),
    INSTRUCTION_ADD(0x35, 1, dec_m16, REG_HL, NULL, 12, 12, "DEC (HL)"),
    INSTRUCTION_ADD(0x36, 2, ld_m16_i8, REG_HL, NULL, 12, 12, "LD (HL), n8"),
    INSTRUCTION_ADD(0x37, 1, scf, NULL, NULL, 4, 4, "SCF"),
    INSTRUCTION_ADD(0x38, 2, jr_c_i8, NULL, NULL, 8, 12, "JR C, e8"),
    INSTRUCTION_ADD(0x39, 1, add_r16_r16, REG_HL, REG_SP, 8, 8, "ADD HL, SP"),
    INSTRUCTION_ADD(0x3a, 1, ldd_r8_m16, REG_A, REG_HL, 8, 8, "LDD A, (HL)"),
    INSTRUCTION_ADD(0x3b, 1, dec_r16, REG_SP, NULL, 8, 8, "DEC SP"),
    INSTRUCTION_ADD(0x3c, 1, inc_r8, REG_A, NULL, 4, 4, "INC A"),
    INSTRUCTION_ADD(0x3d, 1, dec_r8, REG_A, NULL, 4, 4, "DEC A"),
    INSTRUCTION_ADD(0x3e, 2, ld_r8_i8, REG_A, NULL, 8, 8, "LD A, n8"),
    INSTRUCTION_ADD(0x3f, 1, ccf, NULL, NULL, 4, 4, "CCF"),

    /* 0x40 */
    INSTRUCTION_ADD(0x40, 1, ld_r8_r8, REG_B, REG_B, 4, 4, "LD B, B"),
    INSTRUCTION_ADD(0x41, 1, ld_r8_r8, REG_B, REG_C, 4, 4, "LD B, C"),
    INSTRUCTION_ADD(0x42, 1, ld_r8_r8, REG_B, REG_D, 4, 4, "LD B, D"),
    INSTRUCTION_ADD(0x43, 1, ld_r8_r8, REG_B, REG_E, 4, 4, "LD B, E"),
    INSTRUCTION_ADD(0x44, 1, ld_r8_r8, REG_B, REG_H, 4, 4, "LD B, H"),
    INSTRUCTION_ADD(0x45, 1, ld_r8_r8, REG_B, REG_L, 4, 4, "LD B, L"),
    INSTRUCTION_ADD(0x46, 1, ld_r8_m16, REG_B, REG_HL, 8, 8, "LD B, (HL)"),
    INSTRUCTION_ADD(0x47, 1, ld_r8_r8, REG_B, REG_A, 4, 4, "LD B, A"),
    INSTRUCTION_ADD(0x48, 1, ld_r8_r8, REG_C, REG_B, 4, 4, "LD C, B"),
    INSTRUCTION_ADD(0x49, 1, ld_r8_r8, REG_C, REG_C, 4, 4, "LD C, C"),
    INSTRUCTION_ADD(0x4a, 1, ld_r8_r8, REG_C, REG_D, 4, 4, "LD C, D"),
    INSTRUCTION_ADD(0x4b, 1, ld_r8_r8, REG_C, REG_E, 4, 4, "LD C, E"),
    INSTRUCTION_ADD(0x4c, 1, ld_r8_r8, REG_C, REG_H, 4, 4, "LD C, H"),
    INSTRUCTION_ADD(0x4d, 1, ld_r8_r8, REG_C, REG_L, 4, 4, "LD C, L"),
    INSTRUCTION_ADD(0x4e, 1, ld_r8_m16, REG_C, REG_HL, 8, 8, "LD C, (HL)"),
    INSTRUCTION_ADD(0x4f, 1, ld_r8_r8, REG_C, REG_A, 4, 4, "LD C, A"),

    /* 0x50 */
    INSTRUCTION_ADD(0x50, 1, ld_r8_r8, REG_D, REG_B, 4, 4, "LD D, B"),
    INSTRUCTION_ADD(0x51, 1, ld_r8_r8, REG_D, REG_C, 4, 4, "LD D, C"),
    INSTRUCTION_ADD(0x52, 1, ld_r8_r8, REG_D, REG_D, 4, 4, "LD D, D"), /* fancy */
    INSTRUCTION_ADD(0x53, 1, ld_r8_r8, REG_D, REG_E, 4, 4, "LD D, E"),
    INSTRUCTION_ADD(0x54, 1, ld_r8_r8, REG_D, REG_H, 4, 4, "LD D, H"),
    INSTRUCTION_ADD(0x55, 1, ld_r8_r8, REG_D, REG_L, 4, 4, "LD D, L"),
    INSTRUCTION_ADD(0x56, 1, ld_r8_m16, REG_D, REG_HL, 8, 8, "LD D, (HL)"),
    INSTRUCTION_ADD(0x57, 1, ld_r8_r8, REG_D, REG_A, 4, 4, "LD D, A"),
    INSTRUCTION_ADD(0x58, 1, ld_r8_r8, REG_E, REG_B, 4, 4, "LD E, B"),
    INSTRUCTION_ADD(0x59, 1, ld_r8_r8, REG_E, REG_C, 4, 4, "LD E, C"),
    INSTRUCTION_ADD(0x5a, 1, ld_r8_r8, REG_E, REG_D, 4, 4, "LD E, D"),
    INSTRUCTION_ADD(0x5b, 1, ld_r8_r8, REG_E, REG_E, 4, 4, "LD E, E"), /* fancy */
    INSTRUCTION_ADD(0x5c, 1, ld_r8_r8, REG_E, REG_H, 4, 4, "LD E, H"),
    INSTRUCTION_ADD(0x5d, 1, ld_r8_r8, REG_E, REG_L, 4, 4, "LD E, L"),
    INSTRUCTION_ADD(0x5e, 1, ld_r8_m16, REG_E, REG_HL, 8, 8, "LD E, (HL)"),
    INSTRUCTION_ADD(0x5f, 1, ld_r8_r8, REG_E, REG_A, 4, 4, "LD E, A"),

    /* 0x60 */
    INSTRUCTION_ADD(0x60, 1, ld_r8_r8, REG_H, REG_B, 4, 4, "LD H, B"),
    INSTRUCTION_ADD(0x61, 1, ld_r8_r8, REG_H, REG_C, 4, 4, "LD H, C"),
    INSTRUCTION_ADD(0x62, 1, ld_r8_r8, REG_H, REG_D, 4, 4, "LD H, D"),
    INSTRUCTION_ADD(0x63, 1, ld_r8_r8, REG_H, REG_E, 4, 4, "LD H, E"),
    INSTRUCTION_ADD(0x64, 1, ld_r8_r8, REG_H, REG_H, 4, 4, "LD H, H"), /* fancy */
    INSTRUCTION_ADD(0x65, 1, ld_r8_r8, REG_H, REG_L, 4, 4, "LD H, L"),
    INSTRUCTION_ADD(0x66, 1, ld_r8_m16, REG_H, REG_HL, 8, 8, "LD H, (HL)"),
    INSTRUCTION_ADD(0x67, 1, ld_r8_r8, REG_H, REG_A, 4, 4, "LD H, A"),
    INSTRUCTION_ADD(0x68, 1, ld_r8_r8, REG_L, REG_B, 4, 4, "LD L, B"),
    INSTRUCTION_ADD(0x69, 1, ld_r8_r8, REG_L, REG_C, 4, 4, "LD L, C"),
    INSTRUCTION_ADD(0x6a, 1, ld_r8_r8, REG_L, REG_D, 4, 4, "LD L, D"),
    INSTRUCTION_ADD(0x6b, 1, ld_r8_r8, REG_L, REG_E, 4, 4, "LD L, E"),
    INSTRUCTION_ADD(0x6c, 1, ld_r8_r8, REG_L, REG_H, 4, 4, "LD L, H"),
    INSTRUCTION_ADD(0x6d, 1, ld_r8_r8, REG_L, REG_L, 4, 4, "LD L, L"), /* fancy */
    INSTRUCTION_ADD(0x6e, 1, ld_r8_m16, REG_L, REG_HL, 8, 8, "LD L, (HL)"),
    INSTRUCTION_ADD(0x6f, 1, ld_r8_r8, REG_L, REG_A, 4, 4, "LD L, A"),

    /* 0x70 */
    INSTRUCTION_ADD(0x70, 1, ld_m16_r8, REG_HL, REG_B, 8, 8, "LD (HL), B"),
    INSTRUCTION_ADD(0x71, 1, ld_m16_r8, REG_HL, REG_C, 8, 8, "LD (HL), C"),
    INSTRUCTION_ADD(0x72, 1, ld_m16_r8, REG_HL, REG_D, 8, 8, "LD (HL), D"),
    INSTRUCTION_ADD(0x73, 1, ld_m16_r8, REG_HL, REG_E, 8, 8, "LD (HL), E"),
    INSTRUCTION_ADD(0x74, 1, ld_m16_r8, REG_HL, REG_H, 8, 8, "LD (HL), H"),
    INSTRUCTION_ADD(0x75, 1, ld_m16_r8, REG_HL, REG_L, 8, 8, "LD (HL), L"),
    INSTRUCTION_ADD(0x76, 1, halt, NULL, NULL, 4, 4, "HALT"),
    INSTRUCTION_ADD(0x77, 1, ld_m16_r8, REG_HL, REG_A, 8, 8, "LD (HL), A"),
    INSTRUCTION_ADD(0x78, 1, ld_r8_r8, REG_A, REG_B, 4, 4, "LD A, B"),
    INSTRUCTION_ADD(0x79, 1, ld_r8_r8, REG_A, REG_C, 4, 4, "LD A, C"),
    INSTRUCTION_ADD(0x7a, 1, ld_r8_r8, REG_A, REG_D, 4, 4, "LD A, D"),
    INSTRUCTION_ADD(0x7b, 1, ld_r8_r8, REG_A, REG_E, 4, 4, "LD A, E"),
    INSTRUCTION_ADD(0x7c, 1, ld_r8_r8, REG_A, REG_H, 4, 4, "LD A, H"),
    INSTRUCTION_ADD(0x7d, 1, ld_r8_r8, REG_A, REG_L, 4, 4, "LD A, L"),
    INSTRUCTION_ADD(0x7e, 1, ld_r8_m16, REG_A, REG_HL, 8, 8, "LD A, (HL)"),
    INSTRUCTION_ADD(0x7f, 1, ld_r8_r8, REG_A, REG_A, 4, 4, "LD A, A"),

    /* 0x80 */
    INSTRUCTION_ADD(0x80, 1, add_r8_r8, REG_A, REG_B, 4, 4, "ADD A, B"),
    INSTRUCTION_ADD(0x81, 1, add_r8_r8, REG_A, REG_C, 4, 4, "ADD A, C"),
    INSTRUCTION_ADD(0x82, 1, add_r8_r8, REG_A, REG_D, 4, 4, "ADD A, D"),
    INSTRUCTION_ADD(0x83, 1, add_r8_r8, REG_A, REG_E, 4, 4, "ADD A, E"),
    INSTRUCTION_ADD(0x84, 1, add_r8_r8, REG_A, REG_H, 4, 4, "ADD A, H"),
    INSTRUCTION_ADD(0x85, 1, add_r8_r8, REG_A, REG_L, 4, 4, "ADD A, L"),
    INSTRUCTION_ADD(0x86, 1, add_r8_m16, REG_A, REG_HL, 8, 8, "ADD A, (HL)"),
    INSTRUCTION_ADD(0x87, 1, add_r8_r8, REG_A, REG_A, 4, 4, "ADD A, A"),
    INSTRUCTION_ADD(0x88, 1, adc_r8_r8, REG_A, REG_B, 4, 4, "ADC A, B"),
    INSTRUCTION_ADD(0x89, 1, adc_r8_r8, REG_A, REG_C, 4, 4, "ADC A, C"),
    INSTRUCTION_ADD(0x8a, 1, adc_r8_r8, REG_A, REG_D, 4, 4, "ADC A, D"),
    INSTRUCTION_ADD(0x8b, 1, adc_r8_r8, REG_A, REG_E, 4, 4, "ADC A, E"),
    INSTRUCTION_ADD(0x8c, 1, adc_r8_r8, REG_A, REG_H, 4, 4, "ADC A, H"),
    INSTRUCTION_ADD(0x8d, 1, adc_r8_r8, REG_A, REG_L, 4, 4, "ADC A, L"),
    INSTRUCTION_ADD(0x8e, 1, adc_r8_m16, REG_A, REG_HL, 8, 8, "ADC A, (HL)"),
    INSTRUCTION_ADD(0x8f, 1, adc_r8_r8, REG_A, REG_A, 4, 4, "ADC A, A"),

    /* 0x90 */
    INSTRUCTION_ADD(0x90, 1, sub_r8_r8, REG_A, REG_B, 4, 4, "SUB A, B"),
    INSTRUCTION_ADD(0x91, 1, sub_r8_r8, REG_A, REG_C, 4, 4, "SUB A, C"),
    INSTRUCTION_ADD(0x92, 1, sub_r8_r8, REG_A, REG_D, 4, 4, "SUB A, D"),
    INSTRUCTION_ADD(0x93, 1, sub_r8_r8, REG_A, REG_E, 4, 4, "SUB A, E"),
    INSTRUCTION_ADD(0x94, 1, sub_r8_r8, REG_A, REG_H, 4, 4, "SUB A, H"),
    INSTRUCTION_ADD(0x95, 1, sub_r8_r8, REG_A, REG_L, 4, 4, "SUB A, L"),
    INSTRUCTION_ADD(0x96, 1, sub_r8_m16, REG_A, REG_HL, 8, 8, "SUB A, (HL)"),
    INSTRUCTION_ADD(0x97, 1, sub_r8_r8, REG_A, REG_A, 4, 4, "SUB A, A"),
    INSTRUCTION_ADD(0x98, 1, subc_r8_r8, REG_A, REG_B, 4, 4, "SUBC A, B"),
    INSTRUCTION_ADD(0x99, 1, subc_r8_r8, REG_A, REG_C, 4, 4, "SUBC A, C"),
    INSTRUCTION_ADD(0x9a, 1, subc_r8_r8, REG_A, REG_D, 4, 4, "SUBC A, D"),
    INSTRUCTION_ADD(0x9b, 1, subc_r8_r8, REG_A, REG_E, 4, 4, "SUBC A, E"),
    INSTRUCTION_ADD(0x9c, 1, subc_r8_r8, REG_A, REG_H, 4, 4, "SUBC A, H"),
    INSTRUCTION_ADD(0x9d, 1, subc_r8_r8, REG_A, REG_L, 4, 4, "SUBC A, L"),
    INSTRUCTION_ADD(0x9e, 1, subc_r8_m16, REG_A, REG_HL, 8, 8, "SUBC A, (HL)"),
    INSTRUCTION_ADD(0x9f, 1, subc_r8_r8, REG_A, REG_A, 4, 4, "SUBC A, A"),

    /* 0xa0 */
    INSTRUCTION_ADD(0xa0, 1, and_r8_r8, REG_A, REG_B, 4, 4, "AND A, B"),
    INSTRUCTION_ADD(0xa1, 1, and_r8_r8, REG_A, REG_C, 4, 4, "AND A, C"),
    INSTRUCTION_ADD(0xa2, 1, and_r8_r8, REG_A, REG_D, 4, 4, "AND A, D"),
    INSTRUCTION_ADD(0xa3, 1, and_r8_r8, REG_A, REG_E, 4, 4, "AND A, E"),
    INSTRUCTION_ADD(0xa4, 1, and_r8_r8, REG_A, REG_H, 4, 4, "AND A, H"),
    INSTRUCTION_ADD(0xa5, 1, and_r8_r8, REG_A, REG_L, 4, 4, "AND A, L"),
    INSTRUCTION_ADD(0xa6, 1, and_r8_m16, REG_A, REG_HL, 8, 8, "AND A, (HL)"),
    INSTRUCTION_ADD(0xa7, 1, and_r8_r8, REG_A, REG_A, 4, 4, "AND A, A"),
    INSTRUCTION_ADD(0xa8, 1, xor_r8_r8, REG_A, REG_B, 4, 4, "XOR A, B"),
    INSTRUCTION_ADD(0xa9, 1, xor_r8_r8, REG_A, REG_C, 4, 4, "XOR A, C"),
    INSTRUCTION_ADD(0xaa, 1, xor_r8_r8, REG_A, REG_D, 4, 4, "XOR A, D"),
    INSTRUCTION_ADD(0xab, 1, xor_r8_r8, REG_A, REG_E, 4, 4, "XOR A, E"),
    INSTRUCTION_ADD(0xac, 1, xor_r8_r8, REG_A, REG_H, 4, 4, "XOR A, H"),
    INSTRUCTION_ADD(0xad, 1, xor_r8_r8, REG_A, REG_L, 4, 4, "XOR A, L"),
    INSTRUCTION_ADD(0xae, 1, xor_r8_m16, REG_A, REG_HL, 8, 8, "XOR A, (HL)"),
    INSTRUCTION_ADD(0xaf, 1, xor_r8_r8, REG_A, REG_A, 4, 4, "XOR A, A"),

    /* 0xb0 */
    INSTRUCTION_ADD(0xb0, 1, or_r8_r8, REG_A, REG_B, 4, 4, "OR A, B"),
    INSTRUCTION_ADD(0xb1, 1, or_r8_r8, REG_A, REG_C, 4, 4, "OR A, C"),
    INSTRUCTION_ADD(0xb2, 1, or_r8_r8, REG_A, REG_D, 4, 4, "OR A, D"),
    INSTRUCTION_ADD(0xb3, 1, or_r8_r8, REG_A, REG_E, 4, 4, "OR A, E"),
    INSTRUCTION_ADD(0xb4, 1, or_r8_r8, REG_A, REG_H, 4, 4, "OR A, H"),
    INSTRUCTION_ADD(0xb5, 1, or_r8_r8, REG_A, REG_L, 4, 4, "OR A, L"),
    INSTRUCTION_ADD(0xb6, 1, or_r8_m16, REG_A, REG_HL, 8, 8, "OR A, (HL)"),
    INSTRUCTION_ADD(0xb7, 1, or_r8_r8, REG_A, REG_A, 4, 4, "OR A, A"),
    INSTRUCTION_ADD(0xb8, 1, cp_r8_r8, REG_A, REG_B, 4, 4, "CP A, B"),
    INSTRUCTION_ADD(0xb9, 1, cp_r8_r8, REG_A, REG_C, 4, 4, "CP A, C"),
    INSTRUCTION_ADD(0xba, 1, cp_r8_r8, REG_A, REG_D, 4, 4, "CP A, D"),
    INSTRUCTION_ADD(0xbb, 1, cp_r8_r8, REG_A, REG_E, 4, 4, "CP A, E"),
    INSTRUCTION_ADD(0xbc, 1, cp_r8_r8, REG_A, REG_H, 4, 4, "CP A, H"),
    INSTRUCTION_ADD(0xbd, 1, cp_r8_r8, REG_A, REG_L, 4, 4, "CP A, L"),
    INSTRUCTION_ADD(0xbe, 1, cp_r8_m16, REG_A, REG_HL, 8, 8, "CP A, (HL)"),
    INSTRUCTION_ADD(0xbf, 1, cp_r8_r8, REG_A, REG_A, 4, 4, "CP A, A"),

    /* 0xc0 */
    INSTRUCTION_ADD(0xc0, 1, ret_nz, NULL, NULL, 8, 20, "RET NZ"),
    INSTRUCTION_ADD(0xc1, 1, pop_r16, REG_BC, NULL, 12, 12, "POP BC"),
    INSTRUCTION_ADD(0xc2, 3, jp_nz_i16, NULL, NULL, 12, 16, "JP NZ, n16"),
    INSTRUCTION_ADD(0xc3, 3, jp_i16, NULL, NULL, 16, 16, "JP n16"),
    INSTRUCTION_ADD(0xc4, 3, call_nz_i16, NULL, NULL, 12, 24, "CALL NZ, n16"),
    INSTRUCTION_ADD(0xc5, 1, push_r16, REG_BC, NULL, 16, 16, "PUSH BC"),
    INSTRUCTION_ADD(0xc6, 2, add_r8_i8, REG_A, NULL, 8, 8, "ADD A, n8"),
    INSTRUCTION_ADD(0xc7, 1, rst, 0x00, NULL, 16, 16, "RST 00H"),
    INSTRUCTION_ADD(0xc8, 1, ret_z, NULL, NULL, 8, 20, "RET Z"),
    INSTRUCTION_ADD(0xc9, 1, ret, NULL, NULL, 16, 16, "RET"),
    INSTRUCTION_ADD(0xca, 3, jp_z_i16, NULL, NULL, 12, 16, "JP Z, n16"),
    INSTRUCTION_ADD(0xcb, 1, NULL, NULL, NULL, 4, 4, "CB"), /* PREFIX_CB */
    INSTRUCTION_ADD(0xcc, 3, call_z_i16, NULL, NULL, 12, 24, "CALL Z, n16"),
    INSTRUCTION_ADD(0xcd, 3, call_i16, NULL, NULL, 24, 24, "CALL n16"),
    INSTRUCTION_ADD(0xce, 2, adc_r8_i8, REG_A, NULL, 8, 8, "ADC A, n8"),
    INSTRUCTION_ADD(0xcf, 1, rst, 0x08, NULL, 16, 16, "RST 08H"),

    /* 0xd0 */
    INSTRUCTION_ADD(0xd0, 1, ret_nc, NULL, NULL, 8, 20, "RET NC"),
    INSTRUCTION_ADD(0xd1, 1, pop_r16, REG_DE, NULL, 12, 12, "POP DE"),
    INSTRUCTION_ADD(0xd2, 3, jp_nc_i16, NULL, NULL, 12, 16, "JP NC, n16"),
    INSTRUCTION_ADD(0xd3, 1, NULL, NULL, NULL, 4, 4, "NOP"),
    INSTRUCTION_ADD(0xd4, 3, call_nc_i16, NULL, NULL, 12, 24, "CALL NC, n16"),
    INSTRUCTION_ADD(0xd5, 1, push_r16, REG_DE, NULL, 16, 16, "PUSH DE"),
    INSTRUCTION_ADD(0xd6, 2, sub_r8_i8, REG_A, NULL, 8, 8, "SUB A, n8"),
    INSTRUCTION_ADD(0xd7, 1, rst, 0x10, NULL, 16, 16, "RST 10H"),
    INSTRUCTION_ADD(0xd8, 1, ret_c, NULL, NULL, 8, 20, "RET C"),
    INSTRUCTION_ADD(0xd9, 1, reti, NULL, NULL, 16, 16, "RETI"),
    INSTRUCTION_ADD(0xda, 3, jp_c_i16, NULL, NULL, 12, 16, "JP C, n16"),
    INSTRUCTION_ADD(0xdb, 1, NULL, NULL, NULL,  4, 4, "NOP"),
    INSTRUCTION_ADD(0xdc, 3, call_c_i16, NULL, NULL, 12, 24, "CALL C, n16"),
    INSTRUCTION_ADD(0xdd, 1, NULL, NULL, NULL, 4, 4, "NOP"),
    INSTRUCTION_ADD(0xde, 2, subc_r8_i8, REG_A, NULL, 8, 8, "SUBC A, n8"),
    INSTRUCTION_ADD(0xdf, 1, rst, 0x18, NULL, 16, 16, "RST 18H"),

    /* 0xe0 */
    INSTRUCTION_ADD(0xe0, 2, ldh_im8_r8, NULL, REG_A, 12, 12, "LDH (n8), A"),
    INSTRUCTION_ADD(0xe1, 1, pop_r16, REG_HL, NULL, 12, 12, "POP HL"),
    INSTRUCTION_ADD(0xe2, 1, ldh_m8_r8, REG_C, REG_A, 8, 8, "LDH (C), A"),
    INSTRUCTION_ADD(0xe3, 1, NULL, NULL, NULL, 4, 4, "NOP"),
    INSTRUCTION_ADD(0xe4, 1, NULL, NULL, NULL, 4, 4, "NOP"),
    INSTRUCTION_ADD(0xe5, 1, push_r16, REG_HL, NULL, 16, 16, "PUSH HL"),
    INSTRUCTION_ADD(0xe6, 2, and_r8_i8, REG_A, NULL, 8, 8, "AND A, n8"),
    INSTRUCTION_ADD(0xe7, 1, rst, 0x20, NULL, 16, 16, "RST 20H"),
    INSTRUCTION_ADD(0xe8, 2, add_r16_i8, REG_SP, NULL, 16, 16, "ADD SP, n8"),
    INSTRUCTION_ADD(0xe9, 1, jp_r16, REG_HL, NULL, 4, 4, "JP HL"),
    INSTRUCTION_ADD(0xea, 3, ld_im16_r8, NULL, REG_A, 16, 16, "LD (n16), A"),
    INSTRUCTION_ADD(0xeb, 1, NULL, NULL, NULL, 4, 4, "NOP"),
    INSTRUCTION_ADD(0xec, 1, NULL, NULL, NULL, 4, 4, "NOP"),
    INSTRUCTION_ADD(0xed, 1, NULL, NULL, NULL, 4, 4, "NOP"),
    INSTRUCTION_ADD(0xee, 2, xor_r8_i8, REG_A, NULL, 8, 8, "XOR A, n8"),
    INSTRUCTION_ADD(0xef, 1, rst, 0x28, NULL, 16, 16, "RST 28H"),

    /* 0xf0 */
    INSTRUCTION_ADD(0xf0, 2, ldh_r8_im8, REG_A, NULL, 12, 12, "LDH A, (n8)"),
    INSTRUCTION_ADD(0xf1, 1, pop_r16, REG_AF, NULL, 12, 12, "POP AF"),
    INSTRUCTION_ADD(0xf2, 1, ldh_r8_m8, REG_A, REG_C, 8, 8, "LDH A, (C)"),
    INSTRUCTION_ADD(0xf3, 1, di, NULL, NULL, 4, 4, "DI"),
    INSTRUCTION_ADD(0xf4, 1, NULL, NULL, NULL, 4, 4, "NOP"),
    INSTRUCTION_ADD(0xf5, 1, push_r16, REG_AF, NULL, 16, 16, "PUSH AF"),
    INSTRUCTION_ADD(0xf6, 2, or_r8_i8, REG_A, NULL, 8, 8, "OR A, n8"),
    INSTRUCTION_ADD(0xf7, 1, rst, 0x30, NULL, 16, 16, "RST 30H"),
    INSTRUCTION_ADD(0xf8, 2, ld_hl_sp_i8, NULL, NULL, 12, 12, "LD HL, SP+n8"),
    INSTRUCTION_ADD(0xf9, 1, ld_sp_hl, NULL, NULL, 8, 8, "LD SP, HL"),
    INSTRUCTION_ADD(0xfa, 3, ld_r8_im16, REG_A, NULL, 16, 16, "LD A, (n16)"),
    INSTRUCTION_ADD(0xfb, 1, ei, NULL, NULL, 4, 4, "EI"),
    INSTRUCTION_ADD(0xfc, 1, NULL, NULL, NULL, 4, 4, "NOP"),
    INSTRUCTION_ADD(0xfd, 1, NULL, NULL, NULL, 4, 4, "NOP"),
    INSTRUCTION_ADD(0xfe, 2, cp_r8_i8, REG_A, NULL, 8, 8, "CP A, n8"),
    INSTRUCTION_ADD(0xff, 1, rst, 0x38, NULL, 16, 16, "RST 38H"),
};

static instruction_t prefixed_instruction_set[INSTRUCTIONS_SET_SIZE] = {
    /* 0x00 */
    INSTRUCTION_ADD(0x00, 2, cb_rlc_r8, REG_B, NULL, 8, 8, "RLC B"),
    INSTRUCTION_ADD(0x01, 2, cb_rlc_r8, REG_C, NULL, 8, 8, "RLC C"),
    INSTRUCTION_ADD(0x02, 2, cb_rlc_r8, REG_D, NULL, 8, 8, "RLC D"),
    INSTRUCTION_ADD(0x03, 2, cb_rlc_r8, REG_E, NULL, 8, 8, "RLC E"),
    INSTRUCTION_ADD(0x04, 2, cb_rlc_r8, REG_H, NULL, 8, 8, "RLC H"),
    INSTRUCTION_ADD(0x05, 2, cb_rlc_r8, REG_L, NULL, 8, 8, "RLC L"),
    INSTRUCTION_ADD(0x06, 2, cb_rlc_m16, REG_HL, NULL, 16, 16, "RLC (HL)"),
    INSTRUCTION_ADD(0x07, 2, cb_rlc_r8, REG_A, NULL, 8, 8, "RLC A"),
    INSTRUCTION_ADD(0x08, 2, cb_rrc_r8, REG_B, NULL, 8, 8, "RRC B"),
    INSTRUCTION_ADD(0x09, 2, cb_rrc_r8, REG_C, NULL, 8, 8, "RRC C"),
    INSTRUCTION_ADD(0x0a, 2, cb_rrc_r8, REG_D, NULL, 8, 8, "RRC D"),
    INSTRUCTION_ADD(0x0b, 2, cb_rrc_r8, REG_E, NULL, 8, 8, "RRC E"),
    INSTRUCTION_ADD(0x0c, 2, cb_rrc_r8, REG_H, NULL, 8, 8, "RRC H"),
    INSTRUCTION_ADD(0x0d, 2, cb_rrc_r8, REG_L, NULL, 8, 8, "RRC L"),
    INSTRUCTION_ADD(0x0e, 2, cb_rrc_m16, REG_HL, NULL, 16, 16, "RRC (HL)"),
    INSTRUCTION_ADD(0x0f, 2, cb_rrc_r8, REG_A, NULL, 8, 8, "RRC A"),

    /* 0x10 */
    INSTRUCTION_ADD(0x10, 2, cb_rl_r8, REG_B, NULL, 8, 8, "RL B"),
    INSTRUCTION_ADD(0x11, 2, cb_rl_r8, REG_C, NULL, 8, 8, "RL C"),
    INSTRUCTION_ADD(0x12, 2, cb_rl_r8, REG_D, NULL, 8, 8, "RL D"),
    INSTRUCTION_ADD(0x13, 2, cb_rl_r8, REG_E, NULL, 8, 8, "RL E"),
    INSTRUCTION_ADD(0x14, 2, cb_rl_r8, REG_H, NULL, 8, 8, "RL H"),
    INSTRUCTION_ADD(0x15, 2, cb_rl_r8, REG_L, NULL, 8, 8, "RL L"),
    INSTRUCTION_ADD(0x16, 2, cb_rl_m16, REG_HL, NULL, 16, 16, "RL (HL)"),
    INSTRUCTION_ADD(0x17, 2, cb_rl_r8, REG_A, NULL, 8, 8, "RL A"),
    INSTRUCTION_ADD(0x18, 2, cb_rr_r8, REG_B, NULL, 8, 8, "RR B"),
    INSTRUCTION_ADD(0x19, 2, cb_rr_r8, REG_C, NULL, 8, 8, "RR C"),
    INSTRUCTION_ADD(0x1a, 2, cb_rr_r8, REG_D, NULL, 8, 8, "RR D"),
    INSTRUCTION_ADD(0x1b, 2, cb_rr_r8, REG_E, NULL, 8, 8, "RR E"),
    INSTRUCTION_ADD(0x1c, 2, cb_rr_r8, REG_H, NULL, 8, 8, "RR H"),
    INSTRUCTION_ADD(0x1d, 2, cb_rr_r8, REG_L, NULL, 8, 8, "RR L"),
    INSTRUCTION_ADD(0x1e, 2, cb_rr_m16, REG_HL, NULL, 16, 16, "RR (HL)"),
    INSTRUCTION_ADD(0x1f, 2, cb_rr_r8, REG_A, NULL, 8, 8, "RR A"),

    /* 0x20 */
    INSTRUCTION_ADD(0x20, 2, cb_sla_r8, REG_B, NULL, 8, 8, "SLA B"),
    INSTRUCTION_ADD(0x21, 2, cb_sla_r8, REG_C, NULL, 8, 8, "SLA C"),
    INSTRUCTION_ADD(0x22, 2, cb_sla_r8, REG_D, NULL, 8, 8, "SLA D"),
    INSTRUCTION_ADD(0x23, 2, cb_sla_r8, REG_E, NULL, 8, 8, "SLA E"),
    INSTRUCTION_ADD(0x24, 2, cb_sla_r8, REG_H, NULL, 8, 8, "SLA H"),
    INSTRUCTION_ADD(0x25, 2, cb_sla_r8, REG_L, NULL, 8, 8, "SLA L"),
    INSTRUCTION_ADD(0x26, 2, cb_sla_m16, REG_HL, NULL, 16, 16, "SLA (HL)"),
    INSTRUCTION_ADD(0x27, 2, cb_sla_r8, REG_A, NULL, 8, 8, "SLA A"),
    INSTRUCTION_ADD(0x28, 2, cb_sra_r8, REG_B, NULL, 8, 8, "SRA B"),
    INSTRUCTION_ADD(0x29, 2, cb_sra_r8, REG_C, NULL, 8, 8, "SRA C"),
    INSTRUCTION_ADD(0x2a, 2, cb_sra_r8, REG_D, NULL, 8, 8, "SRA D"),
    INSTRUCTION_ADD(0x2b, 2, cb_sra_r8, REG_E, NULL, 8, 8, "SRA E"),
    INSTRUCTION_ADD(0x2c, 2, cb_sra_r8, REG_H, NULL, 8, 8, "SRA H"),
    INSTRUCTION_ADD(0x2d, 2, cb_sra_r8, REG_L, NULL, 8, 8, "SRA L"),
    INSTRUCTION_ADD(0x2e, 2, cb_sra_m16, REG_HL, NULL, 16, 16, "SRA (HL)"),
    INSTRUCTION_ADD(0x2f, 2, cb_sra_r8, REG_A, NULL, 8, 8, "SRA A"),

    /* 0x30 */
    INSTRUCTION_ADD(0x30, 2, cb_swap_r8, REG_B, NULL, 8, 8, "SWAP B"),
    INSTRUCTION_ADD(0x31, 2, cb_swap_r8, REG_C, NULL, 8, 8, "SWAP C"),
    INSTRUCTION_ADD(0x32, 2, cb_swap_r8, REG_D, NULL, 8, 8, "SWAP D"),
    INSTRUCTION_ADD(0x33, 2, cb_swap_r8, REG_E, NULL, 8, 8, "SWAP E"),
    INSTRUCTION_ADD(0x34, 2, cb_swap_r8, REG_H, NULL, 8, 8, "SWAP H"),
    INSTRUCTION_ADD(0x35, 2, cb_swap_r8, REG_L, NULL, 8, 8, "SWAP L"),
    INSTRUCTION_ADD(0x36, 2, cb_swap_m16, REG_HL, NULL, 16, 16, "SWAP (HL)"),
    INSTRUCTION_ADD(0x37, 2, cb_swap_r8, REG_A, NULL, 8, 8, "SWAP A"),
    INSTRUCTION_ADD(0x38, 2, cb_srl_r8, REG_B, NULL, 8, 8, "SRL B"),
    INSTRUCTION_ADD(0x39, 2, cb_srl_r8, REG_C, NULL, 8, 8, "SRL C"),
    INSTRUCTION_ADD(0x3a, 2, cb_srl_r8, REG_D, NULL, 8, 8, "SRL D"),
    INSTRUCTION_ADD(0x3b, 2, cb_srl_r8, REG_E, NULL, 8, 8, "SRL E"),
    INSTRUCTION_ADD(0x3c, 2, cb_srl_r8, REG_H, NULL, 8, 8, "SRL H"),
    INSTRUCTION_ADD(0x3d, 2, cb_srl_r8, REG_L, NULL, 8, 8, "SRL L"),
    INSTRUCTION_ADD(0x3e, 2, cb_srl_m16, REG_HL, NULL, 16, 16, "SRL (HL)"),
    INSTRUCTION_ADD(0x3f, 2, cb_srl_r8, REG_A, NULL, 8, 8, "SRL A"),

    /* 0x40 */
    INSTRUCTION_ADD(0x40, 2, cb_bit_r8, 0, REG_B, 8, 8, "BIT 0, B"),
    INSTRUCTION_ADD(0x41, 2, cb_bit_r8, 0, REG_C, 8, 8, "BIT 0, C"),
    INSTRUCTION_ADD(0x42, 2, cb_bit_r8, 0, REG_D, 8, 8, "BIT 0, D"),
    INSTRUCTION_ADD(0x43, 2, cb_bit_r8, 0, REG_E, 8, 8, "BIT 0, E"),
    INSTRUCTION_ADD(0x44, 2, cb_bit_r8, 0, REG_H, 8, 8, "BIT 0, H"),
    INSTRUCTION_ADD(0x45, 2, cb_bit_r8, 0, REG_L, 8, 8, "BIT 0, L"),
    INSTRUCTION_ADD(0x46, 2, cb_bit_m16, 0, REG_HL, 12, 12, "BIT 0, (HL)"),
    INSTRUCTION_ADD(0x47, 2, cb_bit_r8, 0, REG_A, 8, 8, "BIT 0, A"),
    INSTRUCTION_ADD(0x48, 2, cb_bit_r8, 1, REG_B, 8, 8, "BIT 1, B"),
    INSTRUCTION_ADD(0x49, 2, cb_bit_r8, 1, REG_C, 8, 8, "BIT 1, C"),
    INSTRUCTION_ADD(0x4a, 2, cb_bit_r8, 1, REG_D, 8, 8, "BIT 1, D"),
    INSTRUCTION_ADD(0x4b, 2, cb_bit_r8, 1, REG_E, 8, 8, "BIT 1, E"),
    INSTRUCTION_ADD(0x4c, 2, cb_bit_r8, 1, REG_H, 8, 8, "BIT 1, H"),
    INSTRUCTION_ADD(0x4d, 2, cb_bit_r8, 1, REG_L, 8, 8, "BIT 1, L"),
    INSTRUCTION_ADD(0x4e, 2, cb_bit_m16, 1, REG_HL, 12, 12, "BIT 1, (HL)"),
    INSTRUCTION_ADD(0x4f, 2, cb_bit_r8, 1, REG_A, 8, 8, "BIT 1, A"),

    /* 0x50 */
    INSTRUCTION_ADD(0x50, 2, cb_bit_r8, 2, REG_B, 8, 8, "BIT 2, B"),
    INSTRUCTION_ADD(0x51, 2, cb_bit_r8, 2, REG_C, 8, 8, "BIT 2, C"),
    INSTRUCTION_ADD(0x52, 2, cb_bit_r8, 2, REG_D, 8, 8, "BIT 2, D"),
    INSTRUCTION_ADD(0x53, 2, cb_bit_r8, 2, REG_E, 8, 8, "BIT 2, E"),
    INSTRUCTION_ADD(0x54, 2, cb_bit_r8, 2, REG_H, 8, 8, "BIT 2, H"),
    INSTRUCTION_ADD(0x55, 2, cb_bit_r8, 2, REG_L, 8, 8, "BIT 2, L"),
    INSTRUCTION_ADD(0x56, 2, cb_bit_m16, 2, REG_HL, 12, 12, "BIT 2, (HL)"),
    INSTRUCTION_ADD(0x57, 2, cb_bit_r8, 2, REG_A, 8, 8, "BIT 2, A"),
    INSTRUCTION_ADD(0x58, 2, cb_bit_r8, 3, REG_B, 8, 8, "BIT 3, B"),
    INSTRUCTION_ADD(0x59, 2, cb_bit_r8, 3, REG_C, 8, 8, "BIT 3, C"),
    INSTRUCTION_ADD(0x5a, 2, cb_bit_r8, 3, REG_D, 8, 8, "BIT 3, D"),
    INSTRUCTION_ADD(0x5b, 2, cb_bit_r8, 3, REG_E, 8, 8, "BIT 3, E"),
    INSTRUCTION_ADD(0x5c, 2, cb_bit_r8, 3, REG_H, 8, 8, "BIT 3, H"),
    INSTRUCTION_ADD(0x5d, 2, cb_bit_r8, 3, REG_L, 8, 8, "BIT 3, L"),
    INSTRUCTION_ADD(0x5e, 2, cb_bit_m16, 3, REG_HL, 12, 12, "BIT 3, (HL)"),
    INSTRUCTION_ADD(0x5f, 2, cb_bit_r8, 3, REG_A, 8, 8, "BIT 3, A"),

    /* 0x60 */
    INSTRUCTION_ADD(0x60, 2, cb_bit_r8, 4, REG_B, 8, 8, "BIT 4, B"),
    INSTRUCTION_ADD(0x61, 2, cb_bit_r8, 4, REG_C, 8, 8, "BIT 4, C"),
    INSTRUCTION_ADD(0x62, 2, cb_bit_r8, 4, REG_D, 8, 8, "BIT 4, D"),
    INSTRUCTION_ADD(0x63, 2, cb_bit_r8, 4, REG_E, 8, 8, "BIT 4, E"),
    INSTRUCTION_ADD(0x64, 2, cb_bit_r8, 4, REG_H, 8, 8, "BIT 4, H"),
    INSTRUCTION_ADD(0x65, 2, cb_bit_r8, 4, REG_L, 8, 8, "BIT 4, L"),
    INSTRUCTION_ADD(0x66, 2, cb_bit_m16, 4, REG_HL, 12, 12, "BIT 4, (HL)"),
    INSTRUCTION_ADD(0x67, 2, cb_bit_r8, 4, REG_A, 8, 8, "BIT 4, A"),
    INSTRUCTION_ADD(0x68, 2, cb_bit_r8, 5, REG_B, 8, 8, "BIT 5, B"),
    INSTRUCTION_ADD(0x69, 2, cb_bit_r8, 5, REG_C, 8, 8, "BIT 5, C"),
    INSTRUCTION_ADD(0x6a, 2, cb_bit_r8, 5, REG_D, 8, 8, "BIT 5, D"),
    INSTRUCTION_ADD(0x6b, 2, cb_bit_r8, 5, REG_E, 8, 8, "BIT 5, E"),
    INSTRUCTION_ADD(0x6c, 2, cb_bit_r8, 5, REG_H, 8, 8, "BIT 5, H"),
    INSTRUCTION_ADD(0x6d, 2, cb_bit_r8, 5, REG_L, 8, 8, "BIT 5, L"),
    INSTRUCTION_ADD(0x6e, 2, cb_bit_m16, 5, REG_HL, 12, 12, "BIT 5, (HL)"),
    INSTRUCTION_ADD(0x6f, 2, cb_bit_r8, 5, REG_A, 8, 8, "BIT 5, A"),

    /* 0x70 */
    INSTRUCTION_ADD(0x70, 2, cb_bit_r8, 6, REG_B, 8, 8, "BIT 6, B"),
    INSTRUCTION_ADD(0x71, 2, cb_bit_r8, 6, REG_C, 8, 8, "BIT 6, C"),
    INSTRUCTION_ADD(0x72, 2, cb_bit_r8, 6, REG_D, 8, 8, "BIT 6, D"),
    INSTRUCTION_ADD(0x73, 2, cb_bit_r8, 6, REG_E, 8, 8, "BIT 6, E"),
    INSTRUCTION_ADD(0x74, 2, cb_bit_r8, 6, REG_H, 8, 8, "BIT 6, H"),
    INSTRUCTION_ADD(0x75, 2, cb_bit_r8, 6, REG_L, 8, 8, "BIT 6, L"),
    INSTRUCTION_ADD(0x76, 2, cb_bit_m16, 6, REG_HL, 12, 12, "BIT 6, (HL)"),
    INSTRUCTION_ADD(0x77, 2, cb_bit_r8, 6, REG_A, 8, 8, "BIT 6, A"),
    INSTRUCTION_ADD(0x78, 2, cb_bit_r8, 7, REG_B, 8, 8, "BIT 7, B"),
    INSTRUCTION_ADD(0x79, 2, cb_bit_r8, 7, REG_C, 8, 8, "BIT 7, C"),
    INSTRUCTION_ADD(0x7a, 2, cb_bit_r8, 7, REG_D, 8, 8, "BIT 7, D"),
    INSTRUCTION_ADD(0x7b, 2, cb_bit_r8, 7, REG_E, 8, 8, "BIT 7, E"),
    INSTRUCTION_ADD(0x7c, 2, cb_bit_r8, 7, REG_H, 8, 8, "BIT 7, H"),
    INSTRUCTION_ADD(0x7d, 2, cb_bit_r8, 7, REG_L, 8, 8, "BIT 7, L"),
    INSTRUCTION_ADD(0x7e, 2, cb_bit_m16, 7, REG_HL, 12, 12, "BIT 7, (HL)"),
    INSTRUCTION_ADD(0x7f, 2, cb_bit_r8, 7, REG_A, 8, 8, "BIT 7, A"),

    /* 0x80 */
    INSTRUCTION_ADD(0x80, 2, cb_res_r8, 0, REG_B, 8, 8, "RES 0, B"),
    INSTRUCTION_ADD(0x81, 2, cb_res_r8, 0, REG_C, 8, 8, "RES 0, C"),
    INSTRUCTION_ADD(0x82, 2, cb_res_r8, 0, REG_D, 8, 8, "RES 0, D"),
    INSTRUCTION_ADD(0x83, 2, cb_res_r8, 0, REG_E, 8, 8, "RES 0, E"),
    INSTRUCTION_ADD(0x84, 2, cb_res_r8, 0, REG_H, 8, 8, "RES 0, H"),
    INSTRUCTION_ADD(0x85, 2, cb_res_r8, 0, REG_L, 8, 8, "RES 0, L"),
    INSTRUCTION_ADD(0x86, 2, cb_res_m16, 0, REG_HL, 16, 16, "RES 0, (HL)"),
    INSTRUCTION_ADD(0x87, 2, cb_res_r8, 0, REG_A, 8, 8, "RES 0, A"),
    INSTRUCTION_ADD(0x88, 2, cb_res_r8, 1, REG_B, 8, 8, "RES 1, B"),
    INSTRUCTION_ADD(0x89, 2, cb_res_r8, 1, REG_C, 8, 8, "RES 1, C"),
    INSTRUCTION_ADD(0x8a, 2, cb_res_r8, 1, REG_D, 8, 8, "RES 1, D"),
    INSTRUCTION_ADD(0x8b, 2, cb_res_r8, 1, REG_E, 8, 8, "RES 1, E"),
    INSTRUCTION_ADD(0x8c, 2, cb_res_r8, 1, REG_H, 8, 8, "RES 1, H"),
    INSTRUCTION_ADD(0x8d, 2, cb_res_r8, 1, REG_L, 8, 8, "RES 1, L"),
    INSTRUCTION_ADD(0x8e, 2, cb_res_m16, 1, REG_HL, 16, 16, "RES 1, (HL)"),
    INSTRUCTION_ADD(0x8f, 2, cb_res_r8, 1, REG_A, 8, 8, "RES 1, A"),

    /* 0x90 */
    INSTRUCTION_ADD(0x90, 2, cb_res_r8, 2, REG_B, 8, 8, "RES 2, B"),
    INSTRUCTION_ADD(0x91, 2, cb_res_r8, 2, REG_C, 8, 8, "RES 2, C"),
    INSTRUCTION_ADD(0x92, 2, cb_res_r8, 2, REG_D, 8, 8, "RES 2, D"),
    INSTRUCTION_ADD(0x93, 2, cb_res_r8, 2, REG_E, 8, 8, "RES 2, E"),
    INSTRUCTION_ADD(0x94, 2, cb_res_r8, 2, REG_H, 8, 8, "RES 2, H"),
    INSTRUCTION_ADD(0x95, 2, cb_res_r8, 2, REG_L, 8, 8, "RES 2, L"),
    INSTRUCTION_ADD(0x96, 2, cb_res_m16, 2, REG_HL, 16, 16, "RES 2, (HL)"),
    INSTRUCTION_ADD(0x97, 2, cb_res_r8, 2, REG_A, 8, 8, "RES 2, A"),
    INSTRUCTION_ADD(0x98, 2, cb_res_r8, 3, REG_B, 8, 8, "RES 3, B"),
    INSTRUCTION_ADD(0x99, 2, cb_res_r8, 3, REG_C, 8, 8, "RES 3, C"),
    INSTRUCTION_ADD(0x9a, 2, cb_res_r8, 3, REG_D, 8, 8, "RES 3, D"),
    INSTRUCTION_ADD(0x9b, 2, cb_res_r8, 3, REG_E, 8, 8, "RES 3, E"),
    INSTRUCTION_ADD(0x9c, 2, cb_res_r8, 3, REG_H, 8, 8, "RES 3, H"),
    INSTRUCTION_ADD(0x9d, 2, cb_res_r8, 3, REG_L, 8, 8, "RES 3, L"),
    INSTRUCTION_ADD(0x9e, 2, cb_res_m16, 3, REG_HL, 16, 16, "RES 3, (HL)"),
    INSTRUCTION_ADD(0x9f, 2, cb_res_r8, 3, REG_A, 8, 8, "RES 3, A"),

    /* 0xa0 */
    INSTRUCTION_ADD(0xa0, 2, cb_res_r8, 4, REG_B, 8, 8, "RES 4, B"),
    INSTRUCTION_ADD(0xa1, 2, cb_res_r8, 4, REG_C, 8, 8, "RES 4, C"),
    INSTRUCTION_ADD(0xa2, 2, cb_res_r8, 4, REG_D, 8, 8, "RES 4, D"),
    INSTRUCTION_ADD(0xa3, 2, cb_res_r8, 4, REG_E, 8, 8, "RES 4, E"),
    INSTRUCTION_ADD(0xa4, 2, cb_res_r8, 4, REG_H, 8, 8, "RES 4, H"),
    INSTRUCTION_ADD(0xa5, 2, cb_res_r8, 4, REG_L, 8, 8, "RES 4, L"),
    INSTRUCTION_ADD(0xa6, 2, cb_res_m16, 4, REG_HL, 16, 16, "RES 4, (HL)"),
    INSTRUCTION_ADD(0xa7, 2, cb_res_r8, 4, REG_A, 8, 8, "RES 4, A"),
    INSTRUCTION_ADD(0xa8, 2, cb_res_r8, 5, REG_B, 8, 8, "RES 5, B"),
    INSTRUCTION_ADD(0xa9, 2, cb_res_r8, 5, REG_C, 8, 8, "RES 5, C"),
    INSTRUCTION_ADD(0xaa, 2, cb_res_r8, 5, REG_D, 8, 8, "RES 5, D"),
    INSTRUCTION_ADD(0xab, 2, cb_res_r8, 5, REG_E, 8, 8, "RES 5, E"),
    INSTRUCTION_ADD(0xac, 2, cb_res_r8, 5, REG_H, 8, 8, "RES 5, H"),
    INSTRUCTION_ADD(0xad, 2, cb_res_r8, 5, REG_L, 8, 8, "RES 5, L"),
    INSTRUCTION_ADD(0xae, 2, cb_res_m16, 5, REG_HL, 16, 16, "RES 5, (HL)"),
    INSTRUCTION_ADD(0xaf, 2, cb_res_r8, 5, REG_A, 8, 8, "RES 5, A"),

    /* 0xb0 */
    INSTRUCTION_ADD(0xb0, 2, cb_res_r8, 6, REG_B, 8, 8, "RES 6, B"),
    INSTRUCTION_ADD(0xb1, 2, cb_res_r8, 6, REG_C, 8, 8, "RES 6, C"),
    INSTRUCTION_ADD(0xb2, 2, cb_res_r8, 6, REG_D, 8, 8, "RES 6, D"),
    INSTRUCTION_ADD(0xb3, 2, cb_res_r8, 6, REG_E, 8, 8, "RES 6, E"),
    INSTRUCTION_ADD(0xb4, 2, cb_res_r8, 6, REG_H, 8, 8, "RES 6, H"),
    INSTRUCTION_ADD(0xb5, 2, cb_res_r8, 6, REG_L, 8, 8, "RES 6, L"),
    INSTRUCTION_ADD(0xb6, 2, cb_res_m16, 6, REG_HL, 16, 16, "RES 6, (HL)"),
    INSTRUCTION_ADD(0xb7, 2, cb_res_r8, 6, REG_A, 8, 8, "RES 6, A"),
    INSTRUCTION_ADD(0xb8, 2, cb_res_r8, 7, REG_B, 8, 8, "RES 7, B"),
    INSTRUCTION_ADD(0xb9, 2, cb_res_r8, 7, REG_C, 8, 8, "RES 7, C"),
    INSTRUCTION_ADD(0xba, 2, cb_res_r8, 7, REG_D, 8, 8, "RES 7, D"),
    INSTRUCTION_ADD(0xbb, 2, cb_res_r8, 7, REG_E, 8, 8, "RES 7, E"),
    INSTRUCTION_ADD(0xbc, 2, cb_res_r8, 7, REG_H, 8, 8, "RES 7, H"),
    INSTRUCTION_ADD(0xbd, 2, cb_res_r8, 7, REG_L, 8, 8, "RES 7, L"),
    INSTRUCTION_ADD(0xbe, 2, cb_res_m16, 7, REG_HL, 16, 16, "RES 7, (HL)"),
    INSTRUCTION_ADD(0xbf, 2, cb_res_r8, 7, REG_A, 8, 8, "RES 7, A"),

    /* 0xc0 */
    INSTRUCTION_ADD(0xc0, 2, cb_set_r8, 0, REG_B, 8, 8, "SET 0, B"),
    INSTRUCTION_ADD(0xc1, 2, cb_set_r8, 0, REG_C, 8, 8, "SET 0, C"),
    INSTRUCTION_ADD(0xc2, 2, cb_set_r8, 0, REG_D, 8, 8, "SET 0, D"),
    INSTRUCTION_ADD(0xc3, 2, cb_set_r8, 0, REG_E, 8, 8, "SET 0, E"),
    INSTRUCTION_ADD(0xc4, 2, cb_set_r8, 0, REG_H, 8, 8, "SET 0, H"),
    INSTRUCTION_ADD(0xc5, 2, cb_set_r8, 0, REG_L, 8, 8, "SET 0, L"),
    INSTRUCTION_ADD(0xc6, 2, cb_set_m16, 0, REG_HL, 16, 16, "SET 0, (HL)"),
    INSTRUCTION_ADD(0xc7, 2, cb_set_r8, 0, REG_A, 8, 8, "SET 0, A"),
    INSTRUCTION_ADD(0xc8, 2, cb_set_r8, 1, REG_B, 8, 8, "SET 1, B"),
    INSTRUCTION_ADD(0xc9, 2, cb_set_r8, 1, REG_C, 8, 8, "SET 1, C"),
    INSTRUCTION_ADD(0xca, 2, cb_set_r8, 1, REG_D, 8, 8, "SET 1, D"),
    INSTRUCTION_ADD(0xcb, 2, cb_set_r8, 1, REG_E, 8, 8, "SET 1, E"),
    INSTRUCTION_ADD(0xcc, 2, cb_set_r8, 1, REG_H, 8, 8, "SET 1, H"),
    INSTRUCTION_ADD(0xcd, 2, cb_set_r8, 1, REG_L, 8, 8, "SET 1, L"),
    INSTRUCTION_ADD(0xce, 2, cb_set_m16, 1, REG_HL, 16, 16, "SET 1, (HL)"),
    INSTRUCTION_ADD(0xcf, 2, cb_set_r8, 1, REG_A, 8, 8, "SET 1, A"),

    /* 0xd0 */
    INSTRUCTION_ADD(0xd0, 2, cb_set_r8, 2, REG_B, 8, 8, "SET 2, B"),
    INSTRUCTION_ADD(0xd1, 2, cb_set_r8, 2, REG_C, 8, 8, "SET 2, C"),
    INSTRUCTION_ADD(0xd2, 2, cb_set_r8, 2, REG_D, 8, 8, "SET 2, D"),
    INSTRUCTION_ADD(0xd3, 2, cb_set_r8, 2, REG_E, 8, 8, "SET 2, E"),
    INSTRUCTION_ADD(0xd4, 2, cb_set_r8, 2, REG_H, 8, 8, "SET 2, H"),
    INSTRUCTION_ADD(0xd5, 2, cb_set_r8, 2, REG_L, 8, 8, "SET 2, L"),
    INSTRUCTION_ADD(0xd6, 2, cb_set_m16, 2, REG_HL, 16, 16, "SET 2, (HL)"),
    INSTRUCTION_ADD(0xd7, 2, cb_set_r8, 2, REG_A, 8, 8, "SET 2, A"),
    INSTRUCTION_ADD(0xd8, 2, cb_set_r8, 3, REG_B, 8, 8, "SET 3, B"),
    INSTRUCTION_ADD(0xd9, 2, cb_set_r8, 3, REG_C, 8, 8, "SET 3, C"),
    INSTRUCTION_ADD(0xda, 2, cb_set_r8, 3, REG_D, 8, 8, "SET 3, D"),
    INSTRUCTION_ADD(0xdb, 2, cb_set_r8, 3, REG_E, 8, 8, "SET 3, E"),
    INSTRUCTION_ADD(0xdc, 2, cb_set_r8, 3, REG_H, 8, 8, "SET 3, H"),
    INSTRUCTION_ADD(0xdd, 2, cb_set_r8, 3, REG_L, 8, 8, "SET 3, L"),
    INSTRUCTION_ADD(0xde, 2, cb_set_m16, 3, REG_HL, 16, 16, "SET 3, (HL)"),
    INSTRUCTION_ADD(0xdf, 2, cb_set_r8, 3, REG_A, 8, 8, "SET 3, A"),

    /* 0xe0 */
    INSTRUCTION_ADD(0xe0, 2, cb_set_r8, 4, REG_B, 8, 8, "SET 4, B"),
    INSTRUCTION_ADD(0xe1, 2, cb_set_r8, 4, REG_C, 8, 8, "SET 4, C"),
    INSTRUCTION_ADD(0xe2, 2, cb_set_r8, 4, REG_D, 8, 8, "SET 4, D"),
    INSTRUCTION_ADD(0xe3, 2, cb_set_r8, 4, REG_E, 8, 8, "SET 4, E"),
    INSTRUCTION_ADD(0xe4, 2, cb_set_r8, 4, REG_H, 8, 8, "SET 4, H"),
    INSTRUCTION_ADD(0xe5, 2, cb_set_r8, 4, REG_L, 8, 8, "SET 4, L"),
    INSTRUCTION_ADD(0xe6, 2, cb_set_m16, 4, REG_HL, 16, 16, "SET 4, (HL)"),
    INSTRUCTION_ADD(0xe7, 2, cb_set_r8, 4, REG_A, 8, 8, "SET 4, A"),
    INSTRUCTION_ADD(0xe8, 2, cb_set_r8, 5, REG_B, 8, 8, "SET 5, B"),
    INSTRUCTION_ADD(0xe9, 2, cb_set_r8, 5, REG_C, 8, 8, "SET 5, C"),
    INSTRUCTION_ADD(0xea, 2, cb_set_r8, 5, REG_D, 8, 8, "SET 5, D"),
    INSTRUCTION_ADD(0xeb, 2, cb_set_r8, 5, REG_E, 8, 8, "SET 5, E"),
    INSTRUCTION_ADD(0xec, 2, cb_set_r8, 5, REG_H, 8, 8, "SET 5, H"),
    INSTRUCTION_ADD(0xed, 2, cb_set_r8, 5, REG_L, 8, 8, "SET 5, L"),
    INSTRUCTION_ADD(0xee, 2, cb_set_m16, 5, REG_HL, 16, 16, "SET 5, (HL)"),
    INSTRUCTION_ADD(0xef, 2, cb_set_r8, 5, REG_A, 8, 8, "SET 5, A"),

    /* 0xf0 */
    INSTRUCTION_ADD(0xf0, 2, cb_set_r8, 6, REG_B, 8, 8, "SET 6, B"),
    INSTRUCTION_ADD(0xf1, 2, cb_set_r8, 6, REG_C, 8, 8, "SET 6, C"),
    INSTRUCTION_ADD(0xf2, 2, cb_set_r8, 6, REG_D, 8, 8, "SET 6, D"),
    INSTRUCTION_ADD(0xf3, 2, cb_set_r8, 6, REG_E, 8, 8, "SET 6, E"),
    INSTRUCTION_ADD(0xf4, 2, cb_set_r8, 6, REG_H, 8, 8, "SET 6, H"),
    INSTRUCTION_ADD(0xf5, 2, cb_set_r8, 6, REG_L, 8, 8, "SET 6, L"),
    INSTRUCTION_ADD(0xf6, 2, cb_set_m16, 6, REG_HL, 16, 16, "SET 6, (HL)"),
    INSTRUCTION_ADD(0xf7, 2, cb_set_r8, 6, REG_A, 8, 8, "SET 6, A"),
    INSTRUCTION_ADD(0xf8, 2, cb_set_r8, 7, REG_B, 8, 8, "SET 7, B"),
    INSTRUCTION_ADD(0xf9, 2, cb_set_r8, 7, REG_C, 8, 8, "SET 7, C"),
    INSTRUCTION_ADD(0xfa, 2, cb_set_r8, 7, REG_D, 8, 8, "SET 7, D"),
    INSTRUCTION_ADD(0xfb, 2, cb_set_r8, 7, REG_E, 8, 8, "SET 7, E"),
    INSTRUCTION_ADD(0xfc, 2, cb_set_r8, 7, REG_H, 8, 8, "SET 7, H"),
    INSTRUCTION_ADD(0xfd, 2, cb_set_r8, 7, REG_L, 8, 8, "SET 7, L"),
    INSTRUCTION_ADD(0xfe, 2, cb_set_m16, 7, REG_HL, 16, 16, "SET 7, (HL)"),
    INSTRUCTION_ADD(0xff, 2, cb_set_r8, 7, REG_A, 8, 8, "SET 7, A"),
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
}

instruction_t*
decode(uint8_t *data)
{
    uint8_t opcode = data[0];
    int size = 0;
    instruction_t *inst_set = instruction_set;

    if (opcode == PREFIX_CB) {
        inst_set = prefixed_instruction_set;
        size = -1;
        opcode = READ_I8(data[1]);
    }

    instruction_t *inst = inst_set + opcode;

    inst->r_cycles = inst->cycles;
    size += inst->size;

    if (size != 1) {
        if (size == 2) {
            inst->opcode_ext.i8 = READ_I8(*(data + 1));
        } else if (inst->size == 3) {
            /* immediate value is little-endian */
            inst->opcode_ext.i16 = READ_I16(*(uint16_t*)(data + 1));
        } else {
            LOG_ERROR("Invalid instruction, imme size [%d]", inst->size);
            abort();
        }
    }

    return inst;
}

instruction_t*
decode_mem(memory_read read, uint16_t addr, void *udata)
{
    uint8_t opcode = read(udata, addr);
    int size = 0;
    instruction_t *inst_set = instruction_set;

    if (opcode == PREFIX_CB) {
        inst_set = prefixed_instruction_set;
        size = -1;
        opcode = READ_I8(read(udata, addr + 1));
    }

    instruction_t *inst = inst_set + opcode;

    inst->r_cycles = inst->cycles;
    size += inst->size;

    if (size != 1) {
        if (size == 2) {
            inst->opcode_ext.i8 = READ_I8(read(udata, addr + 1));
        } else if (inst->size == 3) {
            /* immediate value is little-endian */
            uint8_t data[2];
            data[0] = read(udata, addr + 1);
            data[1] = read(udata, addr + 2);
            inst->opcode_ext.i16 = READ_I16(*(uint16_t*)data);
        } else {
            LOG_ERROR("Invalid instruction, imme size [%d]", inst->size);
            abort();
        }
    }

    return inst;
}

#ifdef DEBUG
#include "test_instruction.c"
#endif
