#include "instruction_set.h"
#include "common.h"
#include "cpu.h"

void 
stop(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("STOP: %s\n", ins->name);
}

void 
inc_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("INC r8: %s\n", ins->name);

    size_t reg_offset = (size_t)ins->op1;
    cpu_register_t *regs = &(cpu->regs);    
    uint16_t v = READ_R8(regs, reg_offset);
    uint8_t hc = HALF_CARRY(v, 1);
    v++;
    v &= UINT8_MASK;
    WRITE_R8(regs, reg_offset, (uint8_t)v);

    SET_R_FLAG_VALUE(regs, FLAG_Z, v == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
}

void 
inc_r16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("INC r16: %s\n", ins->name);

    size_t reg_offset = (size_t)ins->op1;
    cpu_register_t *regs = &(cpu->regs);
    uint16_t v = READ_R16(regs, reg_offset);
    v++;
    WRITE_R16(regs, reg_offset, v);
}

void 
inc_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("INC m16: %s\n", ins->name);
    
    size_t reg_offset = (size_t)ins->op1;
    cpu_register_t *regs = &(cpu->regs);
    uint16_t addr = READ_R16(regs, reg_offset);        
    uint16_t v = cpu->mem_read(addr);    
    uint8_t hc = HALF_CARRY(v, 1);

    v++;
    v &= UINT8_MASK;
    cpu->mem_write(addr, (uint8_t)v);

    SET_R_FLAG_VALUE(regs, FLAG_Z, v == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
}

void 
dec_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("DEC r8: %s\n", ins->name);

    size_t reg_offset = (size_t)ins->op1;
    cpu_register_t *regs = &(cpu->regs);    
    uint16_t v = READ_R8(regs, reg_offset);
    printf("%x %x %x\n", v, -1, HALF_CARRY(v, -1));
    uint8_t hc = HALF_CARRY(v, -1);

    v--;
    v &= UINT8_MASK;
    WRITE_R8(regs, reg_offset, (uint8_t)v);

    SET_R_FLAG_VALUE(regs, FLAG_Z, v == 0);
    SET_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
}

void
dec_r16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("DEC r16: %s\n", ins->name);    

    size_t reg_offset = (size_t)ins->op1;
    cpu_register_t *regs = &(cpu->regs);
    uint16_t v = READ_R16(regs, reg_offset);
    v--;
    WRITE_R16(regs, reg_offset, v);  
}

void 
dec_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("DEC m16: %s\n", ins->name);

    size_t reg_offset = (size_t)ins->op1;
    cpu_register_t *regs = &(cpu->regs);
    uint16_t addr = READ_R16(regs, reg_offset);        
    uint16_t v = cpu->mem_read(addr);    
    uint8_t hc = HALF_CARRY(v, -1);

    v--;
    v &= UINT8_MASK;
    cpu->mem_write(addr, (uint8_t)v);

    SET_R_FLAG_VALUE(regs, FLAG_Z, v == 0);
    SET_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
}

void
rlca(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("RLCA: %s\n", ins->name);
    
    cpu_register_t *regs = &(cpu->regs);
    uint16_t v = READ_R8(regs, REG_A);
    uint16_t carry = v >> 7;
    v = (v << 1) | carry;
    WRITE_R8(regs, REG_A, (uint8_t)v);

    CLEAR_R_FLAG(regs, FLAG_Z);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
}

void
rla(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("RLA: %s\n", ins->name);
}

void 
rrca(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("RRCA: %s\n", ins->name);
}

void 
rra(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("RRA: %s\n", ins->name);
}

void
daa(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("DAA: %s\n", ins->name);
}

void
scf(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("SCF: %s\n", ins->name);
}

void
jr_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("JR: %s\n", ins->name);
}

void
jr_nz_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("JR NZ: %s\n", ins->name);
}

void 
jr_nc_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("JR NC: %s\n", ins->name);
}

void 
jr_z_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("JR NZ: %s\n", ins->name);
}

void 
jr_c_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("JR C: %s\n", ins->name);    
}

void 
nop(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("NOP: %s\n", ins->name);
}

void
ld_r16_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("LD r16, i16: %s\n", ins->name);
}

void
ld_sp_hl(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("LD SP, HL: %s\n", ins->name);
}

void
ld_hl_sp_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("LD HL, SP + i8: %s\n", ins->name);
}

void
ld_m8_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("LD m8, r8: %s\n", ins->name);
}

void
ld_r8_m8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("LD r8, m8: %s\n", ins->name);
}

void
ld_r8_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("LD r8, i8: %s\n", ins->name);
}

void
ld_r8_im16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("LD r8, im16: %s\n", ins->name);
}

void
ldi_r8_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("LDI r8, m16: %s\n", ins->name);
}

void
ldi_m16_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("LDI m16, r8: %s\n", ins->name);
}

void
ldd_r8_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("LDD r8, m16: %s\n", ins->name);
}

void
ldd_m16_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("LDD m16, r8: %s\n", ins->name);
}

void
ld_m16_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("LD m16, 8: %s\n", ins->name);
}

void
ld_r8_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("LD r8, m16: %s\n", ins->name);
}

void 
ld_m16_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("LD m16, r8: %s\n", ins->name);
}

void 
ld_im16_r16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("LD im16, r16: %s\n", ins->name);
}

void 
ld_im16_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("LD im16, r8: %s\n", ins->name);
}

void
ld_r8_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("LD r8, r8: %s\n", ins->name);
}

void 
add_r16_r16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("ADD r16, r16: %s\n", ins->name);
}

void 
add_r16_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("ADD r16, i8: %s\n", ins->name);
}

void 
add_r8_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("ADD r8, r8: %s\n", ins->name);
}

void 
add_r8_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("ADD r8, i8: %s\n", ins->name);
}

void 
add_r8_m8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("ADD r8, m8: %s\n", ins->name);
}

void 
add_r8_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("ADD r8, m16: %s\n", ins->name);
}

void 
adc_r8_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("ADC r8, r8: %s\n", ins->name);
}

void 
adc_r8_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("ADC r8, i8: %s\n", ins->name);    
}

void 
adc_r8_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("ADC r8, m16: %s\n", ins->name);
}

void 
sub_r8_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("SUB r8, r8: %s\n", ins->name);
}

void 
sub_r8_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("SUB r8, i8: %s\n", ins->name);
}

void
sub_r8_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("SUB r8, m16: %s\n", ins->name);
}

void
subc_r8_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("SUBC r8, r8: %s\n", ins->name);
}

void
subc_r8_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("SUB r8, i8: %s\n", ins->name);
}

void 
subc_r8_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("SUB r8, m16: %s\n", ins->name);
}

void 
and_r8_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("AND r8, r8: %s\n", ins->name);
}

void
and_r8_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("AND r8, m16: %s\n", ins->name);
}

void
and_r8_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("AND r8, i8: %s\n", ins->name);
}

void 
or_r8_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("OR r8, r8: %s\n", ins->name);
}

void 
or_r8_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("OR r8, i8: %s\n", ins->name);
}

void
or_r8_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("OR r8, m16: %s\n", ins->name);
}

void
xor_r8_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("XOR r8, r8: %s\n", ins->name);
}

void
xor_r8_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("XOR r8, i8: %s\n", ins->name);
}

void    
xor_r8_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("XOR r8, m16: %s\n", ins->name);
}

void
cp_r8_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("CP r8, r8: %s\n", ins->name);
}

void 
cp_r8_i8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("CP r8, i8: %s\n", ins->name);
}

void 
cp_r8_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("CP r8, m16: %s\n", ins->name);
}

void
ret_nz(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("RET NZ: %s\n", ins->name);
}

void
ret_nc(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("RET NC: %s\n", ins->name);
}

void 
ret_z(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("RET Z: %s\n", ins->name);
}

void
ret_c(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("RET C: %s\n", ins->name);
}

void
ret(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("RET: %s\n", ins->name);
}

void
reti(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("RETI: %s\n", ins->name);
}

void
rst(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("RST: %s\n", ins->name);
}

void
jp_nz_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("JP NZ: %s\n", ins->name);
}

void
jp_nc_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("JP NC: %s\n", ins->name);
}

void
jp_c_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("JP: %s\n", ins->name);
}

void 
jp_z_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("JP Z: %s\n", ins->name);
}

void
jp_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("JP I16: %s\n", ins->name);
}

void
jp_r16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("JP R16: %s\n", ins->name);
}

void
pop_r16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("POP r16: %s\n", ins->name);
}

void
push_r16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("PUSH r16: %s\n", ins->name);
}

void
ldh_im8_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("LDH m8, r8: %s\n", ins->name);
}

void
ldh_r8_m8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("LDH r8, m8: %s\n", ins->name);
}

void
ldh_r8_im8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("LDH r8, m8: %s\n", ins->name);
}

void
call_nz_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("CALL NZ: %s\n", ins->name);
}

void
call_nc_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("CALL NC: %s\n", ins->name);
}

void
call_z_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("CALL Z: %s\n", ins->name);
}

void
call_c_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("CALL C: %s\n", ins->name);
}

void
call_i16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("CALL: %s\n", ins->name);
}

void 
cpl(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("CPL: %s\n", ins->name);
}

void 
ccf(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("CCF: %s\n", ins->name);
}

void 
di(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("DI: %s\n", ins->name);
}

void
ei(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("EI: %s\n", ins->name);
}

void 
halt(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("HALT: %s\n", ins->name);
}

void
cb_rlc_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("RLC: %s\n", ins->name);
}

void
cb_rlc_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("RLC: %s\n", ins->name);
}

void
cb_rrc_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("RRC: %s\n", ins->name);
}

void
cb_rrc_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("RRC: %s\n", ins->name);
}

void
cb_rl_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("RL: %s\n", ins->name);
}

void
cb_rl_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("RL: %s\n", ins->name);
}

void
cb_rr_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("RR: %s\n", ins->name);
}

void
cb_rr_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("RR: %s\n", ins->name);
}

