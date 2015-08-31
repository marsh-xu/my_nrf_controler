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

#ifndef BLE_MHS_H_
#define BLE_MHS_H_

#include <ble.h>
#include <ble_gatts.h>
#include <ble_srv_common.h>

#define BLE_UUID_MHS_SERVICE                                0x0200
#define BLE_UUID_MHS_CONTROL_POINT_CHARACTERISTIC           0x0201
#define BLE_UUID_MHS_EVENT_CHARACTERISTIC                   0x0202

#define EVT_NOTIFICATION_WRITE_LEN                          2
#define MHS_EVENT_MAX_TX_CHAR_LEN                           20

// Length of command that received from host application.
#define CTRL_POINT_CHAR_CMD_CODE_LEN                        1
#define CTRL_POINT_CHAR_CMD_CODE_AND_VALUE_LEN              3

typedef struct mhs_event_control_point_cmd_s
{
    uint32_t cmd_value : 24;
    uint32_t cmd_code  : 8;
} mhs_event_control_point_cmd_t;

/**@brief   Control point characteristic command code. */
typedef enum mhs_control_point_cmd_code_e
{
    MHS_CMD_CODE_GET_TEMPERATURE                 = 0x01,
    MHS_CMD_CODE_GET_TEMP_THRESHOLD              = 0x02,
    MHS_CMD_CODE_GET_MOTOR_SPEED                 = 0x03,
    MHS_CMD_CODE_SET_TEMP_THRESHOLD              = 0x04,
    MHS_CMD_CODE_SET_MOTOR_CONTROL               = 0x05,
    MHS_CMD_CODE_SET_MOTOR_SPEED                 = 0x06,
    MHS_CMD_CODE_SET_MOTOR_OFF                   = 0x07,
    MHS_CMD_CODE_SET_MUSIC_CONTROL               = 0x08,
} mhs_control_point_cmd_code_t;

typedef struct mhs_control_point_cmd_s
{
    mhs_control_point_cmd_code_t   cmd_code;        // Command code
    uint16_t                       cmd_value;       // Command value
} __attribute__((__packed__)) mhs_control_point_cmd_t;

/**@brief Sony Advanced Accessory Host Service characteristic type. */
typedef enum ble_mhs_characteristic_e
{
    MHS_CHARACTERISTIC_INVALID = 0,
    MHS_CHARACTERISTIC_EVENT,
    MHS_CHARACTERISTIC_CONTROL_POINT,
} ble_mhs_characteristic_t;

// Forward declaration of the ble_mhs_t type.
typedef struct ble_mhs_s ble_mhs_t;

/**@brief Sony Advanced Accessory Host Service control point characteristic event type. */
typedef enum ble_mhs_control_char_evt_e
{
    BLE_MHS_CONTROL_CHAR_EVT_GET_TEMPERATURE   = 0,
    BLE_MHS_CONTROL_CHAR_EVT_GET_TEMP_SHRESHOLD,
    BLE_MHS_CONTROL_CHAR_EVT_GET_MOTOR_SPEED,
    BLE_MHS_CONTROL_CHAR_EVT_SET_TEMP_SHRESHOLD,
    BLE_MHS_CONTROL_CHAR_EVT_SET_MOTOR_CONTROL,
    BLE_MHS_CONTROL_CHAR_EVT_SET_MOTOR_SPEED,
    BLE_MHS_CONTROL_CHAR_EVT_SET_MOTOR_OFF,
    BLE_MHS_CONTROL_CHAR_EVT_SET_MUSIC_CONTROL,
} ble_mhs_control_char_evt_t;

/**@brief Sony Advanced Accessory Host Service event characteristic event type. */
typedef enum ble_mhs_event_char_evt_e
{
    BLE_MHS_EVENT_CHAR_EVT_ENABLED,          // Event to enable event characteristic.
    BLE_MHS_EVENT_CHAR_EVT_DISABLED,         // Event to disable event characteristic.
} ble_mhs_event_char_evt_t;

/**@brief Sony Advanced Accessory Host Service event structure. This contains the
 *        event type, event data and data length.
 */
typedef struct ble_mhs_evt_s
{
    ble_mhs_characteristic_t       ble_mhs_char;    // Characteristic type.
    union evt_type
    {
        ble_mhs_event_char_evt_t   event_char_evt;  // Event characteristic event.
        ble_mhs_control_char_evt_t control_char_evt;// Control point characteristic event.
    } evt_type;
    union evt_params
    {
        uint8_t *p_event_data;  // Pointer to event data.
    } evt_params;
    uint8_t event_data_len;
} ble_mhs_evt_t;

typedef void (*ble_mhs_evt_handler_t)(ble_mhs_t *p_mhs, ble_mhs_evt_t *p_evt);

typedef struct ble_mhs_s
{
    uint8_t                  uuid_type;
    uint16_t                 service_handle;
    ble_gatts_char_handles_t event_handles;
    ble_gatts_char_handles_t control_point_handles;
    uint16_t                 conn_handle;
    ble_mhs_evt_handler_t    evt_handler;
} ble_mhs_t;

typedef struct ble_mhs_init_s
{
    // Event handler to be called for handling events in the Sony Advanced Accessory Host Service.
    ble_mhs_evt_handler_t    evt_handler;
} ble_mhs_init_t;

typedef enum mhs_event_code_e
{
    MHS_EVENT_CODE_CURRENT_TEMPERATURE = 0,
    MHS_EVENT_CODE_TEMP_THRESHOLD,
    MHS_EVENT_CODE_MOTOR_SPEED,
} mhs_event_code_t;

typedef struct mhs_event_value_s
{
    uint8_t *buff;                    // Pointer point to data needed to be sent.
    uint8_t  len;                     // The length of the buffer.
} mhs_event_value_t;

typedef struct mhs_event_s
{
    mhs_event_code_t   evt_code;
    mhs_event_value_t  evt_value;
} mhs_event_t;



/**@brief       Initialize the MHS protocol.
 *
 * @param[out]  p_mhs       MHS structure. This structure will have to be supplied
 *                          by the application. It will be initialized by this function and will
 *                          later be used to identify this particular service instance.
 * @param[in]   p_mhs_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 *              This function returns NRF_ERROR_NULL if either of the pointers p_mhs or p_mhs_init
 *              is NULL.
 */
uint32_t ble_mhs_init(ble_mhs_t *p_mhs, const ble_mhs_init_t *p_mhs_init);

/**@brief       MHS BLE event handler.
 *
 * @details     The MHS protocol expects the application to call this function each time an
 *              event is received from the S110 SoftDevice. This function processes the event
 *              if it is relevant for it and calls the MHS event handler of the
 *              application if necessary.
 *
 * @param[in]   p_mhs      MHS structure.
 * @param[in]   p_ble_evt  Event received from the S110 SoftDevice.
 */
void ble_mhs_on_ble_evt(ble_mhs_t *p_mhs, ble_evt_t *p_ble_evt);


uint32_t mhs_event_characteristic_notify(mhs_event_t event);

#endif // BLE_MHS_H_
