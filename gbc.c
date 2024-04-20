#include "gbc.h"
#include "cpu.h"
#include "instruction_set.h"

int
gbc_init(gbc_t *gbc)
{
    init_instruction_set();

    gbc_cpu_init(&gbc->cpu);
    gbc_mbc_init(&gbc->mbc);
    gbc_mem_init(&gbc->mem);

    gbc_cpu_connect(&gbc->cpu, &gbc->mem);
    gbc_mbc_connect(&gbc->mbc, &gbc->mem);

    FILE* cartridge = fopen("/Users/Kevin/Development/GBC/tetris_dx.gbc", "rb");
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
    gbc_mbc_init_with_cart(&gbc->mbc, cart);

    if (!cart) {
        printf("Failed to load cartridge\n");
        return 1;
    }

    return 0;
}

void
gbc_run(gbc_t *gbc)
{                
    for (;;) {

        uint64_t t = get_time();
        gbc_cpu_cycle(&gbc->cpu);
        
        /* TODO compensate for the cost longer than CLOCK_CYCLE */
        while (get_time() - t < CLOCK_CYCLE)            
            ;
    }    
}