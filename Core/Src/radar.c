/*
 * radar.c
 *
 *  Created on: Jun 6, 2026
 *      Author: nikis
 */
#include "radar.h"

#include <string.h>
#include <stdio.h>
#include <math.h>

extern void debug_print(char *msg);

#define RADAR_BUFFER_SIZE 4096
#define MAGIC_SIZE 8
#define HEADER_SIZE 40


static UART_HandleTypeDef *radar_cli_uart;
static UART_HandleTypeDef *radar_data_uart;

static uint8_t buffer[RADAR_BUFFER_SIZE];
static uint16_t buffer_len = 0;

static const uint8_t MAGIC[MAGIC_SIZE] =
{
    0x02, 0x01, 0x04, 0x03, 0x06, 0x05, 0x08, 0x07
};

static const char *radar_config[] =
{
    "flushCfg",
    "dfeDataOutputMode 1",
    "channelCfg 15 3 0",
    "adcCfg 2 1",
    "adcbufCfg -1 0 1 1 1",
    "profileCfg 0 60 7 3 57 0 0 70 1 256 5000 0 0 30",
    "chirpCfg 0 0 0 0 0 0 0 1",
    "frameCfg 0 0 64 0 50 1 0",
    "lowPower 0 0",
    "guiMonitor -1 1 0 0 0 0 0",
    "cfarCfg -1 0 2 8 4 3 0 15 1",
    "cfarCfg -1 1 0 4 2 3 1 15 1",
    "multiObjBeamForming -1 0 0",
    "clutterRemoval -1 0",
    "calibDcRangeSig -1 0 -5 8 256",
    "cfarFovCfg -1 0 0 8.92",  // MAX DISTANCE that worked was 8.92
    "cfarFovCfg -1 1 -3 3",
    "aoaFovCfg -1 -90 90 -90 90",
    "extendedMaxVelocity -1 0.01",
    "lvdsStreamCfg -1 0 0 0",
    "measureRangeBiasAndRxChanPhase 0 1.5 0.2",
    "compRangeBiasAndRxChanPhase 0.0 1 0 -1 0 1 0 -1 0 1 0 -1 0 1 0 -1 0 1 0 -1 0 1 0 -1 0",
    "CQRxSatMonitor 0 3 5 121 0",
    "CQSigImgMonitor 0 127 4",
    "analogMonitor 0 0",
    "sensorStart"
}; // change stuff for distance here

static uint32_t read_u32_le(uint8_t *data)
{
    return ((uint32_t)data[0]) |
           ((uint32_t)data[1] << 8) |
           ((uint32_t)data[2] << 16) |
           ((uint32_t)data[3] << 24);
}

static float read_float_le(uint8_t *data)
{
    float value;
    memcpy(&value, data, sizeof(float));
    return value;
}

void Radar_Init(UART_HandleTypeDef *cli_uart, UART_HandleTypeDef *data_uart)
{
    radar_cli_uart = cli_uart;
    radar_data_uart = data_uart;
    buffer_len = 0;
}

void Radar_SendConfig(void)
{
    char line[256];
    char msg[300];
    uint8_t rx;
    uint32_t start_time;

    for (uint32_t i = 0; i < sizeof(radar_config) / sizeof(radar_config[0]); i++)
    {
        snprintf(msg, sizeof(msg),
                 "CMD %lu: %s\r\n",
                 (unsigned long)i,
                 radar_config[i]);

        debug_print(msg);

        snprintf(line, sizeof(line), "%s\r\n", radar_config[i]);

        HAL_UART_Transmit(
            radar_cli_uart,
            (uint8_t *)line,
            strlen(line),
            100
        );

        start_time = HAL_GetTick();

        while ((HAL_GetTick() - start_time) < 200)
        {
            if (HAL_UART_Receive(radar_cli_uart, &rx, 1, 10) == HAL_OK)
            {
                char c[2];
                c[0] = rx;
                c[1] = '\0';
                debug_print(c);
            }
        }

        debug_print("\r\n---\r\n");

        HAL_Delay(50);
    }
}

RadarResult_t Radar_Update(void)
{
    uint8_t byte;

    while (HAL_UART_Receive(radar_data_uart, &byte, 1, 1) == HAL_OK)
    {
        if (buffer_len < RADAR_BUFFER_SIZE)
        {
            buffer[buffer_len++] = byte;
        }
        else
        {
            buffer_len = 0;
        }
    }

    int magic_index = -1;

    for (uint16_t i = 0; i + MAGIC_SIZE <= buffer_len; i++)
    {
        if (memcmp(&buffer[i], MAGIC, MAGIC_SIZE) == 0)
        {
            magic_index = i;
            break;
        }
    }

    if (magic_index < 0)
    {
        if (buffer_len > MAGIC_SIZE)
        {
            memmove(buffer, &buffer[buffer_len - MAGIC_SIZE], MAGIC_SIZE);
            buffer_len = MAGIC_SIZE;
        }

        return RADAR_NO_OBJECT;
    }

    if (buffer_len < magic_index + HEADER_SIZE)
    {
        return RADAR_NO_OBJECT;
    }

    uint8_t *header = &buffer[magic_index];

    uint32_t total_len = read_u32_le(&header[12]);
    uint32_t num_tlvs  = read_u32_le(&header[32]);

    if (total_len == 0 || total_len > RADAR_BUFFER_SIZE)
    {
        buffer_len = 0;
        return RADAR_NO_OBJECT;
    }

    if (buffer_len < magic_index + total_len)
    {
        return RADAR_NO_OBJECT;
    }

    uint8_t *packet = &buffer[magic_index];

    uint32_t offset = HEADER_SIZE;
    float max_speed = 0.0f;

    for (uint32_t t = 0; t < num_tlvs; t++)
    {
        if (offset + 8 > total_len)
        {
            break;
        }

        uint32_t tlv_type = read_u32_le(&packet[offset]);
        uint32_t tlv_len  = read_u32_le(&packet[offset + 4]);

        offset += 8;

        if (offset + tlv_len > total_len)
        {
            break;
        }

        if (tlv_type == 1)
        {
            uint32_t num_points = tlv_len / 16;

            for (uint32_t i = 0; i < num_points; i++)
            {
                uint32_t point_offset = offset + i * 16;

                float x = read_float_le(&packet[point_offset + 0]);
                float y = read_float_le(&packet[point_offset + 4]);
                float z = read_float_le(&packet[point_offset + 8]);
                float doppler = read_float_le(&packet[point_offset + 12]);

                if (x >= 1.0f && doppler < -0.05f)
                {
                    float speed = -doppler;

                    if (speed > max_speed)
                    {
                        max_speed = speed;
                    }
                }
            }
        }

        offset += tlv_len;
    }

    uint16_t remaining = buffer_len - (magic_index + total_len);
    memmove(buffer, &buffer[magic_index + total_len], remaining);
    buffer_len = remaining;

    if (max_speed > 0.1f) // sets min velocity for detection. units = m/s(?) maybe f means something
    {
        if (max_speed <= 3.2f) // change speed range here. switch between yippe1 (slow) and 2 (fast)
        {
            return RADAR_YIPPEE_1;
        }
        else
        {
            return RADAR_YIPPEE_2;
        }
    }

    return RADAR_NO_OBJECT;
}
