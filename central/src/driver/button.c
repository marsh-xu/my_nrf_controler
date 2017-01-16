#include "nrf_gpio.h"
#include "app_button.h"
#include "app_timer.h"

#include "ble_mhs_c.h"
#include "pin_config.h"
#include "uart.h"
#include "oled.h"

#include "SEGGER_RTT.h"

#include "button.h"

#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)
#define MOTOR_NUM                       4

static uint8_t m_motor_speed = 0;
static uint8_t m_motor_index = 0;
static bool is_setting_motor_control = false;

void button_up_event()
{
    uint8_t cmd[3] = {0};
    m_motor_speed += 10;
    if (m_motor_speed > 100)
    {
        m_motor_speed = 100;
    }
    
    cmd[0] = MHS_CMD_CODE_SET_MOTOR_SPEED;
    cmd[1] = m_motor_speed;
    ble_mhs_c_send_cmd(cmd, sizeof(cmd));
}

void button_down_event()
{
    uint8_t cmd[3] = {0};
    m_motor_speed -= 10;
    if (m_motor_speed < 0)
    {
        m_motor_speed = 0;
    }

    cmd[0] = MHS_CMD_CODE_SET_MOTOR_SPEED;
    cmd[1] = m_motor_speed;
    ble_mhs_c_send_cmd(cmd, sizeof(cmd));
}

void button_left_event()
{
    m_motor_index --;
    m_motor_index = m_motor_index % MOTOR_NUM;
}

void button_right_event()
{
    m_motor_index ++;
    m_motor_index = m_motor_index % MOTOR_NUM;
}

void motor_off(void)
{
    if (is_setting_motor_control == true)
    {
        uint8_t cmd = 0x07;
        ble_mhs_c_send_cmd(&cmd, sizeof(cmd));
    }
}

void button_clock_event(void)
{
    if (is_setting_motor_control == false)
    {
        uint8_t cmd[3] = {0};
        cmd[0] = MHS_CMD_CODE_SET_MOTOR_CONTROL;
        cmd[1] = 0x00;
        cmd[2] = m_motor_index;
        ble_mhs_c_send_cmd(cmd, sizeof(cmd));
        is_setting_motor_control = true;
    }
    else
    {
        motor_off();
        is_setting_motor_control = false;
    }
}

void button_unclock_event(void)
{
    if (is_setting_motor_control == false)
    {
        uint8_t cmd[3] = {0};
        cmd[0] = MHS_CMD_CODE_SET_MOTOR_CONTROL;
        cmd[1] = 0x01;
        cmd[2] = m_motor_index;
        ble_mhs_c_send_cmd(cmd, sizeof(cmd));
        is_setting_motor_control = true;
    }
    else
    {
        motor_off();
        is_setting_motor_control = false;
    }
}


/**@brief Handle a button event.
 *
 * @details This function will be called when the state of button has changed.
 *
 * @param[in]   pin_no         Pin that had an event happen.
 * @param[in]   button_event   APP_BUTTON_PUSH or APP_BUTTON_RELEASE.
 */
static void button_event_handler(uint8_t pin_no, uint8_t button_event)
{
    SEGGER_RTT_printf(0, "pin = %p, event = %p\r\n", pin_no, button_event);
    if (button_event == APP_BUTTON_PUSH)
    {
        switch (pin_no)
        {
            case KEY_U_PIN_NUMBER:
            {
                button_up_event();
                break;
            }
            case KEY_D_PIN_NUMBER:
            {
                button_down_event();
                break;
            }
            case KEY_L_PIN_NUMBER:
            {
                button_left_event();
                break;
            }
            case KEY_R_PIN_NUMBER:
            {
                button_right_event();
                break;
            }
            case KEY_A_PIN_NUMBER:
            {
                button_clock_event();
                break;
            }
            case KEY_B_PIN_NUMBER:
            {
                button_unclock_event();
                break;
            }
            default:
                APP_ERROR_HANDLER(pin_no);
                break;
        }
    }
    else if (button_event == APP_BUTTON_RELEASE)
    {
        switch (pin_no)
        {
            case KEY_U_PIN_NUMBER:
                break;
            case KEY_D_PIN_NUMBER:
                break;
            case KEY_L_PIN_NUMBER:
                break;
            case KEY_R_PIN_NUMBER:
                break;
            case KEY_A_PIN_NUMBER:
                break;
            case KEY_B_PIN_NUMBER:
                break;
            default:
                APP_ERROR_HANDLER(pin_no);
                break;
        }
    }
}


void button_init(void)
{
    // Note: Array must be static because a pointer to it will be saved in the Button handler
    //       module.
    static app_button_cfg_t buttons[] =
    {
        {KEY_U_PIN_NUMBER, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_NOPULL, button_event_handler},
        {KEY_D_PIN_NUMBER, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_NOPULL, button_event_handler},
        {KEY_L_PIN_NUMBER, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_NOPULL, button_event_handler},
        {KEY_R_PIN_NUMBER, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_NOPULL, button_event_handler},
        {KEY_A_PIN_NUMBER, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_NOPULL, button_event_handler},
        {KEY_B_PIN_NUMBER, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_NOPULL, button_event_handler},
    };

    app_button_init(buttons, sizeof(buttons) / sizeof(buttons[0]), BUTTON_DETECTION_DELAY);
    app_button_enable();
}

void mhs_c_notification(uint8_t evt_type, uint16_t evt_data)
{
}
