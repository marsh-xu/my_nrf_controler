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
#define UI_TOTAL_NUM                    6

static oled_ui_style_t ui_index = 0;

static bool is_setting_temp_threshold = false;
static bool is_setting_motor_speed = false;
static bool is_setting_motor_control = false;

static uint8_t m_temp_threshold = 0;
static uint8_t m_motor_speed = 0;
static uint8_t m_motor_index = 0;

void button_up_event()
{
    if (is_setting_temp_threshold == true)
    {
        m_temp_threshold += 10;
        if (m_temp_threshold > 100)
        {
            m_temp_threshold = 0;
        }
        oled_show_num(m_temp_threshold);
    }
    else if (is_setting_motor_speed == true)
    {
        m_motor_speed += 10;
        if (m_motor_speed > 100)
        {
            m_motor_speed = 0;
        }
        oled_show_num(m_motor_speed);
    }
    else if (is_setting_motor_control == true)
    {
        m_motor_index ++;
        m_motor_index = m_motor_index % 8;
        oled_show_num(m_motor_index);
    }
    else
    {
        ui_index ++;
        ui_index = ui_index % UI_STYLE_TOTAL_NUM;
        ui_up_update(ui_index);
    }
}

void button_ok_event(void)
{
    switch (ui_index)
    {
        case UI_STYLE_GET_TEMPERATURE:
        {
            uint8_t cmd = MHS_CMD_CODE_GET_TEMPERATURE;
            ble_mhs_c_send_cmd(&cmd, sizeof(cmd));
            oled_clear_num();
            break;
        }
        case UI_STYLE_GET_TEMP_THRESHOLD:
        {
            uint8_t cmd = MHS_CMD_CODE_GET_TEMP_THRESHOLD;
            ble_mhs_c_send_cmd(&cmd, sizeof(cmd));
            oled_clear_num();
            break;
        }
        case UI_STYLE_GET_MOTOR_SPEED:
        {
            uint8_t cmd = MHS_CMD_CODE_GET_MOTOR_SPEED;
            ble_mhs_c_send_cmd(&cmd, sizeof(cmd));
            oled_clear_num();
            break;
        }
        case UI_STYLE_SET_TEMP_THRESHOLD:
        {
            if (is_setting_temp_threshold == false)
            {
                is_setting_temp_threshold = true;
                oled_show_choose_status(true);
                oled_show_num(m_temp_threshold);
            }
            else
            {
                uint8_t cmd[3] = {0};
                cmd[0] = MHS_CMD_CODE_SET_TEMP_THRESHOLD;
                cmd[1] = m_temp_threshold;
                ble_mhs_c_send_cmd(cmd, sizeof(cmd));
                is_setting_temp_threshold = false;
                oled_show_choose_status(false);
                oled_clear_num();
            }
            break;
        }
        case UI_STYLE_SET_MOTOR_SPEED:
        {
            if (is_setting_motor_speed == false)
            {
                is_setting_motor_speed = true;
                oled_show_choose_status(true);
                oled_show_num(m_motor_speed);
            }
            else
            {
                uint8_t cmd[3] = {0};
                cmd[0] = MHS_CMD_CODE_SET_MOTOR_SPEED;
                cmd[1] = m_motor_speed;
                ble_mhs_c_send_cmd(cmd, sizeof(cmd));
                is_setting_motor_speed = false;
                oled_show_choose_status(false);
                oled_clear_num();
            }
            break;
        }
        case UI_STYLE_SET_MOTOR_CONTROL:
        {
            if (is_setting_motor_control == false)
            {
                is_setting_motor_control = true;
                oled_show_choose_status(true);
                oled_show_num(m_motor_index);
            }
            else
            {
                is_setting_motor_control = false;
                oled_show_choose_status(false);
                oled_clear_num();
            }
            break;
        }
        default:
            break;
    }
}

void button_right_event(void)
{
    if (is_setting_motor_control == true)
    {
        uint8_t cmd[3] = {0};
        cmd[0] = MHS_CMD_CODE_SET_MOTOR_CONTROL;
        cmd[1] = 0x00;
        cmd[2] = m_motor_index;
        ble_mhs_c_send_cmd(cmd, sizeof(cmd));
    }
}

void button_left_event(void)
{
    if (is_setting_motor_control == true)
    {
        uint8_t cmd[3] = {0};
        cmd[0] = MHS_CMD_CODE_SET_MOTOR_CONTROL;
        cmd[1] = 0x01;
        cmd[2] = m_motor_index;
        ble_mhs_c_send_cmd(cmd, sizeof(cmd));
    }
}

void motor_off(void)
{
    if (is_setting_motor_control == true)
    {
        uint8_t cmd = 0x07;
        ble_mhs_c_send_cmd(&cmd, sizeof(cmd));
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
            case KEY1_PIN_NUMBER:
            {
                button_ok_event();
                break;
            }
            case KEY2_PIN_NUMBER:
            {
                button_up_event();
                break;
            }
            case KEY3_PIN_NUMBER:
            {
                button_left_event();
                break;
            }
            case KEY4_PIN_NUMBER:
                break;
            case KEY5_PIN_NUMBER:
            {
                button_right_event();
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
            case KEY1_PIN_NUMBER:
                break;
            case KEY2_PIN_NUMBER:
                break;
            case KEY3_PIN_NUMBER:
            {
                motor_off();
                break;
            }
            case KEY4_PIN_NUMBER:
                break;
            case KEY5_PIN_NUMBER:
            {
                motor_off();
                break;
            }
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
        {KEY1_PIN_NUMBER, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_NOPULL, button_event_handler},
        {KEY2_PIN_NUMBER, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_NOPULL, button_event_handler},
        {KEY3_PIN_NUMBER, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_NOPULL, button_event_handler},
        {KEY4_PIN_NUMBER, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_NOPULL, button_event_handler},
        {KEY5_PIN_NUMBER, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_NOPULL, button_event_handler},
    };

    app_button_init(buttons, sizeof(buttons) / sizeof(buttons[0]), BUTTON_DETECTION_DELAY);
    app_button_enable();
}

void mhs_c_notification(uint8_t evt_type, uint16_t evt_data)
{
    switch (evt_type)
    {
        case MHS_EVENT_CODE_CURRENT_TEMPERATURE:
        {
            uint16_t temp = evt_data;
            oled_show_num(temp);
            break;
        }
        case MHS_EVENT_CODE_TEMP_THRESHOLD:
        {
            m_temp_threshold = evt_data;
            oled_show_num(m_temp_threshold);
            break;
        }
        case MHS_EVENT_CODE_MOTOR_SPEED:
        {
            m_motor_speed = evt_data;
            oled_show_num(m_motor_speed);
            break;
        }
        default:
            break;
    }
}
