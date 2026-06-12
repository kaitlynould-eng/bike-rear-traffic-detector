/*
 * haptic.c
 *
 *  Created on: Jun 7, 2026
 *      Author: nikis
 */

#include "haptic.h"

#define DRV2605_ADDR     (0x5A << 1)
#define DRV2605_REG_MODE 0x01
#define DRV2605_REG_RTP  0x02

static I2C_HandleTypeDef *haptic_i2c1;
static I2C_HandleTypeDef *haptic_i2c2;

static void drv2605_write(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t val)
{
    uint8_t data[2] = {reg, val};
    HAL_I2C_Master_Transmit(hi2c, DRV2605_ADDR, data, 2, HAL_MAX_DELAY);
}

static void drv2605_init(I2C_HandleTypeDef *hi2c)
{
    drv2605_write(hi2c, 0x01, 0x00);
    HAL_Delay(10);

    drv2605_write(hi2c, 0x03, 0x01);
    drv2605_write(hi2c, 0x1A, 0x36);

    drv2605_write(hi2c, DRV2605_REG_MODE, 0x05);
    drv2605_write(hi2c, DRV2605_REG_RTP, 0x00);
}

void Haptic_Init(I2C_HandleTypeDef *i2c1, I2C_HandleTypeDef *i2c2)
{
    haptic_i2c1 = i2c1;
    haptic_i2c2 = i2c2;

    drv2605_init(haptic_i2c1);
    drv2605_init(haptic_i2c2);
}

void Haptic_Off(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

    drv2605_write(haptic_i2c1, DRV2605_REG_RTP, 0);
    drv2605_write(haptic_i2c2, DRV2605_REG_RTP, 0);
}

static void Haptic_SetIntensity(uint8_t intensity)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);

    drv2605_write(haptic_i2c1, DRV2605_REG_RTP, intensity);
    drv2605_write(haptic_i2c2, DRV2605_REG_RTP, intensity);
}

//void Haptic_Yippee1(void)
//{
//    Haptic_SetIntensity(200);
//}
//
//void Haptic_Yippee2(void)
//{
//    Haptic_SetIntensity(180);
//}

void Haptic_PlayForMs(uint8_t intensity, uint32_t duration_ms)
{
    Haptic_SetIntensity(intensity);
    HAL_Delay(duration_ms);
    Haptic_Off();
}
//void Haptic_BeepBeepPattern(uint8_t intensity)
//{
//    Haptic_PlayForMs(intensity, 200);
//    HAL_Delay(150);
//
//    Haptic_PlayForMs(intensity, 200);
//    HAL_Delay(700);
//}
void Haptic_LongWarningPattern(uint8_t intensity)
{
    Haptic_PlayForMs(intensity, 500);
    HAL_Delay(250);

    Haptic_PlayForMs(intensity, 500);
    HAL_Delay(250);

    Haptic_PlayForMs(intensity, 1000);
    HAL_Delay(500);
}
