#include "display_renderer.h"
#include "../core/config.h"
#include "../core/state.h"
#include "draw_utils.h"
#include "../assets/fonts/font3x5.h"
#include "../assets/fonts/font3x5caps.h"
#include "../assets/logos/train_logos10x10.h"

// Helper function to calculate text width for alignment
static int calculateTextWidth(const int times[3], int count) {
    int totalWidth = 0;
    for (int i = 0; i < count && i < 3; i++) {
        if (i > 0) totalWidth += 2;  // Comma width
        String numStr = String(times[i]);
        totalWidth += numStr.length() * 4;  // Each digit is 4 pixels wide
    }
    return totalWidth;
}

// Draw train times with right-aligned formatting
static void drawTrainTimes(MatrixPanel_I2S_DMA* display, int x, int y, const int times[3], int count) {
    int totalWidth = calculateTextWidth(times, count);
    int numberX = RIGHT_EDGE_X - totalWidth;
    
    // Ensure numberX is not negative (off-screen)
    if (numberX < 0) numberX = x + 7;  // Fallback to fixed position
    
    display->setFont(&Font3x5Digits);
    display->setTextSize(1);
    display->setTextColor(display->color444(15, 15, 15));
    display->setCursor(numberX, y - 1);  // -1 to align with arrow
    
    for (int i = 0; i < count && i < 3; i++) {
        if (times[i] >= 0) {  // Only draw if valid
            if (i > 0) {
                display->setFont(NULL);  // Default font for comma
                display->print(",");
                display->setFont(&Font3x5Digits);
            }
            display->print(times[i]);
        }
    }
}

// Draw a train section (logo, arrows, and times)
static void drawTrainSection(MatrixPanel_I2S_DMA* display, int startY, 
                             const char* label, const uint16_t* logo,
                             const TrainTimes& trainTimes) {
    // Draw label text
    display->setFont(&Font5x5Caps);
    display->setTextSize(1);
    display->setTextColor(display->color444(15, 15, 15));
    display->setCursor(0, startY + TEXT_Y_OFFSET);
    draw_text(display, label);

    // Draw logo
    display->drawRGBBitmap(0, startY + LOGO_Y_OFFSET, logo, LOGO_SIZE, LOGO_SIZE);

    // Position for arrows and numbers
    int arrowX = LOGO_SIZE + 10;
    int upArrowY = startY + LOGO_Y_OFFSET;
    int downArrowY = startY + LOGO_Y_OFFSET + LOGO_SIZE;

    // Draw up arrow
    display->setFont(&Font5x5Caps);
    display->setTextSize(1);
    display->setTextColor(display->color444(15, 15, 15));
    display->setCursor(arrowX, upArrowY);
    display->print((char)ARROW_UP);

    // Draw UP times
    drawTrainTimes(display, arrowX, upArrowY, trainTimes.upTimes, trainTimes.upCount);

    // Draw down arrow
    display->setFont(&Font5x5Caps);
    display->setTextSize(1);
    display->setTextColor(display->color444(15, 15, 15));
    display->setCursor(arrowX, downArrowY);
    display->print((char)ARROW_DOWN);

    // Draw DOWN times
    drawTrainTimes(display, arrowX, downArrowY, trainTimes.downTimes, trainTimes.downCount);
}

void renderDisplay(MatrixPanel_I2S_DMA* display) {
    display->clearScreen();

    // JAY Section (Top Half)
    drawTrainSection(display, 0, "JAY", a_train_10x10, aTrainTimes);

    // DEK Section (Bottom Half)
    drawTrainSection(display, SECTION_HEIGHT, "DEK", q_train_10x10, qTrainTimes);
}
