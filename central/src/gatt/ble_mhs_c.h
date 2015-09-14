#ifndef BLE_MHS_C_H_
#define BLE_MHS_C_H_

#include <stdint.h>

#define BLE_UUID_MHS_SERVICE                                     0x0200
#define BLE_UUID_MHS_CONTROL_POINT_CHAR                          0x0201
#define BLE_UUID_MHS_EVENT_CHAR                                  0x0202

typedef enum
{
    BLE_MHS_C_EVT_DISCOVERY_COMPLETE = 1,  /**< Event indicating that the Heart Rate Service has been discovered at the peer. */
    BLE_MHS_C_EVT_NOTIFICATION             /**< Event indicating that a notification of the Heart Rate Measurement characteristic has been received from the peer. */
} ble_mhs_c_evt_type_t;

/**@brief Heart Rate Event structure. */
typedef struct
{
    ble_mhs_c_evt_type_t evt_type;  /**< Type of the event. */
    union
    {
        uint32_t hrm;  /**< Heart rate measurement received. This will be filled if the evt_type is @ref BLE_HRS_C_EVT_HRM_NOTIFICATION. */
    } params;
} ble_mhs_c_evt_t;

/** @} */

/**
 * @defgroup mhs_c_types Types
 * @{
 */

// Forward declaration of the ble_bas_t type.
typedef struct ble_mhs_c_s ble_mhs_c_t;

/**@brief   Event handler type.
 *
 * @details This is the type of the event handler that should be provided by the application
 *          of this module in order to receive events.
 */
typedef void (* ble_mhs_c_evt_handler_t) (ble_mhs_c_t * p_ble_mhs_c, ble_mhs_c_evt_t * p_evt);

/**@brief Heart Rate Client structure.
 */
struct ble_mhs_c_s
{
    uint16_t                conn_handle;      /**< Connection handle as provided by the SoftDevice. */
    uint16_t                hrm_cccd_handle;  /**< Handle of the CCCD of the Heart Rate Measurement characteristic. */
    uint16_t                hrm_handle;       /**< Handle of the Heart Rate Measurement characteristic as provided by the SoftDevice. */
    ble_mhs_c_evt_handler_t evt_handler;      /**< Application event handler to be called when there is an event related to the heart rate service. */
};

typedef struct
{
    ble_mhs_c_evt_handler_t evt_handler;
} ble_mhs_c_init_t;


uint32_t ble_mhs_c_init(ble_mhs_c_t * p_ble_mhs_c, ble_mhs_c_init_t * p_ble_mhs_c_init);

uint32_t ble_mhs_c_evt_notif_enable(ble_mhs_c_t * p_ble_mhs_c);


#endif // BLE_MHS_C_H_
