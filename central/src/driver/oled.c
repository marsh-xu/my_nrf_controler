#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include <app_error.h>
#include <app_util_platform.h>
#include <nrf_delay.h>
#include <nrf_gpio.h>
#include <spi_master.h>

#include "pin_config.h"

#include "oled.h"

#include "SEGGER_RTT.h"

#define OLED_COMMAND_DISPLAY_ON           0xAF
#define OLED_COMMAND_DISPLAY_OFF          0xAE

#define OLED_COMMAND_DISPLAY_NORMAL       0xA6
#define OLED_COMMAND_DISPLAY_INVERSE      0xA7

#define OLED_COMMAND_ENTIRE_DISPLAY_ON    0xA5
#define OLED_COMMAND_ENTIRE_DISPLAY_OFF   0xA4

#define OLED_MODE                         0
#define SIZE                              16
#define XLevelL                           0x00
#define XLevelH                           0x10
#define Max_Column                        128
#define Max_Row                           64
#define Brightness                        0xFF
#define X_WIDTH                           128
#define Y_WIDTH                           64

#define OLED_SPI_MASTER_CONFIGURATION                                             \
    {                                                                             \
        SPI_FREQUENCY_FREQUENCY_M4,   /**< Serial clock frequency 4Mbps. */       \
        OLED_SPI_CLOCK_PIN_NUMBER,    /**< SCK pin. */                            \
        SPI_PIN_DISCONNECTED,         /**< MISO pin. */                           \
        OLED_SPI_MOSI_PIN_NUMBER,     /**< MOSI pin. */                           \
        SPI_PIN_DISCONNECTED,         /**< Slave select pin. */                   \
        APP_IRQ_PRIORITY_HIGH,         /**< Interrupt priority LOW. */             \
        SPI_CONFIG_ORDER_MsbFirst,    /**< Bits order MSB. */                     \
        SPI_CONFIG_CPOL_ActiveHigh,   /**< Serial clock polarity ACTIVEHIGH. */   \
        SPI_CONFIG_CPHA_Leading,      /**< Serial clock phase LEADING. */         \
        0                             /**< Don't disable all IRQs. */             \
    };
#define OLED_SPI_MASTER           SPI_MASTER_0       // Choose SPI master 0


typedef enum oled_input_data_type_e
{
    OLED_INPUT_COMMAND = 0,
    OLED_INPUT_DATA,
} oled_input_data_type_t;

typedef enum spi_mode_e
{
    SPI_MODE_GB = 0,
    SPI_MODE_OLED,
} spi_mode_t;

