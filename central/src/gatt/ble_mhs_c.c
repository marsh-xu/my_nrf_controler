#include <stdio.h>

#include "app_error.h"
#include "nordic_common.h"
#include "uart.h"
#include "ble_db_discovery.h"
#include "mhs_c_proxy.h"

#include "ble_mhs_c.h"

#define TX_BUFFER_MASK         0x07                  /**< TX Buffer mask, must be a mask of continuous zeroes, followed by continuous sequence of ones: 000...111. */
#define TX_BUFFER_SIZE         (TX_BUFFER_MASK + 1)  /**< Size of send buffer, which is 1 higher than the mask. */

#define WRITE_MESSAGE_LENGTH   BLE_CCCD_VALUE_LEN    /**< Length of the write message for CCCD. */
#define WRITE_MESSAGE_LENGTH   BLE_CCCD_VALUE_LEN    /**< Length of the write message for CCCD. */

#define MHS_UUID_BASE   {0x1B, 0xC5, 0xD5, 0xA5, 0x02, 0x00, 0x82, 0x86,\
        0xE3, 0x11, 0xCB, 0x37, 0x00, 0x00, 0x00, 0x00}

typedef enum
{
    READ_REQ,  /**< Type identifying that this tx_message is a read request. */
    WRITE_REQ  /**< Type identifying that this tx_message is a write request. */
} tx_request_t;

/**@brief Structure for writing a message to the peer, i.e. CCCD.
 */
typedef struct
{
    uint8_t                  gattc_value[WRITE_MESSAGE_LENGTH];  /**< The message to write. */
    ble_gattc_write_params_t gattc_params;                       /**< GATTC parameters for this message. */
} write_params_t;

/**@brief Structure for holding data to be transmitted to the connected central.
 */
typedef struct
{
    uint16_t     conn_handle;  /**< Connection handle to be used when transmitting this message. */
    tx_request_t type;         /**< Type of this message, i.e. read or write message. */
    union
    {
        uint16_t       read_handle;  /**< Read request message. */
        write_params_t write_req;    /**< Write request message. */
    } req;
} tx_message_t;

static ble_mhs_c_t * mp_ble_mhs_c;
static tx_message_t  m_tx_buffer[TX_BUFFER_SIZE];  /**< Transmit buffer for messages to be transmitted to the central. */
static uint32_t      m_tx_insert_index = 0;        /**< Current index in the transmit buffer where the next message should be inserted. */
static uint32_t      m_tx_index = 0;               /**< Current index in the transmit buffer from where the next message to be transmitted resides. */

/**@brief Function for passing any pending request from the buffer to the stack.
 */
static void tx_buffer_process(void)
{
    if (m_tx_index != m_tx_insert_index)
    {
        uint32_t err_code;

        if (m_tx_buffer[m_tx_index].type == READ_REQ)
        {
            err_code = sd_ble_gattc_read(m_tx_buffer[m_tx_index].conn_handle,
                                         m_tx_buffer[m_tx_index].req.read_handle,
                                         0);
        }
        else
        {
            err_code = sd_ble_gattc_write(m_tx_buffer[m_tx_index].conn_handle,
                                          &m_tx_buffer[m_tx_index].req.write_req.gattc_params);
        }
        if (err_code == NRF_SUCCESS)
        {
            m_tx_index++;
            m_tx_index &= TX_BUFFER_MASK;
        }
        else
        {
        }
    }
}


/**@brief     Function for handling write response events.
 *
 * @param[in] p_ble_mhs_c Pointer to the Heart Rate Client structure.
 * @param[in] p_ble_evt   Pointer to the BLE event received.
 */
static void on_write_rsp(ble_mhs_c_t * p_ble_mhs_c, const ble_evt_t * p_ble_evt)
{
    // Check if there is any message to be sent across to the peer and send it.
    tx_buffer_process();
}


/**@brief     Function for handling events from the database discovery module.
 *
 * @details   This function will handle an event from the database discovery module, and determine
 *            if it relates to the discovery of heart rate service at the peer. If so, it will
 *            call the application's event handler indicating that the heart rate service has been
 *            discovered at the peer. It also populates the event with the service related
 *            information before providing it to the application.
 *
 * @param[in] p_evt Pointer to the event received from the database discovery module.
 *
 */
