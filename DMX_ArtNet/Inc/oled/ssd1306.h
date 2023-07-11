#ifndef __SSD1306_H_
#define __SSD1306_H_

#include "oled/gfx.h"
#include "platform.h"

void SSD1306_Init(void (*i2c_write)(const unsigned char *buffer, unsigned short len, void (*callback)()));
void SSD1306_SendBuffer();

void SSD1306_Sleep(unsigned char sleep);

void SSD1306_ClearBuffer();
void SSD1306_FillRectangle(unsigned char left, unsigned char top, unsigned char width, unsigned char height, unsigned char fill);
void SSD1306_DrawBorder(unsigned char left, unsigned char top, unsigned char width, unsigned char height, unsigned char thickness, unsigned char fill);
void SSD1306_DrawBitmap(const unsigned char *buffer, unsigned short len, unsigned char left, unsigned char top, unsigned char width, unsigned char height);
void SSD1306_DrawPixel(unsigned char x, unsigned char y, unsigned char fill);
void SSD1306_DrawChar(char chr, unsigned char x, unsigned char y, const GFXfont *font, unsigned char fill);
void SSD1306_DrawString(const char *str, unsigned short len, unsigned short x, unsigned short y, const GFXfont *font, unsigned char fill);
void SSD1306_DrawStringHighlighted(const char *str, unsigned short len, unsigned int selectedPos, unsigned short x, unsigned short y, const GFXfont *font, unsigned char fill);

void SSD1306_MeasureString(const char *str, unsigned short len, unsigned short *width, unsigned short *height, const GFXfont *font);

#endif