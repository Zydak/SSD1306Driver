#pragma once

#include "esp_err.h"

#include <stdint.h>

typedef struct i2c_master_bus_t *i2c_master_bus_handle_t;
typedef struct i2c_master_dev_t *i2c_master_dev_handle_t;
typedef int i2c_port_num_t;

class SSD1306Driver
{
public:
    struct Configuration
    {
        i2c_port_num_t I2CPort = 0;
        int SdaIO = 10;
        int SclIO = 11;
        uint32_t I2CSclSpeedHz = 400000;
    };

    SSD1306Driver() = default;

    [[nodiscard]] static esp_err_t New(const Configuration& configuration, SSD1306Driver& outDriver);

    SSD1306Driver(const SSD1306Driver& other) = delete;
    SSD1306Driver& operator=(const SSD1306Driver& other) = delete;
    SSD1306Driver(SSD1306Driver&& other) noexcept;
    SSD1306Driver& operator=(SSD1306Driver&& other) noexcept;

    ~SSD1306Driver();

    [[nodiscard]] esp_err_t SendInitializationSequence(bool flipRendering, bool invertColors);

    // High Level Commands

    [[nodiscard]] esp_err_t ClearDisplay();
    [[nodiscard]] esp_err_t FillPage(uint8_t page, uint8_t value);

    [[nodiscard]] esp_err_t WritePageToRam(uint8_t page);
    [[nodiscard]] esp_err_t WriteColumnsToRam(uint8_t page, uint8_t startColumn, uint8_t endColumn);
    [[nodiscard]] esp_err_t WriteAllPagesToRam();

    [[nodiscard]] esp_err_t WriteToPage(uint8_t page, const void* data, uint8_t size, uint8_t offset);
    [[nodiscard]] esp_err_t WriteToColumn(uint8_t page, uint8_t column, uint8_t data, bool setOrClear);
    [[nodiscard]] esp_err_t WriteToPixel(uint8_t x, uint8_t y, bool value);

    [[nodiscard]] esp_err_t DrawData(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t* data, bool invertColors, bool setOrClear);

    [[nodiscard]] esp_err_t DrawText(uint8_t x, uint8_t y, const char* text, bool invertColors, bool setOrClear);
    [[nodiscard]] esp_err_t DrawTextCentered(uint8_t y, const char* text, bool invertColors, bool setOrClear);

    [[nodiscard]] esp_err_t DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool setOrClear);
    [[nodiscard]] esp_err_t DrawRectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool setOrClear, bool fill);

    //
    // --------------- All raw commands ---------------
    //
    // For more info on the commands consult the chapter 9 (command table) in the data sheet https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf#

    [[nodiscard]] esp_err_t FlushCommandBuffer();
    void AppendControlByteCommand();
    void AppendControlByteData();

    // Charge Pump Commands
    [[nodiscard]] esp_err_t AppendChargePumpSetting(bool enable);

    // Fundamental Commands
    [[nodiscard]] esp_err_t AppendSetContrastControl(uint8_t contrast);
    [[nodiscard]] esp_err_t AppendEntireDisplayOn(bool on);
    [[nodiscard]] esp_err_t AppendSetNormalInverseDisplay(bool invert);
    [[nodiscard]] esp_err_t AppendSetDisplayOnOff(bool on);

    // Scrolling Commands
    [[nodiscard]] esp_err_t AppendContinuousHorizontalScrollSetup(bool leftHorizontalScroll, uint8_t startPageAddress, uint8_t timeInterval, uint8_t endPageAddress);
    [[nodiscard]] esp_err_t AppendContinuousVerticalAndHorizontalScrollSetup(uint8_t direction, uint8_t startPageAddress, uint8_t timeInterval, uint8_t endPageAddress, uint8_t verticalScrollingOffset);
    [[nodiscard]] esp_err_t AppendDeactivateScroll();
    [[nodiscard]] esp_err_t AppendActivateScroll();
    [[nodiscard]] esp_err_t AppendSetVerticalScrollArea(uint8_t numberOfRowsFixed, uint8_t numberOfRowsScroll);
    
    // Addressing Setting Commands
    [[nodiscard]] esp_err_t AppendSetLowerColumnStartAddress(uint8_t addressLow);
    [[nodiscard]] esp_err_t AppendSetHigherColumnStartAddress(uint8_t addressHigh);
    [[nodiscard]] esp_err_t AppendSetMemoryAddressingMode(uint8_t mode);
    [[nodiscard]] esp_err_t AppendSetColumnAddress(uint8_t startAddress, uint8_t endAddress);
    [[nodiscard]] esp_err_t AppendSetPageAddress(uint8_t startAddress, uint8_t endAddress);
    [[nodiscard]] esp_err_t AppendSetPageStartAddress(uint8_t address);

    // Hardware Configuration Commands
    [[nodiscard]] esp_err_t AppendSetDisplayStartLine(uint8_t line);
    [[nodiscard]] esp_err_t AppendSetSegmentRemap(bool remapLeftToRight);
    [[nodiscard]] esp_err_t AppendSetMultiplexRatio(uint8_t ratio);
    [[nodiscard]] esp_err_t AppendSetComOutputScanDirection(bool remapTopToBottom);
    [[nodiscard]] esp_err_t AppendSetDisplayOffset(uint8_t offset);
    [[nodiscard]] esp_err_t AppendSetComPins(bool alternative, bool remap);

    // Timing & Driving Scheme Setting Commands
    [[nodiscard]] esp_err_t AppendSetDisplayClockDivideRatioAndOscillatorFrequency(uint8_t divideRatio, uint8_t oscillatorFrequency);
    [[nodiscard]] esp_err_t AppendSetPreChargePeriod(uint8_t phase1, uint8_t phase2);
    [[nodiscard]] esp_err_t AppendSetVComHDeselectLevel(uint8_t level);
    [[nodiscard]] esp_err_t AppendNOP();
