#include "SSD1306Driver.h"

#include "esp_log.h"

#include <freertos/FreeRTOS.h>

#include <cstring>

#define TAG "SSD1306Driver"

SSD1306Driver::SSD1306Driver(const SSD1306DriverConfiguration &configuration)
{
    i2c_master_bus_config_t busConfig = {};
    busConfig.clk_source = I2C_CLK_SRC_DEFAULT;
    busConfig.glitch_ignore_cnt = 7;
    busConfig.i2c_port = configuration.I2CPort;
    busConfig.scl_io_num = configuration.SclIO;
    busConfig.sda_io_num = configuration.SdaIO;
    busConfig.flags.enable_internal_pullup = true;

    ESP_ERROR_CHECK(i2c_new_master_bus(&busConfig, &m_I2CBusHandle));
    ESP_ERROR_CHECK(i2c_master_probe(m_I2CBusHandle, 0x3C, 5000 / portTICK_PERIOD_MS));
    i2c_device_config_t I2CDeviceConfig = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x3C,
        .scl_speed_hz = configuration.I2CSclSpeedHz
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(m_I2CBusHandle, &I2CDeviceConfig, &m_I2CHandle));

    m_CommandBuffer.reserve(256);

    // Reset SSD1306 RAM
    std::array<uint8_t, 128> nullData;
    nullData.fill(0);
    for (int i = 0; i < PAGES_COUNT; i++)
    {
        m_Pages.WritePage(i, nullData.data(), 128, 0);
    }
    WriteAllPagesToRam();

    // Reset Everything according to data sheet command table https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
    AppendControlByteCommand();
    AppendSetDisplayOnOff(false);
    AppendSetContrastControl(0x7F);
    AppendEntireDisplayOn(false);
    AppendSetNormalInverseDisplay(configuration.InvertColors);
    AppendDeactivateScroll();
    AppendSetLowerColumnStartAddress(0);
    AppendSetHigherColumnStartAddress(0);
    AppendSetMemoryAddressingMode(0b10);
    AppendSetPageStartAddress(0);
    AppendSetDisplayStartLine(0);
    AppendSetSegmentRemap(configuration.FlipRendering);
    AppendSetMultiplexRatio(0b00111111);
    AppendSetComOutputScanDirection(configuration.FlipRendering);
    AppendSetDisplayOffset(0);
    AppendSetComPins(true, false);
    AppendSetDisplayClockDivideRatioAndOscillatorFrequency(0b0000, 0b1000);
    AppendSetPreChargePeriod(0x2, 0x2);
    AppendSetVComHDeselectLevel(0b010);
    AppendSetDisplayOnOff(true);

    FlushCommandBuffer();
}

esp_err_t SSD1306Driver::FlushCommandBuffer()
{
    esp_err_t ret = i2c_master_transmit(m_I2CHandle, m_CommandBuffer.data(), m_CommandBuffer.size(), 1000 / portTICK_PERIOD_MS);
    m_CommandBuffer.clear();

    return ret;
}

void SSD1306Driver::AppendControlByteCommand()
{
    m_CommandBuffer.emplace_back(0x00);
}

