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
	uint16_t xpos = 63;
	uint16_t ypos = 63;
	
	// Wait
	xSemaphoreTake(speedometer_sem, portMAX_DELAY);

	lcdFillScreen(&dev, WHITE);
	lcdDrawCircle(&dev, xpos, ypos, 62, BLACK);
	lcdWriteBuffer(&dev);

	for(;;) {
		if(barrier(speedometer_sem)) {
			lcdFillScreen(&dev, WHITE);
			lcdDrawCircle(&dev, xpos, ypos, 62, BLACK);
			lcdWriteBuffer(&dev);
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
