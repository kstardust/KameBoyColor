#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "cpu.h"
#include "memory.h"
#include "mbc.h"
#include "common.h"
#include "cartridge.h"
#include "instruction_set.h"

#include "gui.h"

#ifndef RUN_TEST

void close_callback(void *udata)
{
    gbc_t *gbc = (gbc_t*)udata;
    gbc->running = 0;
}

int main()
{
    GuiInit();

    #if defined (WIN32)
    char* cartridge = "C:\\Users\\lql97\\Dev\\GameBoyColor\\tetris_dx.gbc";
    char* boot_rom = NULL;
    #else

    //char* cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cgb_sound/rom_singles/01-registers.gb"; // ok
    //char* cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cgb_sound/rom_singles/02-len ctr.gb"; // ok
    //char* cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cgb_sound/rom_singles/03-trigger.gb"; // ok
    //char* cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cgb_sound/rom_singles/04-sweep.gb"; // ok
    //char* cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cgb_sound/rom_singles/05-sweep details.gb"; // ok
    //char* cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cgb_sound/rom_singles/06-overflow on trigger.gb"; // ok
    char* cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cgb_sound/rom_singles/07-len sweep period sync.gb";
    //char* cartridge = "/Users/Kevin/Development/GBC/tetris_dx.gbc";
    char* boot_rom = NULL;//"/Users/Kevin/Development/GBC/gb-test-roms/cgb_boot.bin";

    // FILE* cartridge = fopen("/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/individual/01-special.gb", "rb"); // OK

    //char* boot_rom = NULL;//"/Users/Kevin/Development/GBC/gb-test-roms/cgb_boot.bin";

    // FILE* cartridge = fopen("/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/individual/01-special.gb", "rb"); // OK
    // FILE* cartridge = fopen("/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/individual/02-interrupts.gb", "rb"); // OK
    // FILE* cartridge = fopen("/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/individual/03-op sp,hl.gb", "rb"); // OK
    // FILE* cartridge = fopen("/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/individual/04-op r,imm.gb", "rb"); // OK
    // FILE* cartridge = fopen("/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/individual/05-op rp.gb", "rb"); // OK
    // FILE* cartridge = fopen("/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/individual/06-ld r,r.gb", "rb"); // OK
    // FILE* cartridge = fopen("/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb", "rb");
    // FILE* cartridge = fopen("/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/individual/08-misc instrs.gb", "rb"); // OK
    // FILE* cartridge = fopen("/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/individual/09-op r,r.gb", "rb"); // OK
    // FILE* cartridge = fopen("/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/individual/10-bit ops.gb", "rb"); // OK
    // FILE* cartridge = fopen("/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/individual/11-op a,(hl).gb", "rb"); // OK
    // FILE* cartridge = fopen("/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/cpu_instrs.gb", "rb"); // OK
    // FILE* cartridge = fopen("/Users/Kevin/Development/GBC/gb-test-roms/instr_timing/instr_timing.gb", "rb"); // OK
    // FILE* cartridge = fopen("/Users/Kevin/Development/GBC/gb-test-roms/interrupt_time/interrupt_time.gb", "rb");
    #endif
    gbc_t gbc;
    if (gbc_init(&gbc, cartridge, boot_rom) == 0) {
        GuiSetCloseCallback(close_callback);
        GuiSetUserData(&gbc);
        gbc.io.poll_keypad = GuiPollKeypad;
        gbc.graphic.screen_write = GuiWrite;
        gbc.graphic.screen_update = GuiUpdate;
        gbc.audio.audio_write = GuiAudioWrite;
        gbc.audio.audio_update = GuiAudioUpdate;
        gbc_run(&gbc);
    }

    LOG_INFO("Emulator terminated\n");

    GuiDestroy();
    return 0;
}

#else

void test();
int main()
{
    init_instruction_set();
    test();
    return 0;
}

void
test()
{
    gbc_t gbc;
    gbc_init(&gbc);
    gbc_memory_t mem = gbc.mem;
    gbc_cpu_t cpu = gbc.cpu;
    cpu.mem_write = mem.write;
    cpu.mem_read = mem.read;

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

    test_instructions();
    return;
}

#endif