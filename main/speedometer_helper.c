
#include "speedometer_helper.h"

uint16_t speed_entries = 169;

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

// Test (which works) this displays hex in pixels on the screen, use this later for displaying the flag
// uint8_t test_berf[] = {0xFC, 0x35, 0xFC, 0x3F, 0xFC, 0x35, 0xFC, 0x3F, 0x79, 0x1C, 0x66, 0x6B, 0x43, 0x1D, 0xF4, 0xB2};
// for(int i = 0; i < 16; i++) {
// 	dev->_buffer[i] = test_berf[i];
// }

QueueHandle_t spd_can_queue = NULL;
static QueueHandle_t update_queue = NULL;


uint16_t calculate_pos_from_speed(uint32_t speed)
{
	// make sure this logic does not over flow, not a CTF problem
	// speed can be normalized by "speed/120 * (speed_entries-1)" and rounding down
	return (int)((float)speed/120 * (speed_entries-1));
}

void speedometer_can_helper(void *pvParameters)
{
	uint32_t dummy = 0;
	uint32_t speed = 0;

	twai_message_t rx_msg;

	for (;;) {
		if (xQueueReceive(spd_can_queue, &rx_msg, portMAX_DELAY)) {
			// ESP_LOGI(SPD_TAG, "Got message: arbid = 0x%x", rx_msg.identifier);
			// This is basically impossible unless the payload was borked with
			if (rx_msg.data_length_code == 0xff) {
				continue;
			}
			switch (rx_msg.identifier) {
				case SPEED_ARBID:
					speed = rx_msg.data[0] | (rx_msg.data[1] << 8) | (rx_msg.data[2] << 16) | (rx_msg.data[3] << 24);
					xQueueSend(update_queue, dummy, portMAX_DELAY);
					break;
				default:
					break;
			}
		}
	}
}

// Queue to kick the display task
void wait_for_update()
{
	uint32_t dummy = 0;

	for(;;) {
		if (xQueueReceive(update_queue, &dummy, portMAX_DELAY)) {
			return;
		}
	}
}

// TODO: may want to lock to current postion variable

void speedometer_helper(TFT_t * dev)
{
	uint16_t ax_center = 62;
	uint16_t ay_center = 59;

	uint16_t cur_pos = 0;
	uint16_t tgt_pos = speed_entries-1;

	update_queue = xQueueCreate(1, sizeof(uint32_t));

	ESP_LOGI(SPD_TAG, "Spinning up speedometer can helper task");
	xTaskCreate(speedometer_can_helper, "speedometer_can_helper", 1024*6, NULL, SPD_CAN_HLP_PRIO, NULL);	

	// make sure this logic does not over flow, not a CTF problem
	// speed can be normalized by "speed/120 * (speed_entries-1)" and rounding down

	// Init Screen
	lcdDrawFillArrow(dev, ax_center, ay_center, speed_array[0][0], speed_array[0][1], 2, BLACK);
 	lcdWriteBuffer(dev);
	lcdDrawFillArrow(dev, ax_center, ay_center, speed_array[0][0], speed_array[0][1], 2, WHITE);

	for(;;) {
		bool pos_changed = false;

 		if (cur_pos < tgt_pos) {
 			pos_changed = true;
 			cur_pos++;
 		}
 		else if (cur_pos > tgt_pos){
 			pos_changed = true;
 			cur_pos--;
 		}

 		if (pos_changed) {
			lcdDrawFillArrow(dev, ax_center, ay_center, speed_array[cur_pos][0], speed_array[cur_pos][1], 2, BLACK);
		 	lcdWriteBuffer(dev);
			lcdDrawFillArrow(dev, ax_center, ay_center, speed_array[cur_pos][0], speed_array[cur_pos][1], 2, WHITE);
 		}
		else {
			// Wait for a new position/speed to be reported
			wait_for_update();
		}
	}
}