void SSD1306Driver::AppendControlByteData()
{
    m_CommandBuffer.emplace_back(0x40);
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

esp_err_t SSD1306Driver::AppendSetSegmentRemap(bool remapLeftToRight)
{
    m_CommandBuffer.emplace_back(0b10100000 | remapLeftToRight);
    
    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendSetMultiplexRatio(uint8_t ratio)
{
    if (ratio >= 64 || ratio <= 14)
    {
        ESP_LOGE(TAG, "Ratio must be between 15 and 63! It currently is %d", (int)ratio);
        return ESP_FAIL;
    }

    m_CommandBuffer.emplace_back(0b10101000);
    m_CommandBuffer.emplace_back(ratio);
    
    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendSetComOutputScanDirection(bool remapTopToBottom)
{
    m_CommandBuffer.emplace_back(0b11000000 | (remapTopToBottom << 3));
    
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
    if (phase1 >= 16 || phase1 == 0)
    {
        ESP_LOGE(TAG, "Phase1 must be between 0 and 15 and can't be 0! It currently is %d", (int)phase1);
        return ESP_FAIL;
    }
    if (phase2 >= 16 || phase2 == 0)
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
    if (level != 0b000 && level != 0b010 && level != 0b011)
    {
        ESP_LOGE(TAG, "level has to be either 0b000, 0b010 or 0b011! It currently is %d", (int)level);
        return ESP_FAIL;
    }

    m_CommandBuffer.emplace_back(0b11011011);
    m_CommandBuffer.emplace_back(level << 6);

    return ESP_OK;
}

esp_err_t SSD1306Driver::AppendNOP()
{
    m_CommandBuffer.emplace_back(0b11100011);

    return ESP_OK;
}

esp_err_t SSD1306Driver::WritePageToRam(uint8_t page)
{
    // Most of the time there's no need for updating the entire screen, so the HasChanged flag notes any changes to pages
    // and if there are none skip the writing
    if (!m_Pages.HasPageChanged(page))
        return ESP_OK;
    
    if (m_CommandBuffer.size() > 0)
    {
        ESP_LOGE(TAG, "You have to flush the command buffer before writing to RAM!");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Writing page %d to RAM", page);

    AppendControlByteCommand();
    esp_err_t res = AppendSetPageStartAddress(page); // Page bounds (0-7) are checked here so there's no need for another if statement
    if (res != ESP_OK)
        return res;

    res = FlushCommandBuffer();
    if (res != ESP_OK)
        return res;
    
    esp_err_t ret = i2c_master_transmit(m_I2CHandle, m_Pages.GetPagePtr(page), PAGE_SIZE, 1000 / portTICK_PERIOD_MS);
    m_Pages.PageSentToRam(page);

    return ret;
}

esp_err_t SSD1306Driver::WriteAllPagesToRam()
{
    for (int i = 0; i < PAGES_COUNT; i++)
    {
        esp_err_t res = WritePageToRam(i);
        if (res != ESP_OK)
            return res;
    }

    return ESP_OK;
}

esp_err_t SSD1306Driver::WriteToPage(uint8_t page, void *data, uint8_t size, uint8_t offset)
{
    return m_Pages.WritePage(page, data, size, offset);
}

const uint8_t *SSD1306Driver::Pages::GetPagePtr(uint8_t page)
{
    if (page >= 8)
    {
        ESP_LOGE(TAG, "Page has to be between 0 and 7! It currently is %d", (int)page);
        return nullptr;
    }

    return Buffer.data() + (page * PAGE_SIZE);
}

esp_err_t SSD1306Driver::Pages::WritePage(uint8_t page, void *data, uint8_t size, uint8_t offset)
{
    if (page >= 8)
    {
        ESP_LOGE(TAG, "Page has to be between 0 and 7! It currently is %d", (int)page);
        return ESP_FAIL;
    }
    if (size + offset > 128)
    {
        ESP_LOGE(TAG, "Size + Offset has to be between 0 and 128! It currently is %d", int(size + offset));
        return ESP_FAIL;
    }

    // + 1 because first byte in the Columns is the control data byte (0x40) not the actual data
    uint8_t* pagePtr = Buffer.data() + (page * PAGE_SIZE);

    pagePtr[0] = 0x40; // Set the control byte
    memcpy(pagePtr + 1 + offset, data, size);

    // Set page bit to 1
    HavePagesChanged = HavePagesChanged | (1 << page);

    return ESP_OK;
}

bool SSD1306Driver::Pages::HasPageChanged(uint8_t page)
{
    return (HavePagesChanged & (1 << page)) > 0;
}

void SSD1306Driver::Pages::PageSentToRam(uint8_t page)
{
    // Set page bit to 0
    HavePagesChanged = HavePagesChanged & ~(1 << page);
}