private:

    static constexpr uint8_t DISPLAY_WIDTH = 128;
    static constexpr uint8_t DISPLAY_HEIGHT = 64;
    static constexpr uint8_t PAGES_COUNT = 8;
    // 129 because the first byte in each page is 0x40 (control data byte) and the rest 128 are columns
    // This allows to send the data through the I2C without any copying and "stitching" the control byte when sending full pages
    static constexpr uint8_t PAGE_SIZE = 129;
    static constexpr uint8_t COMMAND_BUFFER_SIZE = 64;

    class CommandBuffer
    {
    public:
        bool PushBack(uint8_t value)
        {
            if (m_Size >= COMMAND_BUFFER_SIZE)
            {
                m_Overflowed = true;
                return false;
            }

            m_Data[m_Size] = value;
            m_Size += 1;
            return true;
        }

        [[nodiscard]] bool IsEmpty() const { return m_Size == 0; }
        [[nodiscard]] uint8_t Size() const { return m_Size; }
        [[nodiscard]] const uint8_t* Data() const { return m_Data; }
        [[nodiscard]] bool HasOverflowed() const { return m_Overflowed; }
        void Clear()
        {
            m_Size = 0;
            m_Overflowed = false;
        }

    private:
        uint8_t m_Data[COMMAND_BUFFER_SIZE] = {};
        uint8_t m_Size = 0;
        bool m_Overflowed = false;
    };

    class Pages
    {
    public:
        Pages();

        [[nodiscard]] const uint8_t* GetPagePtr(uint8_t page);
        [[nodiscard]] const uint8_t* GetColumnPtr(uint8_t page, uint8_t column);
        [[nodiscard]] esp_err_t WritePage(uint8_t page, const void* data, uint8_t size, uint8_t offset);
        [[nodiscard]] esp_err_t WriteColumn(uint8_t page, uint8_t column, uint8_t data, bool setOrClear);
        [[nodiscard]] esp_err_t WritePixel(uint8_t x, uint8_t y, bool value);
        [[nodiscard]] esp_err_t Clear();
        [[nodiscard]] esp_err_t FillPage(uint8_t page, uint8_t value);

        [[nodiscard]] bool IsPageDirty(uint8_t page);
        void UnmarkPageAsDirty(uint8_t page);
        void MarkPageAsDirty(uint8_t page);
        [[nodiscard]] uint8_t GetDirtyPagesMask() const { return m_DirtyPages; }
    private:

        uint8_t m_Buffer[PAGE_SIZE * PAGES_COUNT];
        uint8_t m_DirtyPages = 0; // Bit field
    };

    Pages m_Pages;

    i2c_master_bus_handle_t m_I2CBusHandle = nullptr;
    i2c_master_dev_handle_t m_I2CHandle = nullptr;

    // You usually want to send multiple commands at once, so all commands are saved
    // to the command buffer and sent with FlushCommandBuffer()
    // Some commands also require multiple bytes so this also makes things easier
    CommandBuffer m_CommandBuffer;
};