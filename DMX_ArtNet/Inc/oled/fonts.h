#ifndef __FONTS_H_
#define __FONTS_H_

#include "oled/gfx.h"
#include "platform.h"

// 'Bootlogo', 51x53px
const unsigned char bmp_Bootlogo [] PROGMEM = {
	0x00, 0x00, 0x00, 0x80, 0xf0, 0xfc, 0x1f, 0x07, 0x1f, 0xfc, 0xf0, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x83, 0x86, 0xfe, 0x78, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0xff, 0xff, 0xff, 0x07, 0x07, 0x07, 0x07, 
	0x07, 0x07, 0x07, 0x80, 0xe0, 0xfc, 0x3f, 0x1f, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1f, 0x3f, 0xfc, 
	0xe0, 0x80, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x01, 0x03, 0x07, 0x1f, 0x7d, 0xf0, 0xc0, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe3, 0xe3, 0xe1, 0xe0, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x01, 0x03, 0x03, 0xe0, 0xe0, 0xe0, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xe0, 
	0xe0, 0xe1, 0xe3, 0xe3, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0xff, 
	0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x01, 0x07, 0x1f, 0x7e, 
	0xfc, 0xf0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x03, 0x0f, 0x3f, 0x7e, 0xf8, 0xe0, 0x80, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcf, 
	0x4f, 0x4f, 0x40, 0x40, 0x00, 0x40, 0x40, 0xc0, 0x40, 0x40, 0x00, 0xc1, 0x07, 0x0f, 0x0f, 0xcf, 
	0x0f, 0xc0, 0x40, 0x40, 0x40, 0x40, 0x00, 0xcf, 0x4f, 0x4f, 0x4e, 0x8e, 0x0e, 0xce, 0x8e, 0x0e, 
	0x0e, 0x0e, 0x0e, 0xce, 0x00, 0xc0, 0x40, 0x40, 0x4f, 0x4f, 0x0f, 0x40, 0x40, 0xc0, 0x40, 0x40, 
	0x00, 0x00, 0x1f, 0x12, 0x12, 0x10, 0x10, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x1f, 0x02, 
	0x02, 0x02, 0x1f, 0x00, 0x1f, 0x12, 0x12, 0x10, 0x10, 0x00, 0x1f, 0x02, 0x06, 0x0a, 0x11, 0x00, 
	0x1f, 0x00, 0x01, 0x02, 0x04, 0x08, 0x1f, 0x00, 0x1f, 0x12, 0x12, 0x10, 0x10, 0x00, 0x00, 0x00, 
	0x1f, 0x00, 0x00, 0x00, 0x00
};

