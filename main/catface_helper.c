
// helper file for catface
#include "catface_helper.h"

static uint32_t state_tick = 0;
static uint32_t cat_state = CAT_IDLE;
// static uint32_t cat_expression = EYES_CLOSED;

TaskHandle_t catface_t = NULL;
static TaskHandle_t catface_can_t;

QueueHandle_t cat_can_queue = NULL;

char * rps_str[] = {
	"Rock",
	"Paper",
	"Sicsors",
};

// Non-static to keep in correctr memory range
char* rps_chars[] = {"Rock",
					 "Paper",
					 "Scisors",
					 "Shoot!",
					} ;

// static uint32_t speed = 0;
static uint8_t effect = 0;

static bool amSender 		= true;
static uint8_t my_id;
static uint8_t target_id 	= 0;
static int16_t my_rps 		= RPS_NONE;
static int16_t their_rps 	= RPS_NONE;
static uint8_t score[4];

static uint32_t rps_i = 0;
#define	RPS_RNDS = 3;
static uint8_t rps_rnd = 0;

void logic_as_sender(twai_message_t rx_msg)
{
	switch (rx_msg.data[0]){
	// We hear a ping response, we are now the sender, begin challenge
	case BATTL_PRSP:
		// amSender = true;
		target_id = rx_msg.data[1];
		// Goto challenge
		cat_state = CAT_CHAL;
		break;

	case BATTL_CRSP:
		// Goto rock paper scissors
		rps_rnd = 0;
		score[0] = 'o';
		score[1] = 'o';
		score[2] = 'o';

		cat_state = CAT_RSPS;
		break;

	case BATTL_RRSP:
		their_rps = rx_msg.data[2] << 8 | rx_msg.data[3];
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
		if (rx_msg.data[1] == my_id) return;
		ESP_LOGI(CAT_TAG, "RCV a ping, switching to receiver");
		target_id = rx_msg.data[1];;
		amSender = false;
		send_ping(t_arb, BATTL_PRSP, my_id);
		// Goto challenge
		cat_state = CAT_CHAL;
		break;

	// TODO: use our IDs to obfuscate the traffic
	case BATTL_CSND:
		if (rx_msg.data[1] != my_id) {
			ESP_LOGI(CAT_TAG, "BATTL_CSND: ID did not match");
			return;
		}

		// TODO: send key to modify traffic
		send_challenge(t_arb, target_id, BATTL_CRSP);

		// Goto rock paper scissors
		rps_rnd = 0;
		score[0] = 'o';
		score[1] = 'o';
		score[2] = 'o';

		cat_state = CAT_RSPS;
		// TODO: kick the display task, may need to make another thread for it
		vTaskResume(catface_t);

		break;

	case BATTL_RSND:
		if (rx_msg.data[1] != my_id) {
			ESP_LOGI(CAT_TAG, "BATTL_RSND: ID did not match");
			return;
		}

		// Make sure our value is not the same
		// We can cheat here
		their_rps = rx_msg.data[2] << 8 | rx_msg.data[3];

// #ifndef BOSSBADGE
		my_rps = get_rps();

		while (my_rps == their_rps)
			my_rps = get_rps();
		// For debugging
		// my_rps = 0x2A5;

		send_rps(BATTL_ARBID_SENDR, target_id, BATTL_RRSP, my_rps);

		break;
	}
}

void catface_can_helper(void *pvParameters)
{
	twai_message_t rx_msg;

	for (;;) {
		if (xQueueReceive(cat_can_queue, &rx_msg, portMAX_DELAY)) {
			// ESP_LOGI(CAT_TAG, "Got message: arbid = 0x%x", rx_msg.identifier);
			// This is basically impossible unless the payload was borked with
			if (rx_msg.data_length_code == 0xff) {
				continue;
			}
			switch (rx_msg.identifier) {

				// case SPEED_ARBID:
				// 	speed = rx_msg.data[0] << 24 | (rx_msg.data[1] << 16) | (rx_msg.data[2] << 8) | (rx_msg.data[3]);
				// 	break;

				// We got a message from a RECVR target
				case BATTL_ARBID_SENDR:
					// ESP_LOGI(CAT_TAG, "am sender");
					logic_as_sender(rx_msg);
					break;

				// We got a message from a SENDR target
				case BATTL_ARBID_RECVR:
					// ESP_LOGI(CAT_TAG, "am receiver");
					logic_as_receiver(rx_msg);
					break;

				default:
					break;
			}
		}
	}
}

