/*
 * Copyright (c) 2014 Sony Mobile Communications Inc.
 * All rights, including trade secret rights, reserved.
 */

/**
 * @file
 *
 * @brief    MHS Protocol module.
 *
 * @details  This module implements the MHS Protocol. During initialization
 *           the module adds the MHS Service and its characteristics to the
 *           BLE stack database. MHS events received from the BLE stack will
 *           be passed on to the on_mhs_evt function in the mhs_proxy.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <app_error.h>
#include <ble.h>
#include <ble_gatts.h>
#include <ble_srv_common.h>
#include <nordic_common.h>

#include "mhs_proxy.h"
#include "system_error.h"

#include "ble_mhs.h"

#define MHS_UUID_BASE   {0x1B, 0xC5, 0xD5, 0xA5, 0x02, 0x00, 0x82, 0x86,\
        0xE3, 0x11, 0xCB, 0x37, 0x00, 0x00, 0x00, 0x00}

// Control point structure instance.
static mhs_control_point_cmd_t  m_mhs_control_point;

/**@brief Connect event handler.
 *
 * @param[in]   p_mhs       Sony Advanced Accessory Host Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_mhs_t *p_mhs, ble_evt_t *p_ble_evt)
{
    p_mhs->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Disconnect event handler.
 *
 * @param[in]   p_mhs       Sony Advanced Accessory Host Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_mhs_t *p_mhs, ble_evt_t *p_ble_evt)
{
    p_mhs->conn_handle = BLE_CONN_HANDLE_INVALID;
}


/**@brief MHS service structure initialization.
 *
 * @param[out]  p_mhs       MHS structure. This structure will have to be supplied
 *                          by the application. It will be initialized by this function and will
 *                          later be used to identify this particular service instance.
 * @param[in]   p_mhs_init  Information needed to initialize the service.
 *
 * @return NRF_SUCCESS on successful initialization of service structure, otherwise an error code.
 */
static uint32_t mhs_service_init(ble_mhs_t *p_mhs, const ble_mhs_init_t *p_mhs_init)
{
    uint32_t   err_code;
    ble_uuid_t ble_uuid;
    ble_uuid128_t base_uuid  =
    {
        .uuid128 = MHS_UUID_BASE,
    };

    // Initialize service structure
    p_mhs->evt_handler               = p_mhs_init->evt_handler;
    p_mhs->conn_handle               = BLE_CONN_HANDLE_INVALID;

    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_mhs->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    ble_uuid.type = p_mhs->uuid_type;
    ble_uuid.uuid = BLE_UUID_MHS_SERVICE;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid, &p_mhs->service_handle);

    return err_code;
}


uint32_t mhs_control_point_characteristic_add(ble_mhs_t *p_mhs)
{
    ble_uuid_t          ble_uuid;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_gatts_attr_md_t attr_md;

    if (NULL == p_mhs)
    {
        return NRF_ERROR_NULL;
    }

    ble_uuid.type = p_mhs->uuid_type;
    ble_uuid.uuid = BLE_UUID_MHS_CONTROL_POINT_CHARACTERISTIC;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.write  = 1;
    char_md.char_props.read   = 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;
    char_md.p_sccd_md         = NULL;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc       = BLE_GATTS_VLOC_USER;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(m_mhs_control_point);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = sizeof(m_mhs_control_point);
    attr_char_value.p_value      = (uint8_t *)&m_mhs_control_point;

    return sd_ble_gatts_characteristic_add(p_mhs->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_mhs->control_point_handles);
}


uint32_t mhs_event_characteristic_add(ble_mhs_t *p_mhs)
{
    ble_uuid_t          ble_uuid;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_gatts_attr_md_t attr_md;

    if (NULL == p_mhs)
    {
        return NRF_ERROR_NULL;
    }

    ble_uuid.type = p_mhs->uuid_type;
    ble_uuid.uuid = BLE_UUID_MHS_EVENT_CHARACTERISTIC;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);

    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = 1;
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = MHS_EVENT_MAX_TX_CHAR_LEN;

    return sd_ble_gatts_characteristic_add(p_mhs->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_mhs->event_handles);
}


