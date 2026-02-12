#pragma once

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

void draw_face(MatrixPanel_I2S_DMA *dma_display);
void draw_text(MatrixPanel_I2S_DMA *display, const char *text);