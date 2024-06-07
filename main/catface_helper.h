
#include <string.h>

#include "esp_random.h"
#include "esp_timer.h"

#include "st7565.h"
#include "font6x8.h"
#include "font8x16.h"
#include "font8x8.h"
#include "font32x32.h"

enum cat_expressions {
	EYES_OPEN_FOWARD = 0,
	EYES_OPEN_RIGHT,
	EYES_OPEN_LEFT,
	EYES_CLOSED,
};

enum cat_effects {
	NO_EFFECT = 0,
};

void change_expression(TFT_t * dev, uint32_t base_expression, uint32_t effect);
void catface_helper(TFT_t * dev);
