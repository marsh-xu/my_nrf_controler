#include "app_timer.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

#include "pin_config.h"

#include "SEGGER_RTT.h"

#include "heat.h"

#define HEAT_WAIT_TIMER_INTERVAL APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER) // 30 seconds

static app_timer_id_t       m_heat_wait_timer_id;

static bool                 m_heat_on = false;

static void heat_wait_timeout_handler(void * p_context)
{
    SEGGER_RTT_printf(0, "Start Fan!\r\n");
    nrf_gpio_pin_set(FAN_CONTROL_PIN_NUMBER);
}


/**@brief Stop vibrator sequencer timer.
 */
static void stop_heat_wait_timer(void)
{
    SEGGER_RTT_printf(0, "Stop fan wait timer!\r\n");
    uint32_t err_code;
    err_code = app_timer_stop(m_heat_wait_timer_id);
    APP_ERROR_CHECK(err_code);
}


static void start_heat_wait_timer(void)
{
    uint32_t err_code;

    SEGGER_RTT_printf(0, "Start fan wait timer!\r\n");
    err_code = app_timer_start(m_heat_wait_timer_id, HEAT_WAIT_TIMER_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}


static void create_heat_wait_timer(void)
{
    uint32_t err_code;

    err_code = app_timer_create(&m_heat_wait_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                heat_wait_timeout_handler);
    APP_ERROR_CHECK(err_code);
}

void heat_control_init(void)
{
    nrf_gpio_cfg_output(HEAT_CONTROL_PIN_NUMBER);
    nrf_gpio_pin_clear(HEAT_CONTROL_PIN_NUMBER);
    nrf_gpio_cfg_output(FAN_CONTROL_PIN_NUMBER);
    nrf_gpio_pin_clear(FAN_CONTROL_PIN_NUMBER);
    create_heat_wait_timer();
}

void heat_on(void)
{
    SEGGER_RTT_printf(0, "Start heat!\r\n");
    nrf_gpio_pin_set(HEAT_CONTROL_PIN_NUMBER);
    start_heat_wait_timer();
    m_heat_on = true;
}

void heat_off(void)
{
    SEGGER_RTT_printf(0, "Stop heat!\r\n");
    stop_heat_wait_timer();
    nrf_gpio_pin_clear(HEAT_CONTROL_PIN_NUMBER);
    nrf_gpio_pin_clear(FAN_CONTROL_PIN_NUMBER);
    m_heat_on = false;
}

bool heat_is_on(void)
{
    return m_heat_on;
}
