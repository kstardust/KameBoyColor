#ifndef _CARTRIDGE_H
#define _CARTRIDGE_H

#include <stdint.h>

#define CART_TYPE_ROM_ONLY          0x00
#define CART_TYPE_MBC1              0x01
#define CART_TYPE_MBC1_RAM          0x02
#define CART_TYPE_MBC1_RAM_BATTERY  0x03
#define CART_TYPE_MBC2              0x05
#define CART_TYPE_MBC2_BATTERY      0x06
#define CART_TYPE_ROM_RAM           0x08
#define CART_TYPE_ROM_RAM_BATTERY   0x09
#define CART_TYPE_MMM01             0x0B
#define CART_TYPE_MMM01_RAM         0x0C
#define CART_TYPE_MMM01_RAM_BATTERY 0x0D
#define CART_TYPE_MBC3_TIMER_BATTERY 0x0F
#define CART_TYPE_MBC3_TIMER_RAM_BATTERY 0x10
#define CART_TYPE_MBC3               0x11
#define CART_TYPE_MBC3_RAM           0x12
#define CART_TYPE_MBC3_RAM_BATTERY   0x13
#define CART_TYPE_MBC5               0x19
#define CART_TYPE_MBC5_RAM           0x1A
#define CART_TYPE_MBC5_RAM_BATTERY   0x1B
#define CART_TYPE_MBC5_RUMBLE        0x1C
#define CART_TYPE_MBC5_RUMBLE_RAM    0x1D
#define CART_TYPE_MBC5_RUMBLE_RAM_BATTERY 0x1E
#define CART_TYPE_MB6               0x20
#define CART_TYPE_MBC7_SENSOR_RUMBLE_RAM_BATTERY 0x22
#define CART_TYPE_POCKET_CAMERA     0xFC
#define CART_TYPE_BANDAI_TAMA5      0xFD
#define CART_TYPE_HUC3              0xFE
#define CART_TYPE_HUC1_RAM_BATTERY  0xFF


typedef struct cartridge cartridge_t;

struct cartridge
{
    uint8_t empty[0x100];
    uint8_t entry_point[0x104-0x100];          /* 0x100 - 0x103  Entry Point */
    uint8_t nintendo_logo[0x134-0x104];        /* 0x104 - 0x133  Nintendo Logo */
    uint8_t title[0x144-0x134];                /* 0x134 - 0x143  Title */

    #define cart_manufacturer_code title[0x13f-0x134]     /* 0x13F-0x142    Manufacturer Code */
    #define cart_cgb_flag title[0x143-0x134]              /* 0x143          CGB Flag */

    uint16_t new_licensee_code;                 /* 0x144          New Licensee Code */
    uint8_t  sgb_flag;                          /* 0x146          SGB Flag */
    uint8_t  cartridge_type;                    /* 0x147          Cartridge Type */
    uint8_t  rom_size;                          /* 0x148          ROM Size */
    uint8_t  ram_size;                          /* 0x149          RAM Size */
    uint8_t  destination_code;                  /* 0x14A          Destination Code */
    uint8_t  old_licensee_code;                 /* 0x14B          Old Licensee Code */
    uint8_t  mask_rom_version_number;           /* 0x14C          Mask ROM Version Number */
    uint8_t  header_checksum;                   /* 0x14D          Header Checksum */
    uint16_t global_checksum;                   /* 0x14E-0x14F    Global Checksum */    
    uint8_t  code;                              /* 0x150          ROM */
};

#define cartridge_rom_size(cart)    (32 * (1 << (cart)->rom_size) * 1024)
#define cartridge_rom_banks(cart)   (2 << cart->rom_size)
#define cartridge_code(cart)        ((uint8_t*)&(cart->code))
#define cartridge_code_size(cart)   (cartridge_code(cart) - (uint8_t*)(cart) + cartridge_rom_size((cart)))

cartridge_t* cartridge_load(uint8_t *data);

#endif