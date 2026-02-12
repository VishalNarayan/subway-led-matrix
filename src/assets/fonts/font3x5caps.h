#pragma once
#include <stdint.h>
#include <gfxfont.h>

/*
 * Ultra-small 5×5 capital letter font
 * Includes characters 'A'–'Z' and arrows
 * ASCII 65–90 (A-Z), 91 (up arrow), 92 (down arrow)
 *
 * Width: 5
 * Height: 5
 * xAdvance: 6
 * yOffset: -5 (rise to baseline)
 * xOffset: 0
 */

const uint8_t Font5x5CapsBitmaps[] PROGMEM = {
  // A
  0x74, 0x7F, 0x18, 0x80,
  // B
  0xF4, 0x7D, 0x1F, 0x00,
  // C
  0x74, 0x61, 0x17, 0x00,
  // D
  0xF4, 0x63, 0x1F, 0x00,
  // E
  0xFC, 0x39, 0x0F, 0x80,
  // F
  0xF8, 0xE8, 0x88, 0x80,
  // G
  0x74, 0x61, 0xD7, 0x00,
  // H
  0x8C, 0x7F, 0x18, 0x80,
  // I
  0xF9, 0x08, 0x4F, 0x80,
  // J  
  0x38, 0x85, 0x26, 0x00,
  // K
  0x95, 0x31, 0x49, 0x00,
  // L
  0x88, 0x88, 0x8F, 0x80,
  // M
  0x8E, 0xBB, 0x18, 0x80,
  // N
  0x8E, 0x9B, 0x38, 0x80,
  // O
  0x74, 0x63, 0x17, 0x00,
  // P
  0xF4, 0x7C, 0x88, 0x80,
  // Q
  0x74, 0x63, 0x3A, 0x80,
  // R
  0xF4, 0x7C, 0xA8, 0x80,
  // S
  0x78, 0x83, 0xC1, 0xE0,
  // T
  0xF9, 0x08, 0x42, 0x00,
  // U
  0x8C, 0x63, 0x17, 0x00,
  // V
  0x8C, 0x63, 0x12, 0x00,
  // W
  0x8C, 0x6B, 0xBA, 0x80,
  // X
  0x8A, 0x88, 0xA8, 0x80,
  // Y
  0x8A, 0x88, 0x42, 0x00,
  // Z
  0xF8, 0x88, 0x8F, 0x80,
  // [ (Up Arrow) - triangle pointing up
  0x23, 0xAA, 0x42, 0x00,
  // \ (Down Arrow) - triangle pointing down
  0x21, 0x2A, 0xE2, 0x00,
};

const GFXglyph Font5x5CapsGlyphs[] PROGMEM = {
  {  0, 5,5, 6, 0,-5 }, // A
  {  4, 5,5, 6, 0,-5 }, // B
  {  8, 5,5, 6, 0,-5 }, // C
  {  12, 5,5, 6, 0,-5 }, // D
  {  16, 5,5, 6, 0,-5 }, // E
  {  20, 5,5, 6, 0,-5 }, // F
  {  24, 5,5, 6, 0,-5 }, // G
  {  28, 5,5, 6, 0,-5 }, // H
  {  32, 5,5, 6, 0,-5 }, // I
  {  36, 5,5, 6, 0,-5 }, // J
  {  40, 5,5, 6, 0,-5 }, // K
  {  44, 5,5, 6, 0,-5 }, // L
  {  48, 5,5, 6, 0,-5 }, // M
  {  52, 5,5, 6, 0,-5 }, // N
  {  56, 5,5, 6, 0,-5 }, // O
  {  60, 5,5, 6, 0,-5 }, // P
  {  64, 5,5, 6, 0,-5 }, // Q
  {  68, 5,5, 6, 0,-5 }, // R
  {  72, 5,5, 6, 0,-5 }, // S
  {  76, 5,5, 6, 0,-5 }, // T
  {  80, 5,5, 6, 0,-5 }, // U
  {  84, 5,5, 6, 0,-5 }, // V
  {  88, 5,5, 6, 0,-5 }, // W
  {  92, 5,5, 6, 0,-5 }, // X
  {  96, 5,5, 6, 0,-5 }, // Y
  { 100, 5,5, 6, 0,-5 }, // Z
  { 104, 5,5, 6, 0,-5 }, // [ (Up Arrow)
  { 108, 5,5, 6, 0,-5 }, // \ (Down Arrow)
};

const GFXfont Font5x5Caps PROGMEM = {
  (uint8_t  *)Font5x5CapsBitmaps,
  (GFXglyph *)Font5x5CapsGlyphs,
  65, 92,  // ASCII A–Z, [ (up arrow), \ (down arrow)
  5        // line height
};
