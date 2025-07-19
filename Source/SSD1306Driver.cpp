#include "SSD1306Driver.h"

#include "esp_log.h"

#include <freertos/FreeRTOS.h>

#include <cstring>

#define TAG "SSD1306Driver"

/* FONT DEFINED ONLY FOR ASCII PRINTABLE CHARACTERS */
static const uint8_t s_ASCIIPixelData[128][8] = {
    /* ASCII CONTROL CHARACTERS */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 000 -> 0x00 [NULL]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 001 -> 0x01 [SOH]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 002 -> 0x02 [STX]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 003 -> 0x03 [ETX]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 004 -> 0x04 [EOT]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 005 -> 0x05 [ENQ]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 006 -> 0x06 [ACK]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 007 -> 0x07 [BEL]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 008 -> 0x08 [BS]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 009 -> 0x09 [TAB]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 010 -> 0x0A [LF]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 011 -> 0x0B [VT]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 012 -> 0x0C [FF]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 013 -> 0x0D [CR]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 014 -> 0x0E [SO]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 015 -> 0x0F [SI]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 016 -> 0x10 [DLE]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 017 -> 0x11 [DC1]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 018 -> 0x12 [DC2]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 019 -> 0x13 [DC3]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 020 -> 0x14 [DC4]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 021 -> 0x15 [NAK]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 022 -> 0x16 [SYN]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 023 -> 0x17 [ETB]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 024 -> 0x18 [CAN]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 025 -> 0x19 [EM]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 026 -> 0x1A [SUB]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 027 -> 0x1B [ESC]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 028 -> 0x1C [FS]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 029 -> 0x1D [GS]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 030 -> 0x1E [RS]
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 031 -> 0x1F [US]
    /* ASCII PRINTABLE CHARACTERS */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 032 -> 0x20 ( )
    {0x00, 0x00, 0x00, 0x5F, 0x5F, 0x00, 0x00, 0x00}, // 033 -> 0x21 (!)
    {0x00, 0x07, 0x07, 0x00, 0x07, 0x07, 0x00, 0x00}, // 034 -> 0x22 (")
    {0x14, 0x7F, 0x7F, 0x14, 0x7F, 0x7F, 0x14, 0x00}, // 035 -> 0x23 (#)
    {0x00, 0x24, 0x2A, 0x7F, 0x7F, 0x2A, 0x12, 0x00}, // 036 -> 0x24 ($)
    {0x46, 0x66, 0x30, 0x18, 0x0C, 0x66, 0x62, 0x00}, // 037 -> 0x25 (%)
    {0x30, 0x7A, 0x4F, 0x5D, 0x37, 0x7A, 0x48, 0x00}, // 038 -> 0x26 (&)
    {0x00, 0x00, 0x00, 0x07, 0x07, 0x00, 0x00, 0x00}, // 039 -> 0x27 (')
    {0x00, 0x00, 0x1C, 0x3E, 0x63, 0x41, 0x00, 0x00}, // 040 -> 0x28 (()
    {0x00, 0x00, 0x41, 0x63, 0x3E, 0x1C, 0x00, 0x00}, // 041 -> 0x29 ())
    {0x08, 0x2A, 0x3E, 0x1C, 0x1C, 0x3E, 0x2A, 0x08}, // 042 -> 0x2a (*)
    {0x00, 0x08, 0x08, 0x3E, 0x3E, 0x08, 0x08, 0x00}, // 043 -> 0x2b (+)
    {0x00, 0x00, 0x80, 0xE0, 0x60, 0x00, 0x00, 0x00}, // 044 -> 0x2c (,)
    {0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00}, // 045 -> 0x2d (-)
    {0x00, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x00}, // 046 -> 0x2e (.)
    {0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00}, // 047 -> 0x2f (/)
    {0x3E, 0x7F, 0x51, 0x49, 0x45, 0x7F, 0x3E, 0x00}, // 048 -> 0x30 (0)
    {0x00, 0x40, 0x42, 0x7F, 0x7F, 0x40, 0x40, 0x00}, // 049 -> 0x31 (1)
    {0x00, 0x72, 0x7B, 0x49, 0x49, 0x6F, 0x66, 0x00}, // 050 -> 0x32 (2)
    {0x00, 0x22, 0x63, 0x49, 0x49, 0x7F, 0x36, 0x00}, // 051 -> 0x33 (3)
    {0x18, 0x1C, 0x16, 0x53, 0x7F, 0x7F, 0x50, 0x00}, // 052 -> 0x34 (4)
    {0x00, 0x2F, 0x6F, 0x49, 0x49, 0x79, 0x33, 0x00}, // 053 -> 0x35 (5)
    {0x00, 0x3E, 0x7F, 0x49, 0x49, 0x7B, 0x32, 0x00}, // 054 -> 0x36 (6)
    {0x00, 0x03, 0x03, 0x71, 0x79, 0x0F, 0x07, 0x00}, // 055 -> 0x37 (7)
    {0x00, 0x36, 0x7F, 0x49, 0x49, 0x7F, 0x36, 0x00}, // 056 -> 0x38 (8)
    {0x00, 0x26, 0x6F, 0x49, 0x49, 0x7F, 0x3E, 0x00}, // 057 -> 0x39 (9)
    {0x00, 0x00, 0x00, 0x6C, 0x6C, 0x00, 0x00, 0x00}, // 058 -> 0x3a (:)
    {0x00, 0x00, 0x80, 0xEC, 0x6C, 0x00, 0x00, 0x00}, // 059 -> 0x3b (;)
    {0x00, 0x08, 0x1C, 0x36, 0x63, 0x41, 0x00, 0x00}, // 060 -> 0x3c (<)
    {0x00, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x00}, // 061 -> 0x3d (=)
    {0x00, 0x41, 0x63, 0x36, 0x1C, 0x08, 0x00, 0x00}, // 062 -> 0x3e (>)
    {0x00, 0x06, 0x07, 0x51, 0x59, 0x0F, 0x06, 0x00}, // 063 -> 0x3f (?)
    {0x3E, 0x7F, 0x41, 0x5D, 0x5D, 0x5F, 0x1E, 0x00}, // 064 -> 0x40 (@)
    {0x00, 0x7C, 0x7E, 0x13, 0x13, 0x7E, 0x7C, 0x00}, // 065 -> 0x41 (A)
    {0x41, 0x7F, 0x7F, 0x49, 0x49, 0x7F, 0x36, 0x00}, // 066 -> 0x42 (B)
    {0x1C, 0x3E, 0x63, 0x41, 0x41, 0x63, 0x22, 0x00}, // 067 -> 0x43 (C)
    {0x41, 0x7F, 0x7F, 0x41, 0x63, 0x3E, 0x1C, 0x00}, // 068 -> 0x44 (D)
    {0x41, 0x7F, 0x7F, 0x49, 0x5D, 0x41, 0x63, 0x00}, // 069 -> 0x45 (E)
    {0x41, 0x7F, 0x7F, 0x49, 0x1D, 0x01, 0x03, 0x00}, // 070 -> 0x46 (F)
    {0x1C, 0x3E, 0x63, 0x41, 0x51, 0x73, 0x72, 0x00}, // 071 -> 0x47 (G)
    {0x00, 0x7F, 0x7F, 0x08, 0x08, 0x7F, 0x7F, 0x00}, // 072 -> 0x48 (H)
    {0x00, 0x41, 0x41, 0x7F, 0x7F, 0x41, 0x41, 0x00}, // 073 -> 0x49 (I)
    {0x30, 0x70, 0x40, 0x41, 0x7F, 0x3F, 0x01, 0x00}, // 074 -> 0x4a (J)
    {0x41, 0x7F, 0x7F, 0x08, 0x1C, 0x77, 0x63, 0x00}, // 075 -> 0x4b (K)
    {0x41, 0x7F, 0x7F, 0x41, 0x40, 0x60, 0x70, 0x00}, // 076 -> 0x4c (L)
    {0x7F, 0x7F, 0x0E, 0x1C, 0x0E, 0x7F, 0x7F, 0x00}, // 077 -> 0x4d (M)
    {0x7F, 0x7F, 0x06, 0x0C, 0x18, 0x7F, 0x7F, 0x00}, // 078 -> 0x4e (N)
    {0x1C, 0x3E, 0x63, 0x41, 0x63, 0x3E, 0x1C, 0x00}, // 079 -> 0x4f (O)
    {0x41, 0x7F, 0x7F, 0x49, 0x09, 0x0F, 0x06, 0x00}, // 080 -> 0x50 (P)
    {0x3C, 0x7E, 0x43, 0x51, 0x33, 0x6E, 0x5C, 0x00}, // 081 -> 0x51 (Q)
    {0x41, 0x7F, 0x7F, 0x09, 0x19, 0x7F, 0x66, 0x00}, // 082 -> 0x52 (R)
    {0x00, 0x26, 0x6F, 0x49, 0x49, 0x7B, 0x32, 0x00}, // 083 -> 0x53 (S)
    {0x00, 0x03, 0x41, 0x7F, 0x7F, 0x41, 0x03, 0x00}, // 084 -> 0x54 (T)
    {0x00, 0x3F, 0x7F, 0x40, 0x40, 0x7F, 0x3F, 0x00}, // 085 -> 0x55 (U)
    {0x00, 0x1F, 0x3F, 0x60, 0x60, 0x3F, 0x1F, 0x00}, // 086 -> 0x56 (V)
    {0x7F, 0x7F, 0x30, 0x18, 0x30, 0x7F, 0x7F, 0x00}, // 087 -> 0x57 (W)
    {0x61, 0x73, 0x1E, 0x0C, 0x1E, 0x73, 0x61, 0x00}, // 088 -> 0x58 (X)
    {0x00, 0x07, 0x4F, 0x78, 0x78, 0x4F, 0x07, 0x00}, // 089 -> 0x59 (Y)
    {0x47, 0x63, 0x71, 0x59, 0x4D, 0x67, 0x73, 0x00}, // 090 -> 0x5a (Z)
    {0x00, 0x00, 0x7F, 0x7F, 0x41, 0x41, 0x00, 0x00}, // 091 -> 0x5b ([)
    {0x01, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x00}, // 092 -> 0x5c (\)
    {0x00, 0x00, 0x41, 0x41, 0x7F, 0x7F, 0x00, 0x00}, // 093 -> 0x5d (])
    {0x08, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x08, 0x00}, // 094 -> 0x5e (^)
    {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80}, // 095 -> 0x5f (_)
    {0x00, 0x00, 0x01, 0x03, 0x06, 0x04, 0x00, 0x00}, // 096 -> 0x60 (`)
    {0x20, 0x74, 0x54, 0x54, 0x3C, 0x78, 0x40, 0x00}, // 097 -> 0x61 (a)
    {0x41, 0x7F, 0x3F, 0x44, 0x44, 0x7C, 0x38, 0x00}, // 098 -> 0x62 (b)
    {0x00, 0x38, 0x7C, 0x44, 0x44, 0x6C, 0x28, 0x00}, // 099 -> 0x63 (c)
    {0x38, 0x7C, 0x44, 0x45, 0x3F, 0x7F, 0x40, 0x00}, // 100 -> 0x64 (d)
    {0x00, 0x38, 0x7C, 0x54, 0x54, 0x5C, 0x18, 0x00}, // 101 -> 0x65 (e)
    {0x00, 0x48, 0x7E, 0x7F, 0x49, 0x03, 0x02, 0x00}, // 102 -> 0x66 (f)
    {0x00, 0x98, 0xBC, 0xA4, 0xA4, 0xFC, 0x7C, 0x00}, // 103 -> 0x67 (g)
    {0x41, 0x7F, 0x7F, 0x08, 0x04, 0x7C, 0x78, 0x00}, // 104 -> 0x68 (h)
    {0x00, 0x00, 0x44, 0x7D, 0x7D, 0x40, 0x00, 0x00}, // 105 -> 0x69 (i)
    {0x00, 0x60, 0xE0, 0x80, 0x84, 0xFD, 0x7D, 0x00}, // 106 -> 0x6a (j)
    {0x41, 0x7F, 0x7F, 0x10, 0x38, 0x6C, 0x44, 0x00}, // 107 -> 0x6b (k)
    {0x00, 0x00, 0x41, 0x7F, 0x7F, 0x40, 0x00, 0x00}, // 108 -> 0x6c (l)
    {0x78, 0x7C, 0x0C, 0x38, 0x0C, 0x7C, 0x78, 0x00}, // 109 -> 0x6d (m)
    {0x04, 0x7C, 0x78, 0x04, 0x04, 0x7C, 0x78, 0x00}, // 110 -> 0x6e (n)
    {0x00, 0x38, 0x7C, 0x44, 0x44, 0x7C, 0x38, 0x00}, // 111 -> 0x6f (o)
    {0x84, 0xFC, 0xF8, 0xA4, 0x24, 0x3C, 0x18, 0x00}, // 112 -> 0x70 (p)
    {0x18, 0x3C, 0x24, 0xA4, 0xF8, 0xFC, 0x84, 0x00}, // 113 -> 0x71 (q)
    {0x44, 0x7C, 0x78, 0x4C, 0x04, 0x0C, 0x08, 0x00}, // 114 -> 0x72 (r)
    {0x00, 0x48, 0x5C, 0x54, 0x54, 0x74, 0x20, 0x00}, // 115 -> 0x73 (s)
    {0x00, 0x04, 0x3F, 0x7F, 0x44, 0x64, 0x20, 0x00}, // 116 -> 0x74 (t)
    {0x00, 0x3C, 0x7C, 0x40, 0x40, 0x7C, 0x7C, 0x00}, // 117 -> 0x75 (u)
    {0x00, 0x1C, 0x3C, 0x60, 0x60, 0x3C, 0x1C, 0x00}, // 118 -> 0x76 (v)
    {0x3C, 0x7C, 0x60, 0x38, 0x60, 0x7C, 0x3C, 0x00}, // 119 -> 0x77 (w)
    {0x44, 0x6C, 0x38, 0x10, 0x38, 0x6C, 0x44, 0x00}, // 120 -> 0x78 (x)
    {0x00, 0x9C, 0xBC, 0xA0, 0xA0, 0xFC, 0x7C, 0x00}, // 121 -> 0x79 (y)
    {0x00, 0x4C, 0x64, 0x74, 0x5C, 0x4C, 0x64, 0x00}, // 122 -> 0x7a (z)
    {0x00, 0x08, 0x08, 0x3E, 0x77, 0x41, 0x41, 0x00}, // 123 -> 0x7b ({)
    {0x00, 0x00, 0x00, 0x7F, 0x7F, 0x00, 0x00, 0x00}, // 124 -> 0x7c (|)
    {0x00, 0x41, 0x41, 0x77, 0x3E, 0x08, 0x08, 0x00}, // 125 -> 0x7d (})
    {0x10, 0x18, 0x08, 0x18, 0x10, 0x18, 0x08, 0x00}, // 126 -> 0x7e (~)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 127 -> 0x7F (DEL)
};

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

