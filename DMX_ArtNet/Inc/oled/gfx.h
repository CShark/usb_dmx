#ifndef __GFX_H_
#define __GFX_H_

typedef struct {
    unsigned short BitmapPosition;
    unsigned char Width;
    unsigned char Height;
    unsigned char xAdvance;
    signed char OffsetX;
    signed char OffsetY;
} GFXglyph;

typedef struct {
    unsigned char *Bitmaps;
    GFXglyph *GlyphMap;
    unsigned char StartASCII;
    unsigned char EndASCII;
    unsigned char yAdvance;
} GFXfont;

#endif