// Created by http://oleddisplay.squix.ch/ Consider a donation
// In case of problems make sure that you are using the font file with the correct version!
const uint8_t Dialog_plain_16Bitmaps[] PROGMEM = {

	// Bitmap Data:
	0x00, // ' '
	0xFF,0x30, // '!'
	0x99,0x99, // '"'
	0x0C,0x81,0x10,0x26,0x3F,0xF1,0x90,0x22,0x04,0xC7,0xFE,0x32,0x04,0x40,0x98,0x00, // '#'
	0x10,0x21,0xF6,0x99,0x12,0x1C,0x0E,0x12,0x26,0x5B,0xE1,0x02,0x00, // '$'
	0x70,0x44,0x42,0x22,0x21,0x13,0x08,0x90,0x39,0x00,0x09,0xC0,0x91,0x0C,0x88,0x44,0x44,0x22,0x20,0xE0, // '%'
	0x3C,0x0C,0x41,0x00,0x20,0x02,0x00,0xA0,0x22,0x14,0x22,0x82,0x50,0x31,0x06,0x1F,0x20, // '&'
	0xF0, // '''
	0x36,0x44,0x88,0x88,0x88,0x44,0x63, // '('
	0xC6,0x22,0x11,0x11,0x11,0x22,0x6C, // ')'
	0x10,0x22,0x4B,0xE3,0x9A,0xC4,0x08, // '*'
	0x08,0x04,0x02,0x01,0x0F,0xF8,0x40,0x20,0x10,0x08,0x00, // '+'
	0x56, // ','
	0xF0, // '-'
	0xC0, // '.'
	0x08,0x44,0x21,0x10,0x84,0x42,0x11,0x88,0x00, // '/'
	0x3C,0x42,0x42,0x81,0x81,0x81,0x81,0x81,0x81,0x42,0x42,0x3C, // '0'
	0x71,0xA0,0x40,0x81,0x02,0x04,0x08,0x10,0x20,0x47,0xF0, // '1'
	0x79,0x8A,0x08,0x10,0x20,0x82,0x08,0x20,0x82,0x07,0xF0, // '2'
	0x7C,0x83,0x01,0x01,0x03,0x3C,0x03,0x01,0x01,0x01,0x82,0x7C, // '3'
	0x06,0x05,0x02,0x82,0x42,0x22,0x11,0x09,0x04,0xFF,0x81,0x00,0x80,0x40, // '4'
	0x7E,0x40,0x40,0x40,0x7C,0x42,0x01,0x01,0x01,0x01,0x82,0x7C, // '5'
	0x1C,0x62,0x40,0x80,0xBC,0xC2,0x81,0x81,0x81,0x81,0x42,0x3C, // '6'
	0xFF,0x01,0x02,0x02,0x04,0x04,0x08,0x08,0x08,0x10,0x10,0x20, // '7'
	0x3C,0xC3,0x81,0x81,0xC3,0x3C,0xC3,0x81,0x81,0x81,0x42,0x3C, // '8'
	0x3C,0x42,0x82,0x81,0x81,0x81,0x43,0x3D,0x01,0x02,0x46,0x38, // '9'
	0xC3, // ':'
	0x50,0x05,0x60, // ';'
	0x00,0x40,0xE1,0xC1,0xC0,0x80,0x1C,0x01,0xC0,0x0E,0x00,0x40, // '<'
	0xFF,0xC0,0x00,0x03,0xFF, // '='
	0x80,0x1C,0x00,0xE0,0x0E,0x00,0x40,0xE0,0xE1,0xC0,0x80,0x00, // '>'
	0x7A,0x10,0x41,0x0C,0x63,0x08,0x20,0x02,0x08, // '?'
	0x0F,0xC0,0x60,0xC2,0x01,0x91,0xEA,0xC8,0x66,0x40,0x99,0x02,0x64,0x09,0x90,0x27,0x21,0xA4,0x7B,0x08,0x00,0x18,0x30,0x1F,0x80, // '@'
	0x04,0x00,0x80,0x28,0x05,0x01,0x10,0x22,0x08,0x21,0x04,0x3F,0x88,0x09,0x01,0x40,0x10, // 'A'
	0xFE,0x40,0xE0,0x30,0x18,0x1F,0xFA,0x07,0x01,0x80,0xC0,0x60,0x5F,0xC0, // 'B'
	0x1F,0x10,0xD0,0x10,0x08,0x04,0x02,0x01,0x00,0x80,0x20,0x08,0x63,0xE0, // 'C'
	0xFE,0x20,0xC8,0x0A,0x01,0x80,0x60,0x18,0x06,0x01,0x80,0x60,0x28,0x33,0xF8, // 'D'
	0xFF,0x80,0x80,0x80,0x80,0xFF,0x80,0x80,0x80,0x80,0x80,0xFF, // 'E'
	0xFF,0x02,0x04,0x08,0x1F,0xA0,0x40,0x81,0x02,0x04,0x00, // 'F'
	0x1F,0x88,0x34,0x06,0x00,0x80,0x20,0x08,0x3E,0x01,0x80,0x50,0x12,0x04,0x7E, // 'G'
	0x80,0x60,0x18,0x06,0x01,0x80,0x7F,0xF8,0x06,0x01,0x80,0x60,0x18,0x06,0x01, // 'H'
	0xFF,0xF0, // 'I'
	0x24,0x92,0x49,0x24,0x92,0x70, // 'J'
	0x81,0x41,0x21,0x11,0x09,0x07,0x02,0x81,0x20,0x88,0x42,0x20,0x90,0x20, // 'K'
	0x81,0x02,0x04,0x08,0x10,0x20,0x40,0x81,0x02,0x07,0xF0, // 'L'
	0xC0,0x78,0x0E,0x82,0xD0,0x5A,0x0B,0x22,0x64,0x4C,0x51,0x8A,0x30,0x86,0x00,0xC0,0x10, // 'M'
	0xC0,0x68,0x1A,0x06,0x41,0x88,0x62,0x18,0x46,0x09,0x82,0x60,0x58,0x16,0x03, // 'N'
	0x1F,0x04,0x11,0x01,0x40,0x18,0x03,0x00,0x60,0x0C,0x01,0x80,0x28,0x08,0x82,0x0F,0x80, // 'O'
	0xFC,0x82,0x81,0x81,0x81,0x82,0xFC,0x80,0x80,0x80,0x80,0x80, // 'P'
	0x1F,0x04,0x11,0x01,0x40,0x18,0x03,0x00,0x60,0x0C,0x01,0x80,0x28,0x08,0x83,0x0F,0x80,0x08,0x00,0x80, // 'Q'
	0xFC,0x20,0x88,0x12,0x04,0x81,0x20,0x8F,0xE2,0x08,0x81,0x20,0x48,0x0A,0x02, // 'R'
	0x3C,0xC6,0x80,0x80,0x80,0x70,0x1E,0x03,0x01,0x81,0xC3,0x7C, // 'S'
	0xFF,0x84,0x02,0x01,0x00,0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01,0x00, // 'T'
	0x80,0x60,0x18,0x06,0x01,0x80,0x60,0x18,0x06,0x01,0x80,0x60,0x14,0x08,0xFC, // 'U'
	0x80,0x30,0x05,0x01,0x20,0x22,0x08,0x41,0x04,0x40,0x88,0x11,0x01,0x40,0x28,0x02,0x00, // 'V'
	0x81,0x03,0x02,0x05,0x0A,0x12,0x14,0x24,0x28,0x44,0x51,0x09,0x12,0x12,0x24,0x14,0x50,0x28,0xA0,0x20,0x80,0x41,0x00, // 'W'
	0xC1,0xA0,0x88,0x84,0x41,0x40,0x40,0x20,0x28,0x22,0x11,0x10,0x50,0x10, // 'X'
	0x80,0xA0,0x88,0x84,0x41,0x40,0x40,0x20,0x10,0x08,0x04,0x02,0x01,0x00, // 'Y'
	0xFF,0xC0,0x10,0x08,0x04,0x02,0x01,0x00,0x80,0x40,0x20,0x10,0x08,0x03,0xFF, // 'Z'
	0xF2,0x49,0x24,0x92,0x49,0xC0, // '['
	0x86,0x10,0x84,0x10,0x84,0x10,0x84,0x10,0x80, // '\'
	0xE4,0x92,0x49,0x24,0x93,0xC0, // ']'
	0x0C,0x07,0x82,0x11,0x02, // '^'
	0xFF, // '_'
	0xC6,0x30, // '`'
	0x3C,0x8C,0x0B,0xFC,0x30,0x60,0xE3,0x7A, // 'a'
	0x80,0x80,0x80,0xBC,0xC2,0x81,0x81,0x81,0x81,0x81,0xC2,0xBC, // 'b'
	0x3C,0x86,0x04,0x08,0x10,0x20,0x21,0x3C, // 'c'
	0x01,0x01,0x01,0x3D,0x43,0x81,0x81,0x81,0x81,0x81,0x43,0x3D, // 'd'
	0x38,0x8A,0x0C,0x1F,0xF0,0x20,0x21,0x3C, // 'e'
	0x3A,0x11,0xE4,0x21,0x08,0x42,0x10,0x80, // 'f'
	0x3D,0x43,0x81,0x81,0x81,0x81,0x81,0x43,0x3D,0x01,0x42,0x3C, // 'g'
	0x80,0x80,0x80,0xBC,0xC2,0x81,0x81,0x81,0x81,0x81,0x81,0x81, // 'h'
	0xDF,0xF0, // 'i'
	0x24,0x12,0x49,0x24,0x92,0x70, // 'j'
	0x80,0x80,0x80,0x82,0x84,0x88,0x90,0xE0,0x90,0x88,0x84,0x82, // 'k'
	0xFF,0xF0, // 'l'
	0xBC,0xF6,0x38,0xE0,0x83,0x04,0x18,0x20,0xC1,0x06,0x08,0x30,0x41,0x82,0x08, // 'm'
	0xBC,0xC2,0x81,0x81,0x81,0x81,0x81,0x81,0x81, // 'n'
	0x3C,0x42,0x81,0x81,0x81,0x81,0x81,0x42,0x3C, // 'o'
	0xBC,0xC2,0x81,0x81,0x81,0x81,0x81,0xC2,0xBC,0x80,0x80,0x80, // 'p'
	0x3D,0x43,0x81,0x81,0x81,0x81,0x81,0x43,0x3D,0x01,0x01,0x01, // 'q'
	0xBE,0x21,0x08,0x42,0x10,0x80, // 'r'
	0x7D,0x06,0x06,0x07,0x80,0xC0,0xC1,0x7C, // 's'
	0x42,0x3E,0x84,0x21,0x08,0x42,0x0E, // 't'
	0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x43,0x3D, // 'u'
	0x80,0xA0,0x24,0x11,0x04,0x22,0x08,0x81,0x40,0x70,0x08,0x00, // 'v'
	0x82,0x0C,0x10,0x51,0x44,0x8A,0x24,0x89,0x14,0x50,0xA2,0x82,0x08,0x10,0x40, // 'w'
	0xC3,0x42,0x24,0x24,0x18,0x24,0x24,0x42,0xC3, // 'x'
	0x80,0xA0,0x90,0x44,0x22,0x21,0x10,0x50,0x28,0x0C,0x04,0x02,0x0E,0x00, // 'y'
	0xFE,0x04,0x10,0x41,0x04,0x10,0x40,0xFE, // 'z'
	0x19,0x08,0x42,0x10,0x98,0x21,0x08,0x42,0x10,0x60, // '{'
	0xFF,0xFF, // '|'
	0xC1,0x08,0x42,0x10,0x83,0x21,0x08,0x42,0x13,0x00 // '}'
};
const GFXglyph Dialog_plain_16Glyphs[] PROGMEM = {
// bitmapOffset, width, height, xAdvance, xOffset, yOffset
	  {     0,   1,   1,   6,    0,   -1 }, // ' '
	  {     1,   1,  12,   7,    2,  -12 }, // '!'
	  {     3,   4,   4,   7,    1,  -12 }, // '"'
	  {     5,  11,  11,  14,    1,  -11 }, // '#'
	  {    21,   7,  14,  11,    2,  -12 }, // '$'
	  {    34,  13,  12,  16,    1,  -12 }, // '%'
	  {    54,  11,  12,  13,    1,  -12 }, // '&'
	  {    71,   1,   4,   4,    1,  -12 }, // '''
	  {    72,   4,  14,   7,    1,  -12 }, // '('
	  {    79,   4,  14,   7,    1,  -12 }, // ')'
	  {    86,   7,   8,   9,    1,  -12 }, // '*'
	  {    93,   9,   9,  14,    2,   -9 }, // '+'
	  {   104,   2,   4,   6,    1,   -2 }, // ','
	  {   105,   4,   1,   7,    1,   -5 }, // '-'
	  {   106,   1,   2,   6,    2,   -2 }, // '.'
	  {   107,   5,  13,   6,    0,  -12 }, // '/'
	  {   116,   8,  12,  11,    1,  -12 }, // '0'
	  {   128,   7,  12,  11,    2,  -12 }, // '1'
	  {   139,   7,  12,  11,    1,  -12 }, // '2'
	  {   150,   8,  12,  11,    1,  -12 }, // '3'
	  {   162,   9,  12,  11,    1,  -12 }, // '4'
	  {   176,   8,  12,  11,    1,  -12 }, // '5'
	  {   188,   8,  12,  11,    1,  -12 }, // '6'
	  {   200,   8,  12,  11,    1,  -12 }, // '7'
	  {   212,   8,  12,  11,    1,  -12 }, // '8'
	  {   224,   8,  12,  11,    1,  -12 }, // '9'
	  {   236,   1,   8,   6,    2,   -8 }, // ':'
	  {   237,   2,  10,   6,    1,   -8 }, // ';'
	  {   240,  10,   9,  14,    2,   -9 }, // '<'
	  {   252,  10,   4,  14,    2,   -7 }, // '='
	  {   257,  10,   9,  14,    2,   -9 }, // '>'
	  {   269,   6,  12,  10,    1,  -12 }, // '?'
	  {   278,  14,  14,  17,    1,  -12 }, // '@'
	  {   303,  11,  12,  12,    0,  -12 }, // 'A'
	  {   320,   9,  12,  12,    1,  -12 }, // 'B'
	  {   334,   9,  12,  12,    1,  -12 }, // 'C'
	  {   348,  10,  12,  13,    1,  -12 }, // 'D'
	  {   363,   8,  12,  11,    1,  -12 }, // 'E'
	  {   375,   7,  12,  10,    1,  -12 }, // 'F'
	  {   386,  10,  12,  13,    1,  -12 }, // 'G'
	  {   401,  10,  12,  13,    1,  -12 }, // 'H'
	  {   416,   1,  12,   6,    2,  -12 }, // 'I'
	  {   418,   3,  15,   6,    0,  -12 }, // 'J'
	  {   424,   9,  12,  11,    1,  -12 }, // 'K'
	  {   438,   7,  12,  10,    1,  -12 }, // 'L'
	  {   449,  11,  12,  14,    1,  -12 }, // 'M'
	  {   466,  10,  12,  13,    1,  -12 }, // 'N'
	  {   481,  11,  12,  14,    1,  -12 }, // 'O'
	  {   498,   8,  12,  11,    1,  -12 }, // 'P'
	  {   510,  11,  14,  14,    1,  -12 }, // 'Q'
	  {   530,  10,  12,  12,    1,  -12 }, // 'R'
	  {   545,   8,  12,  11,    1,  -12 }, // 'S'
	  {   557,   9,  12,  10,    0,  -12 }, // 'T'
	  {   571,  10,  12,  13,    1,  -12 }, // 'U'
	  {   586,  11,  12,  12,    0,  -12 }, // 'V'
	  {   603,  15,  12,  18,    1,  -12 }, // 'W'
	  {   626,   9,  12,  12,    1,  -12 }, // 'X'
	  {   640,   9,  12,  10,    0,  -12 }, // 'Y'
	  {   654,  10,  12,  13,    1,  -12 }, // 'Z'
	  {   669,   3,  14,   7,    1,  -12 }, // '['
	  {   675,   5,  13,   6,    0,  -12 }, // '\'
	  {   684,   3,  14,   7,    2,  -12 }, // ']'
	  {   690,  10,   4,  14,    2,  -12 }, // '^'
	  {   695,   8,   1,   9,    0,    3 }, // '_'
	  {   696,   4,   3,   9,    1,  -13 }, // '`'
	  {   698,   7,   9,  10,    1,   -9 }, // 'a'
	  {   706,   8,  12,  11,    1,  -12 }, // 'b'
	  {   718,   7,   9,  10,    1,   -9 }, // 'c'
	  {   726,   8,  12,  11,    1,  -12 }, // 'd'
	  {   738,   7,   9,  10,    1,   -9 }, // 'e'
	  {   746,   5,  12,   7,    1,  -12 }, // 'f'
	  {   754,   8,  12,  11,    1,   -9 }, // 'g'
	  {   766,   8,  12,  11,    1,  -12 }, // 'h'
	  {   778,   1,  12,   4,    1,  -12 }, // 'i'
	  {   780,   3,  15,   4,   -1,  -12 }, // 'j'
	  {   786,   8,  12,  10,    1,  -12 }, // 'k'
	  {   798,   1,  12,   4,    1,  -12 }, // 'l'
	  {   800,  13,   9,  16,    1,   -9 }, // 'm'
	  {   815,   8,   9,  11,    1,   -9 }, // 'n'
	  {   824,   8,   9,  11,    1,   -9 }, // 'o'
	  {   833,   8,  12,  11,    1,   -9 }, // 'p'
	  {   845,   8,  12,  11,    1,   -9 }, // 'q'
	  {   857,   5,   9,   8,    1,   -9 }, // 'r'
	  {   863,   7,   9,  10,    1,   -9 }, // 's'
	  {   871,   5,  11,   7,    0,  -11 }, // 't'
	  {   878,   8,   9,  11,    1,   -9 }, // 'u'
	  {   887,  10,   9,  10,    0,   -9 }, // 'v'
	  {   899,  13,   9,  14,    0,   -9 }, // 'w'
	  {   914,   8,   9,  11,    1,   -9 }, // 'x'
	  {   923,   9,  12,  10,    0,   -9 }, // 'y'
	  {   937,   7,   9,  10,    1,   -9 }, // 'z'
	  {   945,   5,  15,  11,    2,  -12 }, // '{'
	  {   955,   1,  16,   6,    2,  -12 }, // '|'
	  {   957,   5,  15,  11,    2,  -12 } // '}'
};
const GFXfont Dialog_plain_16 PROGMEM = {
(uint8_t  *)Dialog_plain_16Bitmaps,(GFXglyph *)Dialog_plain_16Glyphs,0x20, 0x7E, 19};