const uint8_t F8X16[]=
{
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,// 0
  0x00,0x00,0x00,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x33,0x30,0x00,0x00,0x00,//! 1
  0x00,0x10,0x0C,0x06,0x10,0x0C,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//" 2
  0x40,0xC0,0x78,0x40,0xC0,0x78,0x40,0x00,0x04,0x3F,0x04,0x04,0x3F,0x04,0x04,0x00,//# 3
  0x00,0x70,0x88,0xFC,0x08,0x30,0x00,0x00,0x00,0x18,0x20,0xFF,0x21,0x1E,0x00,0x00,//$ 4
  0xF0,0x08,0xF0,0x00,0xE0,0x18,0x00,0x00,0x00,0x21,0x1C,0x03,0x1E,0x21,0x1E,0x00,//% 5
  0x00,0xF0,0x08,0x88,0x70,0x00,0x00,0x00,0x1E,0x21,0x23,0x24,0x19,0x27,0x21,0x10,//& 6
  0x10,0x16,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//' 7
  0x00,0x00,0x00,0xE0,0x18,0x04,0x02,0x00,0x00,0x00,0x00,0x07,0x18,0x20,0x40,0x00,//( 8
  0x00,0x02,0x04,0x18,0xE0,0x00,0x00,0x00,0x00,0x40,0x20,0x18,0x07,0x00,0x00,0x00,//) 9
  0x40,0x40,0x80,0xF0,0x80,0x40,0x40,0x00,0x02,0x02,0x01,0x0F,0x01,0x02,0x02,0x00,//* 10
  0x00,0x00,0x00,0xF0,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x1F,0x01,0x01,0x01,0x00,//+ 11
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xB0,0x70,0x00,0x00,0x00,0x00,0x00,//, 12
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,//- 13
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x00,0x00,0x00,//. 14
  0x00,0x00,0x00,0x00,0x80,0x60,0x18,0x04,0x00,0x60,0x18,0x06,0x01,0x00,0x00,0x00,/// 15
  0x00,0xE0,0x10,0x08,0x08,0x10,0xE0,0x00,0x00,0x0F,0x10,0x20,0x20,0x10,0x0F,0x00,//0 16
  0x00,0x10,0x10,0xF8,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00,//1 17
  0x00,0x70,0x08,0x08,0x08,0x88,0x70,0x00,0x00,0x30,0x28,0x24,0x22,0x21,0x30,0x00,//2 18
  0x00,0x30,0x08,0x88,0x88,0x48,0x30,0x00,0x00,0x18,0x20,0x20,0x20,0x11,0x0E,0x00,//3 19
  0x00,0x00,0xC0,0x20,0x10,0xF8,0x00,0x00,0x00,0x07,0x04,0x24,0x24,0x3F,0x24,0x00,//4 20
  0x00,0xF8,0x08,0x88,0x88,0x08,0x08,0x00,0x00,0x19,0x21,0x20,0x20,0x11,0x0E,0x00,//5 21
  0x00,0xE0,0x10,0x88,0x88,0x18,0x00,0x00,0x00,0x0F,0x11,0x20,0x20,0x11,0x0E,0x00,//6 22
  0x00,0x38,0x08,0x08,0xC8,0x38,0x08,0x00,0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00,//7 23
  0x00,0x70,0x88,0x08,0x08,0x88,0x70,0x00,0x00,0x1C,0x22,0x21,0x21,0x22,0x1C,0x00,//8 24
  0x00,0xE0,0x10,0x08,0x08,0x10,0xE0,0x00,0x00,0x00,0x31,0x22,0x22,0x11,0x0F,0x00,//9 25
  0x00,0x00,0x00,0xC0,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,0x00,0x00,//: 26
  0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x60,0x00,0x00,0x00,0x00,//; 27
  0x00,0x00,0x80,0x40,0x20,0x10,0x08,0x00,0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x00,//< 28
  0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x00,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x00,//= 29
  0x00,0x08,0x10,0x20,0x40,0x80,0x00,0x00,0x00,0x20,0x10,0x08,0x04,0x02,0x01,0x00,//> 30
  0x00,0x70,0x48,0x08,0x08,0x08,0xF0,0x00,0x00,0x00,0x00,0x30,0x36,0x01,0x00,0x00,//? 31
  0xC0,0x30,0xC8,0x28,0xE8,0x10,0xE0,0x00,0x07,0x18,0x27,0x24,0x23,0x14,0x0B,0x00,//@ 32
  0x00,0x00,0xC0,0x38,0xE0,0x00,0x00,0x00,0x20,0x3C,0x23,0x02,0x02,0x27,0x38,0x20,//A 33
  0x08,0xF8,0x88,0x88,0x88,0x70,0x00,0x00,0x20,0x3F,0x20,0x20,0x20,0x11,0x0E,0x00,//B 34
  0xC0,0x30,0x08,0x08,0x08,0x08,0x38,0x00,0x07,0x18,0x20,0x20,0x20,0x10,0x08,0x00,//C 35
  0x08,0xF8,0x08,0x08,0x08,0x10,0xE0,0x00,0x20,0x3F,0x20,0x20,0x20,0x10,0x0F,0x00,//D 36
  0x08,0xF8,0x88,0x88,0xE8,0x08,0x10,0x00,0x20,0x3F,0x20,0x20,0x23,0x20,0x18,0x00,//E 37
  0x08,0xF8,0x88,0x88,0xE8,0x08,0x10,0x00,0x20,0x3F,0x20,0x00,0x03,0x00,0x00,0x00,//F 38
  0xC0,0x30,0x08,0x08,0x08,0x38,0x00,0x00,0x07,0x18,0x20,0x20,0x22,0x1E,0x02,0x00,//G 39
  0x08,0xF8,0x08,0x00,0x00,0x08,0xF8,0x08,0x20,0x3F,0x21,0x01,0x01,0x21,0x3F,0x20,//H 40
  0x00,0x08,0x08,0xF8,0x08,0x08,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00,//I 41
  0x00,0x00,0x08,0x08,0xF8,0x08,0x08,0x00,0xC0,0x80,0x80,0x80,0x7F,0x00,0x00,0x00,//J 42
  0x08,0xF8,0x88,0xC0,0x28,0x18,0x08,0x00,0x20,0x3F,0x20,0x01,0x26,0x38,0x20,0x00,//K 43
  0x08,0xF8,0x08,0x00,0x00,0x00,0x00,0x00,0x20,0x3F,0x20,0x20,0x20,0x20,0x30,0x00,//L 44
  0x08,0xF8,0xF8,0x00,0xF8,0xF8,0x08,0x00,0x20,0x3F,0x00,0x3F,0x00,0x3F,0x20,0x00,//M 45
  0x08,0xF8,0x30,0xC0,0x00,0x08,0xF8,0x08,0x20,0x3F,0x20,0x00,0x07,0x18,0x3F,0x00,//N 46
  0xE0,0x10,0x08,0x08,0x08,0x10,0xE0,0x00,0x0F,0x10,0x20,0x20,0x20,0x10,0x0F,0x00,//O 47
  0x08,0xF8,0x08,0x08,0x08,0x08,0xF0,0x00,0x20,0x3F,0x21,0x01,0x01,0x01,0x00,0x00,//P 48
  0xE0,0x10,0x08,0x08,0x08,0x10,0xE0,0x00,0x0F,0x18,0x24,0x24,0x38,0x50,0x4F,0x00,//Q 49
  0x08,0xF8,0x88,0x88,0x88,0x88,0x70,0x00,0x20,0x3F,0x20,0x00,0x03,0x0C,0x30,0x20,//R 50
  0x00,0x70,0x88,0x08,0x08,0x08,0x38,0x00,0x00,0x38,0x20,0x21,0x21,0x22,0x1C,0x00,//S 51
  0x18,0x08,0x08,0xF8,0x08,0x08,0x18,0x00,0x00,0x00,0x20,0x3F,0x20,0x00,0x00,0x00,//T 52
  0x08,0xF8,0x08,0x00,0x00,0x08,0xF8,0x08,0x00,0x1F,0x20,0x20,0x20,0x20,0x1F,0x00,//U 53
  0x08,0x78,0x88,0x00,0x00,0xC8,0x38,0x08,0x00,0x00,0x07,0x38,0x0E,0x01,0x00,0x00,//V 54
  0xF8,0x08,0x00,0xF8,0x00,0x08,0xF8,0x00,0x03,0x3C,0x07,0x00,0x07,0x3C,0x03,0x00,//W 55
  0x08,0x18,0x68,0x80,0x80,0x68,0x18,0x08,0x20,0x30,0x2C,0x03,0x03,0x2C,0x30,0x20,//X 56
  0x08,0x38,0xC8,0x00,0xC8,0x38,0x08,0x00,0x00,0x00,0x20,0x3F,0x20,0x00,0x00,0x00,//Y 57
  0x10,0x08,0x08,0x08,0xC8,0x38,0x08,0x00,0x20,0x38,0x26,0x21,0x20,0x20,0x18,0x00,//Z 58
  0x00,0x00,0x00,0xFE,0x02,0x02,0x02,0x00,0x00,0x00,0x00,0x7F,0x40,0x40,0x40,0x00,//[ 59
  0x00,0x0C,0x30,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x06,0x38,0xC0,0x00,//\ 60
  0x00,0x02,0x02,0x02,0xFE,0x00,0x00,0x00,0x00,0x40,0x40,0x40,0x7F,0x00,0x00,0x00,//] 61
  0x00,0x00,0x04,0x02,0x02,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//^ 62
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,//_ 63
  0x00,0x02,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//` 64
  0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x19,0x24,0x22,0x22,0x22,0x3F,0x20,//a 65
  0x08,0xF8,0x00,0x80,0x80,0x00,0x00,0x00,0x00,0x3F,0x11,0x20,0x20,0x11,0x0E,0x00,//b 66
  0x00,0x00,0x00,0x80,0x80,0x80,0x00,0x00,0x00,0x0E,0x11,0x20,0x20,0x20,0x11,0x00,//c 67
  0x00,0x00,0x00,0x80,0x80,0x88,0xF8,0x00,0x00,0x0E,0x11,0x20,0x20,0x10,0x3F,0x20,//d 68
  0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x1F,0x22,0x22,0x22,0x22,0x13,0x00,//e 69
  0x00,0x80,0x80,0xF0,0x88,0x88,0x88,0x18,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00,//f 70
  0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x6B,0x94,0x94,0x94,0x93,0x60,0x00,//g 71
  0x08,0xF8,0x00,0x80,0x80,0x80,0x00,0x00,0x20,0x3F,0x21,0x00,0x00,0x20,0x3F,0x20,//h 72
  0x00,0x80,0x98,0x98,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00,//i 73
  0x00,0x00,0x00,0x80,0x98,0x98,0x00,0x00,0x00,0xC0,0x80,0x80,0x80,0x7F,0x00,0x00,//j 74
  0x08,0xF8,0x00,0x00,0x80,0x80,0x80,0x00,0x20,0x3F,0x24,0x02,0x2D,0x30,0x20,0x00,//k 75
  0x00,0x08,0x08,0xF8,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00,0x00,//l 76
  0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x20,0x3F,0x20,0x00,0x3F,0x20,0x00,0x3F,//m 77
  0x80,0x80,0x00,0x80,0x80,0x80,0x00,0x00,0x20,0x3F,0x21,0x00,0x00,0x20,0x3F,0x20,//n 78
  0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x1F,0x20,0x20,0x20,0x20,0x1F,0x00,//o 79
  0x80,0x80,0x00,0x80,0x80,0x00,0x00,0x00,0x80,0xFF,0xA1,0x20,0x20,0x11,0x0E,0x00,//p 80
  0x00,0x00,0x00,0x80,0x80,0x80,0x80,0x00,0x00,0x0E,0x11,0x20,0x20,0xA0,0xFF,0x80,//q 81
  0x80,0x80,0x80,0x00,0x80,0x80,0x80,0x00,0x20,0x20,0x3F,0x21,0x20,0x00,0x01,0x00,//r 82
  0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x33,0x24,0x24,0x24,0x24,0x19,0x00,//s 83
  0x00,0x80,0x80,0xE0,0x80,0x80,0x00,0x00,0x00,0x00,0x00,0x1F,0x20,0x20,0x00,0x00,//t 84
  0x80,0x80,0x00,0x00,0x00,0x80,0x80,0x00,0x00,0x1F,0x20,0x20,0x20,0x10,0x3F,0x20,//u 85
  0x80,0x80,0x80,0x00,0x00,0x80,0x80,0x80,0x00,0x01,0x0E,0x30,0x08,0x06,0x01,0x00,//v 86
  0x80,0x80,0x00,0x80,0x00,0x80,0x80,0x80,0x0F,0x30,0x0C,0x03,0x0C,0x30,0x0F,0x00,//w 87
  0x00,0x80,0x80,0x00,0x80,0x80,0x80,0x00,0x00,0x20,0x31,0x2E,0x0E,0x31,0x20,0x00,//x 88
  0x80,0x80,0x80,0x00,0x00,0x80,0x80,0x80,0x80,0x81,0x8E,0x70,0x18,0x06,0x01,0x00,//y 89
  0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x21,0x30,0x2C,0x22,0x21,0x30,0x00,//z 90
  0x00,0x00,0x00,0x00,0x80,0x7C,0x02,0x02,0x00,0x00,0x00,0x00,0x00,0x3F,0x40,0x40,//{ 91
  0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,//| 92
  0x00,0x02,0x02,0x7C,0x80,0x00,0x00,0x00,0x00,0x40,0x40,0x3F,0x00,0x00,0x00,0x00,//} 93
  0x00,0x06,0x01,0x01,0x02,0x02,0x04,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//~ 94
};

