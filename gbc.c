#include "gbc.h"
#include "instruction_set.h"

int
gbc_init(gbc_t *gbc)
{
    init_instruction_set();

    gbc_mem_init(&gbc->mem);
    gbc_cpu_init(&gbc->cpu);
    gbc_mbc_init(&gbc->mbc);    
    gbc_io_init(&gbc->io);
    gbc_graphic_init(&gbc->graphic);

    gbc_cpu_connect(&gbc->cpu, &gbc->mem);
    gbc_mbc_connect(&gbc->mbc, &gbc->mem);
    gbc_io_connect(&gbc->io, &gbc->mem);
    gbc_graphic_connect(&gbc->graphic, &gbc->mem);

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
print_stat(gbc_t *gbc)
{
    gbc_cpu_t *cpu = &gbc->cpu;
    cpu_register_t *r = &cpu->regs;

    printf("{PC: 0x%x, SP: 0x%x, AF: 0x%x, BC: 0x%x, DE: 0x%x, HL: 0x%x, C: %d, Z: %d, N: %d, H: %d}\n",
           READ_R16(r, REG_PC), READ_R16(r, REG_SP), READ_R16(r, REG_AF),
           READ_R16(r, REG_BC), READ_R16(r, REG_DE), READ_R16(r, REG_HL),
           READ_R_FLAG(r, FLAG_C), READ_R_FLAG(r, FLAG_Z), READ_R_FLAG(r, FLAG_N), READ_R_FLAG(r, FLAG_H));
}

void
gbc_run(gbc_t *gbc)
{                
    for (;;) {

        //uint64_t t = get_time();
        gbc_cpu_cycle(&gbc->cpu);  
        
        /* TODO compensate for the cost longer than CLOCK_CYCLE */
        //while (get_time() - t < CLOCK_CYCLE)
        //    ;

        #ifdef DEBUG
        print_stat(gbc);        
        #endif
    }    
}