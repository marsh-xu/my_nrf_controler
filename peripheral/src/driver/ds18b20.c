#include <stdbool.h>

#include "nrf51.h"
#include "nrf51_bitfields.h"

#include "app_error.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

#include "pin_config.h"
#include "system_error.h"
#include "SEGGER_RTT.h"

#include "ds18b20.h"

/*!< Pulls SDA line high */
#define DS18B20_SDA_HIGH()   do { \
        NRF_GPIO->OUTSET = (1UL << DS18B20_SDA_PIN_NUMBER);  \
    } while (0)

/*!< Pulls SDA line low  */
#define DS18B20_SDA_LOW()    do { \
        NRF_GPIO->OUTCLR = (1UL << DS18B20_SDA_PIN_NUMBER);  \
    } while (0)

/*!< Configures SDA pin as input  */
#define DS18B20_SDA_INPUT()  do { \
            NRF_GPIO->PIN_CNF[DS18B20_SDA_PIN_NUMBER] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)\
                                        | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)\
                                        | (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)\
                                        | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)\
                                        | (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);\
    } while (0)

/*!< Configures SDA pin as output */
#define DS18B20_SDA_OUTPUT() do { \
        NRF_GPIO->DIRSET = (1UL << DS18B20_SDA_PIN_NUMBER);  \
    } while (0)

/*!< Reads current state of SDA */
#define DS18B20_SDA_READ() ((NRF_GPIO->IN >> DS18B20_SDA_PIN_NUMBER) & 0x1UL)

static uint8_t read_one_byte(void)
{
    uint8_t i = 0;
    uint8_t data = 0;

    for (i = 8; i > 0; i--)
    {
        DS18B20_SDA_OUTPUT();
        DS18B20_SDA_HIGH();
        nrf_delay_us(4);
        DS18B20_SDA_LOW();
        nrf_delay_us(4);
        data >>= 1;
        DS18B20_SDA_INPUT();
        nrf_delay_us(1);
        if (1 == DS18B20_SDA_READ())
        {
            data |= 0x80;
        }
        else
        {
            // No implementation needed here.
        }
        nrf_delay_us(60);
    }

    return data;
}


static void write_byte(uint8_t data)
{
    uint8_t i = 0;

    DS18B20_SDA_OUTPUT();
    for (i = 8; i > 0; i--)
    {
        DS18B20_SDA_HIGH();
        nrf_delay_us(4);
        DS18B20_SDA_LOW();
        nrf_delay_us(4);
        if (data & 0x01)
        {
            DS18B20_SDA_HIGH();
        }
        else
        {
            DS18B20_SDA_LOW();
        }
        nrf_delay_us(60);
        data >>= 1;
    }
    DS18B20_SDA_INPUT();
}


bool ds18b20_init(void)
{
    bool result = true;

    DS18B20_SDA_OUTPUT();
    DS18B20_SDA_HIGH();
    nrf_delay_us(2);
    DS18B20_SDA_LOW();
    nrf_delay_us(500);
    DS18B20_SDA_INPUT();
    nrf_delay_us(60);
    if (0 == DS18B20_SDA_READ())
    {
        result = true;
    }
    else
    {
        result = false;
    }

    nrf_delay_us(420);
    DS18B20_SDA_INPUT();
    if(result == true)
    {
        if (0 == DS18B20_SDA_READ())
        {
            result = false;
        }
    }

    return result;
}


uint16_t ds18b20_read_temperature(void)
{
    uint16_t temperature = 0xFFFF;
    uint8_t temperature_data_low = 0;
    uint8_t temperature_data_high = 0;

    SEGGER_RTT_printf(0, "Start to read temperature!\r\n");
    if (true == ds18b20_init())
    {
        write_byte(0xCC);
        write_byte(0x44);
        nrf_delay_ms(800);
        if (true == ds18b20_init())
        {
            write_byte(0xCC);
            write_byte(0xBE);
            temperature_data_low = read_one_byte();
            temperature_data_high = read_one_byte();

            //SEGGER_RTT_printf(0, "data_low = %d, data_high = %d\r\n", temperature_data_low, temperature_data_high);

            temperature = (temperature_data_high << 8) | temperature_data_low;

            if (temperature < 0xFFF)
            {
                temperature = temperature * 0.0625 * 10 + 0.5;
            }
            else
            {
                temperature = ~temperature + 1;
                temperature = temperature * 0.0625 * 10 + 0.5;
                temperature = temperature | 0x8000;
            }

            //temperature = temperature * 625 / 10000;
            SEGGER_RTT_printf(0, "temperature = %p\r\n", temperature);
            SEGGER_RTT_printf(0, "DS18B20 init %s\r\n", "Success");
        }
        else
        {
            APP_ERROR_CHECK(APP_ERROR_DS18B20_INIT);
        }
    }
    else
    {
        APP_ERROR_CHECK(APP_ERROR_DS18B20_INIT);
    }

    return temperature;
}
