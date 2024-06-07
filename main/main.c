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
#include "catface_helper.h"

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

static QueueHandle_t gpio_evt_queue = NULL;

static TaskHandle_t catface_t = NULL;
static TaskHandle_t dickbutt_t = NULL;
static TaskHandle_t speedometer_t = NULL;

TFT_t g_dev;

TFT_t * copyDisplayInstance(void)
{
	TFT_t * t_dev = malloc(sizeof(TFT_t));
	memcpy(t_dev, &g_dev, sizeof(TFT_t));

	uint8_t * buffer = (uint8_t*)malloc(g_dev._blen);
	t_dev->_buffer = buffer;

	return t_dev;
}

#define NUM_MAIN_TASKS 3

static void task_switch(void *pvParameters)
{
    uint32_t io_num;

    uint32_t c_task = 0;
 	TaskHandle_t tasks[] = {speedometer_t, catface_t, dickbutt_t};

    vTaskResume(tasks[c_task]);

    for(;;) {
	   	if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {

	   		// Check if the HW is currently in use
	        // ESP_LOGI(TAG, "Checking if display is in use...");
	   		xSemaphoreTake(display_lock, portMAX_DELAY);
	   		xSemaphoreGive(display_lock);

	    	vTaskSuspend(tasks[c_task]);
	    	c_task = (c_task + 1) % NUM_MAIN_TASKS;
	        ESP_LOGI(TAG, "Switching task to %d", c_task);
	    	vTaskResume(tasks[c_task]);
	    }
	}
}

void speedometer(void *pvParameters)
{
	// uint16_t cx_center = 63;
	// uint16_t cy_center = 63;

	uint16_t ax_center = 62;
	uint16_t ay_center = 59;

	uint16_t t_pos = 0;
	bool forward = true;

	TFT_t * l_dev =  copyDisplayInstance();

	vTaskSuspend(NULL);

	// As this is the first task this needs to be here, could prolly move this to the main function
	lcdFillScreen(l_dev, WHITE);

	// Test (which works) this displays hex in pixels on the screen, use this later for displaying the flag
	uint8_t test_berf[] = {0xFC, 0x35, 0xFC, 0x3F, 0xFC, 0x35, 0xFC, 0x3F, 0x79, 0x1C, 0x66, 0x6B, 0x43, 0x1D, 0xF4, 0xB2};
	for(int i = 0; i < 16; i++) {
		l_dev->_buffer[i] = test_berf[i];
	}

	for(;;) {
		lcdDrawFillArrow(l_dev, ax_center, ay_center, speed_array[t_pos][0], speed_array[t_pos][1], 2, BLACK);
	 	// write to screen
	 	lcdWriteBuffer(l_dev);
	 	// delete last triagle in buffer without drawing, we can save a write doing this
		lcdDrawFillArrow(l_dev, ax_center, ay_center, speed_array[t_pos][0], speed_array[t_pos][1], 2, WHITE);

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

	TFT_t * l_dev = copyDisplayInstance();

	// Wait
	vTaskSuspend(NULL);

	for(;;) {
		for (uint32_t i = 0; i < 25; i++) {
			// No need for clearing the screen as the animation will over write the entire screen
			// barrier(dickbutt_sem);

			strcpy(file, dickbutt[i]);
			BMPTest(l_dev, file, CONFIG_WIDTH, CONFIG_HEIGHT, true);
		}
	}
}

void catface_task(void *pvParameters)
{
	// Soft copy of the device
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

// Have a CAN task that is high priority that keeps state data for the speedometer and cat face
// it's priority will be the highest, but waiting for a CAN messages by pending on a queue
// it's helpers that actually wait for data will be a level lower but just wait for CAN data

// each state that cares about CAN messages can check to see if there is a message it cares about in a dedicated
// task that it stops 



void app_main(void)
{
	spiffs_init();
	display_init(&g_dev);

	gpio_evt_queue = xQueueCreate(1, sizeof(uint32_t));	

	ESP_LOGI(TAG, "Initializing GPIO (BOOT) interrupt");
	gpio_interrupt_init(&gpio_evt_queue);

	ESP_LOGI(TAG, "Spinning up dickbutt task");
	xTaskCreate(dickbutt_task, "dickbutt", 1024*6, NULL, 9, &dickbutt_t);

	ESP_LOGI(TAG, "Spinning up catface task");
	xTaskCreate(catface_task, "catface", 1024*6, NULL, 9, &catface_t);

	ESP_LOGI(TAG, "Spinning up speedometer task");
	xTaskCreate(speedometer, "speedometer", 1024*6, NULL, 9, &speedometer_t);

	// This task will run if the other tasks are sleeping
	// Need to be a lower priority to ensure it doesn't prempt the others
	ESP_LOGI(TAG, "Spinning up task_switch task");
    xTaskCreate(task_switch, "task_switch", 2048, NULL, 10, NULL);

    while(1) {
        vTaskDelay(portMAX_DELAY);
    }
}
