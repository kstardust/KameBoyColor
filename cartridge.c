#include <stdio.h>
#include "cartridge.h"
#include "utils.h"
#include "common.h"

#define LOGO_ROW 8

static uint8_t NINTENDO_LOGO[] = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

int
validate_logo(uint8_t *data)
{    
    for (int i = 0; i < sizeof(NINTENDO_LOGO); i++) {
        if (data[i] != NINTENDO_LOGO[i]) {        
            printf("Invalid logo at %d: 0x%x - 0x%x\n", i, data[i], NINTENDO_LOGO[i]);
            return 0;
        }
    }

    return 1;
}

/* https://gbdev.io/pandocs/The_Cartridge_Header.html#014d--header-checksum */
int 
validate_checksum(uint8_t *data)
{    
    uint8_t checksum = 0;
    for (int i = 0x134; i <= 0x14C; i++) {
        checksum = checksum - data[i] - 1;
    }

    return checksum == data[0x14D];    
}

cartridge_t* 
cartridge_load(uint8_t *data)
{
    cartridge_t *cartridge = (cartridge_t*)data;

    if (!validate_logo(cartridge->nintendo_logo)) {
        printf("Invalid logo\n");
        return NULL;
    }

    if (!validate_checksum(data)) {
        printf("Invalid checksum\n");
        return NULL;
    }
    
    if (cartridge->cart_cgb_flag != 0x80 && cartridge->cart_cgb_flag != 0xC0) {
        printf("Invalid CGB flag\n");
        return NULL;
    }

    printf("Title: %s\n", cartridge->title);
    printf("ROM Size: %dk\n", cartridge_rom_size(cartridge) / 1024);
    printf("ROM Banks: %d\n", cartridge_rom_banks(cartridge));
    
    switch (cartridge->ram_size) {
        case 0:
            printf("RAM Size: None\n");
            break;
        case 1:
            printf("RAM Size: 2k\n");
            break;
        case 2:
            printf("RAM Size: 8k\n");
            break;
        case 3:
            printf("RAM Size: 32k\n");
            break;
        case 4:
            printf("RAM Size: 128k\n");
            break;
        case 5:
            printf("RAM Size: 64k\n");
            break;
        default:
            LOG_ERROR("RAM Size: Unknown\n");
            abort();
            break;            
    }

    printf("Cartridge Type: $%x\n", cartridge->cartridge_type);

    printf("Manufacturer Code: %c%c%c%c\n", 
        ((char*)&(cartridge->cart_manufacturer_code))[0],
        ((char*)&(cartridge->cart_manufacturer_code))[1],
        ((char*)&(cartridge->cart_manufacturer_code))[2],
        ((char*)&(cartridge->cart_manufacturer_code))[3]
    );

    return cartridge;
}