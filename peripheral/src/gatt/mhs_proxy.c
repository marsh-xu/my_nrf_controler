/*
 * Copyright (c) 2014 Sony Mobile Communications Inc.
 * All rights, including trade secret rights, reserved.
 */

#include <string.h>

#include <app_error.h>

#include "auto_temp.h"
#include "motor.h"
#include "music.h"

#include "SEGGER_RTT.h"

#include "mhs_proxy.h"

static ble_mhs_t m_mhs;    /**< Structure used to identify the MHS. */


/**@brief Handle event received from event characteristic.
 *
 * @param[in]   p_evt   Pointer to the received event.
 */
static uint32_t mhs_event_char_evt_handle(ble_mhs_evt_t *p_evt)
{
    uint32_t error_code = NRF_SUCCESS;

    if (NULL == p_evt)
    {
        error_code = NRF_ERROR_NULL;
    }
    else
    {
        switch (p_evt->evt_type.event_char_evt)
        {
            case BLE_MHS_EVENT_CHAR_EVT_ENABLED:
                break;
            case BLE_MHS_EVENT_CHAR_EVT_DISABLED:
                break;
            default:
                error_code = NRF_ERROR_INVALID_PARAM;
                break;
        }
    }

    return error_code;
}


/**@brief Handle event received from control point characteristic.
 *
 * @param[in]   p_evt   Pointer to the received event.
 */
static uint32_t mhs_control_char_evt_handle(ble_mhs_evt_t *p_evt)
{
    uint32_t error_code = NRF_SUCCESS;

    if (NULL == p_evt)
    {
        return NRF_ERROR_NULL;
    }

    switch (p_evt->evt_type.control_char_evt)
    {
        case BLE_MHS_CONTROL_CHAR_EVT_GET_TEMPERATURE:
            report_current_temperature();
            break;
        case BLE_MHS_CONTROL_CHAR_EVT_GET_TEMP_SHRESHOLD:
            report_temperature_threshold();
            break;
        case BLE_MHS_CONTROL_CHAR_EVT_GET_MOTOR_SPEED:
            break;
        case BLE_MHS_CONTROL_CHAR_EVT_SET_TEMP_SHRESHOLD:
        {
            int16_t temp_threshold = 0;
            memcpy((uint8_t*)&temp_threshold, (uint8_t*)p_evt->evt_params.p_event_data,
                    sizeof(int16_t));
            set_temperature_threshold(temp_threshold);
            break;
        }
        case BLE_MHS_CONTROL_CHAR_EVT_SET_MOTOR_CONTROL:
        {
            uint8_t duty_cycle = p_evt->evt_params.p_event_data[0];
            motor_set_duty_cylce(duty_cycle);
            //motor_on(MOTOR_INDEX_1, duty_cycle);
            break;
        }
        case BLE_MHS_CONTROL_CHAR_EVT_SET_MOTOR_SPEED:
        {
            motor_control_t motor_control;
            memcpy((uint8_t*)&motor_control, (uint8_t*)p_evt->evt_params.p_event_data,
                    sizeof(motor_control_t));
            motor_on(motor_control);
            break;
        }
        case BLE_MHS_CONTROL_CHAR_EVT_SET_MOTOR_OFF:
            motor_off();
            break;
        case BLE_MHS_CONTROL_CHAR_EVT_SET_MUSIC_CONTROL:
        {
            music_control_cmd_t music_cmd = p_evt->evt_params.p_event_data[0];
            SEGGER_RTT_printf(0, "music_cmd = %p\r\n", music_cmd);
            music_control(music_cmd);
            break;
        }
        default:
            error_code = NRF_ERROR_INVALID_PARAM;
            break;
    }

    return error_code;
}


/**@brief Handle event received from MHS protocol.
 *
 * @param[in]   p_mhs   MHS protocol structure.
 * @param[in]   p_evt   Pointer to the received event.
 */
static void on_mhs_evt(ble_mhs_t *p_mhs, ble_mhs_evt_t *p_evt)
{
    uint32_t error_code;

    if (NULL == p_mhs || NULL == p_evt)
    {
        error_code = NRF_ERROR_NULL;
    }
    else
    {
        // Please add your characteristic in the following "switch" statement.
        // If available characteristics are all updated, please remove this
        // information.
        switch (p_evt->ble_mhs_char)
        {
            case MHS_CHARACTERISTIC_EVENT:
                error_code = mhs_event_char_evt_handle(p_evt);
                break;
            case MHS_CHARACTERISTIC_CONTROL_POINT:
                error_code = mhs_control_char_evt_handle(p_evt);
                break;
            default:
                error_code = NRF_ERROR_INVALID_PARAM;
                break;
        }
    }
    APP_ERROR_CHECK(error_code);
}


void mhs_init(void)
{
    uint32_t        err_code;
    ble_mhs_init_t  mhs_init_obj;

    memset(&mhs_init_obj, 0, sizeof(mhs_init_obj));

    mhs_init_obj.evt_handler = on_mhs_evt;

    err_code = ble_mhs_init(&m_mhs, &mhs_init_obj);
    APP_ERROR_CHECK(err_code);
}


ble_mhs_t *get_mhs_obj(void)
{
    return &m_mhs;
}
