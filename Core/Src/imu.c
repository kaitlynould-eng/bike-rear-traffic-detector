/*
 * imu.c
 *
 *  Created on: Jun 8, 2026
 *      Author: nikis
 */


#include "imu.h"
#include <math.h>

#define BNO055_ADDR          (0x28 << 1)

#define BNO055_OPR_MODE      0x3D
#define BNO055_PWR_MODE      0x3E
#define BNO055_PAGE_ID       0x07
#define BNO055_SYS_TRIGGER   0x3F
#define BNO055_UNIT_SEL      0x3B
#define BNO055_MODE_AMG      0x07

#define BNO055_MODE_CONFIG   0x00
#define BNO055_MODE_NDOF     0x0C

#define BNO055_LIA_DATA_X_LSB 0x08

#define IMU_MOTION_THRESHOLD_MS2  0.25f
#define IMU_STOP_TIME_MS          15000

static I2C_HandleTypeDef *imu_i2c;
static uint32_t last_motion_time = 0;
static float accel_mag_last = 0.0f;

static float ax_last = 0.0f;
static float ay_last = 0.0f;
static float az_last = 0.0f;

//static float last_ax = 0.0f;
//static uint32_t delta_ax_start_time = 0;
//static uint8_t delta_ax_active = 0;

static void bno_write(uint8_t reg, uint8_t val)
{
    uint8_t data[2] = {reg, val};
    HAL_I2C_Master_Transmit(imu_i2c, BNO055_ADDR, data, 2, 100);
}

static int16_t read_i16_le(uint8_t *data)
{
    return (int16_t)((data[1] << 8) | data[0]);
}

void IMU_Init(I2C_HandleTypeDef *hi2c)
{
    imu_i2c = hi2c;

    HAL_Delay(700);

    bno_write(BNO055_OPR_MODE, BNO055_MODE_CONFIG);
    HAL_Delay(25);

    bno_write(BNO055_PAGE_ID, 0x00);
    bno_write(BNO055_PWR_MODE, 0x00);
    HAL_Delay(10);

    bno_write(BNO055_UNIT_SEL, 0x00);   // m/s^2 accel units
    bno_write(BNO055_OPR_MODE, BNO055_MODE_AMG);
    HAL_Delay(25);

    last_motion_time = HAL_GetTick();
}

IMUState_t IMU_Update(void)
{
    uint8_t data[6];

    if (HAL_I2C_Mem_Read(
            imu_i2c,
            BNO055_ADDR,
            BNO055_LIA_DATA_X_LSB,
            I2C_MEMADD_SIZE_8BIT,
            data,
            6,
            100
        ) != HAL_OK)
    {
        return IMU_MOVING;
    }

    int16_t raw_x = read_i16_le(&data[0]);
    int16_t raw_y = read_i16_le(&data[2]);
    int16_t raw_z = read_i16_le(&data[4]);

    float ax = raw_x / 100.0f;
    float ay = raw_y / 100.0f;
    float az = raw_z / 100.0f;
    ax_last = ax;
    ay_last = ay;
    az_last = az;


    float accel_mag = sqrtf(ax * ax + ay * ay + az * az);
    accel_mag_last = accel_mag;

    if (accel_mag > IMU_MOTION_THRESHOLD_MS2)
    {
        last_motion_time = HAL_GetTick();
        return IMU_MOVING;
    }

    if ((HAL_GetTick() - last_motion_time) >= IMU_STOP_TIME_MS)
    {
        return IMU_STOPPED;
    }

    return IMU_MOVING;
}
float IMU_GetAccelMagnitude(void)
{
    return accel_mag_last;
}
uint8_t IMU_ReadChipID(void)
{
    uint8_t id = 0;

    HAL_I2C_Mem_Read(
        imu_i2c,
        BNO055_ADDR,
        0x00,
        I2C_MEMADD_SIZE_8BIT,
        &id,
        1,
        100
    );

    return id;
}
void IMU_GetAccel(float *ax, float *ay, float *az)
{
    *ax = ax_last;
    *ay = ay_last;
    *az = az_last;
}
uint8_t IMU_ReadRegister(uint8_t reg)
{
    uint8_t val = 0;

    HAL_I2C_Mem_Read(
        imu_i2c,
        BNO055_ADDR,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        &val,
        1,
        100
    );

    return val;
}

//uint8_t IMU_AxOver5For10s(void)
//{
//    static uint32_t ax_start_time = 0;
//    static uint8_t ax_timer_active = 0;
//
//    if (fabsf(ax_last) > 5.0f) // set delta of 5 here
//    {
//        if (ax_timer_active == 0)
//        {
//            ax_timer_active = 1;
//            ax_start_time = HAL_GetTick();
//        }
//
//        if ((HAL_GetTick() - ax_start_time) >= 10000) // set 10 seconds here
//        {
//            return 1;
//        }
//    }
//    else
//    {
//        ax_timer_active = 0;
//        ax_start_time = 0;
//    }
//
//    return 0;
//}
uint8_t IMU_AxStableFor10s(void)
{
    static float last_ax = 0.0f;
    static uint32_t stable_start_time = 0;
    static uint8_t timer_active = 0;

    float delta_ax = fabsf(ax_last - last_ax);
    last_ax = ax_last;

    if (delta_ax < .5f)   // close enough to zero change
    {
        if (timer_active == 0)
        {
            timer_active = 1;
            stable_start_time = HAL_GetTick();
        }

        if ((HAL_GetTick() - stable_start_time) >= 10000) // set 10 seconds
        {
            return 1;
        }
    }
    else
    {
        timer_active = 0;
        stable_start_time = 0;
    }

    return 0;
}
