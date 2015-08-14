/**
 * @file
 *
 * @brief    MHS protocol API.
 *
 * @details  Marsh Accessory Host Service (MHS) protocol allows
 *           a SmartWear device and a host application running on
 *           a phone to communicate over Bluetooth Low Energy (BLE).
 *           This module is intended to initialize the MHS protocol
 *           and get current data structure of AHS protocol.
 */

#ifndef MHS_PROXY_H_
#define MHS_PROXY_H_

#include "ble_mhs.h"

/**@brief AHS protocol initialization.
 *
 */
void mhs_init(void);

/**@brief Get current data structure of AHS.
 *
 * @return Pointer to AHS structure
 */
ble_mhs_t* get_mhs_obj(void);

#endif // AHS_PROXY_H_
