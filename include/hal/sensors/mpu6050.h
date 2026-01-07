// based on https://github.com/HumansAreWeak/rpi-pico-mpu6050
#ifndef MPU6050_H
#define MPU6050_H

typedef float Angle_t;
typedef enum
{
    COMMAND_NEUTRAL = 0,
    COMMAND_UP,
    COMMAND_DOWN,
    COMMAND_LEFT,
    COMMAND_RIGHT
} CommandAction_t;

typedef struct
{
    Angle_t roll;
    Angle_t pitch;
    CommandAction_t primary_action_roll;
    CommandAction_t primary_action_pitch;
} MotionState_t;

void mpu6050_init();
void mpu6050_read(MotionState_t *state);
void mpu6050_print_motion_state(const MotionState_t *state);

#endif // MPU6050_H