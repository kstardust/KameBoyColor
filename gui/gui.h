#ifndef _GUI_H
#define _GUI_H

#ifdef __cplusplus
extern "C" {
#endif

int GuiInit();
void GuiDestroy();
void GuiUpdate(void *udata);
unsigned char GuiWrite(void *udata, unsigned short addr, unsigned char data);

#ifdef __cplusplus
}
#endif

#endif