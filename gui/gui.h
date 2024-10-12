#ifndef _GUI_H
#define _GUI_H

extern void (*gui_close_callback)(void* udata);
extern void *gui_callback_udata;

#ifdef __cplusplus
extern "C" {
#endif

#include "gbc.h"

/* init the GUI environment */
int GuiInit();

/* destroy the GUI environment */
void GuiDestroy();

/* update the GUI, it will called after every frame */
void GuiUpdate();

/* callback after close the GUI */
void GuiSetCloseCallback(void (*callback)(void *udata));

/* set custom user data */
void GuiSetUserData(void *udata);

/* it will be called after every frame, to see if any key is pressed */
uint8_t GuiPollKeypad();

/* Gameboy sound, left and right channel respectively */
void GuiAudioWrite(int8_t l_sample, int8_t r_sample);

/* update the audio, it will be called after every frame */
void GuiAudioUpdate(void *udata);

/*
pixels of Gameboy screen will be written using this function,
addr is the position of the pixel, left-top is 0, right-bottom is 159x143
data is the RGB555 color data
*/
void GuiWrite(void *udata, unsigned short addr, unsigned short data);

#ifdef __cplusplus
}
#endif

#endif