// Created by http://oleddisplay.squix.ch/ Consider a donation
// In case of problems make sure that you are using the font file with the correct version!
const uint8_t Dialog_plain_8Bitmaps[] PROGMEM = {

	// Bitmap Data:
	0x00, // ' '
	0xF4, // '!'
	0xB4, // '"'
	0x32,0xBE,0xAF,0xB2,0x80, // '#'
	0x27,0xE8,0xF2,0xFC,0x80, // '$'
	0xE9,0x53,0xC0,0xF2,0xA5,0xC0, // '%'
	0x72,0x19,0xDB,0x3C, // '&'
	0xC0, // '''
	0x6A,0xA4, // '('
	0x95,0x58, // ')'
	0xAB,0x9D,0x50, // '*'
	0x21,0x3E,0x42,0x00, // '+'
	0xC0, // ','
	0xC0, // '-'
	0x80, // '.'
	0x25,0x25,0x20, // '/'
	0x69,0x99,0x96, // '0'
	0xC9,0x25,0xC0, // '1'
	0xE1,0x12,0x4F, // '2'
	0xE1,0x16,0x1F, // '3'
	0x26,0x6A,0xF2, // '4'
	0xF8,0xE1,0x1E, // '5'
	0x7C,0x8F,0x97, // '6'
	0xF1,0x22,0x24, // '7'
	0x69,0x96,0x9F, // '8'
	0xE9,0xF1,0x3E, // '9'
	0x90, // ':'
	0x98, // ';'
	0x0B,0xBC,0x10, // '<'
	0xF8,0x3E, // '='
	0x83,0x9F,0x00, // '>'
	0xE5,0x20,0x80, // '?'
	0x3C,0x8E,0xED,0x5B,0xC8,0x0E,0x00, // '@'
	0x21,0x14,0xA7,0x44, // 'A'
	0xF9,0x9E,0x9F, // 'B'
	0x76,0x61,0x0C,0x3C, // 'C'
	0xF4,0x63,0x18,0xF8, // 'D'
	0xF8,0x8F,0x8F, // 'E'
	0xF2,0x79,0x00, // 'F'
	0x76,0x67,0x18,0xB8, // 'G'
	0x99,0x9F,0x99, // 'H'
	0xFC, // 'I'
	0x55,0x57, // 'J'
	0x95,0x31,0xCB,0x4C, // 'K'
	0x88,0x88,0x8F, // 'L'
	0xDE,0xFF,0x5A,0xC4, // 'M'
	0x9D,0xDB,0xB9, // 'N'
	0x74,0x63,0x18,0xB8, // 'O'
	0xF9,0xF8,0x88, // 'P'
	0x74,0x63,0x18,0xB8,0x40, // 'Q'
	0xF4,0xB9,0x69,0x44, // 'R'
	0x78,0xE3,0x1F, // 'S'
	0xF9,0x08,0x42,0x10, // 'T'
	0x99,0x99,0x96, // 'U'
	0x8A,0x94,0xA2,0x10, // 'V'
	0x93,0x55,0xB3,0x66,0xC8,0x80, // 'W'
	0xDA,0x88,0x45,0x44, // 'X'
	0xDA,0x88,0x42,0x10, // 'Y'
	0xF8,0x88,0xC4,0x7C, // 'Z'
	0xEA,0xAC, // '['
	0x91,0x24,0x48, // '\'
	0xD5,0x5C, // ']'
	0x26,0x80, // '^'
	0xF0, // '_'
	0x80, // '`'
	0x7F,0x9F, // 'a'
	0x88,0x8E,0x99,0xE0, // 'b'
	0x72,0x30, // 'c'
	0x11,0x17,0x99,0x70, // 'd'
	0x7F,0x87, // 'e'
	0x69,0x74,0x90, // 'f'
	0x79,0x97,0x16, // 'g'
	0x88,0x8F,0x99,0x90, // 'h'
	0xBC, // 'i'
	0x45,0x57, // 'j'
	0x88,0x8A,0xCC,0xA0, // 'k'
	0xFE, // 'l'
	0xFF,0x26,0x4C,0x90, // 'm'
	0xF9,0x99, // 'n'
	0x69,0x96, // 'o'
	0xE9,0x9E,0x88, // 'p'
	0x79,0x97,0x11, // 'q'
	0xF2,0x40, // 'r'
	0xF8,0xF0, // 's'
	0x5D,0x26, // 't'
	0x99,0x9F, // 'u'
	0x96,0x66, // 'v'
	0xB6,0xD4,0x92, // 'w'
	0xF6,0x6F, // 'x'
	0x96,0x64,0x4C, // 'y'
	0xF2,0x6F, // 'z'
	0x69,0x44,0x98, // '{'
	0xFF, // '|'
	0xC9,0x14,0xB0 // '}'
};
const GFXglyph Dialog_plain_8Glyphs[] PROGMEM = {
// bitmapOffset, width, height, xAdvance, xOffset, yOffset
	  {     0,   1,   1,   4,    0,   -1 }, // ' '
	  {     1,   1,   6,   4,    1,   -6 }, // '!'
	  {     2,   3,   2,   5,    1,   -6 }, // '"'
	  {     3,   5,   7,   8,    1,   -7 }, // '#'
	  {     8,   5,   7,   6,    1,   -6 }, // '$'
	  {    13,   7,   6,   9,    1,   -6 }, // '%'
	  {    19,   5,   6,   7,    1,   -6 }, // '&'
	  {    23,   1,   2,   3,    1,   -6 }, // '''
	  {    24,   2,   7,   4,    1,   -7 }, // '('
	  {    26,   2,   7,   4,    1,   -7 }, // ')'
	  {    28,   5,   4,   5,    0,   -6 }, // '*'
	  {    31,   5,   5,   8,    1,   -5 }, // '+'
	  {    35,   1,   2,   4,    1,   -1 }, // ','
	  {    36,   2,   1,   4,    1,   -3 }, // '-'
	  {    37,   1,   1,   4,    1,   -1 }, // '.'
	  {    38,   3,   7,   4,    0,   -6 }, // '/'
	  {    41,   4,   6,   6,    1,   -6 }, // '0'
	  {    44,   3,   6,   6,    1,   -6 }, // '1'
	  {    47,   4,   6,   6,    1,   -6 }, // '2'
	  {    50,   4,   6,   6,    1,   -6 }, // '3'
	  {    53,   4,   6,   6,    1,   -6 }, // '4'
	  {    56,   4,   6,   6,    1,   -6 }, // '5'
	  {    59,   4,   6,   6,    1,   -6 }, // '6'
	  {    62,   4,   6,   6,    1,   -6 }, // '7'
	  {    65,   4,   6,   6,    1,   -6 }, // '8'
	  {    68,   4,   6,   6,    1,   -6 }, // '9'
	  {    71,   1,   4,   4,    1,   -4 }, // ':'
	  {    72,   1,   5,   4,    1,   -4 }, // ';'
	  {    73,   5,   4,   8,    1,   -5 }, // '<'
	  {    76,   5,   3,   8,    1,   -4 }, // '='
	  {    78,   5,   4,   8,    1,   -5 }, // '>'
	  {    81,   3,   6,   5,    1,   -6 }, // '?'
	  {    84,   7,   7,   9,    1,   -6 }, // '@'
	  {    91,   5,   6,   6,    0,   -6 }, // 'A'
	  {    95,   4,   6,   6,    1,   -6 }, // 'B'
	  {    98,   5,   6,   7,    1,   -6 }, // 'C'
	  {   102,   5,   6,   7,    1,   -6 }, // 'D'
	  {   106,   4,   6,   6,    1,   -6 }, // 'E'
	  {   109,   3,   6,   6,    1,   -6 }, // 'F'
	  {   112,   5,   6,   7,    1,   -6 }, // 'G'
	  {   116,   4,   6,   7,    1,   -6 }, // 'H'
	  {   119,   1,   6,   3,    1,   -6 }, // 'I'
	  {   120,   2,   8,   3,    0,   -6 }, // 'J'
	  {   122,   5,   6,   6,    1,   -6 }, // 'K'
	  {   126,   4,   6,   5,    1,   -6 }, // 'L'
	  {   129,   5,   6,   8,    1,   -6 }, // 'M'
	  {   133,   4,   6,   7,    1,   -6 }, // 'N'
	  {   136,   5,   6,   7,    1,   -6 }, // 'O'
	  {   140,   4,   6,   6,    1,   -6 }, // 'P'
	  {   143,   5,   7,   7,    1,   -6 }, // 'Q'
	  {   148,   5,   6,   7,    1,   -6 }, // 'R'
	  {   152,   4,   6,   6,    1,   -6 }, // 'S'
	  {   155,   5,   6,   6,    0,   -6 }, // 'T'
	  {   159,   4,   6,   7,    1,   -6 }, // 'U'
	  {   162,   5,   6,   6,    0,   -6 }, // 'V'
	  {   166,   7,   6,   9,    0,   -6 }, // 'W'
	  {   172,   5,   6,   6,    0,   -6 }, // 'X'
	  {   176,   5,   6,   6,    0,   -6 }, // 'Y'
	  {   180,   5,   6,   6,    1,   -6 }, // 'Z'
	  {   184,   2,   7,   4,    1,   -6 }, // '['
	  {   186,   3,   7,   4,    0,   -6 }, // '\'
	  {   189,   2,   7,   4,    1,   -6 }, // ']'
	  {   191,   5,   2,   8,    1,   -6 }, // '^'
	  {   193,   4,   1,   5,    0,    1 }, // '_'
	  {   194,   2,   1,   5,    1,   -6 }, // '`'
	  {   195,   4,   4,   6,    1,   -4 }, // 'a'
	  {   197,   4,   7,   6,    1,   -7 }, // 'b'
	  {   201,   3,   4,   5,    1,   -4 }, // 'c'
	  {   203,   4,   7,   6,    1,   -7 }, // 'd'
	  {   207,   4,   4,   6,    1,   -4 }, // 'e'
	  {   209,   3,   7,   4,    0,   -7 }, // 'f'
	  {   212,   4,   6,   6,    1,   -4 }, // 'g'
	  {   215,   4,   7,   6,    1,   -7 }, // 'h'
	  {   219,   1,   6,   3,    1,   -6 }, // 'i'
	  {   220,   2,   8,   3,    0,   -6 }, // 'j'
	  {   222,   4,   7,   6,    1,   -7 }, // 'k'
	  {   226,   1,   7,   3,    1,   -7 }, // 'l'
	  {   227,   7,   4,   9,    1,   -4 }, // 'm'
	  {   231,   4,   4,   6,    1,   -4 }, // 'n'
	  {   233,   4,   4,   6,    1,   -4 }, // 'o'
	  {   235,   4,   6,   6,    1,   -4 }, // 'p'
	  {   238,   4,   6,   6,    1,   -4 }, // 'q'
	  {   241,   3,   4,   4,    1,   -4 }, // 'r'
	  {   243,   3,   4,   5,    1,   -4 }, // 's'
	  {   245,   3,   5,   4,    0,   -5 }, // 't'
	  {   247,   4,   4,   6,    1,   -4 }, // 'u'
	  {   249,   4,   4,   6,    0,   -4 }, // 'v'
	  {   251,   6,   4,   8,    0,   -4 }, // 'w'
	  {   254,   4,   4,   6,    0,   -4 }, // 'x'
	  {   256,   4,   6,   6,    1,   -4 }, // 'y'
	  {   259,   4,   4,   5,    1,   -4 }, // 'z'
	  {   261,   3,   7,   6,    1,   -6 }, // '{'
	  {   264,   1,   8,   4,    1,   -6 }, // '|'
	  {   265,   3,   7,   6,    1,   -6 } // '}'
};
const GFXfont Dialog_plain_8 PROGMEM = {
(uint8_t  *)Dialog_plain_8Bitmaps,(GFXglyph *)Dialog_plain_8Glyphs,0x20, 0x7E, 10};