static void db_discover_evt_handler(ble_db_discovery_evt_t * p_evt)
{
    uint32_t err_code;
    // Check if the Heart Rate Service was discovered.
    if (p_evt->evt_type == BLE_DB_DISCOVERY_COMPLETE &&
        p_evt->params.discovered_db.srv_uuid.uuid == BLE_UUID_MHS_SERVICE &&
        p_evt->params.discovered_db.srv_uuid.type == BLE_UUID_TYPE_VENDOR_BEGIN)
    {
        mp_ble_mhs_c->conn_handle = p_evt->conn_handle;

        // Find the CCCD Handle of the Heart Rate Measurement characteristic.
        uint32_t i;

        for (i = 0; i < p_evt->params.discovered_db.char_count; i++)
        {
            if (p_evt->params.discovered_db.charateristics[i].characteristic.uuid.uuid ==
                BLE_UUID_MHS_CONTROL_POINT_CHAR)
            {
                mp_ble_mhs_c->mhs_ctrl_handle      =
                    p_evt->params.discovered_db.charateristics[i].characteristic.handle_value;
            }
            else if (p_evt->params.discovered_db.charateristics[i].characteristic.uuid.uuid ==
                BLE_UUID_MHS_EVENT_CHAR)
            {
                mp_ble_mhs_c->mhs_ctrl_cccd_handle =
                    p_evt->params.discovered_db.charateristics[i].cccd_handle;
                mp_ble_mhs_c->mhs_event_handle      =
                    p_evt->params.discovered_db.charateristics[i].characteristic.handle_value;
                err_code = ble_mhs_c_evt_notif_enable(get_mhs_obj());
                APP_ERROR_CHECK(err_code);
            }
        }

        ble_mhs_c_evt_t evt;

        evt.evt_type = BLE_MHS_C_EVT_DISCOVERY_COMPLETE;

        mp_ble_mhs_c->evt_handler(mp_ble_mhs_c, &evt);
    }
}


/**@brief     Function for handling Handle Value Notification received from the SoftDevice.
 *
 * @details   This function will uses the Handle Value Notification received from the SoftDevice
 *            and checks if it is a notification of the heart rate measurement from the peer. If
 *            it is, this function will decode the heart rate measurement and send it to the
 *            application.
 *
 * @param[in] p_ble_mhs_c Pointer to the Heart Rate Client structure.
 * @param[in] p_ble_evt   Pointer to the BLE event received.
 */
static void on_hvx(ble_mhs_c_t * p_ble_mhs_c, const ble_evt_t * p_ble_evt)
{
    // Check if this is a heart rate notification.
    if (p_ble_evt->evt.gattc_evt.params.hvx.handle == p_ble_mhs_c->mhs_event_handle)
    {
        ble_mhs_c_evt_t ble_mhs_c_evt;
        uint32_t        index = 0;

        ble_mhs_c_evt.evt_type = BLE_MHS_C_EVT_NOTIFICATION;

        ble_mhs_c_evt.mhs_evt_type = p_ble_evt->evt.gattc_evt.params.hvx.data[index++];

        ble_mhs_c_evt.mhs_evt_data = uint16_decode(&(p_ble_evt->evt.gattc_evt.params.hvx.data[index]));

        p_ble_mhs_c->evt_handler(p_ble_mhs_c, &ble_mhs_c_evt);
    }
}


