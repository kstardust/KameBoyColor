#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "cpu.h"
#include "common.h"
#include "cartridge.h"
#include "instruction_set.h"

int main()
{
    init_instruction_set();
    
    gbc_cpu_t cpu = gbc_cpu_new();    
    FILE* cartridge = fopen("C:\\Users\\liqilong\\Desktop\\Dev\\gbc\\test.gbc", "rb");
    if (!cartridge) {
        printf("Failed to open cartridge\n");
        return 1;
    }

    fseek(cartridge, 0, SEEK_END);
    size_t size = ftell(cartridge);
    rewind(cartridge);

    uint8_t *data = (uint8_t*)malloc_memory(size);
    if (!data) {
        printf("Failed to allocate memory\n");
        return 1;
    }

    size_t n = fread(data, 1, size, cartridge);
    fclose(cartridge);

    cartridge_t *cart = cartridge_load((uint8_t*)data);
    
    WRITE_16(cpu.regs.R_AF.AF, 0xff00);    
    printf("%x %x %x\n", READ_16(cpu.regs.R_AF.AF), READ_8(cpu.regs.R_AF.pair.A), READ_8(cpu.regs.R_AF.pair.F));
    cpu.regs.R_BC.pair.B = 2;    

    // test register 
    WRITE_16(cpu.regs.PC, 0x1);
    WRITE_16(cpu.regs.SP, 0xff);
    WRITE_16(cpu.regs.R_AF.AF, 0x2222);
    WRITE_16(cpu.regs.R_BC.BC, 0x1234);
    WRITE_16(cpu.regs.R_DE.DE, 0x5678);
    WRITE_16(cpu.regs.R_HL.HL, 0x9abc);
    
    cpu_register_t *r = &cpu.regs;
    assert (READ_R16(r, REG_PC) == 0x1);
    assert (READ_R16(r, REG_SP) == 0xff);
    assert (READ_R16(r, REG_AF) == 0x2222);
    assert (READ_R16(r, REG_BC) == 0x1234);
    assert (READ_R16(r, REG_DE) == 0x5678);
    assert (READ_R16(r, REG_HL) == 0x9abc);

    WRITE_8(cpu.regs.R_AF.pair.A, 0x12);
    WRITE_8(cpu.regs.R_AF.pair.F, 0x34);
    WRITE_8(cpu.regs.R_BC.pair.B, 0x56);
    WRITE_8(cpu.regs.R_BC.pair.C, 0x78);
    WRITE_8(cpu.regs.R_DE.pair.D, 0x9a);
    WRITE_8(cpu.regs.R_DE.pair.E, 0xbc);
    WRITE_8(cpu.regs.R_HL.pair.H, 0xde);
    WRITE_8(cpu.regs.R_HL.pair.L, 0xf0);

    assert (READ_R8(r, REG_A) == 0x12);
    assert (READ_R8(r, REG_F) == 0x34);
    assert (READ_R8(r, REG_B) == 0x56);
    assert (READ_R8(r, REG_C) == 0x78);
    assert (READ_R8(r, REG_D) == 0x9a);
    assert (READ_R8(r, REG_E) == 0xbc);
    assert (READ_R8(r, REG_H) == 0xde);
    assert (READ_R8(r, REG_L) == 0xf0);

    WRITE_R16(r, REG_PC, 0x2);
    WRITE_R16(r, REG_SP, 0x100);
    WRITE_R16(r, REG_AF, 0x3333);
    WRITE_R16(r, REG_BC, 0x5678);    
    WRITE_R16(r, REG_DE, 0x9abc);
    WRITE_R16(r, REG_HL, 0xdef0);

    assert (READ_R16(r, REG_PC) == 0x2);
    assert (READ_R16(r, REG_SP) == 0x100);
    assert (READ_R16(r, REG_AF) == 0x3333);
    assert (READ_R16(r, REG_BC) == 0x5678);
    assert (READ_R16(r, REG_DE) == 0x9abc);
    assert (READ_R16(r, REG_HL) == 0xdef0);

    WRITE_R8(r, REG_A, 0x12);
    WRITE_R8(r, REG_F, 0x34);
    WRITE_R8(r, REG_B, 0x56);
    WRITE_R8(r, REG_C, 0x78);
    WRITE_R8(r, REG_D, 0x9a);
    WRITE_R8(r, REG_E, 0xbc);
    WRITE_R8(r, REG_H, 0xde);
    WRITE_R8(r, REG_L, 0xf0);

    assert (READ_R8(r, REG_A) == 0x12);
    assert (READ_R8(r, REG_F) == 0x34);
    assert (READ_R8(r, REG_B) == 0x56);
    assert (READ_R8(r, REG_C) == 0x78);
    assert (READ_R8(r, REG_D) == 0x9a);
    assert (READ_R8(r, REG_E) == 0xbc);
    assert (READ_R8(r, REG_H) == 0xde);
    assert (READ_R8(r, REG_L) == 0xf0);        

    CLEAR_R_FLAG(r, FLAG_Z);
    CLEAR_R_FLAG(r, FLAG_N);
    CLEAR_R_FLAG(r, FLAG_H);
    CLEAR_R_FLAG(r, FLAG_C);

    assert (!READ_R_FLAG(r, FLAG_Z));
    assert (!READ_R_FLAG(r, FLAG_N));
    assert (!READ_R_FLAG(r, FLAG_H));
    assert (!READ_R_FLAG(r, FLAG_C));

    SET_R_FLAG(r, FLAG_Z);
    SET_R_FLAG(r, FLAG_N);

    assert (READ_R_FLAG(r, FLAG_Z));
    assert (READ_R_FLAG(r, FLAG_N));
    assert (!READ_R_FLAG(r, FLAG_H));
    assert (!READ_R_FLAG(r, FLAG_C));

    CLEAR_R_FLAG(r, FLAG_Z);
    CLEAR_R_FLAG(r, FLAG_N);
    SET_R_FLAG(r, FLAG_H);
    SET_R_FLAG(r, FLAG_C);

    assert (READ_R_FLAG(r, FLAG_Z) == 0);
    assert (READ_R_FLAG(r, FLAG_N) == 0);
    assert (READ_R_FLAG(r, FLAG_H) == 1);
    assert (READ_R_FLAG(r, FLAG_C) == 1);


/*     printf("c %d a %d b %d fz %d fn %d\n", 
    cpu.regs.r_bc.pair.c, cpu.regs.r_af.pair.a, cpu.regs.r_bc.pair.b, cpu.regs.r_af.pair.f.z, cpu.regs.r_af.pair.f.n);
 */ 
    return 0;
}