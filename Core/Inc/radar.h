/*
 * radar.h
 *
 *  Created on: Jun 6, 2026
 *      Author: nikis
 */

#ifndef INC_RADAR_H_
#define INC_RADAR_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>
#define RADAR_MIN_DISTANCE_M 2.0f
#define RADAR_MAX_DISTANCE_M 8.92f

typedef enum
{
    RADAR_NO_OBJECT = 0,
    RADAR_YIPPEE_1  = 1,
    RADAR_YIPPEE_2  = 2
} RadarResult_t;

void Radar_Init(UART_HandleTypeDef *cli_uart, UART_HandleTypeDef *data_uart);
void Radar_SendConfig(void);
RadarResult_t Radar_Update(void);


#endif /* INC_RADAR_H_ */
