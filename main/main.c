#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"

#include "st7565.h"
#include "bmpfile.h"
// #include "font6x8.h"
// #include "font8x8.h"
// #include "font8x8_bold.h"

#include "gpio_helper.h"
#include "spiffs_helper.h"
#include "display_helper.h"

// #define	INTERVAL 400
// #define WAIT vTaskDelay(INTERVAL)

static const char *TAG = "MAIN";

char *dickbutt[] = {
	"/spiffs/00001.bmp",
	"/spiffs/00002.bmp",
	"/spiffs/00003.bmp",
	"/spiffs/00004.bmp",
	"/spiffs/00005.bmp",
	"/spiffs/00006.bmp",
	"/spiffs/00007.bmp",
	"/spiffs/00008.bmp",
	"/spiffs/00009.bmp",
	"/spiffs/00010.bmp",
	"/spiffs/00011.bmp",
	"/spiffs/00012.bmp",
	"/spiffs/00013.bmp",
	"/spiffs/00014.bmp",
	"/spiffs/00015.bmp",
	"/spiffs/00016.bmp",
	"/spiffs/00017.bmp",
	"/spiffs/00018.bmp",
	"/spiffs/00019.bmp",
	"/spiffs/00020.bmp",
	"/spiffs/00021.bmp",
	"/spiffs/00022.bmp",
	"/spiffs/00023.bmp",
	"/spiffs/00024.bmp",
	"/spiffs/00025.bmp",
};

static uint16_t speed_entries = 169;

uint16_t speed_array[169][2] = {
	{1,59}, {1,58}, {1,57}, {1,56}, {2,55}, {2,54}, {2,53}, {2,52}, 
	{2,51}, {2,50}, {3,49}, {3,48}, {3,47}, {3,46}, {4,45}, {4,44}, 
	{4,43}, {5,42}, {5,41}, {5,40}, {6,39}, {6,38}, {7,37}, {7,36}, 
	{8,35}, {8,34}, {9,33}, {9,32}, {10,31}, {11,30}, {11,29}, {12,28}, 
	{13,27}, {13,26}, {14,25}, {15,24}, {16,23}, {16,22}, {17,21}, {18,20}, 
	{19,19}, {20,18}, {21,17}, {22,16}, {23,16}, {24,15}, {25,14}, {26,13}, 
	{27,13}, {28,12}, {29,11}, {30,11}, {31,10}, {32,9}, {33,9}, {34,8}, 
	{35,8}, {36,7}, {37,7}, {38,6}, {39,6}, {40,5}, {41,5}, {42,5}, 
	{43,4}, {44,4}, {45,4}, {46,3}, {47,3}, {48,3}, {49,3}, {50,2}, 
	{51,2}, {52,2}, {53,2}, {54,2}, {55,2}, {56,1}, {57,1}, {58,1}, 
	{59,1}, {60,1}, {61,1}, {62,1}, {63,1}, {64,1}, {65,1}, {66,1}, 
	{67,1}, {68,1}, {69,1}, {70,1}, {71,2}, {72,2}, {73,2}, {74,2}, 
	{75,2}, {76,2}, {77,3}, {78,3}, {79,3}, {80,3}, {81,4}, {82,4}, 
	{83,4}, {84,5}, {85,5}, {86,5}, {87,6}, {88,6}, {89,7}, {90,7}, 
	{91,8}, {92,8}, {93,9}, {94,9}, {95,10}, {96,11}, {97,11}, {98,12}, 
	{99,13}, {100,13}, {101,14}, {102,15}, {103,16}, {104,16}, {105,17}, {106,18},
	{107,19}, {108,20}, {109,21}, {110,22}, {110,23}, {111,24}, {112,25}, {113,26}, 
	{113,27}, {114,28}, {115,29}, {115,30}, {116,31}, {117,32}, {117,33}, {118,34}, 
	{118,35}, {119,36}, {119,37}, {120,38}, {120,39}, {121,40}, {121,41}, {121,42}, 
	{122,43}, {122,44}, {122,45}, {123,46}, {123,47}, {123,48}, {123,49}, {124,50}, 
	{124,51}, {124,52}, {124,53}, {124,54}, {124,55}, {125,56}, {125,57}, {125,58}, 
	{125,59}
};

/*

I imagine this driver is going to need some some better state machines.
tasks that are not waiting to do something should be a higher priority and have discrete jobs to complete
I need to use interrupt handlers better to handle high priority stuff

*/

TFT_t dev;

static QueueHandle_t gpio_evt_queue = NULL;

static QueueHandle_t task_switch_queue = NULL;

static SemaphoreHandle_t dickbutt_sem;
static SemaphoreHandle_t speedometer_sem;

