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
/** @example examples/ble_peripheral/ble_app_hrs/main.c
 *
 * @brief Heart Rate Service Sample Application main file.
 *
 * This file contains the source code for a sample application using the Heart Rate service
 * (and also Battery and Device Information services). This application uses the
 * @ref srvlib_conn_params module.
 */

#include <app_error.h>
#include <nrf_soc.h>
#include "nrf_delay.h"

#include "uart.h"

#include "system_init.h"
#include "power_control.h"

/**@brief Function for the Power manager.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for application main entry.
 */
int main(void)
{
    power_control_enable(true);

    uart_init();
    uart_put_uint8(0xAA);

    system_init();

    // Enter main loop.
    for (;;)
    {
        //power_manage();
    }
}


