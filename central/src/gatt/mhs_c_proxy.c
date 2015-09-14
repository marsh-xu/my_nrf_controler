#include <app_error.h>

#include "ble_mhs_c.h"

#include "mhs_c_proxy.h"

static ble_mhs_c_t                  m_ble_mhs_c;

/**@brief MHS Collector Handler.
 */
static void mhs_c_evt_handler(ble_mhs_c_t * p_mhs_c, ble_mhs_c_evt_t * p_mhs_c_evt)
{
    uint32_t err_code;

    switch (p_mhs_c_evt->evt_type)
    {
        case BLE_MHS_C_EVT_DISCOVERY_COMPLETE:
            // Initiate bonding.
            //err_code = dm_security_setup_req(&m_dm_device_handle);
            //APP_ERROR_CHECK(err_code);

            // Heart rate service discovered. Enable notification of Heart Rate Measurement.
            err_code = ble_mhs_c_evt_notif_enable(p_mhs_c);
            APP_ERROR_CHECK(err_code);

            //printf("Heart rate service discovered \r\n");
            break;

        case BLE_MHS_C_EVT_NOTIFICATION:
        {
            //printf("Heart Rate = %d\r\n", p_mhs_c_evt->params.hrm.hr_value);
            break;
        }

        default:
            break;
    }
}


void mhs_c_init(void)
{
    ble_mhs_c_init_t mhs_c_init_obj;

    mhs_c_init_obj.evt_handler = mhs_c_evt_handler;

    uint32_t err_code = ble_mhs_c_init(&m_ble_mhs_c, &mhs_c_init_obj);
    APP_ERROR_CHECK(err_code);
}


ble_mhs_c_t* get_mhs_obj(void)
{
    return &m_ble_mhs_c;
}
