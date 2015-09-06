#include <string.h>

#include <app_error.h>
#include <app_timer.h>

#include "ble_mhs.h"
#include "ds18b20.h"
#include "heat.h"

#include "SEGGER_RTT.h"

#include "auto_temp.h"

#define TEMPERATURE_DETECT_TIMER_INTERVAL APP_TIMER_TICKS(60000, APP_TIMER_PRESCALER)

#define TEMPERATURE_DEFAULT             200
#define TEMPERATURE_MAX                 100
#define TEMPERATURE_MIN                 -30

static app_timer_id_t  m_temperature_detect_timer_id;

static int16_t  m_temperature_threshold = 0;
static int16_t  m_current_temperature   = TEMPERATURE_DEFAULT;

static bool temperature_is_good(int16_t temp)
{
    bool result = true;
    if ((temp >= TEMPERATURE_MIN) && (temp <= TEMPERATURE_MAX))
    {
        result = true;
    }
    else
    {
        result = false;
    }

    return result;
}


static void temperature_detect_timeout_handler(void * p_context)
{
    int16_t temp = 0;
    SEGGER_RTT_printf(0, "temperature detect one time! \r\n");
    temp = ds18b20_read_temperature();

    if (true == temperature_is_good(temp))
    {
        m_current_temperature = temp;

        if (m_current_temperature < m_temperature_threshold)
        {
            if (false == heat_is_on())
            {
                heat_on();
            }
        }
        else
        {
            if (true == heat_is_on())
            {
                heat_off();
            }
        }
    }
}


/**@brief Stop vibrator sequencer timer.
 */
static void stop_temperature_detect_timer(void)
{
    uint32_t err_code;
    err_code = app_timer_stop(m_temperature_detect_timer_id);
    APP_ERROR_CHECK(err_code);
}


static void start_temperature_detect_timer(void)
{
    uint32_t err_code;

    err_code = app_timer_start(m_temperature_detect_timer_id, TEMPERATURE_DETECT_TIMER_INTERVAL,
                                NULL);
    APP_ERROR_CHECK(err_code);
}


static void create_temperature_detect_timer(void)
{
    uint32_t err_code;

    err_code = app_timer_create(&m_temperature_detect_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                temperature_detect_timeout_handler);
    APP_ERROR_CHECK(err_code);
}


void report_current_temperature(void)
{
    uint32_t err_code;
    mhs_event_t event;

    memset(&event, 0, sizeof(mhs_event_t));
    event.evt_code       = MHS_EVENT_CODE_CURRENT_TEMPERATURE;
    event.evt_value.buff = (uint8_t *)&m_current_temperature;
    event.evt_value.len  = sizeof(m_current_temperature);

    err_code = mhs_event_characteristic_notify(event);

    APP_ERROR_CHECK(err_code);
}


void report_temperature_threshold(void)
{
    uint32_t err_code;
    mhs_event_t event;

    memset(&event, 0, sizeof(mhs_event_t));
    event.evt_code       = MHS_EVENT_CODE_TEMP_THRESHOLD;
    event.evt_value.buff = (uint8_t *)&m_temperature_threshold;
    event.evt_value.len  = sizeof(m_temperature_threshold);

    err_code = mhs_event_characteristic_notify(event);

    APP_ERROR_CHECK(err_code);
}


void set_temperature_threshold(int16_t temp_threshold)
{
    m_temperature_threshold = temp_threshold;
}


void auto_temperature_init(void)
{
    temperature_detect_timeout_handler(NULL);
    create_temperature_detect_timer();
    start_temperature_detect_timer();
}
