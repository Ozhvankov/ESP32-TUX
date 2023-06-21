/*
MIT License

Copyright (c) 2022 Sukesh Ashok Kumar

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

//#define LGFX_WT32_SC01 // Wireless Tag / Seeed WT32-SC01

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

//#define 3248S035
// #define SD_SUPPORTED // external SPI (SD or others)

#define SPI_HOST_ID HSPI_HOST

// Since SPI bus is shared, only CS PIN required
#define SD_CS   GPIO_NUM_33

// Portrait
#define TFT_WIDTH   480
#define TFT_HEIGHT  320

//#define SD_SUPPORTED

#define TFT_MOSI    13 
#define TFT_MISO    12  // Set this PIN for using shared SPI option
#define TFT_SCLK    14
#define TFT_DC      2 
#define TFT_CS      15
#define TFT_RST     -1  


class LGFX : public lgfx::LGFX_Device
{
    // provide an instance that matches the type of panel you want to connect to.
    lgfx::Panel_ST7796 _panel_instance;

    // provide an instance that matches the type of bus to which the panel is connected.
    lgfx::Bus_SPI _bus_instance; // Instances of spi buses

    //Prepare an instance that matches the type of touchscreen. 
    lgfx::Touch_GT911  _touch_instance;

    lgfx::Light_PWM     _light_instance;

public:

  LGFX(void)
  {
    {
      // set up bus control.
      auto cfg = _bus_instance.config(); // gets the structure for bus settings.

      // SPI bus settings
      cfg.spi_host = HSPI_HOST;; // Select SPI to use ESP32-S2,C3: SPI2_HOST or SPI3_HOST / ESP32: VSPI_HOST or HSPI_HOST

      //* Due to the ESP-IDF upgrade, the description of VSPI_HOST , HSPI_HOST will be deprecated, so if you get an error, use SPI2_HOST , SPI3_HOST instead.
      cfg.spi_mode = 0;          // Set SPI communication mode (0-3) 
      cfg.freq_write = 65000000; // SPI clock on transmission (up to 80MHz, rounded to 80MHz divided by integer)
      cfg.freq_read = 20000000;  // SPI clock on reception
      cfg.spi_3wire = true;      // Set true when receiving on the MOSI pin
      cfg.use_lock = true;       // set true if transaction lock is used
 
      //  * With the ESP-IDF version upgrade, SPI_DMA_CH_AUTO (automatic setting) of DMA channels is recommended. 
      cfg.dma_channel = SPI_DMA_CH_AUTO; // Set DMA channel to be used (0=DMA not used / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=Auto setting)

      cfg.pin_sclk = 14; // Set SCLK pin number for SPI
      cfg.pin_mosi = 13; // Set SPI MOSI pin number

      // When using the SPI bus, which is common to the SD card, be sure to set MISO without omitting it.
      cfg.pin_miso = 12; // Set THE MSO pin number of spi (-1 = disable)
      cfg.pin_dc = 2;    // Set THE D/C pin number of SPI (-1 = disable)

      _bus_instance.config(cfg);              // reflects the setting value on the bus.
      _panel_instance.setBus(&_bus_instance); // Set the bus to the panel.
    }

    {
      // set the display panel control.
      auto cfg = _panel_instance.config(); // gets the structure for display panel settings.

      cfg.pin_cs = 15;    // Pin number to which CS is connected (-1 = disable)
      cfg.pin_rst = -1;  // Pin number to which RST is connected (-1 = disable)
      cfg.pin_busy = -1;      // Pin number to which BUSY is connected (-1 = disable)

      // the following setting values are set to a general initial value for each panel,
      cfg.panel_width = 320;    // actual visible width
      cfg.panel_height = 480;   // actually visible height
      cfg.offset_x = 0;         // Panel X-direction offset amount
      cfg.offset_y = 0;         // Panel Y offset amount
      cfg.offset_rotation = 3;  // offset of rotational values from 0 to 7 (4 to 7 upside down)
      cfg.dummy_read_pixel = 8; // number of bits in dummy leads before pixel read
      cfg.dummy_read_bits = 1;  // number of bits in dummy leads before reading non-pixel data
      cfg.readable = true;      // set to true if data can be read
      cfg.invert = false;       // set to true if the light and dark of the panel is reversed
      cfg.rgb_order = false;    // set to true if the red and blue of the panel are swapped
      cfg.dlen_16bit = false;   // Set to true for panels that transmit data lengths in 16-bit increments in 16-bit parallel or SPI
      cfg.bus_shared = false;    // Set to true when sharing the bus with sd card (bus control is performed with drawJpgFile, etc.)

      // The following should only be set if the display is misalized by a driver with a variable number of pixels, such as st7735 or ILI9163.
       cfg.memory_width = 320; //Maximum width supported by driver ICs
       cfg.memory_height = 480; //Maximum height supported by driver ICs

      _panel_instance.config(cfg);
    }

    {
      auto cfg = _light_instance.config();    

      cfg.pin_bl = 27;//45;              
      cfg.invert = false;           
      cfg.freq   = 44100;           
      cfg.pwm_channel = 7;          

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);  
    }

    { 
      auto cfg = _touch_instance.config();

       // auto touch = new lgfx::Touch_GT911();
       // auto cfg = _touch_instance.config();
        cfg.pin_int  = -1;   // INT pin number
        cfg.pin_sda  = 33;   // I2C SDA pin number
        cfg.pin_scl  = 32;   // I2C SCL pin number
        cfg.i2c_addr = 0x5D;          // I2C device addr
        cfg.i2c_port = 1;     // I2C port number
        cfg.freq     = 400000;        // I2C freq
        cfg.x_min    =  14;
        cfg.x_max    = 310;
        cfg.y_min    =   5;
        cfg.y_max    = 448;
        cfg.offset_rotation = 0;
        cfg.bus_shared = false;

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);  
    }

    setPanel(&_panel_instance); // Set the panel to be used.
  }
};