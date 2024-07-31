
#include "esp_log.h"

#include "st7565.h"
#include "can_helper.h"

#include "cat_battle_msgs.h"


// #include "font8x16.h"

#define SPD_TAG 				"SPD"

extern QueueHandle_t spd_can_queue;

void speedometer_helper(TFT_t * dev);
