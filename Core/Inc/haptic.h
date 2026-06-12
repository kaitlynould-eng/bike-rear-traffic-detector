/*
 * haptic.h
 *
 *  Created on: Jun 7, 2026
 *      Author: nikis
 */

#ifndef INC_HAPTIC_H_
#define INC_HAPTIC_H_

#include "main.h"
#include <stdint.h>

void Haptic_Init(I2C_HandleTypeDef *i2c1, I2C_HandleTypeDef *i2c2);

void Haptic_Off(void);
//void Haptic_Yippee1(void);
//void Haptic_Yippee2(void);
void Haptic_PlayForMs(uint8_t intensity, uint32_t duration_ms);
//void Haptic_BeepBeepPattern(uint8_t intensity);
void Haptic_LongWarningPattern(uint8_t intensity);



#endif /* INC_HAPTIC_H_ */