uint8_t conv_data[32] = {0};


static void oled_pin_config(void)
{
    nrf_gpio_cfg_output(OLED_VDD_ENABLE_PIN_NUMBER);
    nrf_gpio_cfg_output(OLED_VCC_ENABLE_PIN_NUMBER);
    nrf_gpio_cfg_output(OLED_RESET_PIN_NUMBER);
    nrf_gpio_cfg_output(OLED_CD_CTRL_PIN_NUMBER);
    nrf_gpio_cfg_output(OLED_CE_PIN_NUMBER);
    nrf_gpio_cfg_output(GB_CE_PIN_NUMBER);

    nrf_gpio_pin_clear(OLED_CE_PIN_NUMBER);
    nrf_gpio_pin_set(GB_CE_PIN_NUMBER);

    nrf_gpio_pin_clear(OLED_VDD_ENABLE_PIN_NUMBER);
    nrf_gpio_pin_clear(OLED_VCC_ENABLE_PIN_NUMBER);
    nrf_gpio_pin_set(OLED_RESET_PIN_NUMBER);
    nrf_gpio_pin_clear(OLED_CD_CTRL_PIN_NUMBER);

    nrf_gpio_cfg_output(4);
    nrf_gpio_cfg_output(5);
    nrf_gpio_cfg_output(6);
    nrf_gpio_pin_set(4);
    nrf_gpio_pin_set(5);
    nrf_gpio_pin_set(6);
}


