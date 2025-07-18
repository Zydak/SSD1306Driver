#include "SSD1306Driver.h"

#include "esp_log.h"

#define TAG "SSD1306Driver"

int SSD1306Driver::Foo()
{
    return 5;
}

esp_err_t SSD1306Driver::AppendSetContrastControl(uint8_t contrast)
{
    m_CommandBuffer.emplace_back(0x81);
    m_CommandBuffer.emplace_back(contrast);
    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendEntireDisplayOn(bool on)
{
    m_CommandBuffer.emplace_back(0b10100100 | on);
    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendSetNormalInverseDisplay(bool invert)
{
    m_CommandBuffer.emplace_back(0b10100110 | invert);
    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendSetDisplayOnOff(bool on)
{
    m_CommandBuffer.emplace_back(0b10101110 | on);
    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendContinuousHorizontalScrollSetup(bool leftHorizontalScroll, uint8_t startPageAddress, uint8_t timeInterval, uint8_t endPageAddress)
{
    if (startPageAddress >= 8)
    {
        ESP_LOGE(TAG, "Start Page Address must be between 0 and 7! It currently is %d", (int)startPageAddress);
        return ESP_FAIL;
    }
    if (endPageAddress >= 8)
    {
        ESP_LOGE(TAG, "End Page Address must be between 0 and 7! It currently is %d", (int)endPageAddress);
        return ESP_FAIL;
    }
    if (timeInterval >= 8)
    {
        ESP_LOGE(TAG, "Time Interval must be between 0 and 7! It currently is %d", (int)timeInterval);
        return ESP_FAIL;
    }

    m_CommandBuffer.emplace_back(0b00100110 | leftHorizontalScroll);
    m_CommandBuffer.emplace_back(0x00); // Dummy byte
    m_CommandBuffer.emplace_back(startPageAddress);
    m_CommandBuffer.emplace_back(timeInterval);
    m_CommandBuffer.emplace_back(endPageAddress);
    m_CommandBuffer.emplace_back(0x00); // Dummy byte
    m_CommandBuffer.emplace_back(0xFF); // Dummy byte

    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendContinuousVerticalAndHorizontalScrollSetup(uint8_t direction, uint8_t startPageAddress, uint8_t timeInterval, uint8_t endPageAddress, uint8_t verticalScrollingOffset)
{
    if (direction >= 4)
    {
        ESP_LOGE(TAG, "Direction must be between 0 and 3! It currently is %d", (int)direction);
        return ESP_FAIL;
    }
    if (startPageAddress >= 8)
    {
        ESP_LOGE(TAG, "Start Page Address must be between 0 and 7! It currently is %d", (int)startPageAddress);
        return ESP_FAIL;
    }
    if (endPageAddress >= 8)
    {
        ESP_LOGE(TAG, "End Page Address must be between 0 and 7! It currently is %d", (int)endPageAddress);
        return ESP_FAIL;
    }
    if (timeInterval >= 8)
    {
        ESP_LOGE(TAG, "Time Interval must be between 0 and 7! It currently is %d", (int)timeInterval);
        return ESP_FAIL;
    }
    if (verticalScrollingOffset >= 64)
    {
        ESP_LOGE(TAG, "Vertical Scrolling Offset must be between 0 and 63! It currently is %d", (int)verticalScrollingOffset);
        return ESP_FAIL;
    }

    m_CommandBuffer.emplace_back(0b00101000 | direction);
    m_CommandBuffer.emplace_back(0x00); // Dummy byte
    m_CommandBuffer.emplace_back(startPageAddress);
    m_CommandBuffer.emplace_back(timeInterval);
    m_CommandBuffer.emplace_back(endPageAddress);
    m_CommandBuffer.emplace_back(verticalScrollingOffset);

    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendDeactivateScroll()
{
    m_CommandBuffer.emplace_back(0b00101110);

    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendActivateScroll()
{
    m_CommandBuffer.emplace_back(0b00101111);

    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendSetVerticalScrollArea(uint8_t numberOfRowsFixed, uint8_t numberOfRowsScroll)
{
    if (numberOfRowsFixed >= 64)
    {
        ESP_LOGE(TAG, "Number Of Rows Fixed must be between 0 and 63! It currently is %d", (int)numberOfRowsFixed);
        return ESP_FAIL;
    }
    if (numberOfRowsScroll >= 128)
    {
        ESP_LOGE(TAG, "Vertical Scrolling Offset must be between 0 and 127! It currently is %d", (int)numberOfRowsScroll);
        return ESP_FAIL;
    }

    m_CommandBuffer.emplace_back(0b10100011);
    m_CommandBuffer.emplace_back(numberOfRowsFixed);
    m_CommandBuffer.emplace_back(numberOfRowsScroll);

    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendSetLowerColumnStartAddress(uint8_t addressLow)
{
    if (addressLow >= 16)
    {
        ESP_LOGE(TAG, "Address Low must be between 0 and 15! It currently is %d", (int)addressLow);
        return ESP_FAIL;
    }

    m_CommandBuffer.emplace_back(0b00000000 | addressLow);

    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendSetHigherColumnStartAddress(uint8_t addressHigh)
{
    if (addressHigh >= 16)
    {
        ESP_LOGE(TAG, "Address High must be between 0 and 15! It currently is %d", (int)addressHigh);
        return ESP_FAIL;
    }

    m_CommandBuffer.emplace_back(0b00000000 | addressHigh);

    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendSetMemoryAddressingMode(uint8_t mode)
{
    if (mode >= 4)
    {
        ESP_LOGE(TAG, "Addressing Mode must be between 0 and 3! It currently is %d", (int)mode);
        return ESP_FAIL;
    }

    m_CommandBuffer.emplace_back(0b00100000);
    m_CommandBuffer.emplace_back(mode);

    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendSetColumnAddress(uint8_t startAddress, uint8_t endAddress)
{
    if (startAddress >= 128)
    {
        ESP_LOGE(TAG, "Column Start Address must be between 0 and 127! It currently is %d", (int)startAddress);
        return ESP_FAIL;
    }
    if (endAddress >= 128)
    {
        ESP_LOGE(TAG, "Column End Address must be between 0 and 127! It currently is %d", (int)endAddress);
        return ESP_FAIL;
    }

    m_CommandBuffer.emplace_back(0b00100001);
    m_CommandBuffer.emplace_back(startAddress);
    m_CommandBuffer.emplace_back(endAddress);

    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendSetPageAddress(uint8_t startAddress, uint8_t endAddress)
{
    if (startAddress >= 8)
    {
        ESP_LOGE(TAG, "Page Start Address must be between 0 and 7! It currently is %d", (int)startAddress);
        return ESP_FAIL;
    }
    if (endAddress >= 8)
    {
        ESP_LOGE(TAG, "Page End Address must be between 0 and 7! It currently is %d", (int)endAddress);
        return ESP_FAIL;
    }

    m_CommandBuffer.emplace_back(0b00100010);
    m_CommandBuffer.emplace_back(startAddress);
    m_CommandBuffer.emplace_back(endAddress);
    
    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendSetPageStartAddress(uint8_t address)
{
    if (address >= 8)
    {
        ESP_LOGE(TAG, "Page Start Address must be between 0 and 7! It currently is %d", (int)address);
        return ESP_FAIL;
    }

    m_CommandBuffer.emplace_back(0b10110000 | address);
    
    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendSetDisplayStartLine(uint8_t line)
{
    if (line >= 64)
    {
        ESP_LOGE(TAG, "Line must be between 0 and 63! It currently is %d", (int)line);
        return ESP_FAIL;
    }

    m_CommandBuffer.emplace_back(0b01000000 | line);
    
    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendSetSegmentRemap(bool remap)
{
    m_CommandBuffer.emplace_back(0b10100000 | remap);
    
    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendSetMultiplexRatio(uint8_t ratio)
{
    if (ratio >= 64)
    {
        ESP_LOGE(TAG, "Ratio must be between 0 and 63! It currently is %d", (int)ratio);
        return ESP_FAIL;
    }

    m_CommandBuffer.emplace_back(0b10101000);
    m_CommandBuffer.emplace_back(ratio);
    
    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendSetComOutputScanDirection(bool remap)
{
    m_CommandBuffer.emplace_back(0b11000000 | (remap << 3));
    
    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendSetDisplayOffset(uint8_t offset)
{
    if (offset >= 64)
    {
        ESP_LOGE(TAG, "Offset must be between 0 and 63! It currently is %d", (int)offset);
        return ESP_FAIL;
    }

    m_CommandBuffer.emplace_back(0b11010011);
    m_CommandBuffer.emplace_back(offset);
    
    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendSetComPins(bool alternative, bool remap)
{
    m_CommandBuffer.emplace_back(0b11011010);
    m_CommandBuffer.emplace_back(0b00000010 | (alternative << 4) | (remap << 5));
    
    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendSetDisplayClockDivideRatioAndOscillatorFrequency(uint8_t divideRatio, uint8_t oscillatorFrequency)
{
    if (divideRatio >= 16)
    {
        ESP_LOGE(TAG, "Divide Ratio must be between 0 and 15! It currently is %d", (int)divideRatio);
        return ESP_FAIL;
    }
    if (oscillatorFrequency >= 16)
    {
        ESP_LOGE(TAG, "Oscillator Frequency must be between 0 and 15! It currently is %d", (int)oscillatorFrequency);
        return ESP_FAIL;
    }

    m_CommandBuffer.emplace_back(0b11010101);
    m_CommandBuffer.emplace_back((divideRatio) | (oscillatorFrequency << 7));
    
    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendSetPreChargePeriod(uint8_t phase1, uint8_t phase2)
{
    if (phase1 >= 16 && phase1 != 0)
    {
        ESP_LOGE(TAG, "Phase1 must be between 0 and 15 and can't be 0! It currently is %d", (int)phase1);
        return ESP_FAIL;
    }
    if (phase2 >= 16 && phase2 != 0)
    {
        ESP_LOGE(TAG, "Phase2 must be between 0 and 15 and can't be 0! It currently is %d", (int)phase2);
        return ESP_FAIL;
    }

    m_CommandBuffer.emplace_back(0b11011001);
    m_CommandBuffer.emplace_back((phase1) | (phase2 << 7));

    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendSetVComHDeselectLevel(uint8_t level)
{
    if (level == 0x00 || level == 0x10 || level == 0x20)
    {
        ESP_LOGE(TAG, "level has to be either 0x00, 0x10 or 0x20! It currently is %x", (int)level);
        return ESP_FAIL;
    }

    m_CommandBuffer.emplace_back(0b11011011);
    m_CommandBuffer.emplace_back(level << 6);

    return ESP_OK;
}
