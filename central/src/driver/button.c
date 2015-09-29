#include "nrf_gpio.h"
#include "app_button.h"
#include "app_timer.h"

#include "ble_mhs_c.h"
#include "pin_config.h"
#include "uart.h"

#include "SEGGER_RTT.h"

#include "button.h"

#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)

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
                break;
            case KEY2_PIN_NUMBER:
                break;
            case KEY3_PIN_NUMBER:
                break;
            case KEY4_PIN_NUMBER:
                break;
            case KEY5_PIN_NUMBER:
                break;
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
                break;
            case KEY4_PIN_NUMBER:
                break;
            case KEY5_PIN_NUMBER:
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
        {KEY1_PIN_NUMBER, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_NOPULL, button_event_handler},
        {KEY2_PIN_NUMBER, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_NOPULL, button_event_handler},
        {KEY3_PIN_NUMBER, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_NOPULL, button_event_handler},
        {KEY4_PIN_NUMBER, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_NOPULL, button_event_handler},
        {KEY5_PIN_NUMBER, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_NOPULL, button_event_handler},
    };

    app_button_init(buttons, sizeof(buttons) / sizeof(buttons[0]), BUTTON_DETECTION_DELAY);
    app_button_enable();
}