static void choose_spi_mode(spi_mode_t mode)
{
    if (mode == SPI_MODE_OLED)
    {
        nrf_gpio_pin_clear(OLED_CE_PIN_NUMBER);
        nrf_gpio_pin_set(GB_CE_PIN_NUMBER);
    }
    else
    {
        nrf_gpio_pin_set(OLED_CE_PIN_NUMBER);
        nrf_gpio_pin_clear(GB_CE_PIN_NUMBER);
    }
}


static void enable_oled_vdd(bool enable_vdd)
{
    if (enable_vdd)
    {
        nrf_gpio_pin_set(OLED_VDD_ENABLE_PIN_NUMBER);
    }
    else
    {
        nrf_gpio_pin_clear(OLED_VDD_ENABLE_PIN_NUMBER);
    }
}


static void enable_oled_vcc(bool enable_vcc)
{
    if (enable_vcc)
    {
        nrf_gpio_pin_set(OLED_VCC_ENABLE_PIN_NUMBER);
    }
    else
    {
        nrf_gpio_pin_clear(OLED_VCC_ENABLE_PIN_NUMBER);
    }
}


static void set_input_data_type(oled_input_data_type_t input_type)
{
    if (input_type == OLED_INPUT_COMMAND)
    {
        nrf_gpio_pin_clear(OLED_CD_CTRL_PIN_NUMBER);
    }
    else
    {
        nrf_gpio_pin_set(OLED_CD_CTRL_PIN_NUMBER);
    }
}


static void oled_spi_send_data(uint8_t *data, uint16_t data_length)
{
    set_input_data_type(OLED_INPUT_DATA);
    uint32_t err_code;
    err_code = spi_master_send_recv(OLED_SPI_MASTER, data, data_length, NULL, 0);
    APP_ERROR_CHECK(err_code);
}

static void oled_spi_single_command(uint8_t data)
{
    set_input_data_type(OLED_INPUT_COMMAND);
    uint32_t err_code;
    uint8_t command = data;
    err_code = spi_master_send_recv(OLED_SPI_MASTER, &command, 1, NULL, 0);
    APP_ERROR_CHECK(err_code);
}

static void oled_spi_send_command(uint8_t *data, uint16_t data_length)
{
    set_input_data_type(OLED_INPUT_COMMAND);
    uint32_t err_code;
    err_code = spi_master_send_recv(OLED_SPI_MASTER, data, data_length, NULL, 0);
    APP_ERROR_CHECK(err_code);
}


void spi_master_event_handler(spi_master_evt_t spi_master_evt)
{
    switch (spi_master_evt.evt_type)
    {
        case SPI_MASTER_EVT_TRANSFER_COMPLETED:
            // TODO: add special handle for transfer complete event.
            break;

        default:
            // No implementation needed.
            break;
    }
}


static void oled_spi_init(void)
{
    uint32_t err_code;
    // Structure for SPI master configuration.
    spi_master_config_t spi_config = OLED_SPI_MASTER_CONFIGURATION;

    // Initialize SPI master.
    err_code = spi_master_open(OLED_SPI_MASTER, &spi_config);
    nrf_delay_ms(1000);
    APP_ERROR_CHECK(err_code);

    // Register event handler for SPI master.
    spi_master_evt_handler_reg(OLED_SPI_MASTER, spi_master_event_handler);
}


static void oled_reset(void)
{
    nrf_gpio_pin_clear(OLED_RESET_PIN_NUMBER);
    nrf_delay_us(6);
    nrf_gpio_pin_set(OLED_RESET_PIN_NUMBER);
    nrf_delay_us(2);
}


static void oled_display_on_off(bool on_off)
{
    uint8_t command;
    if (on_off)
    {
        command = OLED_COMMAND_DISPLAY_ON;
    }
    else
    {
        command = OLED_COMMAND_DISPLAY_OFF;
    }

    oled_spi_send_command(&command, sizeof(command));
}


static void oled_power_on_off(bool on_off)
{
    if (on_off)
    {
        enable_oled_vdd(true);
        oled_reset();
        enable_oled_vcc(true);
        oled_display_on_off(true);
    }
    else
    {
        oled_display_on_off(false);
        enable_oled_vcc(true);
        nrf_delay_ms(100);
        enable_oled_vdd(false);
    }
}


static void oled_cmd_set_contrast_control(uint8_t contrast_num)
{
    uint8_t cmd[2] = {0x81, 0x00};
    cmd[1] = contrast_num;
    oled_spi_send_command((uint8_t*)&cmd, sizeof(cmd));
}

static void oled_cmd_set_inverse_display(bool is_inversed)
{
    uint8_t command;
    if (is_inversed)
    {
        command = OLED_COMMAND_DISPLAY_NORMAL;
    }
    else
    {
        command = OLED_COMMAND_DISPLAY_INVERSE;
    }

    oled_spi_send_command(&command, sizeof(command));
}


static void oled_cmd_entire_display(bool is_entire_display)
{
    uint8_t command;
    if (is_entire_display)
    {
        command = OLED_COMMAND_ENTIRE_DISPLAY_ON;
    }
    else
    {
        command = OLED_COMMAND_ENTIRE_DISPLAY_OFF;
    }

    oled_spi_send_command(&command, sizeof(command));
}


