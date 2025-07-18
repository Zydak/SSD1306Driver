#pragma once

#include "esp_err.h"

#include <vector>

class SSD1306Driver
{
public:
    int Foo();

    esp_err_t FlushCommandBuffer();

    //
    // --------------- All raw commands ---------------
    //
    // For more info on the commands consult the chapter 9 command table at https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf#

    // Fundamental Commands
    esp_err_t AppendSetContrastControl(uint8_t contrast);
    esp_err_t AppendEntireDisplayOn(bool on);
    esp_err_t AppendSetNormalInverseDisplay(bool invert);
    esp_err_t AppendSetDisplayOnOff(bool on);

    // Scrolling Commands
    esp_err_t AppendContinuousHorizontalScrollSetup(bool leftHorizontalScroll, uint8_t startPageAddress, uint8_t timeInterval, uint8_t endPageAddress);
    esp_err_t AppendContinuousVerticalAndHorizontalScrollSetup(uint8_t direction, uint8_t startPageAddress, uint8_t timeInterval, uint8_t endPageAddress, uint8_t verticalScrollingOffset);
    esp_err_t AppendDeactivateScroll();
    esp_err_t AppendActivateScroll();
    esp_err_t AppendSetVerticalScrollArea(uint8_t numberOfRowsFixed, uint8_t numberOfRowsScroll);
    
    // Addressing Setting Commands
    esp_err_t AppendSetLowerColumnStartAddress(uint8_t addressLow);
    esp_err_t AppendSetHigherColumnStartAddress(uint8_t addressHigh);
    esp_err_t AppendSetMemoryAddressingMode(uint8_t mode);
    esp_err_t AppendSetColumnAddress(uint8_t startAddress, uint8_t endAddress);
    esp_err_t AppendSetPageAddress(uint8_t startAddress, uint8_t endAddress);
    esp_err_t AppendSetPageStartAddress(uint8_t address);

    // Hardware Configuration Commands
    esp_err_t AppendSetDisplayStartLine(uint8_t line);
    esp_err_t AppendSetSegmentRemap(bool remap);
    esp_err_t AppendSetMultiplexRatio(uint8_t ratio);
    esp_err_t AppendSetComOutputScanDirection(bool remap);
    esp_err_t AppendSetDisplayOffset(uint8_t offset);
    esp_err_t AppendSetComPins(bool alternative, bool remap);

    // Timing & Driving Scheme Setting Commands
    esp_err_t AppendSetDisplayClockDivideRatioAndOscillatorFrequency(uint8_t divideRatio, uint8_t oscillatorFrequency);
    esp_err_t AppendSetPreChargePeriod(uint8_t phase1, uint8_t phase2);
    esp_err_t AppendSetVComHDeselectLevel(uint8_t level);
    esp_err_t AppendNOP();
private:

    // You usually want to send multiple commands at once, so all commands are saved
    // to the command buffer and sent with FlushCommandBuffer()
    // Some commands also require multiple bytes so this also makes things easier
    std::vector<uint8_t> m_CommandBuffer;
};