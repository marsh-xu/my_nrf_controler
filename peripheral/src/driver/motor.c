#include "app_timer.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

#include "pin_config.h"

#include "motor.h"

#define MOTOR_CONTROLTIMER_INTERVAL APP_TIMER_TICKS(1, APP_TIMER_PRESCALER)

#define MOTOR_NUMBER            8

static motor_index_t        m_motor_control_index = MOTOR_INDEX_1;
static app_timer_id_t       m_motor_control_timer_id;

uint8_t                     m_duty_cycle = 0;

uint8_t motor_enable_pin[MOTOR_NUMBER] =
{
    MOTOR_1_ENABLE_PIN_NUMBER,
    MOTOR_2_ENABLE_PIN_NUMBER,
    MOTOR_3_ENABLE_PIN_NUMBER,
    MOTOR_4_ENABLE_PIN_NUMBER,
    MOTOR_5_ENABLE_PIN_NUMBER,
    MOTOR_6_ENABLE_PIN_NUMBER,
    MOTOR_7_ENABLE_PIN_NUMBER,
    MOTOR_8_ENABLE_PIN_NUMBER,
};

static void motor_control_timeout_handler(void * p_context)
{
    if (m_duty_cycle == 0)
    {
        nrf_gpio_pin_clear(motor_enable_pin[m_motor_control_index - MOTOR_INDEX_1]);
    }
    else if (m_duty_cycle < 100)
    {
        nrf_gpio_pin_set(motor_enable_pin[m_motor_control_index - MOTOR_INDEX_1]);
        nrf_delay_us(10 * m_duty_cycle);
        nrf_gpio_pin_clear(motor_enable_pin[m_motor_control_index - MOTOR_INDEX_1]);
    }
    else
    {
        nrf_gpio_pin_set(motor_enable_pin[m_motor_control_index - MOTOR_INDEX_1]);
    }
}


/**@brief Stop vibrator sequencer timer.
 */
static void stop_motor_control_timer(void)
{
    uint32_t err_code;
    err_code = app_timer_stop(m_motor_control_timer_id);
    APP_ERROR_CHECK(err_code);
}


static void start_motor_control_timer(void)
{
    uint32_t err_code;

    err_code = app_timer_start(m_motor_control_timer_id, MOTOR_CONTROLTIMER_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}


static void create_motor_control_timer(void)
{
    uint32_t err_code;

    err_code = app_timer_create(&m_motor_control_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                motor_control_timeout_handler);
    APP_ERROR_CHECK(err_code);
}

void motor_init(void)
{
#if 0
    uint8_t pin = LED_PIN_NUMBER;
    nrf_gpio_cfg_output(pin);
    while(1)
    {
        nrf_gpio_pin_set(pin);
        nrf_delay_ms(2000);
        nrf_gpio_pin_clear(pin);
        nrf_delay_ms(2000);
    }
#endif
    for (uint8_t i = 0; i < MOTOR_NUMBER; i++)
    {
        nrf_gpio_cfg_output(motor_enable_pin[i]);
        nrf_gpio_pin_clear(motor_enable_pin[i]);
    }

    nrf_gpio_cfg_output(MOTOR_IN1_PIN_NUMBER);
    nrf_gpio_pin_clear(MOTOR_IN1_PIN_NUMBER);

    nrf_gpio_cfg_output(MOTOR_IN2_PIN_NUMBER);
    nrf_gpio_pin_clear(MOTOR_IN2_PIN_NUMBER);

    create_motor_control_timer();
}

void motor_on(motor_control_t motor_control)
{
    for (uint8_t i = 0; i < MOTOR_NUMBER; i++)
    {
        nrf_gpio_pin_clear(motor_enable_pin[i]);
    }

    m_motor_control_index = motor_control.motor_index;

    if (motor_control.motor_direction == MOTOR_DIRECTION_CLOCK)
    {
        nrf_gpio_pin_set(MOTOR_IN1_PIN_NUMBER);
        nrf_gpio_pin_clear(MOTOR_IN2_PIN_NUMBER);
    }
    else
    {
        nrf_gpio_pin_set(MOTOR_IN2_PIN_NUMBER);
        nrf_gpio_pin_clear(MOTOR_IN1_PIN_NUMBER);
    }

    start_motor_control_timer();
}

void motor_off()
{
    for (uint8_t i = 0; i < MOTOR_NUMBER; i++)
    {
        nrf_gpio_pin_clear(motor_enable_pin[i]);
    }
    stop_motor_control_timer();
}

void motor_set_duty_cylce(uint8_t duty_cycle)
{
    m_duty_cycle = duty_cycle;
}
