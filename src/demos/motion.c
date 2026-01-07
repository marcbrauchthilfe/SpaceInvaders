#include "demos/motion.h"
#include "hal/sensors/mpu6050.h"
#include "pico/stdlib.h"
#include <stdio.h>

void motion_demo_execute(void)
{
    mpu6050_init();
    while (true)
    {
        MotionState_t current_state;
        mpu6050_read(&current_state);
        mpu6050_print_motion_state(&current_state);
        tight_loop_contents();
    }
}
