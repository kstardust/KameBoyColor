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
#include "rom_dialog.h"

#define USEAGE "Usage: xgbc -r cartridge [-b boot_rom]\n" \
                "  cartridge: path to the gameboy cartridge file\n" \
                "  boot_rom(optional): path to the boot rom\n"

static void
parse_args(int argc, char **argv, char **cartridge, char **boot_rom)
{
    if (argc < 2) {
        printf(USEAGE);
        exit(1);
    }

    *cartridge = NULL;
    *boot_rom = NULL;
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        if (arg[0] != '-') {
            continue;
        }

        switch (arg[1]) {
        case 'r':
            if (++i < argc) {
                *cartridge = argv[i];
            } else {
                printf(USEAGE);
                exit(1);
            }
            break;
        case 'b':
            if (++i < argc) {
                *boot_rom = argv[i];
            } else {
                printf(USEAGE);
                exit(1);
            }
            break;
        default:
            printf(USEAGE);
            exit(1);
            break;
        }
    }
    if (*cartridge == NULL) {
        printf(USEAGE);
        exit(1);
    }
}

static void
close_callback(void *udata)
{
    gbc_t *gbc = (gbc_t*)udata;
    gbc->running = 0;
}

int
main(int argc, char **argv)
{
    GuiInit();

    char* cartridge = NULL;
    char* boot_rom = NULL;
    while (RomDialog(&cartridge, &boot_rom))
        ;

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


/* TEST ROMS
    SOUND
    //char* cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cgb_sound/rom_singles/01-registers.gb"; // ok
    //char* cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cgb_sound/rom_singles/02-len ctr.gb"; // ok
    //char* cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cgb_sound/rom_singles/03-trigger.gb"; // ok
    //char* cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cgb_sound/rom_singles/04-sweep.gb"; // ok
    //char* cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cgb_sound/rom_singles/05-sweep details.gb"; // ok
    //char* cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cgb_sound/rom_singles/06-overflow on trigger.gb"; // ok
    //char* cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cgb_sound/rom_singles/07-len sweep period sync.gb"; // ok
    //char* cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cgb_sound/rom_singles/08-len ctr during power.gb"; // ok
    //char* cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cgb_sound/rom_singles/09-wave read while on.gb"; // ok
    //char* cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cgb_sound/rom_singles/10-wave trigger while on.gb"; // ok
    //char* cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cgb_sound/rom_singles/10-wave trigger while on.gb"; // ok
    // char* cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cgb_sound/rom_singles/11-regs after power.gb"; // ok
    //char* cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cgb_sound/rom_singles/12-wave.gb"; // ok
    //char* cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cgb_sound/cgb_sound.gb"; // ok

    CPU
    // cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/individual/01-special.gb"; // OK
    // cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/individual/02-interrupts.gb"; // OK
    // cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/individual/03-op sp,hl.gb"; // OK
    // cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/individual/04-op r,imm.gb"; // OK
    // cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/individual/05-op rp.gb"; // OK
    // cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/individual/06-ld r,r.gb"; // OK
    // cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb"; // ok
    // cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/individual/08-misc instrs.gb"; // OK
    // cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/individual/09-op r,r.gb"; // OK
    // cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/individual/10-bit ops.gb"; // OK
    // cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/individual/11-op a,(hl).gb"; // OK
    // cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/cpu_instrs/cpu_instrs.gb"; // OK
    // cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/instr_timing/instr_timing.gb"; // ok
    // cartridge = "/Users/Kevin/Development/GBC/gb-test-roms/interrupt_time/interrupt_time.gb"; // ok
*/