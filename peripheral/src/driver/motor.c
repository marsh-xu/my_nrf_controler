#include <string.h>

#include "app_timer.h"
#include "ble_mhs.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

#include "pin_config.h"

#include "motor.h"

#define MOTOR_CONTROLTIMER_INTERVAL APP_TIMER_TICKS(1, APP_TIMER_PRESCALER)

#define MOTOR_NUMBER            5

static motor_index_t        m_motor_control_index = MOTOR_INDEX_1;
static app_timer_id_t       m_motor_control_timer_id;

#define                     DC_MOTOR_ARRAY_SIZE   6
#define                     DC_MOTOR_INDEX           (MOTOR_NUMBER-1)
uint8_t                     m_dc_motor_array_index = 0;
bool                        m_dc_motor_direction = false;
uint8_t                     m_dc_motor_array[DC_MOTOR_ARRAY_SIZE] = {5,4,6,2,3,1};

uint8_t                     m_duty_cycle = 80;

uint8_t motor_enable_pin[MOTOR_NUMBER] =
{
    MOTOR_1_ENABLE_PIN_NUMBER,
    MOTOR_2_ENABLE_PIN_NUMBER,
    MOTOR_3_ENABLE_PIN_NUMBER,
    MOTOR_4_ENABLE_PIN_NUMBER,
    MOTOR_5_ENABLE_PIN_NUMBER,
};

uint8_t motor_led_pin[MOTOR_NUMBER] =
{
    LED1_PIN,
    LED2_PIN,
    LED3_PIN,
    LED4_PIN,
    LED5_PIN,
};

static void motor_control_timeout_handler(void * p_context)
{
    if (m_motor_control_index != DC_MOTOR_INDEX)
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
    else
    {
        uint8_t motor_value = 0;

        if (m_dc_motor_direction == true)
        {
            m_dc_motor_array_index = m_dc_motor_array_index + 1;
        }
        else
        {
            m_dc_motor_array_index = m_dc_motor_array_index + DC_MOTOR_ARRAY_SIZE - 1;
        }
        if (m_dc_motor_array_index >= DC_MOTOR_ARRAY_SIZE)
        {
            m_dc_motor_array_index -= DC_MOTOR_ARRAY_SIZE;
        }

        motor_value = m_dc_motor_array[m_dc_motor_array_index];

        if ((motor_value & 0x4) != 0 )
        {
            nrf_gpio_pin_clear(BT_PWM_AH);
            nrf_gpio_pin_set(BT_PWM_AL);
        }
        else
        {
            nrf_gpio_pin_clear(BT_PWM_AL);
            nrf_gpio_pin_set(BT_PWM_AH);
        }

        if ((motor_value & 0x2) != 0 )
        {
            nrf_gpio_pin_clear(BT_PWM_BH);
            nrf_gpio_pin_set(BT_PWM_BL);
        }
        else
        {
            nrf_gpio_pin_clear(BT_PWM_BL);
            nrf_gpio_pin_set(BT_PWM_BH);
        }

        if ((motor_value & 0x1) != 0 )
        {
            nrf_gpio_pin_clear(BT_PWM_CH);
            nrf_gpio_pin_set(BT_PWM_CL);
        }
        else
        {
            nrf_gpio_pin_clear(BT_PWM_CL);
            nrf_gpio_pin_set(BT_PWM_CH);
        }
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
    for (uint8_t i = 0; i < MOTOR_NUMBER; i++)
    {
        nrf_gpio_cfg_output(motor_enable_pin[i]);
        nrf_gpio_pin_clear(motor_enable_pin[i]);
        nrf_gpio_cfg_output(motor_led_pin[i]);
        nrf_gpio_pin_clear(motor_led_pin[i]);
    }

    nrf_gpio_cfg_output(MOTOR_IN1_PIN_NUMBER);
    nrf_gpio_pin_clear(MOTOR_IN1_PIN_NUMBER);

    nrf_gpio_cfg_output(MOTOR_IN2_PIN_NUMBER);
    nrf_gpio_pin_clear(MOTOR_IN2_PIN_NUMBER);

    nrf_gpio_cfg_output(BT_PWM_AH);
    nrf_gpio_cfg_output(BT_PWM_AL);
    nrf_gpio_cfg_output(BT_PWM_BH);
    nrf_gpio_cfg_output(BT_PWM_BL);
    nrf_gpio_cfg_output(BT_PWM_CH);
    nrf_gpio_cfg_output(BT_PWM_CL);
    nrf_gpio_pin_clear(BT_PWM_AH);
    nrf_gpio_pin_clear(BT_PWM_AL);
    nrf_gpio_pin_clear(BT_PWM_BH);
    nrf_gpio_pin_clear(BT_PWM_BL);
    nrf_gpio_pin_clear(BT_PWM_CH);
    nrf_gpio_pin_clear(BT_PWM_CL);

    create_motor_control_timer();
}

void motor_on(motor_control_t motor_control)
{
    for (uint8_t i = 0; i < MOTOR_NUMBER; i++)
    {
        nrf_gpio_pin_clear(motor_enable_pin[i]);
    }

    m_motor_control_index = motor_control.motor_index;

    nrf_gpio_pin_set(motor_led_pin[m_motor_control_index]);

    if (m_motor_control_index > MOTOR_NUMBER)
    {
        APP_ERROR_CHECK_BOOL(false);
    }

    if (m_motor_control_index != DC_MOTOR_INDEX)
    {
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
    }
    else
    {
        if (motor_control.motor_direction == MOTOR_DIRECTION_CLOCK)
        {
            m_dc_motor_direction = true;
        }
        else
        {
            m_dc_motor_direction = false;
        }
    }

    start_motor_control_timer();
}

void motor_off()
{
    for (uint8_t i = 0; i < MOTOR_NUMBER; i++)
    {
        nrf_gpio_pin_clear(motor_enable_pin[i]);
        nrf_gpio_pin_clear(motor_led_pin[i]);
    }

    stop_motor_control_timer();
}

void motor_set_duty_cylce(uint8_t duty_cycle)
{
    m_duty_cycle = duty_cycle;
}


void report_motor_duty_cycle(void)
{
    uint32_t err_code;
    mhs_event_t event;

    memset(&event, 0, sizeof(mhs_event_t));
    event.evt_code       = MHS_EVENT_CODE_MOTOR_SPEED;
    event.evt_value.buff = (uint8_t *)&m_duty_cycle;
    event.evt_value.len  = sizeof(m_duty_cycle);

    err_code = mhs_event_characteristic_notify(event);

    APP_ERROR_CHECK(err_code);
}
