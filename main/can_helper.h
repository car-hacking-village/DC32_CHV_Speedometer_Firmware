/*
 * can_helper.h
 *
 *  Created on: Jul 12, 2023
 *      Author: ericpoole
 */

#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/twai.h"

#include "speedometer.h"

// TWAI globals
#define TWAI_TAG 						"CAN"
#define TX_GPIO_NUM                     11
#define RX_GPIO_NUM                     14
// #define TX_GPIO_NUM                     11
// #define RX_GPIO_NUM                     14
// #define RX_TASK_PRIO                    20       //Receiving task priority
// #define TX_TASK_PRIO                    20       //Sending task priority
// #define CTRL_TSK_PRIO                   21      //Control task priority

#define SPEED_ARBID						0x100
#define POWER_ARBID						0x101
#define BATTL_ARBID_SENDR				0x10000358
#define BATTL_ARBID_RECVR				0x10000359

extern QueueHandle_t can_rx_queue;
extern SemaphoreHandle_t can_read_lock;

void twai_init(void);

void can_send(uint32_t, uint32_t, uint32_t, uint8_t *);
bool can_recv(twai_message_t *);
