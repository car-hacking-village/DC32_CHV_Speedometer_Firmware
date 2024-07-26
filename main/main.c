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
#include "can_helper.h"
#include "speedometer.h"
// #include "font6x8.h"
// #include "font8x8.h"
// #include "font8x8_bold.h"

#include "gpio_helper.h"
#include "spiffs_helper.h"
#include "speedometer_helper.h"
#include "display_helper.h"
#include "catface_helper.h"

// #define	INTERVAL 400
// #define WAIT vTaskDelay(INTERVAL)

static const char *MAIN_TAG = "MAIN";

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

static uint32_t current_task = 0;

static QueueHandle_t gpio_evt_queue = NULL;

// static TaskHandle_t catface_t = NULL;
static TaskHandle_t dickbutt_t = NULL;
static TaskHandle_t speedometer_t = NULL;

TFT_t g_dev;

// Soft copy of the device
TFT_t * copyDisplayInstance(void)
{
	TFT_t * t_dev = malloc(sizeof(TFT_t));
	memcpy(t_dev, &g_dev, sizeof(TFT_t));

	uint8_t * buffer = (uint8_t*)malloc(g_dev._blen);
	// TODO: REMOVE
	ESP_LOGI(MAIN_TAG, "buffer allocation %x", buffer);
	t_dev->_buffer = buffer;

	return t_dev;
}

// send CAN to the task that is currently active
// each helper has it's own unique queue 
void can_share_task(void *pvParameters)
{
	twai_message_t rx_msg;
	
	spd_can_queue = xQueueCreate(1, sizeof(twai_message_t));
	cat_can_queue = xQueueCreate(1, sizeof(twai_message_t));
	
	QueueHandle_t task_quwu[] = {spd_can_queue, cat_can_queue, NULL};

	twai_init();

	ESP_ERROR_CHECK(twai_start());
    ESP_LOGI(MAIN_TAG, "TWAI driver started");

	for(;;) {
		can_recv(&rx_msg);
    	// ESP_LOGI(MAIN_TAG, "%d %s", current_task, (task_quwu[current_task] != NULL)?"true":"false");
		if (task_quwu[current_task] != NULL) {
    		// ESP_LOGI(MAIN_TAG, "Giving msg to %d", current_task);
			xQueueSend(task_quwu[current_task], &rx_msg, portMAX_DELAY);
		}
  	}
}

static void task_switch(void *pvParameters)
{
    uint32_t dummy_var;

    twai_message_t dummy_can = {
    	.data_length_code = 0xff,
    };

 	TaskHandle_t task_list[] = {speedometer_t, catface_t, dickbutt_t};

    vTaskResume(task_list[current_task]);

    for(;;) {
	   	if (xQueueReceive(gpio_evt_queue, &dummy_var, portMAX_DELAY)) {

	   		// Check if the HW is currently in use, priority inversion will occur is so
	   		xSemaphoreTake(display_lock, portMAX_DELAY);
	   		xSemaphoreGive(display_lock);

	    	vTaskSuspend(task_list[current_task]);

	    	// Send a NULL CAN message to kick the task helper off and wait to send a message
	    	// May need to time out on this
	    	xQueueSend(can_rx_queue, &dummy_can, portMAX_DELAY);

	    	current_task = (current_task + 1) % NUM_MAIN_TASKS;
	        ESP_LOGI(MAIN_TAG, "Switching task to %d", current_task);
	    	vTaskResume(task_list[current_task]);
	    }
	}
}

void speedometer(void *pvParameters)
{
	TFT_t * l_dev =  copyDisplayInstance();

	vTaskSuspend(NULL);

	lcdFillScreen(l_dev, WHITE);

	// Should never return
	speedometer_helper(l_dev);
}

void dickbutt_task(void *pvParameters)
{
	char file[32];

	TFT_t * l_dev = copyDisplayInstance();

	vTaskSuspend(NULL);

	for(;;) {
		for (uint32_t i = 0; i < 25; i++) {
			// No need for clearing the screen as the animation will over write the entire screen

			strcpy(file, dickbutt[i]);
			BMPTest(l_dev, file, CONFIG_WIDTH, CONFIG_HEIGHT, true);
		}
	}
}

void catface_task(void *pvParameters)
{
	TFT_t * l_dev =  copyDisplayInstance();

	vTaskSuspend(NULL);

	// This shold never return
	catface_helper(l_dev);
}

/*
void cargotchi_task(void *pvParameters)
{
		Monsters are born as babies
			- idle (2 states, bouncing around the screen)
			- must be given gas to turn into adult

		there are going to be several states that the 'monster' can be in at any point of time
			- idle 100% (2 states, bouncing around the screen)
			- idle 50% (2 states, bouncing around the screen)
			- idle 25% (2 states, bouncing around the screen)
			- healing (2 states, drinking gas, kinda chugging it)
			- fighting (1 state, attacking)
			- being hit (1 state, being attacked)
			- dead (2 state with little ghost coming out)
			- victory (after winning)

		The monster must have persistent health
		on death they revert to a baby after hatching

		state is stored on file system?
}
*/

void app_main(void)
{
	// Lock for display
	display_lock = xSemaphoreCreateBinary();
	xSemaphoreGive(display_lock);

	can_read_lock = xSemaphoreCreateBinary();
	xSemaphoreGive(display_lock);

	spiffs_init();
	display_init(&g_dev);

	gpio_evt_queue = xQueueCreate(1, sizeof(uint32_t));	

	ESP_LOGI(MAIN_TAG, "Initializing GPIO (BOOT) interrupt");
	gpio_interrupt_init(&gpio_evt_queue);

	ESP_LOGI(MAIN_TAG, "Spinning up dickbutt task");
	xTaskCreate(dickbutt_task, "dickbutt", 1024*6, NULL, DIK_TASK_PRIO, &dickbutt_t);

	ESP_LOGI(MAIN_TAG, "Spinning up catface task");
	xTaskCreate(catface_task, "catface", 1024*6, NULL, CAT_TASK_PRIO, &catface_t);

	ESP_LOGI(MAIN_TAG, "Spinning up speedometer task");
	xTaskCreate(speedometer, "speedometer", 1024*6, NULL, SPD_TASK_PRIO, &speedometer_t);

	ESP_LOGI(MAIN_TAG, "Spinning up can_share_task task");
    // xTaskCreate(can_share_task, "can_share_task", 2048, NULL, CSH_TASK_PRIO, NULL);
    xTaskCreate(can_share_task, "can_share_task", 2048, NULL, CTRL_TSK_PRIO, NULL);

	// This task will run if the other tasks are sleeping
	// Need to be a lower priority to ensure it doesn't prempt the others
	ESP_LOGI(MAIN_TAG, "Spinning up task_switch task");
    xTaskCreate(task_switch, "task_switch", 2048, NULL, SWT_TASK_PRIO, NULL);

    while(1) {
        vTaskDelay(portMAX_DELAY);
    }
}
