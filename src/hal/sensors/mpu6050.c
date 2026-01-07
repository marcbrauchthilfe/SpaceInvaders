#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <string.h>
#include "hal/sensors/mpu6050.h"

#define SCALE_FACTOR 1.700f

// Configuration
#define MPU6050_ADDR 0x68
#define I2C_PORT i2c0
#define PIN_SDA 4
#define PIN_SCL 5

// MPU6050 Register
#define REG_PWR_MGMT_1 0x6B
#define REG_ACCEL_XOUT_H 0x3B
#define REG_CONFIG 0x1A

// Physical constants
#define GYRO_SCALE 131.0f
#define RAD_TO_DEG 57.295779f
#define ALPHA 0.90f // Filter: 90% gyro, 10% accel (Trust in accel)

float gyro_offset_x;
float gyro_offset_y;
float gyro_offset_z;
float tare_roll;
float tare_pitch;
float angle_roll;
float angle_pitch;

static void mpu6050_read_raw(int16_t *ax, int16_t *ay, int16_t *az, int16_t *gx, int16_t *gy, int16_t *gz)
{
    uint8_t buffer[14];
    uint8_t val = REG_ACCEL_XOUT_H;
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &val, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, buffer, 14, false);
    *ax = (int16_t)((buffer[0] << 8) | buffer[1]);
    *ay = (int16_t)((buffer[2] << 8) | buffer[3]);
    *az = (int16_t)((buffer[4] << 8) | buffer[5]);
    *gx = (int16_t)((buffer[8] << 8) | buffer[9]);
    *gy = (int16_t)((buffer[10] << 8) | buffer[11]);
    *gz = (int16_t)((buffer[12] << 8) | buffer[13]);
}

static void mpu6050_calibrate_gyro()
{
    printf("1. Calibrating gyro drift (PLEASE DO NOT MOVE)...\n");
    int32_t sum_x = 0, sum_y = 0, sum_z = 0;
    int16_t ax, ay, az, gx, gy, gz;

    for (int i = 0; i < 2000; i++)
    {
        mpu6050_read_raw(&ax, &ay, &az, &gx, &gy, &gz);
        sum_x += gx;
        sum_y += gy;
        sum_z += gz;
        sleep_ms(1);
    }
    gyro_offset_x = (float)sum_x / 2000.0f;
    gyro_offset_y = (float)sum_y / 2000.0f;
    gyro_offset_z = (float)sum_z / 2000.0f;

    printf("OK! Offsets: X=%.1f, Y=%.1f, Z=%.1f\n", gyro_offset_x, gyro_offset_y, gyro_offset_z);
}

static absolute_time_t last_time;

void mpu6050_init()
{
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_SDA);
    gpio_pull_up(PIN_SCL);
    sleep_ms(3000);
    printf("\n--- START MPU6050 VERTICAL MODE ---\n");

    // MPU Init
    uint8_t init_cmds[] = {REG_PWR_MGMT_1, 0x00, REG_CONFIG, 0x03};
    for (int i = 0; i < 4; i += 2)
        i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &init_cmds[i], 2, false);

    // Step 1: Calibrate gyro drift (PLEASE DO NOT MOVE)
    mpu6050_calibrate_gyro();

    last_time = get_absolute_time();

    // Step 2: Settling & Taring (Set zero point)
    printf("2. Searching zero point (tare)... \n");

    angle_roll = 0.0f;
    angle_pitch = 0.0f;

    for (int i = 0; i < 200; i++)
    {
        int16_t ax, ay, az, gx, gy, gz;
        mpu6050_read_raw(&ax, &ay, &az, &gx, &gy, &gz);

        // Accel:
        float v_ax_virt = (float)az;  // Phys Z -> Virt X
        float v_ay_virt = -(float)ax; // Phys X -> Virt Y (with inversion)
        float v_az_virt = (float)ay;  // Phys Y -> Virt Z (gravity)

        // Gyro:
        // We need to assign the roll and pitch rates of the new axes.
        float v_gx = ((float)gz - gyro_offset_z) / GYRO_SCALE; // Phys Z Rate -> Virt X Rate (Roll)
        float v_gy = ((float)gy - gyro_offset_y) / GYRO_SCALE; // Phys Y Rate -> Virt Y Rate (Pitch)

        // Time delta
        absolute_time_t now = get_absolute_time();
        float dt = absolute_time_diff_us(last_time, now) / 1000000.0f;
        last_time = now;

        // We swap the inputs in atan2 to get 90 degrees!
        float accel_roll = atan2(v_ay_virt, v_az_virt) * RAD_TO_DEG;
        float accel_pitch = atan2(v_ax_virt, v_az_virt) * RAD_TO_DEG;
        // The filter needs a rate that corresponds to the swap.
        // Since accel_pitch now depends on v_ax, it needs the v_gx rate.
        angle_roll = ALPHA * (angle_roll + v_gx * dt) + (1.0f - ALPHA) * accel_roll;
        angle_pitch = ALPHA * (angle_pitch + v_gy * dt) + (1.0f - ALPHA) * accel_pitch;
        sleep_ms(5);
    }

    // Save zero point
    tare_roll = angle_roll;
    tare_pitch = angle_pitch;
    printf("OK! Zero point set at Roll: %.2f, Pitch: %.2f\n", tare_roll, tare_pitch);
    printf("--- MEASUREMENT STARTED (0,0 is Standing) ---\n");
    // --- ENDLESS LOOP ---
}

