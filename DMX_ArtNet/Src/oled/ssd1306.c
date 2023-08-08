#include "oled/ssd1306.h"
#include "systimer.h"

static const void (*i2c_send)(const unsigned char *buffer, unsigned short len, void (*callback)());

typedef enum {
    S1306C_CmdStream = 0x00,
    S1306C_Cmd = 0x80,
    S1306C_Data = 0x40,

    S1306C_AddrMode = 0x20,
    S1306C_AddrCol = 0x21,
    S1306C_AddrPage = 0x22,
    S1306C_ScrollStop = 0x2E,
    S1306C_DispStartLine = 0x40,
    S1306C_Contrast = 0x81,
    S1306C_ChargePump = 0x8D,
    S1306C_SegMap = 0xA0,
    S1306C_SegMapReverse = 0xA1,
    S1306C_DispRAM = 0xA4,
    S1306C_DispFULL = 0xA5,
    S1306C_DispNormal = 0xA6,
    S1306C_DispInvers = 0xA7,
    S1306C_MuxRatio = 0xA8,
    S1306C_DispOff = 0xAE,
    S1306C_DispOn = 0xAF,
    S1306C_ComMap = 0xC0,
    S1306C_ComMapReverse = 0xC8,
    S1306C_DispOffset = 0xD3,
    S1306C_OscFreq = 0xD5,
    S1306C_PreCharge = 0xD9,
    S1306C_ComPinLayout = 0xDA,
    S1306C_VComLevel = 0xDB,
} SSD1306_Commands;

typedef struct {
    unsigned char txBuffer[32 + 1];
    unsigned char counter;
    unsigned char refreshPending;
} txStateData;

static const unsigned char oled_init[] = {
    S1306C_CmdStream,
    S1306C_DispOff,
    S1306C_OscFreq, 0x80,
    S1306C_MuxRatio, 0x3F,
    S1306C_DispOffset, 0x00,
    S1306C_DispStartLine,
    S1306C_ChargePump, 0x14,
    S1306C_AddrMode, 0x00,
    S1306C_SegMapReverse,
    S1306C_ComMapReverse,
    S1306C_ComPinLayout, 0x12,
    S1306C_Contrast, 0xCF,
    S1306C_PreCharge, 0xF1,
    S1306C_VComLevel, 0x40,
    S1306C_DispRAM,
    S1306C_DispNormal,
    S1306C_ScrollStop,
    S1306C_DispOn,
    S1306C_AddrPage, 0x00, 0xFF,
    S1306C_AddrCol, 0x00, 0x7F};

static const unsigned char oled_sleep[] = {S1306C_CmdStream, S1306C_DispOff};
static const unsigned char oled_wake[] = {S1306C_CmdStream, S1306C_DispOn};

static unsigned char buffer[128 * 8];
static txStateData txState;

static void SSD1306_WriteBuffer();

void SSD1306_Init(void (*i2c_write)(const unsigned char *buffer, unsigned short len, void (*callback)())) {
    i2c_send = i2c_write;

    for (int i = 0; i < 128 * 8; i++) {
        buffer[i] = 0x00;
    }

    // Minimal delay to allow SSD1306 bootup
    delay_ms(100);
    i2c_send(oled_init, sizeof(oled_init), SSD1306_SendBuffer);
    delay_ms(100);
}

void SSD1306_SendBuffer() {
    if (txState.counter != 0) {
        txState.refreshPending = 1;
    } else {
        txState.txBuffer[0] = S1306C_Data;
        SSD1306_WriteBuffer();
    }
}

static void SSD1306_WriteBuffer() {
    memcpy(&txState.txBuffer[1], buffer + (txState.counter * 32), 32);

    if (txState.counter < 32) {
        i2c_send(txState.txBuffer, sizeof(txState.txBuffer), SSD1306_WriteBuffer);
        txState.counter++;
    } else {
        txState.counter = 0;

        if (txState.refreshPending) {
            SSD1306_SendBuffer();
            txState.refreshPending = 0;
        }
    }
}

void SSD1306_Sleep(unsigned char sleep) {
    if (sleep) {
        i2c_send(oled_sleep, sizeof(oled_sleep), NULL);
    } else {
        i2c_send(oled_wake, sizeof(oled_wake), NULL);
    }
}

void SSD1306_ClearBuffer() {
    for (int i = 0; i < sizeof(buffer); i++) {
        buffer[i] = 0;
    }
}

void SSD1306_FillRectangle(unsigned char left, unsigned char top, unsigned char width, unsigned char height, unsigned char fill) {
    unsigned char pageStart = top / 8;
    unsigned char pageEnd = (top + height) / 8;
    unsigned char segStart = left;
    unsigned char segEnd = left + width;

    unsigned char startOffset = top - pageStart * 8;
    unsigned char endOffset = (pageEnd + 1) * 8 - (top + height);

    unsigned char startMask = (0xFF << startOffset);
    unsigned char endMask = (0xFF >> endOffset);

    if (fill)
        fill = 0xFF;

    for (int y = pageStart; y <= pageEnd; y++) {
        for (int x = segStart; x < segEnd; x++) {
            unsigned short i = y * 128 + x;

            if (y == pageStart && y == pageEnd) {
                buffer[i] = (buffer[i] & ~(startMask & endMask)) | (fill & (startMask & endMask));
            } else if (y == pageStart) {
                buffer[i] = (buffer[i] & ~startMask) | (fill & startMask);
            } else if (y == pageEnd) {
                buffer[i] = (buffer[i] & ~endMask) | (fill & endMask);
            } else {
                buffer[i] = fill;
            }
        }
    }
}

