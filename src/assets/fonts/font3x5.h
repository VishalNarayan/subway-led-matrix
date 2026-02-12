#pragma once
#include <stdint.h>
#include <gfxfont.h>

/*
 * Ultra-small 3×5 digit font
 * Includes ONLY characters '0'–'9'
 * ASCII 48–57
 *
 * Width: 3
 * Height: 5
 * xAdvance: 4
 * yOffset: -4 (rise to baseline)
 * xOffset: 0
 */

const uint8_t Font3x5DigitsBitmaps[] PROGMEM = {
  // 0
  0xF6, 0xDF,
  // 1
  0x24, 0x92,
  // 2
  0xE7, 0xCF,
  // 3
  0xE7, 0x9F,
  // 4
  0xB7, 0x92,
  // 5
  0xF3, 0x9F,
  // 6
  0xF3, 0xDF,
  // 7
  0xE4, 0x92,
  // 8
  0xF7, 0xDF,
  // 9
  0xF7, 0x93,
};

const GFXglyph Font3x5DigitsGlyphs[] PROGMEM = {
  {   0, 3,5, 4, 0,-4 }, // '0' bitmapOffset = 0
  {   2, 3,5, 4, 0,-4 }, // '1'
  {   4, 3,5, 4, 0,-4 }, // '2'
  {   6, 3,5, 4, 0,-4 }, // '3'
  {   8, 3,5, 4, 0,-4 }, // '4'
  {  10, 3,5, 4, 0,-4 }, // '5'
  {  12, 3,5, 4, 0,-4 }, // '6'
  {  14, 3,5, 4, 0,-4 }, // '7'
  {  16, 3,5, 4, 0,-4 }, // '8'
  {  18, 3,5, 4, 0,-4 }, // '9'
};

const GFXfont Font3x5Digits PROGMEM = {
  (uint8_t  *)Font3x5DigitsBitmaps,
  (GFXglyph *)Font3x5DigitsGlyphs,
  48, 57, // ASCII '0' to '9'
  5       // line height
};