static uint32_t l_state;

void state_handler()
{
	uint32_t t_arb = BATTL_ARBID_RECVR;

	// ESP_LOGI(CAT_TAG, "amSender: %d", amSender);

	switch(cat_state) {
	// When idle, send ping with id after every blink until ping response with id
	case CAT_IDLE:
		if (amSender){
			// ESP_LOGI(CAT_TAG, "Sending ping as sender");
			send_ping(t_arb, BATTL_PSND, my_id);
		}
		// return a expression
		break;

	case CAT_CHAL:
		// Only send once
		if (amSender && (l_state != cat_state)) {
			ESP_LOGI(CAT_TAG, "Sending challenge as sender");
			send_challenge(t_arb, target_id, BATTL_CSND);
		}
		// return a expression
		break;

	case CAT_RSPS:
		// Only send once
		if (amSender && (l_state != cat_state)) {
			ESP_LOGI(CAT_TAG, "Sending rps as sender");
			my_rps = get_rps();
			// For debugging
			// my_rps = 0x2A5;
			send_rps(t_arb, target_id, BATTL_RSND, my_rps);
		}
		// return a expression
		break;

	}
	// Remember the last state
	l_state = cat_state;
}

void change_expression(TFT_t * dev, uint8_t * expression, uint32_t effect)
{
	uint32_t xpos = 0;

	lcdSetFontDirection(dev, DIRECTION0);

	// Complete one calulation and return, save state persitantly to survive thread being stopped
	xpos = (CONFIG_WIDTH - (strlen((char *)expression) * 32)) / 2;
	lcdDrawString2(dev, font32x32, xpos, 50+10, expression, BLACK);
	lcdWriteBuffer(dev);
	lcdDrawString2(dev, font32x32, xpos, 50+10, expression, WHITE);
}

void eye_sparkle(TFT_t * dev)
{
	uint8_t ascii[4];

	long strt_t = xTaskGetTickCount();
	long wait_t = (esp_random() % (1 * 1000)) ^ 1000;

	int i = 0;
	int j = 1;

	do {
		ascii[0] = SYM_32_OID1 + (i % 3);
		ascii[1] = SYM_32_SPAC;
		ascii[2] = SYM_32_OID1 + (j % 3);
		ascii[3] = SYM_32_NULL;

		change_expression(dev, ascii, NO_EFFECT);

		i++;
		j++;

	} while ((xTaskGetTickCount() < (strt_t + wait_t)) && (cat_state == CAT_IDLE));
}

/*
	Each state, besides the eyes being closed, keep a random amount of time they keep track of
	when the tick could expires the eyes close for a set amount and a random state is entered
	again.

	We do not want to hold up a button press, so the tick count is checked quickly, and if the
	image is the same then do not bother redrawing it.
*/

// main task responsible for set up and keeping face active

