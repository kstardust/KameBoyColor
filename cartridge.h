#ifndef _CARTRIDGE_H
#define _CARTRIDGE_H

#include <stdint.h>

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
#define cartridge_code(cart)        ((uint8_t*)&(cart->code))
#define cartridge_code_size(cart)   (cartridge_code(cart) - (uint8_t*)(cart) + cartridge_rom_size((cart)))

cartridge_t* cartridge_load(uint8_t *data);

#endif