//清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样!!!
static void oled_clear(void)
{
    uint8_t i,n;
    for(i=0;i<8;i++)
    {
        oled_spi_single_command (0xb0+i);    //设置页地址（0~7）
        oled_spi_single_command (0x00);      //设置显示位置―列低地址
        oled_spi_single_command (0x10);      //设置显示位置―列高地址
        for(n=0;n<128;n++)
        {
            uint8_t data = 0;
            oled_spi_send_data(&data, 1);
        }
    }
}

static void oled_set_pos(uint8_t x, uint8_t y)
{
    uint8_t data = 0;
    data = 0xb0+y;
    oled_spi_send_command(&data, 1);
    data = ((x&0xf0)>>4)|0x10;
    oled_spi_send_command(&data, 1);
    data = (x&0x0f)|0x01;
    oled_spi_send_command(&data, 1);
}


//在指定位置显示一个字符,包括部分字符
//x:0~127
//y:0~63
//mode:0,反白显示;1,正常显示
//size:选择字体 16/12
void oled_show_char(uint8_t x,uint8_t y,uint8_t chr)
{
    uint8_t c=0,i=0;
    c = chr - ' ';//得到偏移后的值

    if( x > Max_Column-1)
    {
        x=0;
        y=y+2;
    }

    oled_set_pos(x,y);
    for(i=0;i<8;i++)
    {
        uint8_t data = F8X16[c*16+i];
        oled_spi_send_data(&data,1);
    }
    oled_set_pos(x,y+1);
    for(i=0;i<8;i++)
    {
        uint8_t data = F8X16[c*16+i+8];
        oled_spi_send_data(&data,1);
    }
}

void oled_show_string(uint8_t x,uint8_t y,uint8_t *chr)
{
    uint8_t j=0;
    while (chr[j]!='\0')
    {
        oled_show_char(x,y,chr[j]);
        x+=8;
        if(x>120){x=0;y+=2;}
            j++;
    }
}


void convert_chinese_word(uint8_t *dest, uint8_t *src)
{
    uint8_t index = 0;

    for(index = 0; index < 8; index ++)
    {
        uint8_t data = 0;
        for(uint8_t i = 0; i < 8; i++)
        {
            data = data | (((src[i*2 ] >> (8 -1 - index)) & 0x01) << i);
        }

        dest[index] = data;
    }

    for(index = 8; index < 16; index ++)
    {
        uint8_t data = 0;
        for(uint8_t i = 0; i < 8; i++)
        {
            data = data | (((src[i*2 +1] >> (16 -1 - index)) & 0x01) << i);
        }

        dest[index] = data;
    }

    for(index = 16; index < 24; index ++)
    {
        uint8_t data = 0;
        for(uint8_t i = 0; i < 8; i++)
        {
            data = data | (((src[i*2+16 ] >> (24 -1 - index)) & 0x01) << i);
        }

        dest[index] = data;
    }

    for(index = 24; index < 32; index ++)
    {
        uint8_t data = 0;
        for(uint8_t i = 0; i < 8; i++)
        {
            data = data | (((src[i*2+16 +1] >> (32 -1 - index)) & 0x01) << i);
        }

        dest[index] = data;
    }
}


void oled_show_chinese(uint8_t x,uint8_t y,uint8_t *src)
{
    convert_chinese_word(conv_data,src);
    uint8_t i = 0;
    if( x > Max_Column-1)
    {
        x=0;
        y=y+2;
    }

    oled_set_pos(x,y);
    for(i=0;i<16;i++)
    {
        uint8_t data = conv_data[i];
        oled_spi_send_data(&data,1);
    }
    oled_set_pos(x,y+1);
    for(i=0;i<16;i++)
    {
        uint8_t data = conv_data[i+16];
        oled_spi_send_data(&data,1);
    }
}

uint16_t convent_chinese_to_utf8(uint8_t *word)
{
    uint16_t data = 0;
    data = word[2] & 0x3F;
    data |= (word[1] & 0x3F)<<6;
    data |= (word[0] & 0x0F)<<12;

    return data;
}


void show_ui_get_temperature(void)
{
    uint8_t raw_data[6][32] =
    {
        {0x04,0x40,0x04,0x44,0xFF,0xFE,0x04,0x40,0x24,0x28,0x18,0x24,0x10,0x20,0x2B,0xFE,
        0x48,0x20,0x08,0x20,0x18,0x20,0x28,0x50,0xC8,0x50,0x08,0x88,0x29,0x06,0x12,0x04},  //获
        {0x01,0x00,0xFF,0x80,0x22,0x00,0x23,0xFC,0x3E,0x04,0x22,0x84,0x22,0x88,0x3E,0x88,
        0x22,0x48,0x22,0x50,0x22,0x20,0x3E,0x50,0xE2,0x48,0x42,0x8E,0x03,0x04,0x02,0x00},
        {0x01,0x00,0x21,0x08,0x19,0x18,0x09,0x20,0x01,0x00,0x01,0x08,0x7F,0xFC,0x00,0x08,
        0x00,0x08,0x00,0x08,0x3F,0xF8,0x00,0x08,0x00,0x08,0x00,0x08,0x7F,0xF8,0x00,0x08},
        {0x10,0x10,0x0C,0x30,0x04,0x44,0xFF,0xFE,0x00,0x00,0x3E,0x08,0x22,0x48,0x22,0x48,
        0x3E,0x48,0x22,0x48,0x22,0x48,0x3E,0x48,0x22,0x08,0x22,0x08,0x2A,0x28,0x24,0x10},
        {0x00,0x08,0x43,0xFC,0x32,0x08,0x12,0x08,0x83,0xF8,0x62,0x08,0x22,0x08,0x0B,0xF8,
        0x10,0x00,0x27,0xFC,0xE4,0xA4,0x24,0xA4,0x24,0xA4,0x24,0xA4,0x2F,0xFE,0x20,0x00},
        {0x01,0x00,0x00,0x84,0x3F,0xFE,0x22,0x20,0x22,0x28,0x3F,0xFC,0x22,0x20,0x23,0xE0,
        0x20,0x00,0x2F,0xF0,0x22,0x20,0x21,0x40,0x20,0x80,0x43,0x60,0x8C,0x1E,0x30,0x04},
    };

    for (uint8_t i = 0; i < 6; i ++)
    {
        oled_show_chinese(16*i, 4, raw_data[i]);
    }
}

