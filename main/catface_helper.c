
// helper file for catface
#include "catface_helper.h"

static void both_eyes_open_forward(TFT_t * dev)
{
	uint8_t ascii[40];

	lcdSetFontDirection(dev, DIRECTION0);
	ascii[0] = 0x2;
	ascii[1] = 0x1;
	ascii[2] = 0x2;
	ascii[3] = 0x0;
	// strcpy((char *)ascii, "\u0002\u0001\u0001\u0002");

	// Complete one calulation and return, save state persitantly to survive thread being stopped
	int xpos = (CONFIG_WIDTH - (strlen((char *)ascii) * 32)) / 2;
	lcdDrawString2(dev, font32x32, xpos, 50, ascii, BLACK);
	lcdWriteBuffer(dev);
	lcdDrawString2(dev, font32x32, xpos, 50, ascii, WHITE);
}

static void both_eyes_open_right(TFT_t * dev)
{
	uint8_t ascii[40];

	lcdSetFontDirection(dev, DIRECTION0);
	// strcpy((char *)ascii, "^  ^");
	ascii[0] = 0x5;
	ascii[1] = 0x1;
	ascii[2] = 0x5;
	ascii[3] = 0x0;

	// Complete one calulation and return, save state persitantly to survive thread being stopped
	int xpos = (CONFIG_WIDTH - (strlen((char *)ascii) * 32)) / 2;
	lcdDrawString2(dev, font32x32, xpos, 50, ascii, BLACK);
	lcdWriteBuffer(dev);
	lcdDrawString2(dev, font32x32, xpos, 50, ascii, WHITE);
}

static void both_eyes_open_left(TFT_t * dev)
{
	uint8_t ascii[40];

	lcdSetFontDirection(dev, DIRECTION0);
	// strcpy((char *)ascii, "^  ^");
	ascii[0] = 0x4;
	ascii[1] = 0x1;
	ascii[2] = 0x4;
	ascii[3] = 0x0;

	// Complete one calulation and return, save state persitantly to survive thread being stopped
	int xpos = (CONFIG_WIDTH - (strlen((char *)ascii) * 32)) / 2;
	lcdDrawString2(dev, font32x32, xpos, 50, ascii, BLACK);
	lcdWriteBuffer(dev);
	lcdDrawString2(dev, font32x32, xpos, 50, ascii, WHITE);
}

static void both_eyes_closed(TFT_t * dev)
{
	uint8_t ascii[40];

	lcdSetFontDirection(dev, DIRECTION0);
	// strcpy((char *)ascii, "^  ^");
	ascii[0] = 0x3;
	ascii[1] = 0x1;
	ascii[2] = 0x3;
	ascii[3] = 0x0;

	// Complete one calulation and return, save state persitantly to survive thread being stopped
	int xpos = (CONFIG_WIDTH - (strlen((char *)ascii) * 32)) / 2;
	lcdDrawString2(dev, font32x32, xpos, 50, ascii, BLACK);
	lcdWriteBuffer(dev);
	lcdDrawString2(dev, font32x32, xpos, 50, ascii, WHITE);
}

enum cat_states {
	s_both_eyes_open_forward = 0,
	s_both_eyes_open_right,
	s_both_eyes_open_left,
	s_blink,
};

static int cat_state = s_blink;
static uint32_t state_tick = 0;
static int64_t t1 = 0;
static int64_t t2 = 0;

void catface_helper(TFT_t * dev)
{
	/*
		Each state, besides the eyes being closed, keep a random amount of time they keep track of
		when the tick could expires the eyes close for a set amount and a random state is entered
		again.

		We do not want to hold up a button press, so the tick count is checked quickly, and if the
		image is the same then do not bother redrawing it.
	*/
	t2 = esp_timer_get_time();
	if (t1 + state_tick <= t2) {
		
		// blink
		if (cat_state == s_blink) {
			// blink is always the last defined enum so the states will always be size of possbile states - 1
			cat_state = esp_random() % s_blink;
		}
		else cat_state = s_blink;

		switch(cat_state)
		{
		case s_both_eyes_open_forward:
			// 1,000,000 is about 1.0s
			state_tick = (esp_random() % (4 * 1000000)) ^ 400000;
			both_eyes_open_forward(dev);
			break;
		case s_both_eyes_open_right:
			// 1,000,000 is about 1.0s
			state_tick = (esp_random() % (4 * 1000000)) ^ 400000;
			both_eyes_open_right(dev);
			break;
		case s_both_eyes_open_left:
			// 1,000,000 is about 1.0s
			state_tick = (esp_random() % (4 * 1000000)) ^ 400000;
			both_eyes_open_left(dev);
			break;
		case s_blink:
			// average time of a blink is 100-150 ms
			state_tick = 150000;
			both_eyes_closed(dev);
			break;
		}
	t1 = esp_timer_get_time();
	}
}
