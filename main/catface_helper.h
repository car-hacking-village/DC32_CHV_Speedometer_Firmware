
#ifndef CATFACEHELPER_H_
#define CATFACEHELPER_H_

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_random.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "st7565.h"
#include "can_helper.h"
#include "cat_battle_msgs.h"
#include "speedometer.h"

// #include "font6x8.h"
#include "font8x16.h"
#include "font8x8.h"
#include "font32x32.h"

#define CAT_TAG			"CAT"

enum cat_expressions {
	EYES_OPEN_FOWARD = 0,
	EYES_OPEN_RIGHT,
	EYES_OPEN_LEFT,
	EYES_OPEN_ANGRY,
	EYES_OPEN_REAL,
	EYES_CLOSED,
};

enum cat_effects {
	NO_EFFECT = 0,
	EFFECT_BLUSH,
};

enum cat_32_32_syms {
	SYM_32_NULL	= 0x0,
	SYM_32_SPAC = 1,
	SYM_32_OFWD = 2,
	SYM_32_CLSD = 3,
	SYM_32_OLFT = 4,
	SYM_32_ORHT = 5,
	SYM_32_ANGL = 6,
	SYM_32_ANGR = 7,
	SYM_32_RELL = 8,
	SYM_32_RELR = 9,
	SYM_32_OID1 = 10,
	SYM_32_OID2 = 11,
	SYM_32_OID3 = 12,
};

extern TaskHandle_t catface_t;
extern QueueHandle_t cat_can_queue;

void change_expression(TFT_t * dev, uint8_t * expression, uint32_t effect);
void catface_helper(TFT_t * dev);

#endif /* CATFACEHELPER_H_ */