// Created by http://oleddisplay.squix.ch/ Consider a donation
// In case of problems make sure that you are using the font file with the correct version!
const uint8_t Dialog_plain_10Bitmaps[] PROGMEM = {

	// Bitmap Data:
	0x00, // ' '
	0xFA, // '!'
	0xB6,0x80, // '"'
	0x24,0x49,0xF9,0x4F,0xC9,0x12,0x00, // '#'
	0x23,0xE9,0xC3,0x97,0xC4, // '$'
	0xE4,0xA4,0xA8,0xFF,0x15,0x25,0x27, // '%'
	0x30,0x91,0x05,0x98,0xB9,0x9C,0x80, // '&'
	0xE0, // '''
	0x6A,0xAA,0x40, // '('
	0xA5,0x56,0x80, // ')'
	0xAB,0x9D,0x50, // '*'
	0x10,0x20,0x47,0xF1,0x02,0x04,0x00, // '+'
	0xC0, // ','
	0xE0, // '-'
	0x80, // '.'
	0x25,0x24,0xA4, // '/'
	0x74,0x63,0x18,0xC5,0xC0, // '0'
	0xE1,0x08,0x42,0x13,0xE0, // '1'
	0x74,0x42,0x22,0x23,0xE0, // '2'
	0x74,0x42,0xE0,0xC5,0xC0, // '3'
	0x11,0x95,0x2F,0x88,0x40, // '4'
	0xF4,0x3C,0x10,0x87,0xC0, // '5'
	0x7E,0x21,0xE8,0xC5,0xC0, // '6'
	0xF8,0x44,0x22,0x11,0x00, // '7'
	0x74,0x62,0xE8,0xC5,0xC0, // '8'
	0x74,0x62,0xF0,0x8F,0xC0, // '9'
	0x88, // ':'
	0x8C, // ';'
	0x04,0xEC,0x0E,0x04, // '<'
	0xFC,0x0F,0xC0, // '='
	0x81,0xC0,0xDC,0x80, // '>'
	0xF1,0x24,0x40,0x40, // '?'
	0x3E,0x30,0xB0,0x33,0x99,0x5C,0xFB,0x00,0xC4,0x3C,0x00, // '@'
	0x10,0x50,0xA2,0x27,0xC8,0xA0,0x80, // 'A'
	0xF4,0x63,0xE8,0xC7,0xC0, // 'B'
	0x39,0x18,0x20,0x81,0x13,0x80, // 'C'
	0xFA,0x38,0x61,0x86,0x3F,0x80, // 'D'
	0xFC,0x21,0xF8,0x43,0xE0, // 'E'
	0xF8,0x8F,0x88,0x80, // 'F'
	0x7B,0x18,0x27,0x87,0x17,0x80, // 'G'
	0x86,0x18,0x7F,0x86,0x18,0x40, // 'H'
	0xFE, // 'I'
	0x24,0x92,0x49,0xC0, // 'J'
	0x8A,0x4A,0x30,0xA2,0x48,0x80, // 'K'
	0x84,0x21,0x08,0x43,0xE0, // 'L'
	0x83,0x8F,0x1D,0x5A,0xB2,0x60,0x80, // 'M'
	0x87,0x1A,0x69,0x96,0x38,0x40, // 'N'
	0x7B,0x38,0x61,0x87,0x37,0x80, // 'O'
	0xF4,0x63,0xE8,0x42,0x00, // 'P'
	0x7B,0x38,0x61,0x87,0x27,0x02, // 'Q'
	0xF2,0x28,0xBC,0x92,0x28,0x40, // 'R'
	0x74,0x60,0xE0,0xC5,0xC0, // 'S'
	0xF9,0x08,0x42,0x10,0x80, // 'T'
	0x86,0x18,0x61,0x86,0x17,0x80, // 'U'
	0x83,0x05,0x12,0x22,0x85,0x04,0x00, // 'V'
	0x88,0xC4,0x55,0x4A,0xA5,0x51,0x10,0x88, // 'W'
	0xCD,0x23,0x0C,0x31,0x2C,0xC0, // 'X'
	0x82,0x88,0xA0,0x81,0x02,0x04,0x00, // 'Y'
	0xFC,0x21,0x0C,0x21,0x0F,0xC0, // 'Z'
	0xEA,0xAA,0xC0, // '['
	0x91,0x24,0x89, // '\'
	0xD5,0x55,0xC0, // ']'
	0x31,0x28,0x40, // '^'
	0xF8, // '_'
	0x90, // '`'
	0x70,0x5F,0x1F,0x80, // 'a'
	0x84,0x21,0xE8,0xC6,0x3E, // 'b'
	0x78,0x88,0x70, // 'c'
	0x08,0x42,0xF8,0xC6,0x2F, // 'd'
	0x74,0x7F,0x07,0x80, // 'e'
	0x74,0x4E,0x44,0x44, // 'f'
	0x7C,0x63,0x17,0x85,0xC0, // 'g'
	0x84,0x21,0xE8,0xC6,0x31, // 'h'
	0x9F, // 'i'
	0x41,0x55,0x70, // 'j'
	0x84,0x21,0x2A,0x62,0x92, // 'k'
	0xFF, // 'l'
	0xF7,0x44,0x62,0x31,0x18,0x88, // 'm'
	0xF4,0x63,0x18,0x80, // 'n'
	0x74,0x63,0x17,0x00, // 'o'
	0xF4,0x63,0x1F,0x42,0x00, // 'p'
	0x7C,0x63,0x17,0x84,0x20, // 'q'
	0xF2,0x48, // 'r'
	0xF8,0x71,0xF0, // 's'
	0x44,0xF4,0x44,0x70, // 't'
	0x8C,0x63,0x17,0x80, // 'u'
	0x8C,0x54,0xA2,0x00, // 'v'
	0x93,0x56,0xAA,0x24,0x40, // 'w'
	0x8A,0x88,0xA8,0x80, // 'x'
	0x45,0x12,0x8A,0x10,0x46,0x00, // 'y'
	0xF1,0x24,0xF0, // 'z'
	0x32,0x22,0xC2,0x22,0x30, // '{'
	0xFF,0xC0, // '|'
	0xC4,0x44,0x34,0x44,0xC0 // '}'
};
const GFXglyph Dialog_plain_10Glyphs[] PROGMEM = {
// bitmapOffset, width, height, xAdvance, xOffset, yOffset
	  {     0,   1,   1,   4,    0,   -1 }, // ' '
	  {     1,   1,   7,   5,    2,   -7 }, // '!'
	  {     2,   3,   3,   6,    1,   -7 }, // '"'
	  {     4,   7,   7,   9,    1,   -7 }, // '#'
	  {    11,   5,   8,   7,    1,   -7 }, // '$'
	  {    16,   8,   7,  11,    1,   -7 }, // '%'
	  {    23,   7,   7,  10,    1,   -7 }, // '&'
	  {    30,   1,   3,   4,    1,   -7 }, // '''
	  {    31,   2,   9,   5,    1,   -8 }, // '('
	  {    34,   2,   9,   5,    1,   -8 }, // ')'
	  {    37,   5,   4,   6,    0,   -7 }, // '*'
	  {    40,   7,   7,   9,    1,   -7 }, // '+'
	  {    47,   1,   2,   4,    1,   -1 }, // ','
	  {    48,   3,   1,   5,    1,   -3 }, // '-'
	  {    49,   1,   1,   4,    1,   -1 }, // '.'
	  {    50,   3,   8,   4,    0,   -7 }, // '/'
	  {    53,   5,   7,   7,    1,   -7 }, // '0'
	  {    58,   5,   7,   7,    1,   -7 }, // '1'
	  {    63,   5,   7,   7,    1,   -7 }, // '2'
	  {    68,   5,   7,   7,    1,   -7 }, // '3'
	  {    73,   5,   7,   7,    1,   -7 }, // '4'
	  {    78,   5,   7,   7,    1,   -7 }, // '5'
	  {    83,   5,   7,   7,    1,   -7 }, // '6'
	  {    88,   5,   7,   7,    1,   -7 }, // '7'
	  {    93,   5,   7,   7,    1,   -7 }, // '8'
	  {    98,   5,   7,   7,    1,   -7 }, // '9'
	  {   103,   1,   5,   4,    1,   -5 }, // ':'
	  {   104,   1,   6,   4,    1,   -5 }, // ';'
	  {   105,   6,   5,   9,    1,   -6 }, // '<'
	  {   109,   6,   3,   9,    1,   -5 }, // '='
	  {   112,   6,   5,   9,    1,   -6 }, // '>'
	  {   116,   4,   7,   6,    1,   -7 }, // '?'
	  {   120,   9,   9,  12,    1,   -7 }, // '@'
	  {   131,   7,   7,   8,    0,   -7 }, // 'A'
	  {   138,   5,   7,   8,    1,   -7 }, // 'B'
	  {   143,   6,   7,   9,    1,   -7 }, // 'C'
	  {   149,   6,   7,   9,    1,   -7 }, // 'D'
	  {   155,   5,   7,   8,    1,   -7 }, // 'E'
	  {   160,   4,   7,   7,    1,   -7 }, // 'F'
	  {   164,   6,   7,   9,    1,   -7 }, // 'G'
	  {   170,   6,   7,   9,    1,   -7 }, // 'H'
	  {   176,   1,   7,   4,    1,   -7 }, // 'I'
	  {   177,   3,   9,   4,   -1,   -7 }, // 'J'
	  {   181,   6,   7,   8,    1,   -7 }, // 'K'
	  {   187,   5,   7,   7,    1,   -7 }, // 'L'
	  {   192,   7,   7,  10,    1,   -7 }, // 'M'
	  {   199,   6,   7,   9,    1,   -7 }, // 'N'
	  {   205,   6,   7,   9,    1,   -7 }, // 'O'
	  {   211,   5,   7,   8,    1,   -7 }, // 'P'
	  {   216,   6,   8,   9,    1,   -7 }, // 'Q'
	  {   222,   6,   7,   8,    1,   -7 }, // 'R'
	  {   228,   5,   7,   8,    1,   -7 }, // 'S'
	  {   233,   5,   7,   6,    0,   -7 }, // 'T'
	  {   238,   6,   7,   9,    1,   -7 }, // 'U'
	  {   244,   7,   7,   8,    0,   -7 }, // 'V'
	  {   251,   9,   7,  10,    0,   -7 }, // 'W'
	  {   259,   6,   7,   7,    0,   -7 }, // 'X'
	  {   265,   7,   7,   8,    0,   -7 }, // 'Y'
	  {   272,   6,   7,   7,    0,   -7 }, // 'Z'
	  {   278,   2,   9,   5,    1,   -8 }, // '['
	  {   281,   3,   8,   4,    0,   -7 }, // '\'
	  {   284,   2,   9,   5,    1,   -8 }, // ']'
	  {   287,   6,   3,   9,    1,   -7 }, // '^'
	  {   290,   5,   1,   6,    0,    1 }, // '_'
	  {   291,   2,   2,   6,    1,   -8 }, // '`'
	  {   292,   5,   5,   7,    1,   -5 }, // 'a'
	  {   296,   5,   8,   7,    1,   -8 }, // 'b'
	  {   301,   4,   5,   6,    1,   -5 }, // 'c'
	  {   304,   5,   8,   7,    1,   -8 }, // 'd'
	  {   309,   5,   5,   7,    1,   -5 }, // 'e'
	  {   313,   4,   8,   5,    1,   -8 }, // 'f'
	  {   317,   5,   7,   7,    1,   -5 }, // 'g'
	  {   322,   5,   8,   7,    1,   -8 }, // 'h'
	  {   327,   1,   8,   3,    1,   -8 }, // 'i'
	  {   328,   2,  10,   3,    0,   -8 }, // 'j'
	  {   331,   5,   8,   6,    1,   -8 }, // 'k'
	  {   336,   1,   8,   3,    1,   -8 }, // 'l'
	  {   337,   9,   5,  11,    1,   -5 }, // 'm'
	  {   343,   5,   5,   7,    1,   -5 }, // 'n'
	  {   347,   5,   5,   7,    1,   -5 }, // 'o'
	  {   351,   5,   7,   7,    1,   -5 }, // 'p'
	  {   356,   5,   7,   7,    1,   -5 }, // 'q'
	  {   361,   3,   5,   5,    1,   -5 }, // 'r'
	  {   363,   4,   5,   6,    1,   -5 }, // 's'
	  {   366,   4,   7,   5,    0,   -7 }, // 't'
	  {   370,   5,   5,   7,    1,   -5 }, // 'u'
	  {   374,   5,   5,   7,    1,   -5 }, // 'v'
	  {   378,   7,   5,   9,    1,   -5 }, // 'w'
	  {   383,   5,   5,   7,    1,   -5 }, // 'x'
	  {   387,   6,   7,   7,    0,   -5 }, // 'y'
	  {   393,   4,   5,   6,    1,   -5 }, // 'z'
	  {   396,   4,   9,   7,    1,   -8 }, // '{'
	  {   401,   1,  10,   4,    1,   -8 }, // '|'
	  {   403,   4,   9,   7,    1,   -8 } // '}'
};
const GFXfont Dialog_plain_10 PROGMEM = {
(uint8_t  *)Dialog_plain_10Bitmaps,(GFXglyph *)Dialog_plain_10Glyphs,0x20, 0x7E, 13};