// Suspend current task if suspend queue is received
// Returns true if the barrier returns from being blocked
bool barrier(SemaphoreHandle_t current_sem) {
    uint32_t io_num;

    // Wait for GPIO event, checking for a message first and moving on otherwise
    // The queue needs to return something, so humor it
	if(uxQueueMessagesWaiting(gpio_evt_queue) &&
	   xQueueReceive(gpio_evt_queue, &io_num, 0)) {
	   	
	   	// Tell the lower priority switcher there is work to do
        xQueueSend(task_switch_queue, &current_sem, NULL);
		// The current task will wait until called again
	   	xSemaphoreTake(current_sem, portMAX_DELAY);

	   	return true;
	}
	return false;
}

static void task_switch(void *pvParameters)
{
    SemaphoreHandle_t c_sem;

    for(;;) {
        xQueueReceive(task_switch_queue, &c_sem, portMAX_DELAY);

        if (c_sem == dickbutt_sem) {
        	ESP_LOGI(TAG, "00");
    		xSemaphoreGive(speedometer_sem);
        }

        else if (c_sem == speedometer_sem) {
        	ESP_LOGI(TAG, "01");
    		xSemaphoreGive(dickbutt_sem);
    	}

    	else {
        	ESP_LOGI(TAG, "PROBLEM");
    	}
    }
}

void speedometer(void *pvParameters)
{
	uint16_t cx_center = 63;
	uint16_t cy_center = 63;

	uint16_t ax_center = 62;
	uint16_t ay_center = 59;

	uint16_t t_pos = 0;
	bool forward = true;

	// Wait
	// xSemaphoreTake(speedometer_sem, portMAX_DELAY);

	lcdFillScreen(&dev, WHITE);

	uint8_t test_berf[] = {0xFC, 0x35, 0xFC, 0x3F, 0xFC, 0x35, 0xFC, 0x3F, 0x79, 0x1C, 0x66, 0x6B, 0x43, 0x1D, 0xF4, 0xB2};
	for(int i = 0; i < 16; i++) {
		dev._buffer[i] = test_berf[i];
	}
	// lcdDrawCircle(&dev, cx_center, cy_center, 62, BLACK);
	// lcdWriteBuffer(&dev);

	for(;;) {
		if(barrier(speedometer_sem)) {
			// Only needs to be drawn once
			lcdFillScreen(&dev, WHITE);
		}

		lcdDrawFillArrow(&dev, ax_center, ay_center, speed_array[t_pos][0], speed_array[t_pos][1], 2, BLACK);
	 	// write to screen
	 	lcdWriteBuffer(&dev);
	 	// delete last triagle in buffer
		lcdDrawFillArrow(&dev, ax_center, ay_center, speed_array[t_pos][0], speed_array[t_pos][1], 2, WHITE);

 		if (forward) {
 			t_pos++;
 			if (t_pos >= speed_entries) {
 				t_pos = speed_entries - 2;
 				forward = false;
 			}
 		}
 		else {
 			t_pos--;
 			if (t_pos >= speed_entries) {
 				t_pos = 1;
 				forward = true;
 			}
 		}
	}
}

void dickbutt_task(void *pvParameters)
{
	char file[32];

	// Wait
	xSemaphoreTake(dickbutt_sem, portMAX_DELAY);

	for(;;) {
		for (uint32_t i = 0; i < 25; i++) {
			barrier(dickbutt_sem);

			strcpy(file, dickbutt[i]);
			BMPTest(&dev, file, CONFIG_WIDTH, CONFIG_HEIGHT, true);
		}
	}
}

void app_main(void)
{
	spiffs_init();
	display_init(&dev);

	task_switch_queue = xQueueCreate(1, sizeof(SemaphoreHandle_t));	
	gpio_evt_queue = xQueueCreate(1, sizeof(uint32_t));	

	dickbutt_sem= xSemaphoreCreateBinary();
	speedometer_sem = xSemaphoreCreateBinary();

	ESP_LOGI(TAG, "Initializing GPIO (BOOT) interrupt");
	gpio_interrupt_init(&gpio_evt_queue);

	ESP_LOGI(TAG, "Spinning up dickbutt task");
	xTaskCreate(dickbutt_task, "dickbutt", 1024*6, NULL, 9, NULL);

	ESP_LOGI(TAG, "Spinning up speedometer task");
	xTaskCreate(speedometer, "speedometer", 1024*6, NULL, 9, NULL);

	// This task will run if the other tasks are sleeping
	// Need to be a lower priority to ensure it doesn't prempt the others
	ESP_LOGI(TAG, "Spinning up task_switch task");
    xTaskCreate(task_switch, "task_switch", 2048, NULL, 8, NULL);

	xSemaphoreGive(speedometer_sem);

    while(1) {
        vTaskDelay(portMAX_DELAY);
    }
}