void
cb_sla_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("SLA: %s\n", ins->name);
}

void
cb_sla_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("SLA: %s\n", ins->name);
}

void
cb_sra_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("SRA: %s\n", ins->name);
}

void
cb_sra_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("SRA: %s\n", ins->name);
}

void
cb_swap_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("SWAP: %s\n", ins->name);
}

void
cb_swap_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("SWAP: %s\n", ins->name);
}

void
cb_srl_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("SRL: %s\n", ins->name);
}

void
cb_srl_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("SRL: %s\n", ins->name);
}

void
cb_bit_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("BIT: %s\n", ins->name);
}

void
cb_bit_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("BIT: %s\n", ins->name);
}

void
cb_res_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("RES: %s\n", ins->name);
}

void
cb_res_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("RES: %s\n", ins->name);
}

void
cb_set_r8(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("SET: %s\n", ins->name);
}

void
cb_set_m16(gbc_cpu_t *cpu, instruction_t *ins)
{
    LOG_INFO("SET: %s\n", ins->name);
}


static instruction_t instruction_set[INSTRUCTIONS_SET_SIZE] = {
    /* 0x00 */
    INSTRUCTION_ADD(0x00, 1, nop, NULL, NULL, "NOP"),
    INSTRUCTION_ADD(0x01, 3, ld_r16_i16, REG_BC, NULL, "LD BC, n16"),    
    INSTRUCTION_ADD(0x02, 1, ld_m16_r8, REG_BC, REG_A, "LD (BC), A"),
    INSTRUCTION_ADD(0x03, 1, inc_r16, REG_BC, NULL, "INC BC"),
    INSTRUCTION_ADD(0x04, 1, inc_r8, REG_B, NULL, "INC B"),
    INSTRUCTION_ADD(0x05, 1, dec_r8, REG_B, NULL, "DEC B"),
    INSTRUCTION_ADD(0x06, 2, ld_r8_i8, REG_B, NULL, "LD B, n8"),    
    INSTRUCTION_ADD(0x07, 1, rlca, NULL, NULL, "RLCA"),
    INSTRUCTION_ADD(0x08, 3, ld_im16_r16, NULL, REG_SP, "LD (n16), SP"),
    INSTRUCTION_ADD(0x09, 1, add_r16_r16, REG_HL, REG_BC, "ADD HL, BC"),
    INSTRUCTION_ADD(0x0a, 1, ld_r8_m16, REG_A, REG_BC, "LD A, (BC)"),
    INSTRUCTION_ADD(0x0b, 1, dec_r16, REG_BC, NULL, "DEC BC"),
    INSTRUCTION_ADD(0x0c, 1, inc_r8, REG_C, NULL, "INC C"),
    INSTRUCTION_ADD(0x0d, 1, dec_r8, REG_C, NULL, "DEC C"),
    INSTRUCTION_ADD(0x0e, 2, ld_r8_i8, REG_C, NULL, "LD C, n8"),
    INSTRUCTION_ADD(0x0f, 1, rrca, NULL, NULL, "RRCA"),

    /* 0x10 */
    INSTRUCTION_ADD(0x10, 2, stop, NULL, NULL, "STOP"),    
    INSTRUCTION_ADD(0x11, 3, ld_r16_i16, REG_DE, NULL, "LD DE, n16"),
    INSTRUCTION_ADD(0x12, 1, ld_m16_r8, REG_DE, REG_A, "LD (DE), A"),
    INSTRUCTION_ADD(0x13, 1, inc_r16, REG_DE, NULL, "INC DE"),
    INSTRUCTION_ADD(0x14, 1, inc_r8, REG_D, NULL, "INC D"),
    INSTRUCTION_ADD(0x15, 1, dec_r8, REG_D, NULL, "DEC D"),
    INSTRUCTION_ADD(0x16, 2, ld_r8_i8, REG_D, NULL, "LD D, n8"),
    INSTRUCTION_ADD(0x17, 1, rla, NULL, NULL, "RLA"),
    INSTRUCTION_ADD(0x18, 2, jr_i8, NULL, NULL, "JR e8"),
    INSTRUCTION_ADD(0x19, 1, add_r16_r16, REG_HL, REG_DE, "ADD HL, DE"),
    INSTRUCTION_ADD(0x1a, 1, ld_r8_m16, REG_A, REG_DE, "LD A, (DE)"),
    INSTRUCTION_ADD(0x1b, 1, dec_r16, REG_DE, NULL, "DEC DE"),
    INSTRUCTION_ADD(0x1c, 1, inc_r8, REG_E, NULL, "INC E"),
    INSTRUCTION_ADD(0x1d, 1, dec_r8, REG_E, NULL, "DEC E"),
    INSTRUCTION_ADD(0x1e, 2, ld_r8_i8, REG_E, NULL, "LD E, n8"),
    INSTRUCTION_ADD(0x1f, 1, rra, NULL, NULL, "RRA"),

    /* 0x20 */
    INSTRUCTION_ADD(0x20, 2, jr_nz_i8, NULL, NULL, "JR NZ, e8"),
    INSTRUCTION_ADD(0x21, 3, ld_r16_i16, REG_HL, NULL, "LD HL, n16"),
    INSTRUCTION_ADD(0x22, 1, ldi_m16_r8, REG_HL, REG_A, "LDI (HL), A"),    
    INSTRUCTION_ADD(0x23, 1, inc_r16, REG_HL, NULL, "INC HL"),
    INSTRUCTION_ADD(0x24, 1, inc_r8, REG_H, NULL, "INC H"),
    INSTRUCTION_ADD(0x25, 1, dec_r8, REG_H, NULL, "DEC H"),
    INSTRUCTION_ADD(0x26, 2, ld_r8_i8, REG_H, NULL, "LD H, n8"),
    INSTRUCTION_ADD(0x27, 1, daa, NULL, NULL, "DAA"),
    INSTRUCTION_ADD(0x28, 2, jr_z_i8, NULL, NULL, "JR Z, e8"),
    INSTRUCTION_ADD(0x29, 1, add_r16_r16, REG_HL, REG_HL, "ADD HL, HL"),
    INSTRUCTION_ADD(0x2a, 1, ldi_r8_m16, REG_A, REG_HL, "LDI A, (HL)"),    
    INSTRUCTION_ADD(0x2b, 1, dec_r16, REG_HL, NULL, "DEC HL"),
    INSTRUCTION_ADD(0x2c, 1, inc_r8, REG_L, NULL, "INC L"),
    INSTRUCTION_ADD(0x2d, 1, dec_r8, REG_L, NULL, "DEC L"),
    INSTRUCTION_ADD(0x2e, 2, ld_r8_i8, REG_L, NULL, "LD L, n8"),
    INSTRUCTION_ADD(0x2f, 1, cpl, NULL, NULL, "CPL"),

    /* 0x30 */
    INSTRUCTION_ADD(0x30, 2, jr_nc_i8, NULL, NULL, "JR NC, e8"),
    INSTRUCTION_ADD(0x31, 3, ld_r16_i16, REG_SP, NULL, "LD SP, n16"),
    INSTRUCTION_ADD(0x32, 1, ldd_m16_r8, REG_HL, REG_A, "LDD (HL), A"),
    INSTRUCTION_ADD(0x33, 1, inc_r16, REG_SP, NULL, "INC SP"),
    INSTRUCTION_ADD(0x34, 1, inc_m16, REG_HL, NULL, "INC (HL)"),
    INSTRUCTION_ADD(0x35, 1, dec_m16, REG_HL, NULL, "DEC (HL)"),
    INSTRUCTION_ADD(0x36, 2, ld_m16_i8, REG_HL, NULL, "LD (HL), n8"),    
    INSTRUCTION_ADD(0x37, 1, scf, NULL, NULL, "SCF"),
    INSTRUCTION_ADD(0x38, 2, jr_c_i8, NULL, NULL, "JR C, e8"),
    INSTRUCTION_ADD(0x39, 1, add_r16_r16, REG_HL, REG_SP, "ADD HL, SP"),
    INSTRUCTION_ADD(0x3a, 1, ldd_r8_m16, REG_A, REG_HL, "LDD A, (HL)"),
    INSTRUCTION_ADD(0x3b, 1, dec_r16, REG_SP, NULL, "DEC SP"),
    INSTRUCTION_ADD(0x3c, 1, inc_r8, REG_A, NULL, "INC A"),
    INSTRUCTION_ADD(0x3d, 1, dec_r8, REG_A, NULL, "DEC A"),
    INSTRUCTION_ADD(0x3e, 2, ld_r8_i8, REG_A, NULL, "LD A, n8"),
    INSTRUCTION_ADD(0x3f, 1, ccf, NULL, NULL, "CCF"),

    /* 0x40 */    
    INSTRUCTION_ADD(0x40, 1, ld_r8_r8, REG_B, REG_B, "LD B, B"), /* fancy */
    INSTRUCTION_ADD(0x41, 1, ld_r8_r8, REG_B, REG_C, "LD B, C"),
    INSTRUCTION_ADD(0x42, 1, ld_r8_r8, REG_B, REG_D, "LD B, D"),
    INSTRUCTION_ADD(0x43, 1, ld_r8_r8, REG_B, REG_E, "LD B, E"),
    INSTRUCTION_ADD(0x44, 1, ld_r8_r8, REG_B, REG_H, "LD B, H"),
    INSTRUCTION_ADD(0x45, 1, ld_r8_r8, REG_B, REG_L, "LD B, L"),
    INSTRUCTION_ADD(0x46, 1, ld_r8_m16, REG_B, REG_HL, "LD B, (HL)"),
    INSTRUCTION_ADD(0x47, 1, ld_r8_r8, REG_B, REG_A, "LD B, A"),
    INSTRUCTION_ADD(0x48, 1, ld_r8_r8, REG_C, REG_B, "LD C, B"),
    INSTRUCTION_ADD(0x49, 1, ld_r8_r8, REG_C, REG_C, "LD C, C"), /* fancy */
    INSTRUCTION_ADD(0x4a, 1, ld_r8_r8, REG_C, REG_D, "LD C, D"),
    INSTRUCTION_ADD(0x4b, 1, ld_r8_r8, REG_C, REG_E, "LD C, E"),
    INSTRUCTION_ADD(0x4c, 1, ld_r8_r8, REG_C, REG_H, "LD C, H"),    
    INSTRUCTION_ADD(0x4d, 1, ld_r8_r8, REG_C, REG_L, "LD C, L"),
    INSTRUCTION_ADD(0x4e, 1, ld_r8_m16, REG_C, REG_HL, "LD C, (HL)"),
    INSTRUCTION_ADD(0x4f, 1, ld_r8_r8, REG_C, REG_A, "LD C, A"),

    /* 0x50 */
    INSTRUCTION_ADD(0x50, 1, ld_r8_r8, REG_D, REG_B, "LD D, B"),
    INSTRUCTION_ADD(0x51, 1, ld_r8_r8, REG_D, REG_C, "LD D, C"),
    INSTRUCTION_ADD(0x52, 1, ld_r8_r8, REG_D, REG_D, "LD D, D"), /* fancy */
    INSTRUCTION_ADD(0x53, 1, ld_r8_r8, REG_D, REG_E, "LD D, E"),
    INSTRUCTION_ADD(0x54, 1, ld_r8_r8, REG_D, REG_H, "LD D, H"),
    INSTRUCTION_ADD(0x55, 1, ld_r8_r8, REG_D, REG_L, "LD D, L"),
    INSTRUCTION_ADD(0x56, 1, ld_r8_m16, REG_D, REG_HL, "LD D, (HL)"),
    INSTRUCTION_ADD(0x57, 1, ld_r8_r8, REG_D, REG_A, "LD D, A"),
    INSTRUCTION_ADD(0x58, 1, ld_r8_r8, REG_E, REG_B, "LD E, B"),
    INSTRUCTION_ADD(0x59, 1, ld_r8_r8, REG_E, REG_C, "LD E, C"),
    INSTRUCTION_ADD(0x5a, 1, ld_r8_r8, REG_E, REG_D, "LD E, D"),
    INSTRUCTION_ADD(0x5b, 1, ld_r8_r8, REG_E, REG_E, "LD E, E"), /* fancy */
    INSTRUCTION_ADD(0x5c, 1, ld_r8_r8, REG_E, REG_H, "LD E, H"),
    INSTRUCTION_ADD(0x5d, 1, ld_r8_r8, REG_E, REG_L, "LD E, L"),
    INSTRUCTION_ADD(0x5e, 1, ld_r8_m16, REG_E, REG_HL, "LD E, (HL)"),
    INSTRUCTION_ADD(0x5f, 1, ld_r8_r8, REG_E, REG_A, "LD E, A"),

    /* 0x60 */
    INSTRUCTION_ADD(0x60, 1, ld_r8_r8, REG_H, REG_B, "LD H, B"),
    INSTRUCTION_ADD(0x61, 1, ld_r8_r8, REG_H, REG_C, "LD H, C"),
    INSTRUCTION_ADD(0x62, 1, ld_r8_r8, REG_H, REG_D, "LD H, D"),
    INSTRUCTION_ADD(0x63, 1, ld_r8_r8, REG_H, REG_E, "LD H, E"),
    INSTRUCTION_ADD(0x64, 1, ld_r8_r8, REG_H, REG_H, "LD H, H"), /* fancy */
    INSTRUCTION_ADD(0x65, 1, ld_r8_r8, REG_H, REG_L, "LD H, L"),
    INSTRUCTION_ADD(0x66, 1, ld_r8_m16, REG_H, REG_HL, "LD H, (HL)"),
    INSTRUCTION_ADD(0x67, 1, ld_r8_r8, REG_H, REG_A, "LD H, A"),
    INSTRUCTION_ADD(0x68, 1, ld_r8_r8, REG_L, REG_B, "LD L, B"),
    INSTRUCTION_ADD(0x69, 1, ld_r8_r8, REG_L, REG_C, "LD L, C"),
    INSTRUCTION_ADD(0x6a, 1, ld_r8_r8, REG_L, REG_D, "LD L, D"),
    INSTRUCTION_ADD(0x6b, 1, ld_r8_r8, REG_L, REG_E, "LD L, E"),
    INSTRUCTION_ADD(0x6c, 1, ld_r8_r8, REG_L, REG_H, "LD L, H"),
    INSTRUCTION_ADD(0x6d, 1, ld_r8_r8, REG_L, REG_L, "LD L, L"), /* fancy */
    INSTRUCTION_ADD(0x6e, 1, ld_r8_m16, REG_L, REG_HL, "LD L, (HL)"),
    INSTRUCTION_ADD(0x6f, 1, ld_r8_r8, REG_L, REG_A, "LD L, A"),

    /* 0x70 */
    INSTRUCTION_ADD(0x70, 1, ld_m16_r8, REG_HL, REG_B, "LD (HL), B"),
    INSTRUCTION_ADD(0x71, 1, ld_m16_r8, REG_HL, REG_C, "LD (HL), C"),
    INSTRUCTION_ADD(0x72, 1, ld_m16_r8, REG_HL, REG_D, "LD (HL), D"),
    INSTRUCTION_ADD(0x73, 1, ld_m16_r8, REG_HL, REG_E, "LD (HL), E"),
    INSTRUCTION_ADD(0x74, 1, ld_m16_r8, REG_HL, REG_H, "LD (HL), H"),
    INSTRUCTION_ADD(0x75, 1, ld_m16_r8, REG_HL, REG_L, "LD (HL), L"),
    INSTRUCTION_ADD(0x76, 1, halt, NULL, NULL, "HALT"),    
    INSTRUCTION_ADD(0x77, 1, ld_m16_r8, REG_HL, REG_A, "LD (HL), A"),
    INSTRUCTION_ADD(0x78, 1, ld_r8_r8, REG_A, REG_B, "LD A, B"),
    INSTRUCTION_ADD(0x79, 1, ld_r8_r8, REG_A, REG_C, "LD A, C"),
    INSTRUCTION_ADD(0x7a, 1, ld_r8_r8, REG_A, REG_D, "LD A, D"),
    INSTRUCTION_ADD(0x7b, 1, ld_r8_r8, REG_A, REG_E, "LD A, E"),
    INSTRUCTION_ADD(0x7c, 1, ld_r8_r8, REG_A, REG_H, "LD A, H"),
    INSTRUCTION_ADD(0x7d, 1, ld_r8_r8, REG_A, REG_L, "LD A, L"),
    INSTRUCTION_ADD(0x7e, 1, ld_r8_m16, REG_A, REG_HL, "LD A, (HL)"),
    INSTRUCTION_ADD(0x7f, 1, ld_r8_r8, REG_A, REG_A, "LD A, A"), /* fancy */

    /* 0x80 */
    INSTRUCTION_ADD(0x80, 1, add_r8_r8, REG_A, REG_B, "ADD A, B"),
    INSTRUCTION_ADD(0x81, 1, add_r8_r8, REG_A, REG_C, "ADD A, C"),
    INSTRUCTION_ADD(0x82, 1, add_r8_r8, REG_A, REG_D, "ADD A, D"),
    INSTRUCTION_ADD(0x83, 1, add_r8_r8, REG_A, REG_E, "ADD A, E"),
    INSTRUCTION_ADD(0x84, 1, add_r8_r8, REG_A, REG_H, "ADD A, H"),
    INSTRUCTION_ADD(0x85, 1, add_r8_r8, REG_A, REG_L, "ADD A, L"),
    INSTRUCTION_ADD(0x86, 1, add_r8_m16, REG_A, REG_HL, "ADD A, (HL)"),
    INSTRUCTION_ADD(0x87, 1, add_r8_r8, REG_A, REG_A, "ADD A, A"),
    INSTRUCTION_ADD(0x88, 1, adc_r8_r8, REG_A, REG_B, "ADC A, B"),
    INSTRUCTION_ADD(0x89, 1, adc_r8_r8, REG_A, REG_C, "ADC A, C"),
    INSTRUCTION_ADD(0x8a, 1, adc_r8_r8, REG_A, REG_D, "ADC A, D"),
    INSTRUCTION_ADD(0x8b, 1, adc_r8_r8, REG_A, REG_E, "ADC A, E"),
    INSTRUCTION_ADD(0x8c, 1, adc_r8_r8, REG_A, REG_H, "ADC A, H"),
    INSTRUCTION_ADD(0x8d, 1, adc_r8_r8, REG_A, REG_L, "ADC A, L"),
    INSTRUCTION_ADD(0x8e, 1, adc_r8_m16, REG_A, REG_HL, "ADC A, (HL)"),
    INSTRUCTION_ADD(0x8f, 1, adc_r8_r8, REG_A, REG_A, "ADC A, A"),

    /* 0x90 */    
    INSTRUCTION_ADD(0x90, 1, sub_r8_r8, REG_A, REG_B, "SUB A, B"),
    INSTRUCTION_ADD(0x91, 1, sub_r8_r8, REG_A, REG_C, "SUB A, C"),
    INSTRUCTION_ADD(0x92, 1, sub_r8_r8, REG_A, REG_D, "SUB A, D"),
    INSTRUCTION_ADD(0x93, 1, sub_r8_r8, REG_A, REG_E, "SUB A, E"),
    INSTRUCTION_ADD(0x94, 1, sub_r8_r8, REG_A, REG_H, "SUB A, H"),
    INSTRUCTION_ADD(0x95, 1, sub_r8_r8, REG_A, REG_L, "SUB A, L"),
    INSTRUCTION_ADD(0x96, 1, sub_r8_m16, REG_A, REG_HL, "SUB A, (HL)"),
    INSTRUCTION_ADD(0x97, 1, sub_r8_r8, REG_A, REG_A, "SUB A, A"),
    INSTRUCTION_ADD(0x98, 1, subc_r8_r8, REG_A, REG_B, "SUBC A, B"),
    INSTRUCTION_ADD(0x99, 1, subc_r8_r8, REG_A, REG_C, "SUBC A, C"),
    INSTRUCTION_ADD(0x9a, 1, subc_r8_r8, REG_A, REG_D, "SUBC A, D"),
    INSTRUCTION_ADD(0x9b, 1, subc_r8_r8, REG_A, REG_E, "SUBC A, E"),
    INSTRUCTION_ADD(0x9c, 1, subc_r8_r8, REG_A, REG_H, "SUBC A, H"),
    INSTRUCTION_ADD(0x9d, 1, subc_r8_r8, REG_A, REG_L, "SUBC A, L"),
    INSTRUCTION_ADD(0x9e, 1, subc_r8_m16, REG_A, REG_HL, "SUBC A, (HL)"),
    INSTRUCTION_ADD(0x9f, 1, subc_r8_r8, REG_A, REG_A, "SUBC A, A"),

    /* 0xa0 */
    INSTRUCTION_ADD(0xa0, 1, and_r8_r8, REG_A, REG_B, "AND A, B"),
    INSTRUCTION_ADD(0xa1, 1, and_r8_r8, REG_A, REG_C, "AND A, C"),
    INSTRUCTION_ADD(0xa2, 1, and_r8_r8, REG_A, REG_D, "AND A, D"),
    INSTRUCTION_ADD(0xa3, 1, and_r8_r8, REG_A, REG_E, "AND A, E"),
    INSTRUCTION_ADD(0xa4, 1, and_r8_r8, REG_A, REG_H, "AND A, H"),
    INSTRUCTION_ADD(0xa5, 1, and_r8_r8, REG_A, REG_L, "AND A, L"),
    INSTRUCTION_ADD(0xa6, 1, and_r8_m16, REG_A, REG_HL, "AND A, (HL)"),
    INSTRUCTION_ADD(0xa7, 1, and_r8_r8, REG_A, REG_A, "AND A, A"),
    INSTRUCTION_ADD(0xa8, 1, xor_r8_r8, REG_A, REG_B, "XOR A, B"),
    INSTRUCTION_ADD(0xa9, 1, xor_r8_r8, REG_A, REG_C, "XOR A, C"),
    INSTRUCTION_ADD(0xaa, 1, xor_r8_r8, REG_A, REG_D, "XOR A, D"),
    INSTRUCTION_ADD(0xab, 1, xor_r8_r8, REG_A, REG_E, "XOR A, E"),
    INSTRUCTION_ADD(0xac, 1, xor_r8_r8, REG_A, REG_H, "XOR A, H"),
    INSTRUCTION_ADD(0xad, 1, xor_r8_r8, REG_A, REG_L, "XOR A, L"),
    INSTRUCTION_ADD(0xae, 1, xor_r8_m16, REG_A, REG_HL, "XOR A, (HL)"),
    INSTRUCTION_ADD(0xaf, 1, xor_r8_r8, REG_A, REG_A, "XOR A, A"),

    /* 0xb0 */
    INSTRUCTION_ADD(0xb0, 1, or_r8_r8, REG_A, REG_B, "OR A, B"),
    INSTRUCTION_ADD(0xb1, 1, or_r8_r8, REG_A, REG_C, "OR A, C"),
    INSTRUCTION_ADD(0xb2, 1, or_r8_r8, REG_A, REG_D, "OR A, D"),
    INSTRUCTION_ADD(0xb3, 1, or_r8_r8, REG_A, REG_E, "OR A, E"),
    INSTRUCTION_ADD(0xb4, 1, or_r8_r8, REG_A, REG_H, "OR A, H"),
    INSTRUCTION_ADD(0xb5, 1, or_r8_r8, REG_A, REG_L, "OR A, L"),
    INSTRUCTION_ADD(0xb6, 1, or_r8_m16, REG_A, REG_HL, "OR A, (HL)"),
    INSTRUCTION_ADD(0xb7, 1, or_r8_r8, REG_A, REG_A, "OR A, A"),
    INSTRUCTION_ADD(0xb8, 1, cp_r8_r8, REG_A, REG_B, "CP A, B"),
    INSTRUCTION_ADD(0xb9, 1, cp_r8_r8, REG_A, REG_C, "CP A, C"),
    INSTRUCTION_ADD(0xba, 1, cp_r8_r8, REG_A, REG_D, "CP A, D"),
    INSTRUCTION_ADD(0xbb, 1, cp_r8_r8, REG_A, REG_E, "CP A, E"),
    INSTRUCTION_ADD(0xbc, 1, cp_r8_r8, REG_A, REG_H, "CP A, H"),
    INSTRUCTION_ADD(0xbd, 1, cp_r8_r8, REG_A, REG_L, "CP A, L"),
    INSTRUCTION_ADD(0xbe, 1, cp_r8_m16, REG_A, REG_HL, "CP A, (HL)"),
    INSTRUCTION_ADD(0xbf, 1, cp_r8_r8, REG_A, REG_A, "CP A, A"),

    /* 0xc0 */
    INSTRUCTION_ADD(0xc0, 1, ret_nz, NULL, NULL, "RET NZ"),
    INSTRUCTION_ADD(0xc1, 1, pop_r16, REG_BC, NULL, "POP BC"),
    INSTRUCTION_ADD(0xc2, 3, jp_nz_i16, NULL, NULL, "JP NZ, n16"),
    INSTRUCTION_ADD(0xc3, 3, jp_i16, NULL, NULL, "JP n16"),
    INSTRUCTION_ADD(0xc4, 3, call_nz_i16, NULL, NULL, "CALL NZ, n16"),
    INSTRUCTION_ADD(0xc5, 1, push_r16, REG_BC, NULL, "PUSH BC"),
    INSTRUCTION_ADD(0xc6, 2, add_r8_i8, REG_A, NULL, "ADD A, n8"),
    INSTRUCTION_ADD(0xc7, 1, rst, 0x00, NULL, "RST 00H"),            
    INSTRUCTION_ADD(0xc8, 1, ret_z, NULL, NULL, "RET Z"),
    INSTRUCTION_ADD(0xc9, 1, ret, NULL, NULL, "RET"),
    INSTRUCTION_ADD(0xca, 3, jp_z_i16, NULL, NULL, "JP Z, n16"),    
    INSTRUCTION_ADD(0xcb, 1, NULL, NULL, NULL, "CB"), /* PREFIX_CB */
    INSTRUCTION_ADD(0xcc, 3, call_z_i16, NULL, NULL, "CALL Z, n16"),
    INSTRUCTION_ADD(0xcd, 3, call_i16, NULL, NULL, "CALL n16"),
    INSTRUCTION_ADD(0xce, 2, adc_r8_i8, REG_A, NULL, "ADC A, n8"),
    INSTRUCTION_ADD(0xcf, 1, rst, 0x08, NULL, "RST 08H"),

    /* 0xd0 */
    INSTRUCTION_ADD(0xd0, 1, ret_nc, NULL, NULL, "RET NC"),
    INSTRUCTION_ADD(0xd1, 1, pop_r16, REG_DE, NULL, "POP DE"),
    INSTRUCTION_ADD(0xd2, 3, jp_nc_i16, NULL, NULL, "JP NC, n16"),
    INSTRUCTION_ADD(0xd3, 1, NULL, NULL, NULL, "NOP"),
    INSTRUCTION_ADD(0xd4, 3, call_nc_i16, NULL, NULL, "CALL NC, n16"),
    INSTRUCTION_ADD(0xd5, 1, push_r16, REG_DE, NULL, "PUSH DE"),
    INSTRUCTION_ADD(0xd6, 2, sub_r8_i8, REG_A, NULL, "SUB A, n8"),
    INSTRUCTION_ADD(0xd7, 1, rst, 0x10, NULL, "RST 10H"),   
    INSTRUCTION_ADD(0xd8, 1, ret_c, NULL, NULL, "RET C"),
    INSTRUCTION_ADD(0xd9, 1, reti, NULL, NULL, "RETI"),       
    INSTRUCTION_ADD(0xda, 3, jp_c_i16, NULL, NULL, "JP C, n16"),
    INSTRUCTION_ADD(0xdb, 1, NULL, NULL, NULL, "NOP"),
    INSTRUCTION_ADD(0xdc, 3, call_c_i16, NULL, NULL, "CALL C, n16"),
    INSTRUCTION_ADD(0xdd, 1, NULL, NULL, NULL, "NOP"),
    INSTRUCTION_ADD(0xde, 2, subc_r8_i8, REG_A, NULL, "SBC A, n8"),
    INSTRUCTION_ADD(0xdf, 1, rst, 0x18, NULL, "RST 18H"),

    /* 0xe0 */
    INSTRUCTION_ADD(0xe0, 2, ldh_im8_r8, NULL, REG_A, "LDH (n8), A"),
    INSTRUCTION_ADD(0xe1, 1, pop_r16, REG_HL, NULL, "POP HL"),    
    INSTRUCTION_ADD(0xe2, 1, ldh_r8_m8, REG_C, REG_A, "LDH (C), A"),
    INSTRUCTION_ADD(0xe3, 1, NULL, NULL, NULL, "NOP"),
    INSTRUCTION_ADD(0xe4, 1, NULL, NULL, NULL, "NOP"),
    INSTRUCTION_ADD(0xe5, 1, push_r16, REG_HL, NULL, "PUSH HL"),
    INSTRUCTION_ADD(0xe6, 2, and_r8_i8, REG_A, NULL, "AND A, n8"),
    INSTRUCTION_ADD(0xe7, 1, rst, 0x20, NULL, "RST 20H"),
    INSTRUCTION_ADD(0xe8, 2, add_r16_i8, REG_SP, NULL, "ADD SP, n8"),
    INSTRUCTION_ADD(0xe9, 1, jp_r16, REG_HL, NULL, "JP HL"),
    INSTRUCTION_ADD(0xea, 3, ld_im16_r8, NULL, NULL, "LD (n16), A"), 
    INSTRUCTION_ADD(0xeb, 1, NULL, NULL, NULL, "NOP"),
    INSTRUCTION_ADD(0xec, 1, NULL, NULL, NULL, "NOP"),
    INSTRUCTION_ADD(0xed, 1, NULL, NULL, NULL, "NOP"),
    INSTRUCTION_ADD(0xee, 2, xor_r8_i8, REG_A, NULL, "XOR A, n8"),
    INSTRUCTION_ADD(0xef, 1, rst, 0x28, NULL, "RST 28H"),

    /* 0xf0 */
    INSTRUCTION_ADD(0xf0, 2, ldh_r8_im8, REG_A, NULL, "LDH A, (n8)"),
    INSTRUCTION_ADD(0xf1, 1, pop_r16, REG_AF, NULL, "POP AF"),
    INSTRUCTION_ADD(0xf2, 1, ldh_r8_m8, REG_A, REG_C, "LDH A, (C)"),    
    INSTRUCTION_ADD(0xf3, 1, di, NULL, NULL, "DI"),
    INSTRUCTION_ADD(0xf4, 1, NULL, NULL, NULL, "NOP"),
    INSTRUCTION_ADD(0xf5, 1, push_r16, REG_AF, NULL, "PUSH AF"),
    INSTRUCTION_ADD(0xf6, 2, or_r8_i8, REG_A, NULL, "OR A, n8"),
    INSTRUCTION_ADD(0xf7, 1, rst, 0x30, NULL, "RST 30H"),
    INSTRUCTION_ADD(0xf8, 2, ld_hl_sp_i8, NULL, NULL, "LD HL, SP+n8"),
    INSTRUCTION_ADD(0xf9, 1, ld_sp_hl, NULL, NULL, "LD SP, HL"),
    INSTRUCTION_ADD(0xfa, 3, ld_r8_m16, REG_A, NULL, "LD A, (n16)"),
    INSTRUCTION_ADD(0xfb, 1, ei, NULL, NULL, "EI"),    
    INSTRUCTION_ADD(0xfc, 1, NULL, NULL, NULL, "NOP"),
    INSTRUCTION_ADD(0xfd, 1, NULL, NULL, NULL, "NOP"),
    INSTRUCTION_ADD(0xfe, 2, cp_r8_i8, REG_A, NULL, "CP A, n8"),
    INSTRUCTION_ADD(0xff, 1, rst, 0x38, NULL, "RST 38H"),
};
 
