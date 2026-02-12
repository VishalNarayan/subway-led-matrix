#include "display_config.h"
#include "../core/config.h"
#include "../assets/fonts/font3x5caps.h"

MatrixPanel_I2S_DMA* initDisplay() {
    HUB75_I2S_CFG mxconfig(DISPLAY_WIDTH, DISPLAY_HEIGHT, 1);
    mxconfig.gpio.b = DISPLAY_GPIO_B;
    mxconfig.gpio.e = DISPLAY_GPIO_E;
    mxconfig.clkphase = false;

    MatrixPanel_I2S_DMA* display = new MatrixPanel_I2S_DMA(mxconfig);
    display->begin();
    display->clearScreen();
    display->setBrightness(DISPLAY_BRIGHTNESS);
    
    // Initialize font settings
    display->setFont(&Font5x5Caps);
    display->setTextSize(1);
    display->setTextColor(display->color444(15, 15, 15));
    
    delay(100);  // Let display stabilize
    
    return display;
}
