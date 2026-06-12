/*
 * imu.h
 *
 *  Created on: Jun 8, 2026
 *      Author: nikis
 */

#ifndef INC_IMU_H_
#define INC_IMU_H_
#include "main.h"

typedef enum
{
    IMU_MOVING = 0,
    IMU_STOPPED = 1
} IMUState_t;

void IMU_Init(I2C_HandleTypeDef *hi2c);
IMUState_t IMU_Update(void);
float IMU_GetAccelMagnitude(void);
uint8_t IMU_ReadChipID(void);
void IMU_GetAccel(float *ax, float *ay, float *az);
uint8_t IMU_ReadRegister(uint8_t reg);
//uint8_t IMU_AxOver5For10s(void);
uint8_t IMU_AxStableFor10s(void);

#endif /* INC_IMU_H_ */