static instruction_t prefixed_instruction_set[INSTRUCTIONS_SET_SIZE] = {
    /* 0x00 */
    INSTRUCTION_ADD(0x00, 1, cb_rlc_r8, REG_B, NULL, "RLC B"),
    INSTRUCTION_ADD(0x01, 1, cb_rlc_r8, REG_C, NULL, "RLC C"),
    INSTRUCTION_ADD(0x02, 1, cb_rlc_r8, REG_D, NULL, "RLC D"),
    INSTRUCTION_ADD(0x03, 1, cb_rlc_r8, REG_E, NULL, "RLC E"),
    INSTRUCTION_ADD(0x04, 1, cb_rlc_r8, REG_H, NULL, "RLC H"),
    INSTRUCTION_ADD(0x05, 1, cb_rlc_r8, REG_L, NULL, "RLC L"),
    INSTRUCTION_ADD(0x06, 1, cb_rlc_m16, REG_HL, NULL, "RLC (HL)"),
    INSTRUCTION_ADD(0x07, 1, cb_rlc_r8, REG_A, NULL, "RLC A"),
    INSTRUCTION_ADD(0x08, 1, cb_rrc_r8, REG_B, NULL, "RRC B"),
    INSTRUCTION_ADD(0x09, 1, cb_rrc_r8, REG_C, NULL, "RRC C"),
    INSTRUCTION_ADD(0x0a, 1, cb_rrc_r8, REG_D, NULL, "RRC D"),
    INSTRUCTION_ADD(0x0b, 1, cb_rrc_r8, REG_E, NULL, "RRC E"),
    INSTRUCTION_ADD(0x0c, 1, cb_rrc_r8, REG_H, NULL, "RRC H"),
    INSTRUCTION_ADD(0x0d, 1, cb_rrc_r8, REG_L, NULL, "RRC L"),
    INSTRUCTION_ADD(0x0e, 1, cb_rrc_m16, REG_HL, NULL, "RRC (HL)"),
    INSTRUCTION_ADD(0x0f, 1, cb_rrc_r8, REG_A, NULL, "RRC A"),        

    /* 0x10 */    
    INSTRUCTION_ADD(0x10, 1, cb_rl_r8, REG_B, NULL, "RL B"),
    INSTRUCTION_ADD(0x11, 1, cb_rl_r8, REG_C, NULL, "RL C"),
    INSTRUCTION_ADD(0x12, 1, cb_rl_r8, REG_D, NULL, "RL D"),
    INSTRUCTION_ADD(0x13, 1, cb_rl_r8, REG_E, NULL, "RL E"),
    INSTRUCTION_ADD(0x14, 1, cb_rl_r8, REG_H, NULL, "RL H"),
    INSTRUCTION_ADD(0x15, 1, cb_rl_r8, REG_L, NULL, "RL L"),
    INSTRUCTION_ADD(0x16, 1, cb_rl_m16, REG_HL, NULL, "RL (HL)"),
    INSTRUCTION_ADD(0x17, 1, cb_rl_r8, REG_A, NULL, "RL A"),
    INSTRUCTION_ADD(0x18, 1, cb_rr_r8, REG_B, NULL, "RR B"),
    INSTRUCTION_ADD(0x19, 1, cb_rr_r8, REG_C, NULL, "RR C"),
    INSTRUCTION_ADD(0x1a, 1, cb_rr_r8, REG_D, NULL, "RR D"),
    INSTRUCTION_ADD(0x1b, 1, cb_rr_r8, REG_E, NULL, "RR E"),
    INSTRUCTION_ADD(0x1c, 1, cb_rr_r8, REG_H, NULL, "RR H"),
    INSTRUCTION_ADD(0x1d, 1, cb_rr_r8, REG_L, NULL, "RR L"),
    INSTRUCTION_ADD(0x1e, 1, cb_rr_m16, REG_HL, NULL, "RR (HL)"),
    INSTRUCTION_ADD(0x1f, 1, cb_rr_r8, REG_A, NULL, "RR A"),

    /* 0x20 */
    INSTRUCTION_ADD(0x20, 1, cb_sla_r8, REG_B, NULL, "SLA B"),
    INSTRUCTION_ADD(0x21, 1, cb_sla_r8, REG_C, NULL, "SLA C"),
    INSTRUCTION_ADD(0x22, 1, cb_sla_r8, REG_D, NULL, "SLA D"),
    INSTRUCTION_ADD(0x23, 1, cb_sla_r8, REG_E, NULL, "SLA E"),
    INSTRUCTION_ADD(0x24, 1, cb_sla_r8, REG_H, NULL, "SLA H"),
    INSTRUCTION_ADD(0x25, 1, cb_sla_r8, REG_L, NULL, "SLA L"),
    INSTRUCTION_ADD(0x26, 1, cb_sla_m16, REG_HL, NULL, "SLA (HL)"),
    INSTRUCTION_ADD(0x27, 1, cb_sla_r8, REG_A, NULL, "SLA A"),
    INSTRUCTION_ADD(0x28, 1, cb_sra_r8, REG_B, NULL, "SRA B"),
    INSTRUCTION_ADD(0x29, 1, cb_sra_r8, REG_C, NULL, "SRA C"),
    INSTRUCTION_ADD(0x2a, 1, cb_sra_r8, REG_D, NULL, "SRA D"),
    INSTRUCTION_ADD(0x2b, 1, cb_sra_r8, REG_E, NULL, "SRA E"),
    INSTRUCTION_ADD(0x2c, 1, cb_sra_r8, REG_H, NULL, "SRA H"),
    INSTRUCTION_ADD(0x2d, 1, cb_sra_r8, REG_L, NULL, "SRA L"),
    INSTRUCTION_ADD(0x2e, 1, cb_sra_m16, REG_HL, NULL, "SRA (HL)"),
    INSTRUCTION_ADD(0x2f, 1, cb_sra_r8, REG_A, NULL, "SRA A"),

    /* 0x30 */
    INSTRUCTION_ADD(0x30, 1, cb_swap_r8, REG_B, NULL, "SWAP B"),
    INSTRUCTION_ADD(0x31, 1, cb_swap_r8, REG_C, NULL, "SWAP C"),    
    INSTRUCTION_ADD(0x32, 1, cb_swap_r8, REG_D, NULL, "SWAP D"),
    INSTRUCTION_ADD(0x33, 1, cb_swap_r8, REG_E, NULL, "SWAP E"),
    INSTRUCTION_ADD(0x34, 1, cb_swap_r8, REG_H, NULL, "SWAP H"),
    INSTRUCTION_ADD(0x35, 1, cb_swap_r8, REG_L, NULL, "SWAP L"),
    INSTRUCTION_ADD(0x36, 1, cb_swap_m16, REG_HL, NULL, "SWAP (HL)"),
    INSTRUCTION_ADD(0x37, 1, cb_swap_r8, REG_A, NULL, "SWAP A"),
    INSTRUCTION_ADD(0x38, 1, cb_srl_r8, REG_B, NULL, "SRL B"),
    INSTRUCTION_ADD(0x39, 1, cb_srl_r8, REG_C, NULL, "SRL C"),
    INSTRUCTION_ADD(0x3a, 1, cb_srl_r8, REG_D, NULL, "SRL D"),
    INSTRUCTION_ADD(0x3b, 1, cb_srl_r8, REG_E, NULL, "SRL E"),
    INSTRUCTION_ADD(0x3c, 1, cb_srl_r8, REG_H, NULL, "SRL H"),
    INSTRUCTION_ADD(0x3d, 1, cb_srl_r8, REG_L, NULL, "SRL L"),
    INSTRUCTION_ADD(0x3e, 1, cb_srl_m16, REG_HL, NULL, "SRL (HL)"),
    INSTRUCTION_ADD(0x3f, 1, cb_srl_r8, REG_A, NULL, "SRL A"),

    /* 0x40 */
    INSTRUCTION_ADD(0x40, 1, cb_bit_r8, 0, REG_B, "BIT 0, B"),
    INSTRUCTION_ADD(0x41, 1, cb_bit_r8, 0, REG_C, "BIT 0, C"),        
    INSTRUCTION_ADD(0x42, 1, cb_bit_r8, 0, REG_D, "BIT 0, D"),
    INSTRUCTION_ADD(0x43, 1, cb_bit_r8, 0, REG_E, "BIT 0, E"),
    INSTRUCTION_ADD(0x44, 1, cb_bit_r8, 0, REG_H, "BIT 0, H"),
    INSTRUCTION_ADD(0x45, 1, cb_bit_r8, 0, REG_L, "BIT 0, L"),
    INSTRUCTION_ADD(0x46, 1, cb_bit_m16, 0, REG_HL, "BIT 0, (HL)"),    
    INSTRUCTION_ADD(0x47, 1, cb_bit_r8, 0, REG_A, "BIT 0, A"),
    INSTRUCTION_ADD(0x48, 1, cb_bit_r8, 1, REG_B, "BIT 1, B"),
    INSTRUCTION_ADD(0x49, 1, cb_bit_r8, 1, REG_C, "BIT 1, C"),
    INSTRUCTION_ADD(0x4a, 1, cb_bit_r8, 1, REG_D, "BIT 1, D"),
    INSTRUCTION_ADD(0x4b, 1, cb_bit_r8, 1, REG_E, "BIT 1, E"),
    INSTRUCTION_ADD(0x4c, 1, cb_bit_r8, 1, REG_H, "BIT 1, H"),
    INSTRUCTION_ADD(0x4d, 1, cb_bit_r8, 1, REG_L, "BIT 1, L"),
    INSTRUCTION_ADD(0x4e, 1, cb_bit_m16, 1, REG_HL, "BIT 1, (HL)"),
    INSTRUCTION_ADD(0x4f, 1, cb_bit_r8, 1, REG_A, "BIT 1, A"),

    /* 0x50 */
    INSTRUCTION_ADD(0x50, 1, cb_bit_r8, 2, REG_B, "BIT 2, B"),
    INSTRUCTION_ADD(0x51, 1, cb_bit_r8, 2, REG_C, "BIT 2, C"),
    INSTRUCTION_ADD(0x52, 1, cb_bit_r8, 2, REG_D, "BIT 2, D"),
    INSTRUCTION_ADD(0x53, 1, cb_bit_r8, 2, REG_E, "BIT 2, E"),
    INSTRUCTION_ADD(0x54, 1, cb_bit_r8, 2, REG_H, "BIT 2, H"),
    INSTRUCTION_ADD(0x55, 1, cb_bit_r8, 2, REG_L, "BIT 2, L"),
    INSTRUCTION_ADD(0x56, 1, cb_bit_m16, 2, REG_HL, "BIT 2, (HL)"),
    INSTRUCTION_ADD(0x57, 1, cb_bit_r8, 2, REG_A, "BIT 2, A"),
    INSTRUCTION_ADD(0x58, 1, cb_bit_r8, 3, REG_B, "BIT 3, B"),
    INSTRUCTION_ADD(0x59, 1, cb_bit_r8, 3, REG_C, "BIT 3, C"),
    INSTRUCTION_ADD(0x5a, 1, cb_bit_r8, 3, REG_D, "BIT 3, D"),
    INSTRUCTION_ADD(0x5b, 1, cb_bit_r8, 3, REG_E, "BIT 3, E"),
    INSTRUCTION_ADD(0x5c, 1, cb_bit_r8, 3, REG_H, "BIT 3, H"),
    INSTRUCTION_ADD(0x5d, 1, cb_bit_r8, 3, REG_L, "BIT 3, L"),
    INSTRUCTION_ADD(0x5e, 1, cb_bit_m16, 3, REG_HL, "BIT 3, (HL)"),
    INSTRUCTION_ADD(0x5f, 1, cb_bit_r8, 3, REG_A, "BIT 3, A"),

    /* 0x60 */
    INSTRUCTION_ADD(0x60, 1, cb_bit_r8, 4, REG_B, "BIT 4, B"),
    INSTRUCTION_ADD(0x61, 1, cb_bit_r8, 4, REG_C, "BIT 4, C"),
    INSTRUCTION_ADD(0x62, 1, cb_bit_r8, 4, REG_D, "BIT 4, D"),
    INSTRUCTION_ADD(0x63, 1, cb_bit_r8, 4, REG_E, "BIT 4, E"),
    INSTRUCTION_ADD(0x64, 1, cb_bit_r8, 4, REG_H, "BIT 4, H"),
    INSTRUCTION_ADD(0x65, 1, cb_bit_r8, 4, REG_L, "BIT 4, L"),
    INSTRUCTION_ADD(0x66, 1, cb_bit_m16, 4, REG_HL, "BIT 4, (HL)"),
    INSTRUCTION_ADD(0x67, 1, cb_bit_r8, 4, REG_A, "BIT 4, A"),
    INSTRUCTION_ADD(0x68, 1, cb_bit_r8, 5, REG_B, "BIT 5, B"),
    INSTRUCTION_ADD(0x69, 1, cb_bit_r8, 5, REG_C, "BIT 5, C"),
    INSTRUCTION_ADD(0x6a, 1, cb_bit_r8, 5, REG_D, "BIT 5, D"),
    INSTRUCTION_ADD(0x6b, 1, cb_bit_r8, 5, REG_E, "BIT 5, E"),
    INSTRUCTION_ADD(0x6c, 1, cb_bit_r8, 5, REG_H, "BIT 5, H"),
    INSTRUCTION_ADD(0x6d, 1, cb_bit_r8, 5, REG_L, "BIT 5, L"),    
    INSTRUCTION_ADD(0x6e, 1, cb_bit_m16, 5, REG_HL, "BIT 5, (HL)"),
    INSTRUCTION_ADD(0x6f, 1, cb_bit_r8, 5, REG_A, "BIT 5, A"),

    /* 0x70 */
    INSTRUCTION_ADD(0x70, 1, cb_bit_r8, 6, REG_B, "BIT 6, B"),
    INSTRUCTION_ADD(0x71, 1, cb_bit_r8, 6, REG_C, "BIT 6, C"),
    INSTRUCTION_ADD(0x72, 1, cb_bit_r8, 6, REG_D, "BIT 6, D"),
    INSTRUCTION_ADD(0x73, 1, cb_bit_r8, 6, REG_E, "BIT 6, E"),
    INSTRUCTION_ADD(0x74, 1, cb_bit_r8, 6, REG_H, "BIT 6, H"),
    INSTRUCTION_ADD(0x75, 1, cb_bit_r8, 6, REG_L, "BIT 6, L"),
    INSTRUCTION_ADD(0x76, 1, cb_bit_m16, 6, REG_HL, "BIT 6, (HL)"),
    INSTRUCTION_ADD(0x77, 1, cb_bit_r8, 6, REG_A, "BIT 6, A"), 
    INSTRUCTION_ADD(0x78, 1, cb_bit_r8, 7, REG_B, "BIT 7, B"),
    INSTRUCTION_ADD(0x79, 1, cb_bit_r8, 7, REG_C, "BIT 7, C"),
    INSTRUCTION_ADD(0x7a, 1, cb_bit_r8, 7, REG_D, "BIT 7, D"),
    INSTRUCTION_ADD(0x7b, 1, cb_bit_r8, 7, REG_E, "BIT 7, E"),
    INSTRUCTION_ADD(0x7c, 1, cb_bit_r8, 7, REG_H, "BIT 7, H"),
    INSTRUCTION_ADD(0x7d, 1, cb_bit_r8, 7, REG_L, "BIT 7, L"),
    INSTRUCTION_ADD(0x7e, 1, cb_bit_m16, 7, REG_HL, "BIT 7, (HL)"),
    INSTRUCTION_ADD(0x7f, 1, cb_bit_r8, 7, REG_A, "BIT 7, A"),

    /* 0x80 */
    INSTRUCTION_ADD(0x80, 1, cb_res_r8, 0, REG_B, "RES 0, B"),
    INSTRUCTION_ADD(0x81, 1, cb_res_r8, 0, REG_C, "RES 0, C"),
    INSTRUCTION_ADD(0x82, 1, cb_res_r8, 0, REG_D, "RES 0, D"),
    INSTRUCTION_ADD(0x83, 1, cb_res_r8, 0, REG_E, "RES 0, E"),
    INSTRUCTION_ADD(0x84, 1, cb_res_r8, 0, REG_H, "RES 0, H"),
    INSTRUCTION_ADD(0x85, 1, cb_res_r8, 0, REG_L, "RES 0, L"),
    INSTRUCTION_ADD(0x86, 1, cb_res_m16, 0, REG_HL, "RES 0, (HL)"),
    INSTRUCTION_ADD(0x87, 1, cb_res_r8, 0, REG_A, "RES 0, A"),
    INSTRUCTION_ADD(0x88, 1, cb_res_r8, 1, REG_B, "RES 1, B"),
    INSTRUCTION_ADD(0x89, 1, cb_res_r8, 1, REG_C, "RES 1, C"),
    INSTRUCTION_ADD(0x8a, 1, cb_res_r8, 1, REG_D, "RES 1, D"),
    INSTRUCTION_ADD(0x8b, 1, cb_res_r8, 1, REG_E, "RES 1, E"),
    INSTRUCTION_ADD(0x8c, 1, cb_res_r8, 1, REG_H, "RES 1, H"),
    INSTRUCTION_ADD(0x8d, 1, cb_res_r8, 1, REG_L, "RES 1, L"),
    INSTRUCTION_ADD(0x8e, 1, cb_res_m16, 1, REG_HL, "RES 1, (HL)"),
    INSTRUCTION_ADD(0x8f, 1, cb_res_r8, 1, REG_A, "RES 1, A"),

    /* 0x90 */
    INSTRUCTION_ADD(0x90, 1, cb_res_r8, 2, REG_B, "RES 2, B"),
    INSTRUCTION_ADD(0x91, 1, cb_res_r8, 2, REG_C, "RES 2, C"),
    INSTRUCTION_ADD(0x92, 1, cb_res_r8, 2, REG_D, "RES 2, D"),
    INSTRUCTION_ADD(0x93, 1, cb_res_r8, 2, REG_E, "RES 2, E"),
    INSTRUCTION_ADD(0x94, 1, cb_res_r8, 2, REG_H, "RES 2, H"),
    INSTRUCTION_ADD(0x95, 1, cb_res_r8, 2, REG_L, "RES 2, L"),
    INSTRUCTION_ADD(0x96, 1, cb_res_m16, 2, REG_HL, "RES 2, (HL)"),
    INSTRUCTION_ADD(0x97, 1, cb_res_r8, 2, REG_A, "RES 2, A"),
    INSTRUCTION_ADD(0x98, 1, cb_res_r8, 3, REG_B, "RES 3, B"),
    INSTRUCTION_ADD(0x99, 1, cb_res_r8, 3, REG_C, "RES 3, C"),
    INSTRUCTION_ADD(0x9a, 1, cb_res_r8, 3, REG_D, "RES 3, D"),
    INSTRUCTION_ADD(0x9b, 1, cb_res_r8, 3, REG_E, "RES 3, E"),
    INSTRUCTION_ADD(0x9c, 1, cb_res_r8, 3, REG_H, "RES 3, H"),
    INSTRUCTION_ADD(0x9d, 1, cb_res_r8, 3, REG_L, "RES 3, L"),
    INSTRUCTION_ADD(0x9e, 1, cb_res_m16, 3, REG_HL, "RES 3, (HL)"),
    INSTRUCTION_ADD(0x9f, 1, cb_res_r8, 3, REG_A, "RES 3, A"),

    /* 0xa0 */
    INSTRUCTION_ADD(0xa0, 1, cb_res_r8, 4, REG_B, "RES 4, B"),
    INSTRUCTION_ADD(0xa1, 1, cb_res_r8, 4, REG_C, "RES 4, C"),
    INSTRUCTION_ADD(0xa2, 1, cb_res_r8, 4, REG_D, "RES 4, D"),
    INSTRUCTION_ADD(0xa3, 1, cb_res_r8, 4, REG_E, "RES 4, E"),
    INSTRUCTION_ADD(0xa4, 1, cb_res_r8, 4, REG_H, "RES 4, H"),
    INSTRUCTION_ADD(0xa5, 1, cb_res_r8, 4, REG_L, "RES 4, L"),
    INSTRUCTION_ADD(0xa6, 1, cb_res_m16, 4, REG_HL, "RES 4, (HL)"),
    INSTRUCTION_ADD(0xa7, 1, cb_res_r8, 4, REG_A, "RES 4, A"),
    INSTRUCTION_ADD(0xa8, 1, cb_res_r8, 5, REG_B, "RES 5, B"),
    INSTRUCTION_ADD(0xa9, 1, cb_res_r8, 5, REG_C, "RES 5, C"),
    INSTRUCTION_ADD(0xaa, 1, cb_res_r8, 5, REG_D, "RES 5, D"),
    INSTRUCTION_ADD(0xab, 1, cb_res_r8, 5, REG_E, "RES 5, E"),
    INSTRUCTION_ADD(0xac, 1, cb_res_r8, 5, REG_H, "RES 5, H"),
    INSTRUCTION_ADD(0xad, 1, cb_res_r8, 5, REG_L, "RES 5, L"),
    INSTRUCTION_ADD(0xae, 1, cb_res_m16, 5, REG_HL, "RES 5, (HL)"),
    INSTRUCTION_ADD(0xaf, 1, cb_res_r8, 5, REG_A, "RES 5, A"),

    /* 0xb0 */
    INSTRUCTION_ADD(0xb0, 1, cb_res_r8, 6, REG_B, "RES 6, B"),
    INSTRUCTION_ADD(0xb1, 1, cb_res_r8, 6, REG_C, "RES 6, C"),
    INSTRUCTION_ADD(0xb2, 1, cb_res_r8, 6, REG_D, "RES 6, D"),
    INSTRUCTION_ADD(0xb3, 1, cb_res_r8, 6, REG_E, "RES 6, E"),
    INSTRUCTION_ADD(0xb4, 1, cb_res_r8, 6, REG_H, "RES 6, H"),
    INSTRUCTION_ADD(0xb5, 1, cb_res_r8, 6, REG_L, "RES 6, L"),
    INSTRUCTION_ADD(0xb6, 1, cb_res_m16, 6, REG_HL, "RES 6, (HL)"),
    INSTRUCTION_ADD(0xb7, 1, cb_res_r8, 6, REG_A, "RES 6, A"),
    INSTRUCTION_ADD(0xb8, 1, cb_res_r8, 7, REG_B, "RES 7, B"),
    INSTRUCTION_ADD(0xb9, 1, cb_res_r8, 7, REG_C, "RES 7, C"),
    INSTRUCTION_ADD(0xba, 1, cb_res_r8, 7, REG_D, "RES 7, D"),
    INSTRUCTION_ADD(0xbb, 1, cb_res_r8, 7, REG_E, "RES 7, E"),
    INSTRUCTION_ADD(0xbc, 1, cb_res_r8, 7, REG_H, "RES 7, H"),
    INSTRUCTION_ADD(0xbd, 1, cb_res_r8, 7, REG_L, "RES 7, L"),
    INSTRUCTION_ADD(0xbe, 1, cb_res_m16, 7, REG_HL, "RES 7, (HL)"),
    INSTRUCTION_ADD(0xbf, 1, cb_res_r8, 7, REG_A, "RES 7, A"),            

    /* 0xc0 */
    INSTRUCTION_ADD(0xc0, 1, cb_set_r8, 0, REG_B, "SET 0, B"),
    INSTRUCTION_ADD(0xc1, 1, cb_set_r8, 0, REG_C, "SET 0, C"),
    INSTRUCTION_ADD(0xc2, 1, cb_set_r8, 0, REG_D, "SET 0, D"),
    INSTRUCTION_ADD(0xc3, 1, cb_set_r8, 0, REG_E, "SET 0, E"),
    INSTRUCTION_ADD(0xc4, 1, cb_set_r8, 0, REG_H, "SET 0, H"),
    INSTRUCTION_ADD(0xc5, 1, cb_set_r8, 0, REG_L, "SET 0, L"),
    INSTRUCTION_ADD(0xc6, 1, cb_set_m16, 0, REG_HL, "SET 0, (HL)"),
    INSTRUCTION_ADD(0xc7, 1, cb_set_r8, 0, REG_A, "SET 0, A"),
    INSTRUCTION_ADD(0xc8, 1, cb_set_r8, 1, REG_B, "SET 1, B"),
    INSTRUCTION_ADD(0xc9, 1, cb_set_r8, 1, REG_C, "SET 1, C"),
    INSTRUCTION_ADD(0xca, 1, cb_set_r8, 1, REG_D, "SET 1, D"),
    INSTRUCTION_ADD(0xcb, 1, cb_set_r8, 1, REG_E, "SET 1, E"),
    INSTRUCTION_ADD(0xcc, 1, cb_set_r8, 1, REG_H, "SET 1, H"),
    INSTRUCTION_ADD(0xcd, 1, cb_set_r8, 1, REG_L, "SET 1, L"),
    INSTRUCTION_ADD(0xce, 1, cb_set_m16, 1, REG_HL, "SET 1, (HL)"),
    INSTRUCTION_ADD(0xcf, 1, cb_set_r8, 1, REG_A, "SET 1, A"),

    /* 0xd0 */
    INSTRUCTION_ADD(0xd0, 1, cb_set_r8, 2, REG_B, "SET 2, B"),
    INSTRUCTION_ADD(0xd1, 1, cb_set_r8, 2, REG_C, "SET 2, C"),
    INSTRUCTION_ADD(0xd2, 1, cb_set_r8, 2, REG_D, "SET 2, D"),
    INSTRUCTION_ADD(0xd3, 1, cb_set_r8, 2, REG_E, "SET 2, E"),
    INSTRUCTION_ADD(0xd4, 1, cb_set_r8, 2, REG_H, "SET 2, H"),
    INSTRUCTION_ADD(0xd5, 1, cb_set_r8, 2, REG_L, "SET 2, L"),
    INSTRUCTION_ADD(0xd6, 1, cb_set_m16, 2, REG_HL, "SET 2, (HL)"),
    INSTRUCTION_ADD(0xd7, 1, cb_set_r8, 2, REG_A, "SET 2, A"),
    INSTRUCTION_ADD(0xd8, 1, cb_set_r8, 3, REG_B, "SET 3, B"),
    INSTRUCTION_ADD(0xd9, 1, cb_set_r8, 3, REG_C, "SET 3, C"),
    INSTRUCTION_ADD(0xda, 1, cb_set_r8, 3, REG_D, "SET 3, D"),
    INSTRUCTION_ADD(0xdb, 1, cb_set_r8, 3, REG_E, "SET 3, E"),
    INSTRUCTION_ADD(0xdc, 1, cb_set_r8, 3, REG_H, "SET 3, H"),
    INSTRUCTION_ADD(0xdd, 1, cb_set_r8, 3, REG_L, "SET 3, L"),
    INSTRUCTION_ADD(0xde, 1, cb_set_m16, 3, REG_HL, "SET 3, (HL)"),
    INSTRUCTION_ADD(0xdf, 1, cb_set_r8, 3, REG_A, "SET 3, A"),

    /* 0xe0 */
    INSTRUCTION_ADD(0xe0, 1, cb_set_r8, 4, REG_B, "SET 4, B"),
    INSTRUCTION_ADD(0xe1, 1, cb_set_r8, 4, REG_C, "SET 4, C"),
    INSTRUCTION_ADD(0xe2, 1, cb_set_r8, 4, REG_D, "SET 4, D"),
    INSTRUCTION_ADD(0xe3, 1, cb_set_r8, 4, REG_E, "SET 4, E"),
    INSTRUCTION_ADD(0xe4, 1, cb_set_r8, 4, REG_H, "SET 4, H"),
    INSTRUCTION_ADD(0xe5, 1, cb_set_r8, 4, REG_L, "SET 4, L"),
    INSTRUCTION_ADD(0xe6, 1, cb_set_m16, 4, REG_HL, "SET 4, (HL)"),
    INSTRUCTION_ADD(0xe7, 1, cb_set_r8, 4, REG_A, "SET 4, A"),
    INSTRUCTION_ADD(0xe8, 1, cb_set_r8, 5, REG_B, "SET 5, B"),
    INSTRUCTION_ADD(0xe9, 1, cb_set_r8, 5, REG_C, "SET 5, C"),
    INSTRUCTION_ADD(0xea, 1, cb_set_r8, 5, REG_D, "SET 5, D"),
    INSTRUCTION_ADD(0xeb, 1, cb_set_r8, 5, REG_E, "SET 5, E"),
    INSTRUCTION_ADD(0xec, 1, cb_set_r8, 5, REG_H, "SET 5, H"),
    INSTRUCTION_ADD(0xed, 1, cb_set_r8, 5, REG_L, "SET 5, L"),
    INSTRUCTION_ADD(0xee, 1, cb_set_m16, 5, REG_HL, "SET 5, (HL)"),
    INSTRUCTION_ADD(0xef, 1, cb_set_r8, 5, REG_A, "SET 5, A"),

    /* 0xf0 */
    INSTRUCTION_ADD(0xf0, 1, cb_set_r8, 6, REG_B, "SET 6, B"),
    INSTRUCTION_ADD(0xf1, 1, cb_set_r8, 6, REG_C, "SET 6, C"),
    INSTRUCTION_ADD(0xf2, 1, cb_set_r8, 6, REG_D, "SET 6, D"),
    INSTRUCTION_ADD(0xf3, 1, cb_set_r8, 6, REG_E, "SET 6, E"),
    INSTRUCTION_ADD(0xf4, 1, cb_set_r8, 6, REG_H, "SET 6, H"),
    INSTRUCTION_ADD(0xf5, 1, cb_set_r8, 6, REG_L, "SET 6, L"),
    INSTRUCTION_ADD(0xf6, 1, cb_set_m16, 6, REG_HL, "SET 6, (HL)"),
    INSTRUCTION_ADD(0xf7, 1, cb_set_r8, 6, REG_A, "SET 6, A"),
    INSTRUCTION_ADD(0xf8, 1, cb_set_r8, 7, REG_B, "SET 7, B"),
    INSTRUCTION_ADD(0xf9, 1, cb_set_r8, 7, REG_C, "SET 7, C"),
    INSTRUCTION_ADD(0xfa, 1, cb_set_r8, 7, REG_D, "SET 7, D"),
    INSTRUCTION_ADD(0xfb, 1, cb_set_r8, 7, REG_E, "SET 7, E"),
    INSTRUCTION_ADD(0xfc, 1, cb_set_r8, 7, REG_H, "SET 7, H"),
    INSTRUCTION_ADD(0xfd, 1, cb_set_r8, 7, REG_L, "SET 7, L"),
    INSTRUCTION_ADD(0xfe, 1, cb_set_m16, 7, REG_HL, "SET 7, (HL)"),
    INSTRUCTION_ADD(0xff, 1, cb_set_r8, 7, REG_A, "SET 7, A"),            
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

instruction_t 
decode(uint8_t *data)
{
    uint8_t opcode = data[0];    
    int prefix_size = 0;
    instruction_t *inst_set = instruction_set;

    if (opcode == PREFIX_CB) {        
        inst_set = prefixed_instruction_set;
        prefix_size = 1;
        opcode = data[1];
    }
    
    instruction_t inst = inst_set[opcode];
    inst.opcode = opcode;

    if (inst.size != 1) {
        if (inst.size == 2) {
            inst.opcode_ext.a8 = READ_8(*(data + 1));
        } else if (inst.size == 3) {
            /* immediate value is little-endian */
            inst.opcode_ext.n16 = READ_16(*(uint16_t*)(data + 1));
        } else {
            LOG_ERROR("Invalid instruction, imme size [%d]", inst.size);
            abort();
        }
    }

    inst.size += prefix_size;

    return inst;
}

#ifdef DEBUG
#include "memory.h"
#include <assert.h>

static uint8_t _flat_mem[0xffff]; // 64KB

static uint8_t 
_mem_write(uint16_t addr, uint8_t data)
{
    printf("Writing to memory at address %x [%x]\n", addr, data);
    _flat_mem[addr] = data;
    return data;
}

static uint8_t 
_mem_read(uint16_t addr)
{
    printf("Reading from memory at address %x\n", addr);
    return _flat_mem[addr];
}

void 
test_inc_r16(gbc_cpu_t *cpu)
{   
    cpu_register_t *reg = &(cpu->regs);
    WRITE_R16(reg, REG_BC, 0x1234);
    uint8_t code[] = {0x03}; // INC BC
    instruction_t inst = decode(code);
    inst.func(cpu, &inst);
    assert(READ_R16(reg, REG_BC) == 0x1235);

    WRITE_R16(reg, REG_BC, 0xffff);    
    inst.func(cpu, &inst);
    assert(READ_R16(reg, REG_BC) == 0x0000);
}

void 
test_dec_r16(gbc_cpu_t *cpu)
{
    cpu_register_t *reg = &(cpu->regs);
    WRITE_R16(reg, REG_BC, 0x1234);
    uint8_t code[] = {0x0b}; // DEC BC
    instruction_t inst = decode(code);
    inst.func(cpu, &inst);
    assert(READ_R16(reg, REG_BC) == 0x1233);

    WRITE_R16(reg, REG_BC, 0x0000);
    inst.func(cpu, &inst);
    assert(READ_R16(reg, REG_BC) == 0xffff);    
}

void
test_inc_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *reg = &(cpu->regs);
    WRITE_R8(reg, REG_B, 0x34);
    uint8_t code[] = {0x04}; // INC B

    instruction_t inst = decode(code);
    inst.func(cpu, &inst);

    assert(READ_R8(reg, REG_B) == 0x35);
    assert(READ_R_FLAG(reg, FLAG_Z) == 0);
    assert(READ_R_FLAG(reg, FLAG_N) == 0);
    assert(READ_R_FLAG(reg, FLAG_H) == 0);

    WRITE_R8(reg, REG_B, 0xff);

    uint8_t h = READ_R8(reg, REG_C);

    inst = decode(code);
    inst.func(cpu, &inst);
    
    assert(READ_R8(reg, REG_B) == 0);
    assert(READ_R8(reg, REG_C) == h);
    assert(READ_R_FLAG(reg, FLAG_Z) == 1);
    assert(READ_R_FLAG(reg, FLAG_N) == 0);
    assert(READ_R_FLAG(reg, FLAG_H) == 1);
}