void mpu6050_read(MotionState_t *state)
{
    int16_t ax, ay, az, gx, gy, gz;
    mpu6050_read_raw(&ax, &ay, &az, &gx, &gy, &gz);

    // --- AXIS SWAP: Y becomes Z ---
    float v_ax = (float)az;
    float v_ay = -(float)ax;
    float v_az = (float)ay;

    // Correct and scale gyro
    float v_gx = ((float)gz - gyro_offset_z) / GYRO_SCALE;
    float v_gy = ((float)gy - gyro_offset_y) / GYRO_SCALE;

    // Time delta
    absolute_time_t now = get_absolute_time();
    // ASSUMPTION: last_time was initialized BEFORE the loop.
    float dt = absolute_time_diff_us(last_time, now) / 1000000.0f;
    last_time = now;

    // Filter
    float accel_roll = atan2(v_ay, v_az) * RAD_TO_DEG;
    float accel_pitch = atan2(v_ax, v_az) * RAD_TO_DEG;

    angle_roll = ALPHA * (angle_roll + v_gx * dt) + (1.0f - ALPHA) * accel_roll;
    angle_pitch = ALPHA * (angle_pitch + v_gy * dt) + (1.0f - ALPHA) * accel_pitch;

    // Output relative to zero point (tare)
    float out_roll = angle_roll - tare_roll;
    float out_pitch = angle_pitch - tare_pitch;

    // **FINAL CORRECTIONS:**
    // 1. Roll scaling (corrects 52.9° to 90°)
    out_roll = out_roll * SCALE_FACTOR;

    // 2. Pitch inversion (corrects the -25.25° offset to 0°)
    out_pitch = -out_pitch;

    // Correction of 360° overflow (for the -90° rotation)
    if (out_roll > 180.0f)
        out_roll -= 360.0f;

    // Deadzone
    if (out_roll > -0.1f && out_roll < 0.1f)
        out_roll = 0.0f;
    // Pitch Deadzone can now also be set to 0.1, as the correction has been made.
    if (out_pitch > -0.1f && out_pitch < 0.1f)
        out_pitch = 0.0f;

    const float THRESHOLD = 10.0f;
    float abs_roll = fabs(out_roll);
    float abs_pitch = fabs(out_pitch);

    // 1. Update continuous angles (out_roll/out_pitch)
    state->roll = out_roll;
    state->pitch = out_pitch;

    // Initially set actions to NEUTRAL
    state->primary_action_roll = COMMAND_NEUTRAL;
    state->primary_action_pitch = COMMAND_NEUTRAL;

    // 2. PRIORITIZED LOGIC
    // A. Check roll axis (UP/DOWN)
    if (abs_roll >= THRESHOLD && abs_roll >= abs_pitch)
    {
        // Roll is dominant
        if (out_roll > 0)
        {
            state->primary_action_roll = COMMAND_UP;
        }
        else
        {
            state->primary_action_roll = COMMAND_DOWN;
        }
    }

    // B. Check pitch axis (LEFT/RIGHT)
    else if (abs_pitch >= THRESHOLD && abs_pitch > abs_roll)
    {
        // Pitch is dominant
        if (out_pitch < 0)
        {
            state->primary_action_pitch = COMMAND_RIGHT;
        }
        else
        {
            state->primary_action_pitch = COMMAND_LEFT;
        }
    }
}

const char *command_to_string(CommandAction_t action)
{
    switch (action)
    {
    case COMMAND_NEUTRAL:
        return "NEUTRAL";
    case COMMAND_UP:
        return "UP";
    case COMMAND_DOWN:
        return "DOWN";
    case COMMAND_LEFT:
        return "LEFT";
    case COMMAND_RIGHT:
        return "RIGHT";
    default:
        return "ERROR";
    }
}

void mpu6050_print_motion_state(const MotionState_t *state)
{
    // 1. Determine dominant action
    const char *roll_cmd = command_to_string(state->primary_action_roll);
    const char *pitch_cmd = command_to_string(state->primary_action_pitch);
    const char *final_cmd_str = "NEUTRAL";

    if (state->primary_action_roll != COMMAND_NEUTRAL)
    {
        final_cmd_str = roll_cmd;
    }
    else if (state->primary_action_pitch != COMMAND_NEUTRAL)
    {
        final_cmd_str = pitch_cmd;
    }

    // 2. Single-line printf output
    printf("Roll: %.2f \t Pitch: %.2f \t Cmd: %s\n",
           state->roll,
           state->pitch,
           final_cmd_str);
}