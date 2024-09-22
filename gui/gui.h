#ifndef _GUI_H
#define _GUI_H

extern void (*gui_close_callback)(void* udata);
extern void *gui_callback_udata;

#ifdef __cplusplus
extern "C" {
#endif

#include "gbc.h"

int GuiInit();
void GuiDestroy();
void GuiUpdate();
void GuiSetCloseCallback(void (*callback)(void *udata));
void GuiSetUserData(void *udata);
uint8_t GuiPollKeypad();
void GuiAudio(uint8_t sample);
unsigned short GuiWrite(void *udata, unsigned short addr, unsigned short data);

#ifdef __cplusplus
}
#endif

#endif