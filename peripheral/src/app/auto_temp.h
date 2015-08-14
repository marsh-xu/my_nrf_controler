#ifndef AUTO_TEMP_H_
#define AUTO_TEMP_H_

#include <stdint.h>

/**@brief   Get current temperature value.
 */
void report_current_temperature(void);

/**@brief   Set temperature threshold.
 *
 * @param   temp_threshold   The temperature threshold which to set.
 */
void set_temperature_threshold(int16_t temp_threshold);


/**@brief   Get temperature threshold.
 */
void report_temperature_threshold(void);

void auto_temperature_init(void);

#endif