void show_ui_get_temp_threshold(void)
{
    uint8_t raw_data[6][32] =
    {
        {0x04,0x40,0x04,0x44,0xFF,0xFE,0x04,0x40,0x24,0x28,0x18,0x24,0x10,0x20,0x2B,0xFE,
        0x48,0x20,0x08,0x20,0x18,0x20,0x28,0x50,0xC8,0x50,0x08,0x88,0x29,0x06,0x12,0x04},  //获
        {0x01,0x00,0xFF,0x80,0x22,0x00,0x23,0xFC,0x3E,0x04,0x22,0x84,0x22,0x88,0x3E,0x88,
        0x22,0x48,0x22,0x50,0x22,0x20,0x3E,0x50,0xE2,0x48,0x42,0x8E,0x03,0x04,0x02,0x00},
        {0x00,0x08,0x43,0xFC,0x32,0x08,0x12,0x08,0x83,0xF8,0x62,0x08,0x22,0x08,0x0B,0xF8,
        0x10,0x00,0x27,0xFC,0xE4,0xA4,0x24,0xA4,0x24,0xA4,0x24,0xA4,0x2F,0xFE,0x20,0x00},
        {0x01,0x00,0x00,0x84,0x3F,0xFE,0x22,0x20,0x22,0x28,0x3F,0xFC,0x22,0x20,0x23,0xE0,
        0x20,0x00,0x2F,0xF0,0x22,0x20,0x21,0x40,0x20,0x80,0x43,0x60,0x8C,0x1E,0x30,0x04},
        {0x20,0x04,0x17,0xFE,0x40,0xA4,0x40,0x94,0x7F,0xFC,0x40,0x84,0x5E,0x84,0x52,0x94,
        0x52,0x54,0x52,0x64,0x5E,0x44,0x40,0x54,0x7E,0xB4,0x41,0x14,0x42,0x04,0x40,0x0C},
        {0x08,0x40,0x08,0x48,0x0F,0xFC,0x10,0x40,0x10,0x48,0x33,0xFC,0x52,0x08,0x93,0xF8,
        0x12,0x08,0x13,0xF8,0x12,0x08,0x13,0xF8,0x12,0x08,0x12,0x08,0x1F,0xFE,0x10,0x00},
    };

    for (uint8_t i = 0; i < 6; i ++)
    {
        oled_show_chinese(16*i, 4, raw_data[i]);
    }
}

void show_ui_set_temp_threshold(void)
{
    uint8_t raw_data[6][32] =
    {
        {0x01,0xF0,0x21,0x10,0x11,0x10,0x11,0x10,0x01,0x10,0x02,0x0E,0xF4,0x00,0x13,0xF8,
        0x11,0x08,0x11,0x10,0x10,0x90,0x10,0xA0,0x14,0x40,0x18,0xB0,0x13,0x0E,0x0C,0x04},
        {0x3F,0xF8,0x24,0x48,0x3F,0xF8,0x01,0x00,0x7F,0xFC,0x01,0x00,0x1F,0xF0,0x10,0x10,
        0x1F,0xF0,0x10,0x10,0x1F,0xF0,0x10,0x10,0x1F,0xF0,0x10,0x14,0xFF,0xFE,0x00,0x00},
        {0x00,0x08,0x43,0xFC,0x32,0x08,0x12,0x08,0x83,0xF8,0x62,0x08,0x22,0x08,0x0B,0xF8,
        0x10,0x00,0x27,0xFC,0xE4,0xA4,0x24,0xA4,0x24,0xA4,0x24,0xA4,0x2F,0xFE,0x20,0x00},
        {0x01,0x00,0x00,0x84,0x3F,0xFE,0x22,0x20,0x22,0x28,0x3F,0xFC,0x22,0x20,0x23,0xE0,
        0x20,0x00,0x2F,0xF0,0x22,0x20,0x21,0x40,0x20,0x80,0x43,0x60,0x8C,0x1E,0x30,0x04},
        {0x20,0x04,0x17,0xFE,0x40,0xA4,0x40,0x94,0x7F,0xFC,0x40,0x84,0x5E,0x84,0x52,0x94,
        0x52,0x54,0x52,0x64,0x5E,0x44,0x40,0x54,0x7E,0xB4,0x41,0x14,0x42,0x04,0x40,0x0C},
        {0x08,0x40,0x08,0x48,0x0F,0xFC,0x10,0x40,0x10,0x48,0x33,0xFC,0x52,0x08,0x93,0xF8,
        0x12,0x08,0x13,0xF8,0x12,0x08,0x13,0xF8,0x12,0x08,0x12,0x08,0x1F,0xFE,0x10,0x00},
    };

    for (uint8_t i = 0; i < 6; i ++)
    {
        oled_show_chinese(16*i, 4, raw_data[i]);
    }
}

