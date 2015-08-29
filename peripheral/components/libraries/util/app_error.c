/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 *
 * @defgroup app_error Common application error handler
 * @{
 * @ingroup app_common
 *
 * @brief Common application error handler.
 */

#include "nrf.h"
#include "app_error.h"
#include "compiler_abstraction.h"
#include "nordic_common.h"

#include "SEGGER_RTT.h"

/* global error variables - in order to prevent removal by optimizers */
uint32_t m_error_code;
uint32_t m_line_num;
const uint8_t * m_p_file_name;

/**@brief Function for error handling, which is called when an error has occurred.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of error.
 *
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the handler is called.
 * @param[in] p_file_name Pointer to the file name.
 *
 * Function is implemented as weak so that it can be overwritten by custom application error handler
 * when needed.
 */

/*lint -save -e14 */
__WEAK void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    SEGGER_RTT_printf(0, "Error code = 0x%x,line_num = %d, file = %s\r\n", error_code, line_num, p_file_name);
    NVIC_SystemReset();
}
/*lint -restore */