uint32_t ble_mhs_init(ble_mhs_t *p_mhs, const ble_mhs_init_t *p_mhs_init)
{
    uint32_t   err_code;

    if (NULL == p_mhs)
    {
        return NRF_ERROR_NULL;
    }

    // Initializing the MHS service structure.
    err_code = mhs_service_init(p_mhs, p_mhs_init);

    // Add control point characteristic.
    if (err_code == NRF_SUCCESS)
    {
        err_code = mhs_control_point_characteristic_add(p_mhs);
    }

    // Add event characteristic.
    if (err_code == NRF_SUCCESS)
    {
        err_code = mhs_event_characteristic_add(p_mhs);
    }

    return err_code;
}


uint32_t on_write_for_control_point_characteristic(ble_mhs_t *p_mhs, ble_evt_t *p_ble_evt)
{
    uint32_t error_code = NRF_SUCCESS;
    ble_mhs_evt_t evt;
    ble_gatts_evt_write_t *p_evt_write;

    if ((p_mhs == NULL) || (p_ble_evt == NULL) || (p_mhs->evt_handler == NULL))
    {
        return NRF_ERROR_NULL;
    }

    p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if ((CTRL_POINT_CHAR_CMD_CODE_LEN == p_evt_write->len)
            || (CTRL_POINT_CHAR_CMD_CODE_AND_VALUE_LEN == p_evt_write->len))
    {
        memset(&evt, 0, sizeof(ble_mhs_evt_t));
        evt.ble_mhs_char = MHS_CHARACTERISTIC_CONTROL_POINT;

        switch (m_mhs_control_point.cmd_code)
        {
            case MHS_CMD_CODE_GET_TEMPERATURE:
                evt.evt_type.control_char_evt = BLE_MHS_CONTROL_CHAR_EVT_GET_TEMPERATURE;
                break;
            case MHS_CMD_CODE_GET_TEMP_THRESHOLD:
                evt.evt_type.control_char_evt = BLE_MHS_CONTROL_CHAR_EVT_GET_TEMP_SHRESHOLD;
                break;
            case MHS_CMD_CODE_GET_MOTOR_SPEED:
                evt.evt_type.control_char_evt = BLE_MHS_CONTROL_CHAR_EVT_GET_MOTOR_SPEED;
                break;
            case MHS_CMD_CODE_SET_TEMP_THRESHOLD:
                evt.evt_type.control_char_evt = BLE_MHS_CONTROL_CHAR_EVT_SET_TEMP_SHRESHOLD;
                evt.evt_params.p_event_data = (uint8_t *)&m_mhs_control_point.cmd_value;
                evt.event_data_len = sizeof(m_mhs_control_point.cmd_value);
                break;
            case MHS_CMD_CODE_SET_MOTOR_CONTROL:
                evt.evt_type.control_char_evt = BLE_MHS_CONTROL_CHAR_EVT_SET_MOTOR_CONTROL;
                evt.evt_params.p_event_data = (uint8_t *)&m_mhs_control_point.cmd_value;
                evt.event_data_len = sizeof(m_mhs_control_point.cmd_value);
                break;
            case MHS_CMD_CODE_SET_MOTOR_SPEED:
                evt.evt_type.control_char_evt = BLE_MHS_CONTROL_CHAR_EVT_SET_MOTOR_SPEED;
                evt.evt_params.p_event_data = (uint8_t *)&m_mhs_control_point.cmd_value;
                evt.event_data_len = sizeof(m_mhs_control_point.cmd_value);
                break;
            case MHS_CMD_CODE_SET_MOTOR_OFF:
                evt.evt_type.control_char_evt = BLE_MHS_CONTROL_CHAR_EVT_SET_MOTOR_OFF;
                break;
            case MHS_CMD_CODE_SET_MUSIC_CONTROL:
                evt.evt_type.control_char_evt = BLE_MHS_CONTROL_CHAR_EVT_SET_MUSIC_CONTROL;
                evt.evt_params.p_event_data = (uint8_t *)&m_mhs_control_point.cmd_value;
                evt.event_data_len = sizeof(m_mhs_control_point.cmd_value);
                break;
            default:
                return NRF_ERROR_INVALID_PARAM;
        }
        p_mhs->evt_handler(p_mhs, &evt);
    }
    else
    {
        error_code = NRF_ERROR_INTERNAL;
    }

    return error_code;
}


