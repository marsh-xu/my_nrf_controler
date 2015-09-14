#ifndef UART_H_
#define UART_H_

#include <stdint.h>

/**@brief Function for initializing the UART.
 *
 * @details This function will configure and activate the UART.
 *
 */
void uart_init(void);

/**@brief Function for reporting 8-bits UART info.
 *
 * @param[in]   info   Info need to be reported.
 */
void uart_put_uint8(uint8_t info);

/**@brief Function for reporting 16-bits UART info.
 *
 * @param[in]   info   Info need to be reported.
 */
void uart_put_uint16(uint16_t info);

void uart_put_uint32(uint32_t info);

void uart_put_string(uint8_t* info);

#endif // UART_H_
