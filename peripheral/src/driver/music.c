#include "app_error.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

#include "pin_config.h"

#include "music.h"

void music_control_init(void)
{
    nrf_gpio_cfg_output(MP3_PRV_PIN_NUMBER);
    nrf_gpio_pin_clear(MP3_PRV_PIN_NUMBER);

    nrf_gpio_cfg_output(MP3_NEXT_PIN_NUMBER);
    nrf_gpio_pin_clear(MP3_NEXT_PIN_NUMBER);

    nrf_gpio_cfg_output(MP3_VOL_P_PIN_NUMBER);
    nrf_gpio_pin_clear(MP3_VOL_P_PIN_NUMBER);

    nrf_gpio_cfg_output(MP3_VOL_N_PIN_NUMBER);
    nrf_gpio_pin_clear(MP3_VOL_N_PIN_NUMBER);

    nrf_gpio_cfg_output(MP3_PLAY_PIN_NUMBER);
    nrf_gpio_pin_clear(MP3_PLAY_PIN_NUMBER);
}

void music_control(music_control_cmd_t cmd)
{
    switch (cmd)
    {
        case MUSIC_PLAY_PAUSE:
            nrf_gpio_pin_set(MP3_PLAY_PIN_NUMBER);
            nrf_delay_ms(100);
            nrf_gpio_pin_clear(MP3_PLAY_PIN_NUMBER);
            break;
        case MUSIC_PREVIOUS_SONG:
            nrf_gpio_pin_set(MP3_PRV_PIN_NUMBER);
            nrf_delay_ms(100);
            nrf_gpio_pin_clear(MP3_PRV_PIN_NUMBER);
            break;
        case MUSIC_NEXT_SONG:
            nrf_gpio_pin_set(MP3_NEXT_PIN_NUMBER);
            nrf_delay_ms(100);
            nrf_gpio_pin_clear(MP3_NEXT_PIN_NUMBER);
            break;
        case MUSIC_VOL_PLUS:
            nrf_gpio_pin_set(MP3_VOL_P_PIN_NUMBER);
            nrf_delay_ms(100);
            nrf_gpio_pin_clear(MP3_VOL_P_PIN_NUMBER);
            break;
        case MUSIC_VOL_SUB:
            nrf_gpio_pin_set(MP3_VOL_N_PIN_NUMBER);
            nrf_delay_ms(100);
            nrf_gpio_pin_clear(MP3_VOL_N_PIN_NUMBER);
            break;
        default:
            APP_ERROR_CHECK_BOOL(false);
    }
}
