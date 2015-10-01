/**@file
 *
 * @brief File of OLED module.
 */


#ifndef OLED_H_
#define OLED_H_

typedef enum oled_ui_style_e
{
    UI_STYLE_GET_TEMPERATURE = 0,
    UI_STYLE_GET_TEMP_THRESHOLD,
    UI_STYLE_GET_MOTOR_SPEED,
    UI_STYLE_SET_TEMP_THRESHOLD,
    UI_STYLE_SET_MOTOR_SPEED,
    UI_STYLE_SET_MOTOR_CONTROL,
    UI_STYLE_TOTAL_NUM,
} oled_ui_style_t;

/**@brief   Initialize the OLED module.
 *
 * @details True if initialize successfully, otherwise false.
 */
void oled_init(void);

void ui_up_update(oled_ui_style_t index);

void oled_show_num(uint8_t num);

void oled_clear_num(void);

void oled_show_connect_status(bool is_connect);
void oled_show_choose_status(bool is_choose);

#endif // OLED_H_
