
// helper file for catface
#include "catface_helper.h"

static uint32_t state_tick = 0;
static uint32_t cat_state = CAT_IDLE;
static uint32_t cat_expression = EYES_CLOSED;

QueueHandle_t cat_can_queue = NULL;

uint32_t speed = 0;

static bool amSender = true;
static uint8_t my_id;
static uint8_t target_id = 0;
static uint8_t my_rps = 0;

void logic_as_sender(twai_message_t rx_msg)
{
	switch (rx_msg.data[1]){
	// We hear a ping response, we are now the sender, begin challenge
	case BATTL_PRSP:
		amSender = true;
		target_id = rx_msg.data[1];
		// Goto challenge
		cat_state = CAT_CHAL;
		break;

	case BATTL_CRSP:
		// Goto rock paper scissors
		cat_state = CAT_RSPS;
		break;

	case BATTL_RRSP:
		// Determine from response if we are sending still
		if (!didWinRPS(my_rps, rx_msg.data[2]))
			amSender = false;

		// CAT BATTLE TIME
		cat_state = CAT_BATL;
		break;

	default:
		break;
	}
}

void logic_as_receiver(twai_message_t rx_msg)
{
	// Send response to sender taget
	uint32_t t_arb = BATTL_ARBID_SENDR;

	switch (rx_msg.data[0]){

	// We hear a ping send, we will now establish as a receiver 
	case BATTL_PSND:
		amSender = false;
		target_id = rx_msg.data[1];
		send_ping(t_arb, BATTL_PRSP, my_id);
		// Goto challenge
		cat_state = CAT_CHAL;
		break;

	case BATTL_CSND:
		// TODO: send key to modify traffic
		send_challenge(t_arb, target_id, BATTL_CRSP);
		// Goto rock paper scissors
		cat_state = CAT_RSPS;
		break;

	// Todo: to garuntee winning RPS you can cheat if you are the receiver
	case BATTL_RSND:
		my_rps = get_rps();

		// Make sure our value is not the same
		// We can cheat here
		while (my_rps == rx_msg.data[2])
			my_rps = get_rps();

		send_rps(t_arb, target_id, BATTL_RRSP, my_rps);

		if (didWinRPS(my_rps, rx_msg.data[2]))
			amSender = true;

		cat_state = CAT_RSPS;
		break;
	}
}

void catface_can_helper(void *pvParameters)
{
	twai_message_t rx_msg;

	for (;;) {
		ESP_LOGI(CAT_TAG, "Got message from the CAN distributer");
		if (xQueueReceive(cat_can_queue, &rx_msg, portMAX_DELAY)) {
			// This is basically impossible unless the payload was borked with
			if (rx_msg.data_length_code == 0xff) {
				continue;
			}
			switch (rx_msg.identifier) {

				case SPEED_ARBID:
					speed = rx_msg.data[0] | (rx_msg.data[1] << 8) | (rx_msg.data[2] << 16) | (rx_msg.data[3] << 24);
					break;

				// We got a message from a RECVR target
				case BATTL_ARBID_SENDR:
					logic_as_sender(rx_msg);
					break;

				// We got a message from a SENDR target
				case BATTL_ARBID_RECVR:
					logic_as_receiver(rx_msg);
					break;

				default:
					break;
			}
		}
	}
}

void state_handler()
{
	uint32_t t_arb = BATTL_ARBID_RECVR;

	switch(cat_state) {

	// When idle, send ping with id after every blink until ping response with id
	case CAT_IDLE:
		if (amSender) send_ping(t_arb, BATTL_PSND, my_id);
		// return a expression
		break;

	case CAT_CHAL:
		if (amSender) send_challenge(t_arb, target_id, BATTL_CSND);
		// return a expression
		break;

	case CAT_RSPS:
		if (amSender) {
			my_rps = get_rps();
			send_rps(t_arb, target_id, BATTL_RSND, my_rps);
		}
		// return a expression
		break;
	}
}

void change_expression(TFT_t * dev, uint8_t * expression, uint32_t effect)
{
	uint32_t xpos = 0;
	// uint8_t ascii[40];

	lcdSetFontDirection(dev, DIRECTION0);

	// Complete one calulation and return, save state persitantly to survive thread being stopped
	xpos = (CONFIG_WIDTH - (strlen((char *)expression) * 32)) / 2;
	lcdDrawString2(dev, font32x32, xpos, 50, expression, BLACK);
	lcdWriteBuffer(dev);
	lcdDrawString2(dev, font32x32, xpos, 50, expression, WHITE);
}

/*
	Each state, besides the eyes being closed, keep a random amount of time they keep track of
	when the tick could expires the eyes close for a set amount and a random state is entered
	again.

	We do not want to hold up a button press, so the tick count is checked quickly, and if the
	image is the same then do not bother redrawing it.
*/

// main task responsible for set up and keeping face active

void catface_helper(TFT_t * dev)
{
	uint8_t ascii[4];

	// from cat_battle_msg, random id at startup
	my_id = esp_random() % (sizeof(uint8_t) + 1);

	lcdFillScreen(dev, WHITE);

	ESP_LOGI(CAT_TAG, "Spinning up speedometer can helper task");
	xTaskCreate(catface_can_helper, "catface_can_helper", 1024*6, NULL, CAT_CAN_HLP_PRIO, NULL);

	for(;;) {
		// Todo: make this a function call to different sets of animations depending on the state

		if (cat_expression == EYES_CLOSED) {

			state_handler();

			// blink is always the last defined enum so the states will always be size of possbile states - 1
			cat_expression = esp_random() % EYES_CLOSED;
			// This is where a cat face state helper could be queried
			// cat_expression = get_global_cat_state();
		}
		else cat_expression = EYES_CLOSED;

		switch(cat_expression)
		{
		case EYES_OPEN_FOWARD:
			// 1,000,000 is about 1.0s
			state_tick = (esp_random() % (4 * 1000)) ^ 600;
			ascii[0] = SYM_32_OFWD;
			ascii[1] = SYM_32_SPAC;
			ascii[2] = SYM_32_OFWD;
			ascii[3] = SYM_32_NULL;
			break;
		case EYES_OPEN_RIGHT:
			// 1,000,000 is about 1.0s
			state_tick = (esp_random() % (4 * 1000)) ^ 600;
			ascii[0] = SYM_32_ORHT;
			ascii[1] = SYM_32_SPAC;
			ascii[2] = SYM_32_ORHT;
			ascii[3] = SYM_32_NULL;
			break;
		case EYES_OPEN_LEFT:
			// 1,000,000 is about 1.0s
			state_tick = (esp_random() % (4 * 1000)) ^ 600;
			ascii[0] = SYM_32_OLFT;
			ascii[1] = SYM_32_SPAC;
			ascii[2] = SYM_32_OLFT;
			ascii[3] = SYM_32_NULL;
			break;
		case EYES_CLOSED:
			// average time of a blink is 100-150 ms
			state_tick = 150;
			ascii[0] = SYM_32_CLSD;
			ascii[1] = SYM_32_SPAC;
			ascii[2] = SYM_32_CLSD;
			ascii[3] = SYM_32_NULL;
			break;
		}

		change_expression(dev, ascii, NO_EFFECT);

		vTaskDelay(state_tick / portTICK_PERIOD_MS);
	}
}
