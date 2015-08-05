/*
 * Copyright (c) 2014 Sony Mobile Communications Inc.
 * All rights, including trade secret rights, reserved.
 */

#include "nrf_gpio.h"

#include "system_init.h"

void system_init(void)
{
    nrf_gpio_cfg_output(7);
    nrf_gpio_pin_set(7);
}