void show_ui_get_motor_speed(void)
{
    uint8_t raw_data[6][32] =
    {
        {0x04,0x40,0x04,0x44,0xFF,0xFE,0x04,0x40,0x24,0x28,0x18,0x24,0x10,0x20,0x2B,0xFE,
        0x48,0x20,0x08,0x20,0x18,0x20,0x28,0x50,0xC8,0x50,0x08,0x88,0x29,0x06,0x12,0x04},  //获
        {0x01,0x00,0xFF,0x80,0x22,0x00,0x23,0xFC,0x3E,0x04,0x22,0x84,0x22,0x88,0x3E,0x88,
        0x22,0x48,0x22,0x50,0x22,0x20,0x3E,0x50,0xE2,0x48,0x42,0x8E,0x03,0x04,0x02,0x00},
        {0x00,0x20,0x3F,0xF0,0x00,0x20,0x08,0x20,0x08,0x20,0x08,0x20,0x08,0x20,0x08,0x24,
        0x0F,0xFE,0x00,0x04,0x00,0x24,0xFF,0xF4,0x00,0x04,0x00,0x04,0x00,0x28,0x00,0x10},
        {0x00,0x80,0x40,0x80,0x20,0x80,0x20,0x80,0x00,0x88,0x0F,0xFC,0xE0,0x80,0x21,0x00,
        0x21,0x40,0x22,0x20,0x22,0x10,0x24,0x18,0x28,0x08,0x50,0x06,0x8F,0xFC,0x00,0x00},
        {0x00,0x40,0x40,0x44,0x37,0xFE,0x10,0x40,0x07,0xFC,0x04,0x44,0xF4,0x44,0x17,0xFC,
        0x14,0x44,0x10,0xE0,0x11,0x58,0x12,0x48,0x14,0x40,0x28,0x46,0x47,0xFC,0x00,0x00},
        {0x01,0x00,0x00,0x84,0x3F,0xFE,0x22,0x20,0x22,0x28,0x3F,0xFC,0x22,0x20,0x23,0xE0,
        0x20,0x00,0x2F,0xF0,0x22,0x20,0x21,0x40,0x20,0x80,0x43,0x60,0x8C,0x1E,0x30,0x04},
    };

    for (uint8_t i = 0; i < 6; i ++)
    {
        oled_show_chinese(16*i, 4, raw_data[i]);
    }
}


void show_ui_set_motor_speed(void)
{
    uint8_t raw_data[6][32] =
    {
        {0x01,0xF0,0x21,0x10,0x11,0x10,0x11,0x10,0x01,0x10,0x02,0x0E,0xF4,0x00,0x13,0xF8,
        0x11,0x08,0x11,0x10,0x10,0x90,0x10,0xA0,0x14,0x40,0x18,0xB0,0x13,0x0E,0x0C,0x04},
        {0x3F,0xF8,0x24,0x48,0x3F,0xF8,0x01,0x00,0x7F,0xFC,0x01,0x00,0x1F,0xF0,0x10,0x10,
        0x1F,0xF0,0x10,0x10,0x1F,0xF0,0x10,0x10,0x1F,0xF0,0x10,0x14,0xFF,0xFE,0x00,0x00},
        {0x00,0x20,0x3F,0xF0,0x00,0x20,0x08,0x20,0x08,0x20,0x08,0x20,0x08,0x20,0x08,0x24,
        0x0F,0xFE,0x00,0x04,0x00,0x24,0xFF,0xF4,0x00,0x04,0x00,0x04,0x00,0x28,0x00,0x10},
        {0x00,0x80,0x40,0x80,0x20,0x80,0x20,0x80,0x00,0x88,0x0F,0xFC,0xE0,0x80,0x21,0x00,
        0x21,0x40,0x22,0x20,0x22,0x10,0x24,0x18,0x28,0x08,0x50,0x06,0x8F,0xFC,0x00,0x00},
        {0x00,0x40,0x40,0x44,0x37,0xFE,0x10,0x40,0x07,0xFC,0x04,0x44,0xF4,0x44,0x17,0xFC,
        0x14,0x44,0x10,0xE0,0x11,0x58,0x12,0x48,0x14,0x40,0x28,0x46,0x47,0xFC,0x00,0x00},
        {0x01,0x00,0x00,0x84,0x3F,0xFE,0x22,0x20,0x22,0x28,0x3F,0xFC,0x22,0x20,0x23,0xE0,
        0x20,0x00,0x2F,0xF0,0x22,0x20,0x21,0x40,0x20,0x80,0x43,0x60,0x8C,0x1E,0x30,0x04},
    };

    for (uint8_t i = 0; i < 6; i ++)
    {
        oled_show_chinese(16*i, 4, raw_data[i]);
    }
}


void show_ui_motor_control(void)
{
    uint8_t raw_data[6][32] =
    {
        {0x00,0x20,0x3F,0xF0,0x00,0x20,0x08,0x20,0x08,0x20,0x08,0x20,0x08,0x20,0x08,0x24,
        0x0F,0xFE,0x00,0x04,0x00,0x24,0xFF,0xF4,0x00,0x04,0x00,0x04,0x00,0x28,0x00,0x10},
        {0x00,0x80,0x40,0x80,0x20,0x80,0x20,0x80,0x00,0x88,0x0F,0xFC,0xE0,0x80,0x21,0x00,
        0x21,0x40,0x22,0x20,0x22,0x10,0x24,0x18,0x28,0x08,0x50,0x06,0x8F,0xFC,0x00,0x00},
        {0x10,0x40,0x10,0x20,0x13,0xFE,0x12,0x02,0xFC,0x54,0x10,0x88,0x15,0x04,0x18,0x00,
        0x30,0x08,0xD3,0xFC,0x10,0x20,0x10,0x20,0x10,0x20,0x10,0x24,0x57,0xFE,0x20,0x00},
        {0x3F,0xF8,0x24,0x48,0x3F,0xF8,0x01,0x00,0x7F,0xFC,0x01,0x00,0x1F,0xF0,0x10,0x10,
        0x1F,0xF0,0x10,0x10,0x1F,0xF0,0x10,0x10,0x1F,0xF0,0x10,0x14,0xFF,0xFE,0x00,0x00},
    };

    for (uint8_t i = 0; i < 6; i ++)
    {
        oled_show_chinese(16*i, 4, raw_data[i]);
    }
}


