#pragma once

#include <driver/i2c_master.h>
#include "esp_err.h"

#include <vector>
#include <array>
#include <string>

class SSD1306Driver
{
public:
    struct SSD1306DriverConfiguration
    {
        i2c_port_num_t I2CPort = I2C_NUM_0;
        gpio_num_t SdaIO = GPIO_NUM_10;
        gpio_num_t SclIO = GPIO_NUM_11;
        uint32_t I2CSclSpeedHz = 400000;
        bool FlipRendering = false;
        bool InvertColors = false;
    };

    SSD1306Driver(const SSD1306DriverConfiguration& configuration);

    esp_err_t FlushCommandBuffer();

    //
    // --------------- All raw commands ---------------
    //
    // For more info on the commands consult the chapter 9 (command table) in the data sheet https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf#

    void AppendControlByteCommand();
    void AppendControlByteData();

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
    esp_err_t AppendSetSegmentRemap(bool remapLeftToRight);
    esp_err_t AppendSetMultiplexRatio(uint8_t ratio);
    esp_err_t AppendSetComOutputScanDirection(bool remapTopToBottom);
    esp_err_t AppendSetDisplayOffset(uint8_t offset);
    esp_err_t AppendSetComPins(bool alternative, bool remap);

    // Timing & Driving Scheme Setting Commands
    esp_err_t AppendSetDisplayClockDivideRatioAndOscillatorFrequency(uint8_t divideRatio, uint8_t oscillatorFrequency);
    esp_err_t AppendSetPreChargePeriod(uint8_t phase1, uint8_t phase2);
    esp_err_t AppendSetVComHDeselectLevel(uint8_t level);
    esp_err_t AppendNOP();

    esp_err_t WritePageToRam(uint8_t page);
    esp_err_t WriteAllPagesToRam();

    esp_err_t WriteToPage(uint8_t page, const void* data, uint8_t size, uint8_t offset);
    esp_err_t WriteToColumn(uint8_t page, uint8_t column, uint8_t data, bool overwrite);

    esp_err_t WriteText(uint8_t x, uint8_t y, std::string text, bool invert);
private:

    class Pages
    {
    public:
        const uint8_t* GetPagePtr(uint8_t page);
        esp_err_t WritePage(uint8_t page, const void* data, uint8_t size, uint8_t offset);
        esp_err_t WriteColumn(uint8_t page, uint8_t column, uint8_t data, bool overwrite);

        bool HasPageChanged(uint8_t page);
        void PageSentToRam(uint8_t page);
    private:
        #define PAGES_COUNT 8

        // 129 because the first byte in each page is 0x40 (control data byte) and the rest 128 are columns
        // This allows to send the data through the I2C without any copying and "stitching" the control byte
        #define PAGE_SIZE 129

        // Store everything in a single continous buffer so it's more memory effiecient
        std::array<uint8_t, PAGE_SIZE * PAGES_COUNT> Buffer;
        uint8_t HavePagesChanged = 0; // Bit field
    };

    Pages m_Pages;

    i2c_master_bus_handle_t m_I2CBusHandle = nullptr;
    i2c_master_dev_handle_t m_I2CHandle = nullptr;

    // You usually want to send multiple commands at once, so all commands are saved
    // to the command buffer and sent with FlushCommandBuffer()
    // Some commands also require multiple bytes so this also makes things easier
    std::vector<uint8_t> m_CommandBuffer;
};