#ifndef MOTOR_H_
#define MOTOR_H_

typedef enum motor_index_e
{
    MOTOR_INDEX_1 = 0,
    MOTOR_INDEX_2,
    MOTOR_INDEX_3,
    MOTOR_INDEX_4,
    MOTOR_INDEX_5,
    MOTOR_INDEX_6,
    MOTOR_INDEX_7,
    MOTOR_INDEX_8,
} motor_index_t;

typedef enum motor_direction_e
{
    MOTOR_DIRECTION_CLOCK = 0,
    MOTOR_DIRECTION_ANTICLOCK,
} motor_direction_t;

typedef struct motor_control_s
{
    motor_index_t  motor_index;
    motor_direction_t motor_direction;
} motor_control_t;

void motor_init(void);

void motor_on(motor_control_t motor_control);

void motor_off();

void motor_set_duty_cylce(uint8_t duty_cycle);

void report_motor_duty_cycle(void);

#endif // MOTOR_H_
