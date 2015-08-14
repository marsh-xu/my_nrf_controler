/**@file
 *
 * @brief System init module.
 */

#ifndef SYSTEM_INIT_H
#define SYSTEM_INIT_H

/**@brief Initialing the system.
 *
 * @details The function will perform the system initializing tasks. Such as
 *          BLE stack initialization, scheduler initialization, adverting and
 *          connection parameters initialization, etc.
 *          Also, it in charges of all the peripherals initialization.
 */
void system_init(void);

#endif