// Created by http://oleddisplay.squix.ch/ Consider a donation
// In case of problems make sure that you are using the font file with the correct version!
const uint8_t Dialog_plain_12Bitmaps[] PROGMEM = {

	// Bitmap Data:
	0x00, // ' '
	0xFD,0x80, // '!'
	0xB6,0x80, // '"'
	0x12,0x14,0x7F,0x24,0x24,0xFE,0x28,0x48, // '#'
	0x23,0xAB,0x4E,0x1C,0xB5,0x71,0x08, // '$'
	0x61,0x24,0x89,0x22,0x50,0x6D,0x82,0x91,0x24,0x49,0x21,0x80, // '%'
	0x30,0x48,0x40,0x60,0x51,0x89,0x86,0xC4,0x7B, // '&'
	0xE0, // '''
	0x69,0x49,0x24,0x49,0x80, // '('
	0xC9,0x12,0x49,0x4B,0x00, // ')'
	0x25,0x5C,0xEA,0x90, // '*'
	0x10,0x20,0x47,0xF1,0x02,0x04,0x00, // '+'
	0xE0, // ','
	0xE0, // '-'
	0xC0, // '.'
	0x11,0x22,0x24,0x44,0x88, // '/'
	0x79,0x28,0x61,0x86,0x18,0x52,0x78, // '0'
	0xE1,0x08,0x42,0x10,0x84,0xF8, // '1'
	0x7A,0x30,0x41,0x08,0x42,0x10,0xFC, // '2'
	0x7A,0x10,0x41,0x38,0x10,0x61,0x78, // '3'
	0x18,0x62,0x92,0x4A,0x2F,0xC2,0x08, // '4'
	0xFA,0x08,0x3E,0x0C,0x10,0x63,0x78, // '5'
	0x39,0x18,0x2E,0xCE,0x18,0x53,0x78, // '6'
	0xFC,0x10,0x82,0x10,0x42,0x08,0x40, // '7'
	0x7A,0x18,0x61,0x7A,0x18,0x61,0x78, // '8'
	0x7B,0x28,0x61,0xCD,0xD0,0x62,0x70, // '9'
	0xCC, // ':'
	0xCE, // ';'
	0x03,0x1E,0xE0,0xE0,0x1E,0x03, // '<'
	0xFF,0x00,0xFF, // '='
	0xC0,0x78,0x07,0x07,0x78,0xC0, // '>'
	0x74,0x42,0x22,0x10,0x04,0x20, // '?'
	0x1F,0x04,0x19,0x01,0x47,0x99,0x13,0x22,0x64,0x54,0x7C,0x40,0x04,0x10,0x7C,0x00, // '@'
	0x18,0x18,0x24,0x24,0x24,0x42,0x7E,0x42,0x81, // 'A'
	0xFA,0x18,0x61,0xFA,0x18,0x61,0xF8, // 'B'
	0x39,0x18,0x20,0x82,0x08,0x11,0x38, // 'C'
	0xF9,0x0A,0x0C,0x18,0x30,0x60,0xC2,0xF8, // 'D'
	0xFE,0x08,0x20,0xFE,0x08,0x20,0xFC, // 'E'
	0xFC,0x21,0x0F,0xC2,0x10,0x80, // 'F'
	0x3C,0x86,0x04,0x08,0xF0,0x60,0xA1,0x3C, // 'G'
	0x83,0x06,0x0C,0x1F,0xF0,0x60,0xC1,0x82, // 'H'
	0xFF,0x80, // 'I'
	0x24,0x92,0x49,0x27,0x00, // 'J'
	0x85,0x12,0x45,0x0C,0x14,0x24,0x44,0x84, // 'K'
	0x84,0x21,0x08,0x42,0x10,0xF8, // 'L'
	0x81,0xC3,0xC3,0xA5,0xA5,0x99,0x99,0x81,0x81, // 'M'
	0xC3,0x86,0x8D,0x19,0x31,0x62,0xC3,0x86, // 'N'
	0x38,0x8A,0x0C,0x18,0x30,0x60,0xA2,0x38, // 'O'
	0xFA,0x18,0x61,0xFA,0x08,0x20,0x80, // 'P'
	0x38,0x8A,0x0C,0x18,0x30,0x60,0xA2,0x38,0x10,0x10, // 'Q'
	0xF9,0x0A,0x14,0x2F,0x91,0x21,0x42,0x82, // 'R'
	0x7A,0x18,0x20,0x78,0x10,0x61,0x78, // 'S'
	0xFE,0x20,0x40,0x81,0x02,0x04,0x08,0x10, // 'T'
	0x83,0x06,0x0C,0x18,0x30,0x60,0xE3,0x7C, // 'U'
	0x40,0x90,0x22,0x10,0x84,0x21,0x04,0x81,0x20,0x30,0x0C,0x00, // 'V'
	0x84,0x28,0x89,0x11,0x25,0x22,0xA8,0x55,0x0A,0xA0,0x88,0x11,0x00, // 'W'
	0xC6,0x88,0xA1,0x41,0x05,0x0A,0x22,0x82, // 'X'
	0x82,0x89,0x11,0x42,0x82,0x04,0x08,0x10, // 'Y'
	0xFE,0x04,0x10,0x41,0x04,0x10,0x40,0xFE, // 'Z'
	0xEA,0xAA,0xAC, // '['
	0x88,0x44,0x42,0x22,0x11, // '\'
	0xD5,0x55,0x5C, // ']'
	0x18,0x24,0x42, // '^'
	0xFC, // '_'
	0x44, // '`'
	0x7A,0x10,0x5F,0x86,0x37,0x40, // 'a'
	0x82,0x08,0x3E,0xCE,0x18,0x61,0xCF,0xE0, // 'b'
	0x76,0x61,0x08,0x65,0xC0, // 'c'
	0x04,0x10,0x5F,0xCE,0x18,0x61,0xCD,0xF0, // 'd'
	0x7B,0x38,0x7F,0x83,0x17,0x80, // 'e'
	0x34,0x4F,0x44,0x44,0x44, // 'f'
	0x7F,0x38,0x61,0x87,0x37,0xC1,0x4C,0xE0, // 'g'
	0x82,0x08,0x2E,0xC6,0x18,0x61,0x86,0x10, // 'h'
	0xBF,0x80, // 'i'
	0x45,0x55,0x57, // 'j'
	0x82,0x08,0x22,0x92,0x8C,0x28,0x92,0x20, // 'k'
	0xFF,0xC0, // 'l'
	0xF7,0x44,0x62,0x31,0x18,0x8C,0x46,0x22, // 'm'
	0xBB,0x18,0x61,0x86,0x18,0x40, // 'n'
	0x7B,0x38,0x61,0x87,0x37,0x80, // 'o'
	0xFB,0x38,0x61,0x87,0x3F,0xA0,0x82,0x00, // 'p'
	0x7F,0x38,0x61,0x87,0x37,0xC1,0x04,0x10, // 'q'
	0xBC,0x88,0x88,0x80, // 'r'
	0x74,0x60,0xE0,0xC5,0xC0, // 's'
	0x44,0xF4,0x44,0x44,0x70, // 't'
	0x86,0x18,0x61,0x86,0x37,0x40, // 'u'
	0x86,0x14,0x92,0x48,0xC3,0x00, // 'v'
	0x88,0xC4,0x55,0x4A,0xA5,0x51,0x10,0x88, // 'w'
	0x85,0x24,0x8C,0x49,0x28,0x40, // 'x'
	0x86,0x14,0x92,0x28,0xC1,0x04,0x23,0x00, // 'y'
	0xF8,0x44,0x44,0x43,0xE0, // 'z'
	0x39,0x08,0x42,0x60,0x84,0x21,0x0E, // '{'
	0xFF,0xF0, // '|'
	0xE1,0x08,0x42,0x0C,0x84,0x21,0x38 // '}'
};
const GFXglyph Dialog_plain_12Glyphs[] PROGMEM = {
// bitmapOffset, width, height, xAdvance, xOffset, yOffset
	  {     0,   1,   1,   5,    0,   -1 }, // ' '
	  {     1,   1,   9,   6,    2,   -9 }, // '!'
	  {     3,   3,   3,   6,    1,   -9 }, // '"'
	  {     5,   8,   8,  11,    1,   -8 }, // '#'
	  {    13,   5,  11,   9,    2,   -9 }, // '$'
	  {    20,  10,   9,  12,    0,   -9 }, // '%'
	  {    32,   8,   9,  11,    1,   -9 }, // '&'
	  {    41,   1,   3,   4,    1,   -9 }, // '''
	  {    42,   3,  11,   6,    1,  -10 }, // '('
	  {    47,   3,  11,   6,    1,  -10 }, // ')'
	  {    52,   5,   6,   7,    1,   -9 }, // '*'
	  {    56,   7,   7,  11,    1,   -7 }, // '+'
	  {    63,   1,   3,   5,    1,   -2 }, // ','
	  {    64,   3,   1,   5,    1,   -4 }, // '-'
	  {    65,   1,   2,   5,    1,   -2 }, // '.'
	  {    66,   4,  10,   5,    0,   -9 }, // '/'
	  {    71,   6,   9,   9,    1,   -9 }, // '0'
	  {    78,   5,   9,   9,    1,   -9 }, // '1'
	  {    84,   6,   9,   9,    1,   -9 }, // '2'
	  {    91,   6,   9,   9,    1,   -9 }, // '3'
	  {    98,   6,   9,   9,    1,   -9 }, // '4'
	  {   105,   6,   9,   9,    1,   -9 }, // '5'
	  {   112,   6,   9,   9,    1,   -9 }, // '6'
	  {   119,   6,   9,   9,    1,   -9 }, // '7'
	  {   126,   6,   9,   9,    1,   -9 }, // '8'
	  {   133,   6,   9,   9,    1,   -9 }, // '9'
	  {   140,   1,   6,   5,    1,   -6 }, // ':'
	  {   141,   1,   7,   5,    1,   -6 }, // ';'
	  {   142,   8,   6,  11,    1,   -7 }, // '<'
	  {   148,   8,   3,  11,    1,   -5 }, // '='
	  {   151,   8,   6,  11,    1,   -7 }, // '>'
	  {   157,   5,   9,   7,    0,   -9 }, // '?'
	  {   163,  11,  11,  14,    1,   -9 }, // '@'
	  {   179,   8,   9,   9,    0,   -9 }, // 'A'
	  {   188,   6,   9,   9,    1,   -9 }, // 'B'
	  {   195,   6,   9,   9,    1,   -9 }, // 'C'
	  {   202,   7,   9,  10,    1,   -9 }, // 'D'
	  {   210,   6,   9,   9,    1,   -9 }, // 'E'
	  {   217,   5,   9,   8,    1,   -9 }, // 'F'
	  {   223,   7,   9,  10,    1,   -9 }, // 'G'
	  {   231,   7,   9,  10,    1,   -9 }, // 'H'
	  {   239,   1,   9,   4,    1,   -9 }, // 'I'
	  {   241,   3,  11,   4,   -1,   -9 }, // 'J'
	  {   246,   7,   9,   8,    1,   -9 }, // 'K'
	  {   254,   5,   9,   7,    1,   -9 }, // 'L'
	  {   260,   8,   9,  11,    1,   -9 }, // 'M'
	  {   269,   7,   9,  10,    1,   -9 }, // 'N'
	  {   277,   7,   9,  10,    1,   -9 }, // 'O'
	  {   285,   6,   9,   9,    1,   -9 }, // 'P'
	  {   292,   7,  11,  10,    1,   -9 }, // 'Q'
	  {   302,   7,   9,   9,    1,   -9 }, // 'R'
	  {   310,   6,   9,   9,    1,   -9 }, // 'S'
	  {   317,   7,   9,   8,    0,   -9 }, // 'T'
	  {   325,   7,   9,  10,    1,   -9 }, // 'U'
	  {   333,  10,   9,   9,   -1,   -9 }, // 'V'
	  {   345,  11,   9,  12,    0,   -9 }, // 'W'
	  {   358,   7,   9,   8,    0,   -9 }, // 'X'
	  {   366,   7,   9,   8,    0,   -9 }, // 'Y'
	  {   374,   7,   9,  10,    1,   -9 }, // 'Z'
	  {   382,   2,  11,   6,    2,   -9 }, // '['
	  {   385,   4,  10,   5,    0,   -9 }, // '\'
	  {   390,   2,  11,   6,    1,   -9 }, // ']'
	  {   393,   8,   3,  11,    1,   -9 }, // '^'
	  {   396,   6,   1,   7,    0,    2 }, // '_'
	  {   397,   3,   2,   7,    1,  -10 }, // '`'
	  {   398,   6,   7,   9,    1,   -7 }, // 'a'
	  {   404,   6,  10,   9,    1,  -10 }, // 'b'
	  {   412,   5,   7,   8,    1,   -7 }, // 'c'
	  {   417,   6,  10,   9,    1,  -10 }, // 'd'
	  {   425,   6,   7,   9,    1,   -7 }, // 'e'
	  {   431,   4,  10,   5,    0,  -10 }, // 'f'
	  {   436,   6,  10,   9,    1,   -7 }, // 'g'
	  {   444,   6,  10,   9,    1,  -10 }, // 'h'
	  {   452,   1,   9,   4,    1,   -9 }, // 'i'
	  {   454,   2,  12,   4,    0,   -9 }, // 'j'
	  {   457,   6,  10,   8,    1,  -10 }, // 'k'
	  {   465,   1,  10,   4,    1,  -10 }, // 'l'
	  {   467,   9,   7,  12,    1,   -7 }, // 'm'
	  {   475,   6,   7,   9,    1,   -7 }, // 'n'
	  {   481,   6,   7,   9,    1,   -7 }, // 'o'
	  {   487,   6,  10,   9,    1,   -7 }, // 'p'
	  {   495,   6,  10,   9,    1,   -7 }, // 'q'
	  {   503,   4,   7,   6,    1,   -7 }, // 'r'
	  {   507,   5,   7,   8,    1,   -7 }, // 's'
	  {   512,   4,   9,   6,    0,   -9 }, // 't'
	  {   517,   6,   7,   9,    1,   -7 }, // 'u'
	  {   523,   6,   7,   7,    0,   -7 }, // 'v'
	  {   529,   9,   7,  10,    0,   -7 }, // 'w'
	  {   537,   6,   7,   7,    0,   -7 }, // 'x'
	  {   543,   6,  10,   7,    0,   -7 }, // 'y'
	  {   551,   5,   7,   6,    0,   -7 }, // 'z'
	  {   556,   5,  11,   9,    2,   -9 }, // '{'
	  {   563,   1,  12,   5,    2,   -9 }, // '|'
	  {   565,   5,  11,   9,    1,   -9 } // '}'
};
const GFXfont Dialog_plain_12 PROGMEM = {
(uint8_t  *)Dialog_plain_12Bitmaps,(GFXglyph *)Dialog_plain_12Glyphs,0x20, 0x7E, 15};

#endif
