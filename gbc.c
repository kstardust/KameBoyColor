#include "gbc.h"
#include "instruction_set.h"
#include "gui/gui.h"


void
gbc_load_boot_rom(gbc_t *gbc, const char *rom_path)
{
    FILE *rom = fopen(rom_path, "rb");    
    fread(gbc->mem.boot_rom, 1, GBC_BOOT_ROM_SIZE, rom);
    //fread(gbc->mbc.rom_banks, 1, GBC_BOOT_ROM_SIZE, rom);
    fclose(rom);
    gbc->mem.boot_rom_enabled = 1;
}

int
gbc_init(gbc_t *gbc, const char *game_rom, const char *boot_rom)
{
    init_instruction_set();

    gbc_mem_init(&gbc->mem);
    gbc_cpu_init(&gbc->cpu);
    gbc_mbc_init(&gbc->mbc);
    gbc_timer_init(&gbc->timer);
    gbc_io_init(&gbc->io);
    gbc_graphic_init(&gbc->graphic);

    gbc_cpu_connect(&gbc->cpu, &gbc->mem);
    gbc_mbc_connect(&gbc->mbc, &gbc->mem);
    gbc_timer_connect(&gbc->timer, &gbc->mem);
    gbc_io_connect(&gbc->io, &gbc->mem);
    gbc_graphic_connect(&gbc->graphic, &gbc->mem);

    FILE *cartridge = fopen(game_rom, "rb");

    if (!cartridge) {
        LOG_ERROR("Failed to open cartridge\n");
        return 1;
    }

    fseek(cartridge, 0, SEEK_END);
    size_t size = ftell(cartridge);
    rewind(cartridge);

    uint8_t *data = (uint8_t*)malloc_memory(size);
    if (!data) {
        LOG_ERROR("Failed to allocate memory\n");
        return 1;
    }

    size_t n = fread(data, 1, size, cartridge);
    fclose(cartridge);
    
    cartridge_t *cart = cartridge_load((uint8_t*)data);
    gbc_mbc_init_with_cart(&gbc->mbc, cart);
    gbc->mbc.rom_banks = data;

    if (!cart) {
        LOG_ERROR("Failed to load cartridge\n");
        return 1;
    }

    WRITE_R16(&gbc->cpu, REG_PC, 0x0100);

    /* initial values https://gbdev.io/pandocs/Power_Up_Sequence.html  */
    IO_PORT_WRITE(&(gbc->mem), IO_PORT_LCDC, 0x91);    

    if (boot_rom) {
        gbc_load_boot_rom(gbc, boot_rom);        /* boot rom starts at 0x0000 */
        WRITE_R16(&gbc->cpu, REG_PC, 0x0000);
    }

    gbc->running = 1;
    gbc->paused = 0;
    return 0;
}

void
gbc_run(gbc_t *gbc)
{                
    uint64_t lastf = get_time(), now = 0, delta = 0;
    uint64_t cycles = 0;    
    
    for (;;) {
        if (!gbc->running)
            break;        

        cycles = gbc->cpu.cycles;
        if (gbc->paused) {
                if (gbc->debug_steps == 0) {
                /* TODO: not a very good way, but we need to keep the GUI responsive */            
                gbc->graphic.screen_update(&gbc->graphic);
                continue;
            }
            /* forwards an instruction */
            if (gbc->debug_steps > 0 && gbc->cpu.ins_cycles == 1) {
                gbc->debug_steps--;
            }
        }

        now = get_time();
        delta = now - lastf;
        lastf = now;

        gbc_cpu_cycle(&gbc->cpu);
        gbc_timer_cycle(&gbc->timer);
        gbc_graphic_cycle(&gbc->graphic, delta);
        gbc_io_cycle(&gbc->io);  

        /* TODO compensate for the cost longer than CLOCK_CYCLE */
        //while (get_time() - t < CLOCK_CYCLE)
        //    ;
    }    
}