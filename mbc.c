#include "mbc.h"

/* 
 * We should dynmically allocate these space, but considering the future plan
 * of running the emulator on a bare-metal RPi(with no OS, thus no malloc), I
 * chose to use static arrays. With the sacrifice of only one mbc instance can 
 * be created at a time.
 */

static uint8_t _rom_banks[MAX_ROM_BANKS * ROM_BANK_SIZE];
static uint8_t _ram_banks[MAX_RAM_BANKS * RAM_BANK_SIZE];

uint8_t mbc1_read(gbc_mbc_t *mbc, uint16_t addr);
uint8_t mbc1_write(gbc_mbc_t *mbc, uint16_t addr, uint8_t data);

uint8_t mbc3_read(gbc_mbc_t *mbc, uint16_t addr);
uint8_t mbc3_write(gbc_mbc_t *mbc, uint16_t addr, uint8_t data);


uint8_t mbc_read(void *udata, uint16_t addr)
{
    gbc_mbc_t *mbc = (gbc_mbc_t*)udata;
    return mbc->read(mbc, addr);
}

uint8_t mbc_write(void *udata, uint16_t addr, uint8_t data)
{
    gbc_mbc_t *mbc = (gbc_mbc_t*)udata;
    return mbc->write(mbc, addr, data);    
}

gbc_mbc_t
gbc_mbc_new()
{      
    gbc_mbc_t mbc;
    mbc.rom_banks = _rom_banks;
    mbc.ram_banks = _ram_banks;
    mbc.rom_bank = 0;
    mbc.ram_bank = 0;
    mbc.ram_enabled = 0;
    mbc.mode = 0;
    mbc.mem = NULL;
    return mbc;
}

void
gbc_mbc_connect(gbc_mbc_t *mbc, gbc_memory_t *mem)
{
    mbc->mem = mem;

    /* Although BANK 0 is not inside MPC, but write to it will change the
     * registers in memory bank. */
    memory_map_entry_t entry;
    entry.id = ROM_BANK_0_ID;
    entry.addr_begin = ROM_BANK_0_BEGIN;
    entry.addr_end = ROM_BANK_0_END;
    entry.read = mbc_read;
    entry.write = mbc_write;
    entry.udata = mbc;    

    register_memory_map(mem, &entry);

    entry.id = ROM_BANK_N_ID;
    entry.addr_begin = ROM_BANK_N_BEGIN;
    entry.addr_end = ROM_BANK_N_END;
    entry.read = mbc_read;
    entry.write = mbc_write;
    entry.udata = mbc;

    register_memory_map(mem, &entry);    

    entry.id = WRAM_BANK_0_ID;
    entry.addr_begin = WRAM_BANK_0_BEGIN;
    entry.addr_end = WRAM_BANK_0_END;
    entry.read = mbc_read;
    entry.write = mbc_write;
    entry.udata = mbc;

    register_memory_map(mem, &entry);
}

void
gbc_mbc_init_with_cart(gbc_mbc_t *mbc, cartridge_t *cart)
{
    mbc->rom_bank_size = cart->rom_size > 0 ? (cart->rom_size << 1) : 0;

    uint8_t ram_size = 0;
    switch (cart->ram_size) {
        case 0: ram_size = 0; break;        
        case 2: ram_size = 1; break;
        case 3: ram_size = 4; break;
        case 4: ram_size = 16; break;
        case 5: ram_size = 8; break;
        default: 
            LOG_ERROR("Invalid RAM size %d\n", cart->ram_size);
            abort();
    }
    mbc->ram_bank_size = ram_size;
    mbc->rom_bank = 0;
    mbc->ram_bank = 0;
    mbc->ram_enabled = 0;
    mbc->mode = 0;
    mbc->type = cart->cartridge_type;

    switch (mbc->type)
    {
        case CART_TYPE_MBC1:
        case CART_TYPE_MBC1_RAM:
        case CART_TYPE_MBC1_RAM_BATTERY:        

            mbc->read = mbc1_read;
            mbc->write = mbc1_write;
            break;

        case CART_TYPE_MBC3:
        case CART_TYPE_MBC3_RAM:
        case CART_TYPE_MBC3_RAM_BATTERY:
        
            mbc->read = mbc3_read;
            mbc->write = mbc3_write;
            break;
            
        default:
            LOG_ERROR("Unsupported MBC type %d\n", mbc->type);
            abort();
    }
}

uint8_t
mbc1_read(gbc_mbc_t *mbc, uint16_t addr)
{
    LOG_DEBUG("Reading from MBC1 at address %x\n", addr);
    return 0;
}

uint8_t
mbc1_write(gbc_mbc_t *mbc, uint16_t addr, uint8_t data)
{
    LOG_DEBUG("Writing to MBC1 at address %x [%x]\n", addr, data);
    return data;
}

uint8_t
mbc3_read(gbc_mbc_t *mbc, uint16_t addr)
{
    LOG_DEBUG("Reading from MBC3 at address %x\n", addr);
    return 0;
}

uint8_t
mbc3_write(gbc_mbc_t *mbc, uint16_t addr, uint8_t data)
{
    LOG_DEBUG("Writing to MBC3 at address %x [%x]\n", addr, data);
    return data;
}

uint32_t
translate_mem_addr()
{
}