void
test_dec_r8(gbc_cpu_t *cpu)
{
    cpu_register_t *reg = &(cpu->regs);
    WRITE_R8(reg, REG_B, 0x01);
    uint8_t code[] = {0x05}; // DEC B

    instruction_t inst = decode(code);
    inst.func(cpu, &inst);

    assert(READ_R8(reg, REG_B) == 0);
    assert(READ_R_FLAG(reg, FLAG_Z) == 1);
    assert(READ_R_FLAG(reg, FLAG_N) == 1);
    //assert(READ_R_FLAG(reg, FLAG_H) == 0);

    WRITE_R8(reg, REG_B, 0x00);

    uint8_t h = READ_R8(reg, REG_C);

    inst = decode(code);
    inst.func(cpu, &inst);
    
    assert(READ_R8(reg, REG_B) == 0xff);
    assert(READ_R8(reg, REG_C) == h);
    assert(READ_R_FLAG(reg, FLAG_Z) == 0);
    assert(READ_R_FLAG(reg, FLAG_N) == 1);
    assert(READ_R_FLAG(reg, FLAG_H) == 1);
}

void 
test_inc_m16(gbc_cpu_t *cpu)
{
    cpu_register_t *reg = &(cpu->regs);
    const uint16_t addr = 0x34;
    WRITE_R16(reg, REG_HL, addr);
    cpu->mem_write(addr, 0x34);
    uint8_t code[] = {0x34}; // INC HL

    instruction_t inst = decode(code);
    inst.func(cpu, &inst);

    assert(cpu->mem_read(addr) == 0x35);
    assert(READ_R_FLAG(reg, FLAG_Z) == 0);
    assert(READ_R_FLAG(reg, FLAG_N) == 0);
    assert(READ_R_FLAG(reg, FLAG_H) == 0);

    cpu->mem_write(addr, 0xff);
    uint8_t h = READ_R8(reg, REG_C);

    inst = decode(code);
    inst.func(cpu, &inst);
    
    assert(cpu->mem_read(addr) == 0);        
    assert(READ_R_FLAG(reg, FLAG_Z) == 1);
    assert(READ_R_FLAG(reg, FLAG_N) == 0);
    assert(READ_R_FLAG(reg, FLAG_H) == 1);
}

void 
test_instructions() 
{
    gbc_cpu_t cpu = gbc_cpu_new(); 
    gbc_memory_t mem = gbc_mem_new();
    mem.read = _mem_read;
    mem.write = _mem_write;

    cpu.mem_read = mem.read;
    cpu.mem_write = mem.write;

    test_inc_r16(&cpu);
    test_inc_r8(&cpu);
    test_inc_m16(&cpu);

    test_dec_r16(&cpu);
    test_dec_r8(&cpu);
    test_inc_m16(&cpu);    
}

#endif
