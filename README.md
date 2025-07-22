# SSD1306Driver
![photo_2025-07-20_11-48-32](https://github.com/user-attachments/assets/b3b9ce14-04b4-4e04-af10-eb70784b485e)

Simple and extensible SSD1306 OLED 128x64 display driver for ESP32 written in C++ 23 using ESP-IDF. It handles I2C bus and the display through easy to work with SSD1306Driver object.

## Usage
1. Clean your project with 
```
idf.py clean
```
2. Add the dependency
```
idf.py add-dependency --path application --git https://github.com/Zydak/SSD1306Driver.git SSD1306Driver
```
3. Update the dependecy just in case
```
idf.py update-dependencies
```
4. SSD1306Driver relies on `driver` module so you might also need to add driver PRIV_REQUIRES to your main project like so:
```
idf_component_register(
    SRCS "main.cpp"
    PRIV_REQUIRES driver <- This
)
```
5. Build
```
idf.py build
```

## Basic Example

Simple example showcasing some of the features of the driver, that is
 - Drawing text
 - Drawing rectangles
 - Drawing lines
 - Drawing custom data (github logo in this case)
 - Writing to SSD1306 RAM
```
#include "SSD1306Driver.h"
#include <math.h>

#include "freertos/FreeRTOS.h"

#define PI 3.14159265

static const uint8_t githubLogo[4][32] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xC0, 0xE0, 0xF0, 0xF0, 0xF0, 0xF8, 0xF8, 0xF8,
     0xF8, 0xF8, 0xF8, 0xF0, 0xF0, 0xF0, 0xE0, 0xC0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0xF0, 0xFC, 0xFF, 0xFF, 0x1F, 0x00, 0x00, 0x01, 0x01, 0x03, 0x03, 0x01, 0x01,
     0x01, 0x01, 0x03, 0x03, 0x01, 0x01, 0x00, 0x00, 0x1F, 0xFF, 0xFE, 0xFC, 0xE0, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x0F, 0x3F, 0x7F, 0xFF, 0xF0, 0xE0, 0xC0, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0xC0, 0xE0, 0xF0, 0xFF, 0x7F, 0x3F, 0x07, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x07, 0x0F, 0x0F, 0x01, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x01, 0x0F, 0x0F, 0x07, 0x03, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

void DrawStar(SSD1306Driver& driver, int centerX, int centerY, int radius, int rotationDegrees)
{
   float angle = (-PI / 2.0) + (rotationDegrees * PI / 180); // start at the top + rotation
   float angleStep = (2.0 * PI) / 5; // 72 degrees

   int x[5], y[5];
   for (int i = 0; i < 5; ++i)
   {
      x[i] = centerX + radius * std::cos(angle);
      y[i] = centerY + radius * std::sin(angle);
      angle += angleStep;
   }

   for (int i = 0; i < 5; ++i)
   {
      int index = (i * 2) % 5;
      int indexNext = ((i + 1) * 2) % 5;
      ESP_ERROR_CHECK(driver.DrawLine(x[index], y[index], x[indexNext], y[indexNext], true));
   }
}

extern "C" void app_main(void)
{
   SSD1306Driver::SSD1306DriverConfiguration configuration{};
   configuration.I2CPort = I2C_NUM_0;
   configuration.SclIO = GPIO_NUM_11;
   configuration.SdaIO = GPIO_NUM_10;
   configuration.I2CSclSpeedHz = 400000;

   SSD1306Driver driver = SSD1306Driver::New(configuration).value();

   ESP_ERROR_CHECK(driver.SendInitializationSequence(true, false));

   int i = 0;
   while(true)
   {
      ESP_ERROR_CHECK(driver.DrawText(0, 35, "github.com/Zydak", false, true));
      ESP_ERROR_CHECK(driver.DrawLine(0, 45, 127, 45, true));
      ESP_ERROR_CHECK(driver.DrawData(0, 0, 32, 32, (uint8_t*)&githubLogo, i % 20 > 10, true));

      DrawStar(driver, 50,  10, 10, 0 + i * 15);
      DrawStar(driver, 70,  20, 10, 45 + i * 15);
      DrawStar(driver, 90,  10, 10, 180 + i * 15);
      DrawStar(driver, 110, 20, 10, 90 + i * 15);

      ESP_ERROR_CHECK(driver.DrawRectangle(0, 55, 128, 9, true, true));
      ESP_ERROR_CHECK(driver.DrawTextCentered(56, "SSD1306 Driver", false, false));

      ESP_ERROR_CHECK(driver.WriteAllPagesToRam());

      ESP_ERROR_CHECK(driver.ClearDisplay());
      vTaskDelay(100 / portTICK_PERIOD_MS);
      i++;
   }
}
```
#### Example output
![photo_2025-07-20_11-48-26](https://github.com/user-attachments/assets/7c136dd0-cae7-4468-9ba1-fd24505fb44e)

## All Features
> [!TIP]  
> For more detailed descriptions read the comments in the code.

### High Level Methods
High level methods are functions that allow you to interface with SSD1306 display without having to know much about it's architecture.

`ClearDisplay()`: Clears the entire display buffer by setting all pixels to OFF.

`FillPage(uint8_t page, uint8_t value)`: Fills an entire page (8-pixel high row) with the specified byte value, where each bit in the value represents a vertical pixel column within that page.

`WritePageToRam(uint8_t page)`: Transfers a single page from the internal buffer to the display RAM via I2C, but only if the page has been modified since the last transfer (dirty page optimization).

`WriteColumnsToRam(uint8_t page, uint8_t startColumn, uint8_t endColumn)`: Transfers a specific range of columns within a page from the internal buffer to the display RAM, allowing for partial page updates to improve performance.

`WriteAllPagesToRam()`: Transfers all 8 pages from the internal buffer to the display RAM, effectively updating the entire screen with any changes made to the buffer.

`WriteToPage(uint8_t page, const void* data, uint8_t size, uint8_t offset)`: Writes raw byte data directly to a specific page in the buffer at the given offset, useful for writing pre-formatted bitmap data.

`WriteToColumn(uint8_t page, uint8_t column, uint8_t data, bool setOrClear)`: Writes a single byte to a specific column within a page, with the option to either overwrite existing data or combine it using bitwise OR.

`WriteToPixel(uint8_t x, uint8_t y, bool value)`: Sets or clears an individual pixel at the specified coordinates, providing precise control over single pixel manipulation.

`DrawData(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t* data, bool invertColors, bool setOrClear)`: Renders a rectangular block of bitmap data to the display buffer at the specified position, with options to invert colors and control how the data is combined with existing pixels.

`DrawText(uint8_t x, uint8_t y, const std::string& text, bool invertColors, bool setOrClear)`: Renders ASCII text using a built-in 8x8 pixel font, supporting special characters like newlines, tabs, and spaces, with options for color inversion and pixel combination modes.

`DrawTextCentered(uint8_t y, const std::string& text, bool invertColors, bool setOrClear)`: Renders ASCII text horizontally centered on the display at the specified vertical position, automatically calculating the proper x-coordinate for center alignment.

`DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool setOrClear)`: Draws a straight line between two points using Bresenham's line algorithm, ensuring smooth lines at any angle with efficient pixel-by-pixel rendering.

`DrawRectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool setOrClear, bool fill)`: Draws either a filled rectangle or just the border outline, with optimized column-based rendering for filled rectangles to improve performance over line-by-line drawing.

### Low Level Methods
If you need more granular control over SSD1306 you can use Low level methods. They allow you to send direct commands to the SSD1306 display. They're implemented according to the [SSD1306 datasheet](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf) command table in [chapter 9](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf#%5B%7B%22num%22%3A2326%2C%22gen%22%3A0%7D%2C%7B%22name%22%3A%22XYZ%22%7D%2C0%2C842%2Cnull%5D). All of the functions descriptions are there in detail so I won't be going over them here.