void ble_mhs_c_on_ble_evt(ble_mhs_c_t * p_ble_mhs_c, const ble_evt_t * p_ble_evt)
{
    if ((p_ble_mhs_c == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            p_ble_mhs_c->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;

        case BLE_GATTC_EVT_HVX:
            on_hvx(p_ble_mhs_c, p_ble_evt);
            break;

        case BLE_GATTC_EVT_WRITE_RSP:
            on_write_rsp(p_ble_mhs_c, p_ble_evt);
            break;

        default:
            break;
    }
}


/**@brief Function for creating a message for writing to the CCCD.
 */
static uint32_t cccd_configure(uint16_t conn_handle, uint16_t handle_cccd, bool enable)
{
    tx_message_t * p_msg;
    uint16_t       cccd_val = enable ? BLE_GATT_HVX_NOTIFICATION : 0;

    p_msg              = &m_tx_buffer[m_tx_insert_index++];
    m_tx_insert_index &= TX_BUFFER_MASK;

    p_msg->req.write_req.gattc_params.handle   = handle_cccd;
    p_msg->req.write_req.gattc_params.len      = WRITE_MESSAGE_LENGTH;
    p_msg->req.write_req.gattc_params.p_value  = p_msg->req.write_req.gattc_value;
    p_msg->req.write_req.gattc_params.offset   = 0;
    p_msg->req.write_req.gattc_params.write_op = BLE_GATT_OP_WRITE_REQ;
    p_msg->req.write_req.gattc_value[0]        = LSB(cccd_val);
    p_msg->req.write_req.gattc_value[1]        = MSB(cccd_val);
    p_msg->conn_handle                         = conn_handle;
    p_msg->type                                = WRITE_REQ;

    tx_buffer_process();
    return NRF_SUCCESS;
}


static uint32_t ble_mhs_send_control_point_cmd(ble_mhs_c_t * p_ble_mhs_c, uint8_t *cmd, uint8_t len)
{
    tx_message_t * p_msg;

    p_msg              = &m_tx_buffer[m_tx_insert_index++];
    m_tx_insert_index &= TX_BUFFER_MASK;

    p_msg->req.write_req.gattc_params.handle   = p_ble_mhs_c->mhs_ctrl_handle;
    p_msg->req.write_req.gattc_params.len      = len;
    p_msg->req.write_req.gattc_params.p_value  = p_msg->req.write_req.gattc_value;
    p_msg->req.write_req.gattc_params.offset   = 0;
    p_msg->req.write_req.gattc_params.write_op = BLE_GATT_OP_WRITE_REQ;
    for(uint8_t i = 0; i < len; i++)
    {
        p_msg->req.write_req.gattc_value[i] = *(cmd + i);
    }
    p_msg->conn_handle                         = p_ble_mhs_c->conn_handle;
    p_msg->type                                = WRITE_REQ;

    tx_buffer_process();
    return NRF_SUCCESS;
}


uint32_t ble_mhs_c_init(ble_mhs_c_t * p_ble_mhs_c, ble_mhs_c_init_t * p_ble_mhs_c_init)
{
    uint32_t err_code;

    if ((p_ble_mhs_c == NULL) || (p_ble_mhs_c_init == NULL))
    {
        return NRF_ERROR_NULL;
    }

    ble_uuid_t mhs_uuid;

    ble_uuid128_t base_uuid  =
    {
        .uuid128 = MHS_UUID_BASE,
    };

    mhs_uuid.type = BLE_UUID_TYPE_VENDOR_BEGIN;
    mhs_uuid.uuid = BLE_UUID_MHS_SERVICE;

    err_code = sd_ble_uuid_vs_add(&base_uuid, &(mhs_uuid.type));
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    mp_ble_mhs_c = p_ble_mhs_c;

    mp_ble_mhs_c->evt_handler     = p_ble_mhs_c_init->evt_handler;
    mp_ble_mhs_c->conn_handle     = BLE_CONN_HANDLE_INVALID;
    mp_ble_mhs_c->mhs_ctrl_cccd_handle = BLE_GATT_HANDLE_INVALID;

    return ble_db_discovery_evt_register(&mhs_uuid,
                                         db_discover_evt_handler);
}


uint32_t ble_mhs_c_evt_notif_enable(ble_mhs_c_t * p_ble_mhs_c)
{
    if (p_ble_mhs_c == NULL)
    {
        return NRF_ERROR_NULL;
    }

    return cccd_configure(p_ble_mhs_c->conn_handle, p_ble_mhs_c->mhs_ctrl_cccd_handle, true);
}


void ble_mhs_c_get_temperature(void)
{
    uint8_t cmd = 0x02;
    ble_mhs_send_control_point_cmd(get_mhs_obj(), &cmd, sizeof(cmd));
}
