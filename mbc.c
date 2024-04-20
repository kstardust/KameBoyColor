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

#define IN_RANGE(addr, begin, end) ((addr) >= (begin) && (addr) <= (end))

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

void
gbc_mbc_init(gbc_mbc_t *mbc)
{      
    mbc->rom_banks = _rom_banks;
    mbc->ram_banks = _ram_banks;
    mbc->rom_bank = 0;
    mbc->ram_bank = 0;
    mbc->ram_enabled = 0;
    mbc->mode = 0;
    mbc->mem = NULL;    
}

void
gbc_mbc_connect(gbc_mbc_t *mbc, gbc_memory_t *mem)
{
    mbc->mem = mem;

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

    entry.id = EXRAM_ID;
    entry.addr_begin = EXRAM_BEGIN;
    entry.addr_end = EXRAM_END;
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
    mbc->cart = cart;

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

/*  
    I got confused by the descirption in the pandocs. 
    https://gbdev.io/pandocs/MBC1.html#60007fff--banking-mode-select-write-only

    Instead I consulted the gbdev wiki, which is more clear.
    https://gbdev.gg8.se/wiki/articles/Memory_Bank_Controllers#6000-7FFF_-_ROM.2FRAM_Mode_Select_.28Write_Only.29              

    The trick for selecting ROM bank zero is NOT implemented here. I don't know if it is necessary.
    But it is not very difficult to implement either.
    See: https://gbdev.io/pandocs/MBC1.html#20003fff--rom-bank-number-write-only                
*/    
static uint32_t
translate_mbc1_addr(gbc_mbc_t *mbc, uint16_t addr)
{
    return addr;
}

uint8_t
mbc1_read(gbc_mbc_t *mbc, uint16_t addr)
{
    LOG_DEBUG("[MBC1]Reading from MBC1 at address %x\n", addr);

    if (IN_RANGE(addr, MBC1_ROM_BANK0_BEGIN, MBC1_ROM_BANK0_END)) {
        return cartridge_code(mbc->cart)[addr];
    } else if (IN_RANGE(addr, MBC1_ROM_BANK_N_BEGIN, MBC1_ROM_BANK_N_END)) {
        uint32_t mbc1_rom_addr = translate_mbc1_addr(mbc, addr);
        
    } else if (IN_RANGE(addr, MBC1_RAM_BEGIN, MBC1_RAM_END)) {

    } else {
        LOG_ERROR("[MBC1]Invalid read: addr: %x\n", addr);
        abort();
    }
    
    return 0;
}

uint8_t
mbc1_write(gbc_mbc_t *mbc, uint16_t addr, uint8_t data)
{
    LOG_DEBUG("[MBC1]Writing to MBC1 at address %x [%x]\n", addr, data);
    uint8_t result;

    if (IN_RANGE(addr, MBC1_ROM_BEGIN, MBC1_ROM_END)) {

        if (IN_RANGE(addr, MBC1_REG_RAM_ENABLE_BEGIN, MBC1_REG_RAM_ENABLE_END)) {            
            result = data & 0x0f;
            mbc->ram_enabled = result;
            LOG_DEBUG("[MBC1]RAM enabled: %d\n", mbc->ram_enabled);

        } else if (IN_RANGE(addr, MBC1_REG_ROM_BANK_BEGIN, MBC1_REG_ROM_BANK_END)) {            
            result = data & MBC1_ROM_BANK_MASK;
            if (result == 0) result = 1; /* If this register is set to $00, it behaves as if it is set to $01. */
            mbc->rom_bank = result;
            LOG_DEBUG("[MBC1]Set ROM bank: %d\n", mbc->rom_bank);

        } else if (IN_RANGE(addr, MBC1_REG_RAM_BANK_BEGIN, MBC1_REG_RAM_BANK_END)) {
            result = data & MBC1_RAM_BANK_MASK;
            mbc->ram_bank = result;
            LOG_DEBUG("[MBC1]Set RAM bank: %d\n", mbc->ram_bank);
            
        } else if (IN_RANGE(addr, MBC1_REG_BANKING_MODE_BEGIN, MBC1_REG_BANKING_MODE_END)) {
            result = data & 0x01;
            mbc->mode = result;
            LOG_DEBUG("[MBC1]Set banking mode: %d\n", mbc->mode);

        } else {
            LOG_ERROR("[MBC1]It is not possible to reach here: %x\n", addr);
            abort();
        }

    } else if (IN_RANGE(addr, MBC1_RAM_BEGIN, MBC1_RAM_END)) {
        /* external RAM */
        if (!mbc->ram_enabled) {
            result = 0;
            LOG_INFO("[MBC1]Invalid write: addr %x data: [%x]. External RAM is not enabled. This write is ignored.\n", addr, data);
        } else {

            uint32_t mbc1_ram_addr = translate_mbc1_addr(mbc, addr);

            uint16_t raddr = mbc1_ram_addr & RAM_ADDR_MASK;
            uint8_t bank = (mbc1_ram_addr >> RAM_ADDR_MASK_SHIFT) & MBC1_RAM_BANK_MASK;

            if (bank >= mbc->ram_bank_size) {
                LOG_ERROR("[MBC1]Invalid write: addr: %x data: [%x]. Trying to write to invalid RAM bank: %d, bank_size: %d\n",
                            addr, data, bank, mbc->ram_bank_size);             
                abort();
            }
            mbc->ram_banks[bank * RAM_BANK_SIZE + raddr] = data;
            result = data;
        }

    } else {
        LOG_ERROR("[MBC1] Invalid write: addr: %x data: [%x]", addr, data);
        abort();
    }
        
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

