/*
 * can_helper.c
 *
 *  Created on: Jul 12, 2023
 *      Author: ericpoole
 */

#include "can_helper.h"

QueueHandle_t can_rx_queue;
static QueueHandle_t can_tx_queue;

static const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

SemaphoreHandle_t can_read_lock;

static void twai_receive_task(void *arg)
{
	twai_message_t rx_msg;

	while (true) {
    	if (twai_receive(&rx_msg, portMAX_DELAY) != ESP_ERR_TIMEOUT)
    	{
			// print("[+] RECV: {}".format(binascii.hexlify(bytes(data))));
			// print("[+] RECV: {}".format(data));
			// ESP_LOGI(TWAI_TAG, "RECV msg");

			xQueueSend(can_rx_queue, &rx_msg, portMAX_DELAY);
    	}
    }
}

static void twai_transmit_task(void *arg)
{
    while (true) {
    	twai_message_t tx_msg;
        if (xQueueReceive(can_tx_queue, &tx_msg, portMAX_DELAY) == pdTRUE)
        {
			ESP_LOGI(TWAI_TAG, "SEND msg");
        	twai_transmit(&tx_msg, portMAX_DELAY);
        }
    }
}

void twai_init(void)
{
	// TODO: This ends up reading 10 false CAN messages... w/e
	can_rx_queue = xQueueCreate(10, sizeof(twai_message_t));
	can_tx_queue = xQueueCreate(10, sizeof(twai_message_t));

	xTaskCreatePinnedToCore(twai_receive_task, "TWAI_rx", 4096, NULL, RX_TASK_PRIO, NULL, tskNO_AFFINITY);
	xTaskCreatePinnedToCore(twai_transmit_task, "TWAI_tx", 4096, NULL, TX_TASK_PRIO, NULL, tskNO_AFFINITY);
	// Messy, but syncronization is needed in the main task, it doesn't need to be that pretty
	// xTaskCreatePinnedToCore(twai_control_task, "TWAI_ctrl", 4096, NULL, CTRL_TSK_PRIO, NULL, tskNO_AFFINITY);

	ESP_LOGI(TWAI_TAG, "Spinning up CAN");

	ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
}

// CAN Interface to make our lives easier

/*
   uint32_t identifier;                //< 11 or 29 bit identifier
    uint8_t data_length_code;          //< Data length code
    uint8_t data[8];
*/
//static void can_send(twai_message_t *msg, uint32_t arbid)
void can_send(uint32_t arbid, uint32_t extd, uint32_t dlc, uint8_t *data)
{
	twai_message_t msg;
	msg.identifier = arbid;
	msg.extd	   = extd;
	msg.data_length_code = dlc;

	// The structure uses a hard coded array, write the passed data into here
	for (int i = 0; i < 8; i++)
	{
		msg.data[i] = data[i];

	}

	xQueueSend(can_tx_queue, &msg, portMAX_DELAY);
}


//static bool can_recv(twai_message_t *msg, uint32_t arbid)
bool can_recv(twai_message_t *msg)
{
	bool ret = false;

	while(1)
	{
		// xSemaphoreTake(can_read_lock, portMAX_DELAY);

		// Cycle through the queue unitl we get a message or something blows up
		ret = xQueueReceive(can_rx_queue, msg, portMAX_DELAY);

		// xSemaphoreGive(can_read_lock);
		
		if (ret) return true;
		else return false;
	}
}