uint32_t on_write_for_event_characteristic(ble_mhs_t *p_mhs, ble_evt_t *p_ble_evt)
{
    uint32_t error_code = NRF_SUCCESS;
    ble_mhs_evt_t evt;
    ble_gatts_evt_write_t *p_evt_write;

    if ((p_mhs == NULL) || (p_ble_evt == NULL))
    {
        return NRF_ERROR_NULL;
    }

    p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if (EVT_NOTIFICATION_WRITE_LEN == p_evt_write->len)
    {
        memset(&evt, 0, sizeof(ble_mhs_evt_t));

        evt.ble_mhs_char = MHS_CHARACTERISTIC_EVENT;

        if (ble_srv_is_notification_enabled(p_evt_write->data))
        {
            evt.evt_type.event_char_evt = BLE_MHS_EVENT_CHAR_EVT_ENABLED;
        }
        else
        {
            evt.evt_type.event_char_evt = BLE_MHS_EVENT_CHAR_EVT_DISABLED;
        }

        if (p_mhs->evt_handler != NULL)
        {
            p_mhs->evt_handler(p_mhs, &evt);
        }
        else
        {
            error_code = NRF_ERROR_INVALID_PARAM;
        }
    }
    else
    {
        error_code = NRF_ERROR_INTERNAL;
    }

    return error_code;
}



void ble_mhs_on_ble_evt(ble_mhs_t *p_mhs, ble_evt_t *p_ble_evt)
{
    if ((p_mhs == NULL) || (p_ble_evt == NULL))
    {
        APP_ERROR_CHECK(NRF_ERROR_NULL);
    }
    else
    {
        switch (p_ble_evt->header.evt_id)
        {
            case BLE_GAP_EVT_CONNECTED:
                on_connect(p_mhs, p_ble_evt);
                break;

            case BLE_GAP_EVT_DISCONNECTED:
                on_disconnect(p_mhs, p_ble_evt);
                break;

            case BLE_GATTS_EVT_WRITE:
            {
                ble_gatts_evt_write_t *p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

                if (p_evt_write->handle == p_mhs->event_handles.cccd_handle)
                {
                    APP_ERROR_CHECK(on_write_for_event_characteristic(p_mhs, p_ble_evt));
                }
                else if (p_evt_write->handle == p_mhs->control_point_handles.value_handle)
                {
                    APP_ERROR_CHECK(on_write_for_control_point_characteristic(p_mhs, p_ble_evt));
                }
                else
                {
                    // This event is not relevant to mode characteristic.
                    // No implementation needed.
                }
            }
            break;

            case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
            {
                ble_gatts_evt_rw_authorize_request_t *p_evt_rw_authorize;

                p_evt_rw_authorize = &p_ble_evt->evt.gatts_evt.params.authorize_request;
                if (p_evt_rw_authorize->type == BLE_GATTS_AUTHORIZE_TYPE_READ)
                {
                    // Check whether this read authorization event is for mode characteristic.
                }
                else
                {
                    // This event is not a read event.
                    // No implementation needed.
                }
            }
            break;

            default:
                // No implementation needed.
                break;
        }
    }
}


uint32_t mhs_event_characteristic_notify(mhs_event_t event)
{
    ble_mhs_t *p_mhs = get_mhs_obj();
    uint16_t len = 0;
    uint8_t  data_buff[MHS_EVENT_MAX_TX_CHAR_LEN] = {0};
    ble_gatts_hvx_params_t hvx_params;

    if (NULL == p_mhs)
    {
        return NRF_ERROR_NULL;
    }

    if (BLE_CONN_HANDLE_INVALID == p_mhs->conn_handle)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    memcpy(data_buff, &event.evt_code, sizeof(mhs_event_code_t));
    len = sizeof(mhs_event_code_t);

    if (event.evt_value.len > MHS_EVENT_MAX_TX_CHAR_LEN)
    {
        return APP_ERROR_INVALID_LENGTH;
    }

    if (0 != event.evt_value.len)
    {
        if (NULL != event.evt_value.buff)
        {
            memcpy(&data_buff[len], event.evt_value.buff, event.evt_value.len);
            len += event.evt_value.len;
        }
        else
        {
            return APP_ERROR_NULL;
        }
    }
    else
    {
        // Only send the event code.
    }

    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle   = p_mhs->event_handles.value_handle;
    hvx_params.type     = BLE_GATT_HVX_NOTIFICATION;
    hvx_params.p_len    = &len;
    hvx_params.p_data   = data_buff;

    return sd_ble_gatts_hvx(p_mhs->conn_handle, &hvx_params);
}
