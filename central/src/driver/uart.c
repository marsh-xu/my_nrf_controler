
#include "nrf51.h"
#include "nrf_gpio.h"

#include "pin_config.h"

#include "uart.h"

#define BYTE_BITS                              8
#define BYTE_MASK                              0xFF

/**@brief Function for initializing the UART.
 *
 * @details This function will configure and activate the UART.
 *
 */
void uart_init(void)
{
    // Configure UART0 pins.
    nrf_gpio_cfg_output(31);
    nrf_gpio_cfg_input(31, NRF_GPIO_PIN_PULLUP);

    NRF_UART0->PSELTXD         = 31;
    NRF_UART0->PSELRXD         = 1;
    NRF_UART0->BAUDRATE        = UART_BAUDRATE_BAUDRATE_Baud19200;

    // Clean possible events from earlier operations.
    NRF_UART0->EVENTS_RXDRDY   = 0;
    NRF_UART0->EVENTS_TXDRDY   = 0;
    NRF_UART0->EVENTS_ERROR    = 0;

    // Activate UART.
    NRF_UART0->ENABLE          = UART_ENABLE_ENABLE_Enabled;
    NRF_UART0->INTENSET        = 0;
    NRF_UART0->TASKS_STARTTX   = 1;
    NRF_UART0->TASKS_STARTRX   = 1;
}


/**@brief Function for reporting 8-bits UART info.
 *
 * @param[in]   info   Info need to be reported.
 */
void uart_put_uint8(uint8_t info)
{
    NRF_UART0->TXD = info;

    while (NRF_UART0->EVENTS_TXDRDY != 1)
    {
        // Wait for TXD data to be sent.
    }

    NRF_UART0->EVENTS_TXDRDY = 0;
}

/**@brief Function for reporting 16-bits UART info.
 *
 * @param[in]   info   Info need to be reported.
 */
void uart_put_uint16(uint16_t info)
{
    // Report command status on the UART.
    // Transmit MSB of the result.
    uart_put_uint8((uint8_t)(info >> BYTE_BITS) & BYTE_MASK);
    // Transmit LSB of the result.
    uart_put_uint8((uint8_t)info & BYTE_MASK);
}


void uart_put_uint32(uint32_t info)
{
    // Report command status on the UART.
    // Transmit MSB of the result.
    uart_put_uint16((uint16_t)(info >> 16) & 0xFFFF);
    // Transmit LSB of the result.
    uart_put_uint16((uint16_t)info & 0xFFFF);
}

void uart_put_string(uint8_t* info)
{
    uint8_t *p;
    p = info;
    while((*p) != '\0')
    {
        uart_put_uint8(*p);
        p++;
    }
}
