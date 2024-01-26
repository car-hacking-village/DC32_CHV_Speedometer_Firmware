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

TickType_t BMPTest(TFT_t * dev, char * file, int width, int height, bool inverse);
void display_init(TFT_t * dev);
