#include "draw_utils.h"
#include "../assets/fonts/font3x5caps.h"

void draw_face(MatrixPanel_I2S_DMA *dma_display) {
    uint16_t BLACK  = dma_display->color565(0, 0, 0);
    uint16_t YELLOW = dma_display->color565(255, 255, 0);
    
    // Fill background black
    dma_display->fillScreen(BLACK);

    // Draw the smiley face
    int cx = 32;  // center of face
    int cy = 32;
    int r  = 22;  // radius of the head

    // Draw circular yellow face
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if (x*x + y*y <= r*r) {
                dma_display->drawPixel(cx + x, cy + y, YELLOW);
            }
        }
    }

    // Eyes
    dma_display->fillCircle(cx - 10, cy - 8, 3, BLACK);
    dma_display->fillCircle(cx + 10, cy - 8, 3, BLACK);

    // Smile (corrected to curve downward and sit lower)
    int smileRadius = 14;
    for (int angle = 20; angle <= 160; angle++) {
        float rad = angle * 3.14159 / 180.0;
        int x = cx + int(cos(rad) * smileRadius);
        int y = cy + int(sin(rad) * smileRadius);
        
        dma_display->drawPixel(x, y, BLACK);
        dma_display->drawPixel(x, y + 1, BLACK); // slightly thicker
    }
}

void draw_text(MatrixPanel_I2S_DMA *display, const char *text) {
    // Save current cursor location 
    int16_t cx = display->getCursorX();
    int16_t cy = display->getCursorY();

    display->setFont(&Font5x5Caps);
    display->setTextSize(1);
    display->setTextColor(display->color444(15, 15, 15));
    display->print(text);

    // Restore cursor
    display->setCursor(cx, cy);
}