void oled_show_connect_status(bool is_connect)
{
    if (is_connect)
    {
        uint8_t data[32] = {
        0x00, 0x00, 0x00, 0x1E, 0x00, 0x32, 0x00, 0x22, 0x01, 0xEE, 0x03, 0x38, 0x06, 0x10, 0x0C, 0x30,
        0x08, 0x60, 0x3E, 0xC0, 0x23, 0x80, 0x63, 0x00, 0x4E, 0x00, 0x58, 0x00, 0x70, 0x00, 0x00, 0x00,};

        oled_show_chinese(96, 0, data);
    }
    else
    {
        uint8_t data[32] = {
        0x00, 0x00, 0x0C, 0x1E, 0x04, 0x32, 0x04, 0x22, 0x07, 0xEE, 0x03, 0x38, 0x06, 0x10, 0x0F, 0x30,
        0x09, 0x60, 0x3F, 0xC0, 0x23, 0xC0, 0x63, 0x40, 0x4E, 0x20, 0x58, 0x30, 0x70, 0x00, 0x00, 0x00,};

        oled_show_chinese(96, 0, data);
    }
}


void oled_show_choose_status(bool is_choose)
{
    if (is_choose)
    {
        uint8_t data[32] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x18, 0x00, 0x30,
        0x00, 0x60, 0x20, 0xC0, 0x31, 0x80, 0x1F, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,};

        oled_show_chinese(112, 4, data);
    }
    else
    {
        uint8_t data[32] = {0};

        oled_show_chinese(112, 4, data);
    }
}


void oled_init(void)
{
    oled_pin_config();

    choose_spi_mode(SPI_MODE_OLED);

    oled_spi_init();
    oled_power_on_off(true);

    oled_spi_single_command(0xAE);//--turn off oled panel
    oled_spi_single_command(0x00);//---set low column address
    oled_spi_single_command(0x10);//---set high column address
    oled_spi_single_command(0x40);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    oled_spi_single_command(0x81);//--set contrast control register
    oled_spi_single_command(0xCF); // Set SEG Output Current Brightness
    oled_spi_single_command(0xA1);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
    oled_spi_single_command(0xC8);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
    oled_spi_single_command(0xA6);//--set normal display
    oled_spi_single_command(0xA8);//--set multiplex ratio(1 to 64)
    oled_spi_single_command(0x3f);//--1/64 duty
    oled_spi_single_command(0xD3);//-set display offset   Shift Mapping RAM Counter (0x00~0x3F)
    oled_spi_single_command(0x00);//-not offset
    oled_spi_single_command(0xd5);//--set display clock divide ratio/oscillator frequency
    oled_spi_single_command(0x80);//--set divide ratio, Set Clock as 100 Frames/Sec
    oled_spi_single_command(0xD9);//--set pre-charge period
    oled_spi_single_command(0xF1);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    oled_spi_single_command(0xDA);//--set com pins hardware configuration
    oled_spi_single_command(0x12);
    oled_spi_single_command(0xDB);//--set vcomh
    oled_spi_single_command(0x40);//Set VCOM Deselect Level
    oled_spi_single_command(0x20);//-Set Page Addressing Mode (0x00/0x01/0x02)
    oled_spi_single_command(0x02);//
    oled_spi_single_command(0x8D);//--set Charge Pump enable/disable
    oled_spi_single_command(0x14);//--set(0x10) disable
    oled_spi_single_command(0xA4);// Disable Entire Display On (0xa4/0xa5)
    oled_spi_single_command(0xA6);// Disable Inverse Display On (0xa6/a7)
    oled_spi_single_command(0xAF);//--turn on oled panel
    oled_spi_single_command(0xAF); /*display ON*/
    oled_clear();
    oled_set_pos(0,0);


    oled_show_connect_status(false);

    //show_ui_get_temperature();
    //show_ui_get_temp_threshold();
    //show_ui_get_motor_speed();
}

void ui_up_update(oled_ui_style_t index)
{
    switch (index)
    {
        case UI_STYLE_GET_TEMPERATURE:
            show_ui_get_temperature();
            break;
        case UI_STYLE_GET_TEMP_THRESHOLD:
            show_ui_get_temp_threshold();
            break;
        case UI_STYLE_GET_MOTOR_SPEED:
            show_ui_get_motor_speed();
            break;
        case UI_STYLE_SET_TEMP_THRESHOLD:
            show_ui_set_temp_threshold();
            break;
        case UI_STYLE_SET_MOTOR_SPEED:
            show_ui_set_motor_speed();
            break;
        case UI_STYLE_SET_MOTOR_CONTROL:
            show_ui_motor_control();
            break;
        default:
            break;
    }
}


void oled_show_num(uint8_t num)
{
    uint8_t data[4] = {0};

    oled_clear_num();

    if (num >= 100)
    {
        data[0] = num / 100 + '0';
        data[1] = (num % 100) / 10 + '0';
        data[2] = num % 10 + '0';
    }
    else if (num >= 10)
    {
        data[0] = num / 10 + '0';
        data[1] = num % 10 + '0';
    }
    else
    {
        data[0] = num + '0';
    }

    oled_show_string(32, 6, data);
}

void oled_clear_num(void)
{
    uint8_t data = ' ';
    oled_show_char(32+0, 6, data);
    oled_show_char(32+8, 6, data);
    oled_show_char(32+16, 6, data);
}

