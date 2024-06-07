
// helper file for catface
#include "catface_helper.h"

static int32_t cat_expression = EYES_CLOSED;
static int64_t t1 = 0;
static int64_t t2 = 0;
static uint32_t state_tick = 0;

void change_expression(TFT_t * dev, uint32_t base_expression, uint32_t effect)
{
	uint32_t xpos = 0;
	uint8_t ascii[40];

	lcdSetFontDirection(dev, DIRECTION0);

	switch(base_expression)
	{
		case EYES_OPEN_FOWARD:
			ascii[0] = 0x2;
			ascii[1] = 0x1;
			ascii[2] = 0x2;
			ascii[3] = 0x0;
			break;
		case EYES_OPEN_RIGHT:
			ascii[0] = 0x5;
			ascii[1] = 0x1;
			ascii[2] = 0x5;
			ascii[3] = 0x0;
			break;
		case EYES_OPEN_LEFT:
			ascii[0] = 0x4;
			ascii[1] = 0x1;
			ascii[2] = 0x4;
			ascii[3] = 0x0;
			break;
		case EYES_CLOSED:
			ascii[0] = 0x3;
			ascii[1] = 0x1;
			ascii[2] = 0x3;
			ascii[3] = 0x0;
			break;
	}

	// Complete one calulation and return, save state persitantly to survive thread being stopped
	xpos = (CONFIG_WIDTH - (strlen((char *)ascii) * 32)) / 2;
	lcdDrawString2(dev, font32x32, xpos, 50, ascii, BLACK);
	lcdWriteBuffer(dev);
	lcdDrawString2(dev, font32x32, xpos, 50, ascii, WHITE);
}

/*
	Each state, besides the eyes being closed, keep a random amount of time they keep track of
	when the tick could expires the eyes close for a set amount and a random state is entered
	again.

	We do not want to hold up a button press, so the tick count is checked quickly, and if the
	image is the same then do not bother redrawing it.
*/

void catface_helper(TFT_t * dev)
{
	lcdFillScreen(l_dev, WHITE);

	for(;;) {
		t2 = esp_timer_get_time();
		if (t1 + state_tick <= t2) {
			
			if (cat_expression == EYES_CLOSED) {
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
					state_tick = (esp_random() % (4 * 1000000)) ^ 400000;
					change_expression(dev, EYES_OPEN_FOWARD, NO_EFFECT);
					break;
				case EYES_OPEN_RIGHT:
					// 1,000,000 is about 1.0s
					state_tick = (esp_random() % (4 * 1000000)) ^ 400000;
					change_expression(dev, EYES_OPEN_RIGHT, NO_EFFECT);
					break;
				case EYES_OPEN_LEFT:
					// 1,000,000 is about 1.0s
					state_tick = (esp_random() % (4 * 1000000)) ^ 400000;
					change_expression(dev, EYES_OPEN_LEFT, NO_EFFECT);
					break;
				case EYES_CLOSED:
					// average time of a blink is 100-150 ms
					state_tick = 150000;
					change_expression(dev, EYES_CLOSED, NO_EFFECT);
					break;
			}
		t1 = esp_timer_get_time();
		}
	}
}
