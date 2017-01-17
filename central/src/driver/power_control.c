#include "nrf_gpio.h"

#include "pin_config.h"

#include "power_control.h"

void power_control_init(void)
{
    //nrf_gpio_cfg_output(POWER_CONTROL);
    //nrf_gpio_pin_clear(POWER_CONTROL);
}

void power_control_enable(bool enable)
{
    //if (enable)
    //{
    //    nrf_gpio_pin_set(POWER_CONTROL);
    //}
    //else
    //{
    //    nrf_gpio_pin_clear(POWER_CONTROL);
    //}
}