void SSD1306_DrawBorder(unsigned char left, unsigned char top, unsigned char width, unsigned char height, unsigned char thickness, unsigned char fill) {
    SSD1306_FillRectangle(left, top, width, thickness, fill);
    SSD1306_FillRectangle(left, top, thickness, height, fill);
    SSD1306_FillRectangle(left + width - thickness, top, thickness, height, fill);
    SSD1306_FillRectangle(left, top + height - thickness, width, thickness, fill);
}

void SSD1306_DrawBitmap(const unsigned char *bitmap, unsigned short len, unsigned char left, unsigned char top, unsigned char width, unsigned char height) {
    if (width * height / 8 > len)
        return;

    unsigned char pageStart = top / 8;
    unsigned char pageEnd = (top + height) / 8;
    unsigned char segStart = left;
    unsigned char segEnd = left + width;

    unsigned char startOffset = top - pageStart * 8;
    unsigned char endOffset = (pageEnd + 1) * 8 - (top + height);

    unsigned char startMask = (0xFF << startOffset);
    unsigned char endMask = (0xFF >> endOffset);

    for (int y = pageStart; y <= pageEnd; y++) {
        for (int x = segStart; x < segEnd; x++) {
            unsigned short i = y * 128 + x;
            unsigned char bitmapSlice = bitmap[(y - pageStart) * width + (x - segStart)] << startOffset;

            if ((y - pageStart) > 0) {
                bitmapSlice |= bitmap[(y - pageStart - 1) * width + (x - segStart)] >> (8 - startOffset);
            }

            if (y == pageStart && y == pageEnd) {
                buffer[i] = (buffer[i] & ~(startMask & endMask)) | (bitmapSlice & (startMask & endMask));
            } else if (y == pageStart) {
                buffer[i] = (buffer[i] & ~startMask) | (bitmapSlice & startMask);
            } else if (y == pageEnd) {
                buffer[i] = (buffer[i] & ~endMask) | (bitmapSlice & endMask);
            } else {
                buffer[i] = bitmapSlice;
            }
        }
    }
}

void SSD1306_DrawPixel(short x, short y, unsigned char fill) {
    unsigned char page = y / 8;
    unsigned char mask = 1 << (y - page * 8);

    if (x > 128 || y > 64 || x < 0 || y < 0)
        return;

    if (fill)
        fill = 0xFF;

    buffer[page * 128 + x] = (buffer[page * 128 + x] & ~mask) | (fill & mask);
}

void SSD1306_DrawChar(char chr, short left, short top, const GFXfont *font, unsigned char fill) {
    GFXglyph *glyph = &font->GlyphMap[chr - font->StartASCII];

    if (glyph->Width > 0 && glyph->Height > 0) {
        unsigned char *bitmap = font->Bitmaps;
        unsigned short bitmapOffset = glyph->BitmapPosition;
        unsigned char bit = 0;
        unsigned char bits = 0;

        for (short y = 0; y < glyph->Height; y++) {
            for (short x = 0; x < glyph->Width; x++) {
                if (!(bit++ & 7)) {
                    bits = bitmap[bitmapOffset++];
                }

                if (bits & 0x80) {
                    SSD1306_DrawPixel(x + glyph->OffsetX + left, y + glyph->OffsetY + top, fill);
                }

                bits <<= 1;
            }
        }
    }
}

void SSD1306_DrawString(const char *str, unsigned short len, short x, short y, const GFXfont *font, unsigned char fill) {
    short posX = x;
    short posY = y;

    for (int i = 0; i < len; i++) {
        if (str[i] == 0) {
            return;
        } else if (str[i] == '\n') {
            posY += font->yAdvance;
            posX = x;
        } else if (str[i] >= font->StartASCII && str[i] <= font->EndASCII) {
            GFXglyph *glyph = &font->GlyphMap[str[i] - font->StartASCII];
            SSD1306_DrawChar(str[i], posX, posY, font, fill);

            posX += glyph->xAdvance;
        }
    }
}

void SSD1306_DrawStringHighlighted(const char *str, unsigned short len, unsigned int selectedPos, short x, short y, const GFXfont *font, unsigned char fill) {
    SSD1306_DrawString(str, len, x, y, font, fill);

    if (len <= selectedPos) {
        return;
    }

    for (int i = 0; i < selectedPos; i++) {
        if (str[i] == 0) {
            return;
        }
    }

    unsigned short width;
    unsigned short height;
    unsigned short charWidth;

    SSD1306_MeasureString(str, selectedPos, &width, &height, font);

    charWidth = font->GlyphMap[str[selectedPos] - font->StartASCII].xAdvance;

    SSD1306_FillRectangle(x + width, y + height - font->yAdvance + 2, charWidth, 1, fill);
}

void SSD1306_MeasureString(const char *str, unsigned short len, unsigned short *width, unsigned short *height, const GFXfont *font) {
    *width = 0;
    *height = font->yAdvance;

    unsigned short lineMax = 0;

    for (int i = 0; i < len; i++) {
        if (str[i] == 0) {
            return;
        } else if (str[i] == '\n') {
            *height += font->yAdvance;
            lineMax = 0;
        } else if (str[i] >= font->StartASCII && str[i] <= font->EndASCII) {
            GFXglyph *glyph = &font->GlyphMap[str[i] - font->StartASCII];

            lineMax += glyph->xAdvance;

            if (lineMax > *width) {
                *width = lineMax;
            }
        }
    }
}