esp_err_t SSD1306Driver::WriteToPage(uint8_t page, const void* data, uint8_t size, uint8_t offset)
{
    return m_Pages.WritePage(page, data, size, offset);
}

esp_err_t SSD1306Driver::WriteToColumn(uint8_t page, uint8_t column, uint8_t data, bool overwrite)
{
    return m_Pages.WriteColumn(page, column, data, overwrite);
}

esp_err_t SSD1306Driver::WriteText(uint8_t x, uint8_t y, std::string text, bool invert)
{
    size_t textLen = text.size();

    uint8_t column = x;
    uint8_t page = y / 8;
    uint8_t yOffset = y % 8;

    for (int i = 0; i < textLen; i++)
    {
        const uint8_t* characterPixelData;
        if (text[i] == '\n')
        {
            column = x;
            page += 1;
            continue;
        }
        if (text[i] == ' ') // Skip rendering spaces since it's a waste of time
        {
            column += 8;
            continue;
        }
        if (text[i] == '\t')
        {
            column += 8 * 4; // 1 tab = 4 spaces
            continue;
        }
        else if ((uint8_t)text[i] < 128)
            characterPixelData = s_ASCIIPixelData[(uint8_t)text[i]];
        else
            characterPixelData = s_ASCIIPixelData[(uint8_t)'?'];

        for (int j = 0; j < 8 && column < 128; j++)
        {
            uint8_t characterColumnPixelData = characterPixelData[j];

            if (invert)
                characterColumnPixelData = ~characterColumnPixelData;

            if (yOffset == 0) // Character fits in a single page
            {
                m_Pages.WriteColumn(page, column, characterColumnPixelData, false);
            }
            else
            {
                uint8_t characterLowerPart = characterColumnPixelData >> yOffset;
                uint8_t characterUpperPart = characterColumnPixelData << (8 - yOffset);

                m_Pages.WriteColumn(page, column, characterUpperPart, false);
                if (page + 1 < 8)
                    m_Pages.WriteColumn(page + 1, column, characterLowerPart, false);
            }

            column += 1;
        }
    }

    return ESP_OK;
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

esp_err_t SSD1306Driver::Pages::WritePage(uint8_t page, const void* data, uint8_t size, uint8_t offset)
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

esp_err_t SSD1306Driver::Pages::WriteColumn(uint8_t page, uint8_t column, uint8_t data, bool overwrite)
{
    if (page >= 8)
    {
        ESP_LOGE(TAG, "Page has to be between 0 and 7! It currently is %d", (int)page);
        return ESP_FAIL;
    }
    if (column >= 128)
    {
        ESP_LOGE(TAG, "Column has to be between 0 and 127! It currently is %d", (int)column);
        return ESP_FAIL;
    }

    uint8_t* pagePtr = Buffer.data() + (page * PAGE_SIZE);
    uint8_t* columnPtr = pagePtr + 1 + column; // + 1 because first byte is control data byte (0x40)

    if (overwrite)
        *columnPtr = data;
    else
        *columnPtr |= data; // Don't everwrite existing data, just add new one

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
