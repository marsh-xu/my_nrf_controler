#include <app_error.h>

#include "ble_mhs_c.h"
#include "uart.h"
#include "button.h"

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
        {
           err_code = ble_mhs_c_evt_notif_enable(p_mhs_c);
            APP_ERROR_CHECK(err_code);
            break;
        }
        case BLE_MHS_C_EVT_NOTIFICATION:
        {
            mhs_c_notification(p_mhs_c_evt->mhs_evt_type, p_mhs_c_evt->mhs_evt_data);
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