void cat_idle_state(TFT_t * dev)
{
	uint8_t ascii[4];

	static uint32_t idle_expression = EYES_CLOSED;

	if (idle_expression == EYES_CLOSED) {
			// blink is always the last defined enum so the states will always be size of possbile states - 1
			idle_expression = esp_random() % EYES_CLOSED;
			// This is where a cat face state helper could be queried
			// idle_expression = get_global_cat_state();
		}
		else idle_expression = EYES_CLOSED;

		switch(idle_expression)
		{
		case EYES_OPEN_FOWARD:
			eye_sparkle(dev);
			return;
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
		case EYES_OPEN_ANGRY:
			// 1,000,000 is about 1.0s
			state_tick = (esp_random() % (4 * 1000)) ^ 600;
			ascii[0] = SYM_32_ANGL;
			ascii[1] = SYM_32_SPAC;
			ascii[2] = SYM_32_ANGR;
			ascii[3] = SYM_32_NULL;
			break;
		case EYES_OPEN_REAL:
			// 1,000,000 is about 1.0s
			state_tick = (esp_random() % (4 * 1000)) ^ 600;
			ascii[0] = SYM_32_RELL;
			ascii[1] = SYM_32_SPAC;
			ascii[2] = SYM_32_RELR;
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

void lcdDrawCentered(TFT_t * dev, uint8_t * font, uint16_t x_offset, uint16_t y, uint8_t * ascii, uint8_t color)
{
	uint32_t xpos = 0;

	xpos = ((CONFIG_WIDTH - (strlen((char *)ascii) * 8)) / 2) + x_offset;
	lcdDrawString2(dev, font, xpos, y, 	ascii, color);
}

void cat_rps_state(TFT_t * dev)
{

	// this breaks stuff but seems to be able to effect sync
	if (my_rps == RPS_NONE || their_rps == RPS_NONE){
		l_state = CAT_SYNC;
		return;
	}

	// what happens if this is !=? read information out to the screen? hmmmmm
	if (rps_i < 4) {
		uint8_t t_str[12];
		strcpy((char *)t_str, rps_chars[rps_i]);

		lcdDrawCentered(dev, font8x16, 0, 50, t_str, BLACK);
		lcdWriteBuffer(dev);
		lcdDrawCentered(dev, font8x16, 0, 50, t_str, WHITE);
		
		rps_i++;
	}

	state_tick = 500;
	vTaskDelay(state_tick / portTICK_PERIOD_MS);

	// wait to send message until animation done
	if (rps_i == 4) {
		rps_i = 0;
		
		// // If values are not valid, try again
		// if (my_rps == RPS_NONE || their_rps == RPS_NONE){
		// 	ESP_LOGI(CAT_TAG, "my_rps %x, my_rps %x", my_rps, their_rps);
		// 	// Send a restart?
		// 	if (amSender) {
		// 		// send message again if sender
		// 		l_state = CAT_SYNC;
		// 	}
		// 	return;
		// }
		cat_state = CAT_RPSR;
	}
}

static bool firstLose 	= false;
static bool firstWin 	= false;

/*
	We do not check the bounds of 'their_rps', so add another screen
	showing the result of the match, "rock beats scisors", the sender
	can send anything which can the size of the type (make it large)
	Make the buffers for the display in bss and they can be offset to
	from the value.
*/

void cat_rpsr_state(TFT_t * dev)
{
	uint8_t my_str[12];
	uint8_t w_or_l[12];
	uint8_t their_str[12];

	// ESP_LOGI(CAT_TAG, "rps_str allocation %x", rps_str);
	// ESP_LOGI(CAT_TAG, "rps_str + their_rps %x", rps_str + their_rps);
	// ESP_LOGI(CAT_TAG, "rps_str + their_rps stores %x", rps_str[their_rps]);

	// Skrew you pal!
	if ((void *)rps_str[their_rps] < (void *)0x3ffc0000) their_rps = their_rps % 3;

	// make global and modify as it goes, reseting the winner
	if (didWinRPS(my_rps, their_rps)) {

		// 'o' but filled
		score[rps_rnd] = 0x80;

		if (firstWin == true && rps_rnd >= 1) {
			cat_state = CAT_VICT;
		}
		strcpy((char *)my_str, rps_str[my_rps]);
		strcpy((char *)w_or_l, "beats");
		strcpy((char *)their_str, rps_str[their_rps]);
		firstWin = true;
	}
	else {
		// 'x'
		score[rps_rnd] = 0x78;

		if (firstLose == true && rps_rnd >= 1) {
			cat_state = CAT_DEDD;
		}
		strcpy((char *)my_str, rps_str[my_rps]);
		strcpy((char *)w_or_l, "loses to");
		strcpy((char *)their_str, rps_str[their_rps]);
		firstLose = true;
	}


	lcdDrawCentered(dev, font8x16, 0, 40-18, my_str, BLACK);
	lcdDrawCentered(dev, font8x16, 0, 40, 	 w_or_l, BLACK);
	lcdDrawCentered(dev, font8x16, 0, 40+18, their_str, BLACK);
	lcdDrawString2(dev, font8x8, CONFIG_WIDTH-(8*3)-8, CONFIG_HEIGHT-4, score, BLACK);

	lcdWriteBuffer(dev);
	
	lcdDrawCentered(dev, font8x16, 0, 40-18, my_str, WHITE);
	lcdDrawCentered(dev, font8x16, 0, 40, 	 w_or_l, WHITE);
	lcdDrawCentered(dev, font8x16, 0, 40+18, their_str, WHITE);	
	lcdDrawString2(dev, font8x8, CONFIG_WIDTH-(8*3)-8, CONFIG_HEIGHT-4, score, WHITE);
	
	state_tick = 4000;
	vTaskDelay(state_tick / portTICK_PERIOD_MS);

	// One more round
	if (cat_state == CAT_RPSR){
		l_state = CAT_RPSR;
		cat_state = CAT_RSPS;
	}

	rps_rnd++;
}

void cat_vict_state(TFT_t * dev)
{
	uint32_t xpos = 0;
	uint8_t t_str[12];

	strcpy((char *)t_str, "Cat-tastic!");

	xpos = (CONFIG_WIDTH - (strlen((char *)t_str) * 8)) / 2;
	lcdDrawString2(dev, font8x16, xpos, 50, t_str, BLACK);
	lcdWriteBuffer(dev);
	lcdDrawString2(dev, font8x16, xpos, 50, t_str, WHITE);
	
	state_tick = 4000;
	vTaskDelay(state_tick / portTICK_PERIOD_MS);

	cat_state = CAT_IDLE;
}

// We can make ghosty here
void cat_dedd_state(TFT_t * dev)
{
	uint32_t xpos = 0;
	uint8_t t_str[12];

	// Stay "dedd"
	vTaskSuspend(catface_can_t);

	strcpy((char *)t_str, "boo hoo!");

	xpos = (CONFIG_WIDTH - (strlen((char *)t_str) * 8)) / 2;
	lcdDrawString2(dev, font8x16, xpos, 50, t_str, BLACK);
	lcdWriteBuffer(dev);
	lcdDrawString2(dev, font8x16, xpos, 50, t_str, WHITE);
	
	state_tick = 4000;
	vTaskDelay(state_tick / portTICK_PERIOD_MS);
}

void catface_helper(TFT_t * dev)
{
	// from cat_battle_msg, random id at startup
	my_id = esp_random() % (256);
	ESP_LOGI(CAT_TAG, "My id is 0x%x", my_id);

	lcdFillScreen(dev, WHITE);

	ESP_LOGI(CAT_TAG, "Spinning up speedometer can helper task");
	xTaskCreate(catface_can_helper, "catface_can_helper", 1024*6, NULL, CAT_CAN_HLP_PRIO, &catface_can_t);

	for(;;) {
		// Todo: make this a function call to different sets of animations depending on the state
		state_handler();

		switch(cat_state) {
		// case CAT_CHAL:
		// 	effect = EFFECT_BLUSH;
		case CAT_IDLE:
			firstLose = false;
			firstWin = false;
			cat_idle_state(dev);
			break;
		case CAT_RSPS:
			effect = NO_EFFECT;
			cat_rps_state(dev);
			break;
		case CAT_RPSR:
			cat_rpsr_state(dev);
			break;
		case CAT_VICT:
			cat_vict_state(dev);
			break;
		case CAT_DEDD:
			cat_dedd_state(dev);
			break;
		}
	}
}
