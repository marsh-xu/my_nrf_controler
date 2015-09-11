#include <stdbool.h>

#include <app_error.h>
#include <app_util_platform.h>
#include <nrf_delay.h>
#include <nrf_gpio.h>
#include <spi_master.h>

#include "pin_config.h"

#include "oled.h"

#define OLED_COMMAND_DISPLAY_ON           0xAF
#define OLED_COMMAND_DISPLAY_OFF          0xAE

#define OLED_COMMAND_DISPLAY_NORMAL       0xA6
#define OLED_COMMAND_DISPLAY_INVERSE      0xA7

#define OLED_COMMAND_ENTIRE_DISPLAY_ON    0xA5
#define OLED_COMMAND_ENTIRE_DISPLAY_OFF   0xA4

#define OLED_SPI_MASTER_CONFIGURATION                                             \
    {                                                                             \
        SPI_FREQUENCY_FREQUENCY_M4,   /**< Serial clock frequency 4Mbps. */       \
        OLED_SPI_CLOCK_PIN_NUMBER,    /**< SCK pin. */                            \
        SPI_PIN_DISCONNECTED,         /**< MISO pin. */                           \
        OLED_SPI_MOSI_PIN_NUMBER,     /**< MOSI pin. */                           \
        SPI_PIN_DISCONNECTED,         /**< Slave select pin. */                   \
        APP_IRQ_PRIORITY_LOW,         /**< Interrupt priority LOW. */             \
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


static void oled_pin_config(void)
{
    nrf_gpio_cfg_output(OLED_VDD_ENABLE_PIN_NUMBER);
    nrf_gpio_cfg_output(OLED_VCC_ENABLE_PIN_NUMBER);
    nrf_gpio_cfg_output(OLED_RESET_PIN_NUMBER);
    nrf_gpio_cfg_output(OLED_CD_CTRL_PIN_NUMBER);

    nrf_gpio_pin_clear(OLED_VDD_ENABLE_PIN_NUMBER);
    nrf_gpio_pin_clear(OLED_VCC_ENABLE_PIN_NUMBER);
    nrf_gpio_pin_set(OLED_RESET_PIN_NUMBER);
    nrf_gpio_pin_clear(OLED_CD_CTRL_PIN_NUMBER);
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
    // Structure for SPI master configuration.
    spi_master_config_t spi_config = OLED_SPI_MASTER_CONFIGURATION;

    // Initialize SPI master.
    APP_ERROR_CHECK(spi_master_open(OLED_SPI_MASTER, &spi_config));

    // Register event handler for SPI master.
    spi_master_evt_handler_reg(OLED_SPI_MASTER, spi_master_event_handler);
}


static void oled_reset(void)
{
    nrf_gpio_pin_clear(OLED_RESET_PIN_NUMBER);
    nrf_delay_us(5);
    nrf_gpio_pin_set(OLED_RESET_PIN_NUMBER);
    nrf_delay_us(1);
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

    oled_spi_send_data(&command, sizeof(command));
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

    oled_spi_send_data(&command, sizeof(command));
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

    oled_spi_send_data(&command, sizeof(command));
}


void oled_init(void)
{
    oled_pin_config();
    oled_spi_init();
    oled_power_on_off(true);

    oled_cmd_entire_display(true);
}
