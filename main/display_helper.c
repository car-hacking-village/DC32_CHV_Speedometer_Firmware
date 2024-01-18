
#include "display_helper.h"

static const char *TAG = "ST7565";

TickType_t BMPTest(TFT_t * dev, char * file, int width, int height) {
	TickType_t startTick, endTick, diffTick;
	startTick = xTaskGetTickCount();

	lcdSetFontDirection(dev, 0);
	lcdFillScreen(dev, BLACK);

	// open BMP file
	esp_err_t ret;
	FILE* fp = fopen(file, "rb");
	if (fp == NULL) {
		ESP_LOGW(__FUNCTION__, "File not found [%s]", file);
		return 0;
	}

	// read bmp header
	bmpfile_t *result = (bmpfile_t*)malloc(sizeof(bmpfile_t));
	ret = fread(result->header.magic, 1, 2, fp);
	assert(ret == 2);
	ESP_LOGD(__FUNCTION__,"result->header.magic=%c %c", result->header.magic[0], result->header.magic[1]);
	if (result->header.magic[0]!='B' || result->header.magic[1] != 'M') {
		ESP_LOGW(__FUNCTION__, "File is not BMP");
		free(result);
		fclose(fp);
		return 0;
	}
	ret = fread(&result->header.filesz, 4, 1 , fp);
	assert(ret == 1);
	ESP_LOGD(__FUNCTION__,"result->header.filesz=%d", result->header.filesz);
	ret = fread(&result->header.creator1, 2, 1, fp);
	assert(ret == 1);
	ret = fread(&result->header.creator2, 2, 1, fp);
	assert(ret == 1);
	ret = fread(&result->header.offset, 4, 1, fp);
	assert(ret == 1);

	// read dib header
	ret = fread(&result->dib.header_sz, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.width, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.height, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.nplanes, 2, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.depth, 2, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.compress_type, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.bmp_bytesz, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.hres, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.vres, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.ncolors, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.nimpcolors, 4, 1, fp);
	assert(ret == 1);

	ESP_LOGD(__FUNCTION__, "result->dib.header_sz=%d", result->dib.header_sz);
	ESP_LOGD(__FUNCTION__, "result->dib.depth=%d", result->dib.depth);
	ESP_LOGD(__FUNCTION__, "result->dib.compress_type=%d", result->dib.compress_type);
	ESP_LOGD(__FUNCTION__, "result->dib.width=%d", result->dib.width);
	ESP_LOGD(__FUNCTION__, "result->dib.height=%d", result->dib.height);
	ESP_LOGD(__FUNCTION__, "result->dib.bmp_bytesz=%d", result->dib.bmp_bytesz);
	ESP_LOGD(__FUNCTION__, "result->dib.ncolors=%d", result->dib.ncolors);
	if((result->dib.depth == 1) && (result->dib.compress_type == 0)) {
		// BMP rows are padded (if needed) to 4-byte boundary
		//uint32_t rowSize = (result->dib.width * 3 + 3) & ~3;
		//uint32_t rowSize = (result->dib.width * 1 + 3) & ~3;
		uint32_t rowSize = result->dib.bmp_bytesz/result->dib.height;
		ESP_LOGD(__FUNCTION__,"rowSize=%d", rowSize);
		int w = result->dib.width;
		int h = result->dib.height;
		ESP_LOGD(__FUNCTION__,"w=%d h=%d", w, h);
		int _x;
		int _w;
		int _cols;
		int _cole;
		if (width >= w) {
			_x = (width - w) / 2;
			_w = w;
			_cols = 0;
			_cole = w - 1;
		} else {
			_x = 0;
			_w = width;
			_cols = (w - width) / 2;
			_cole = _cols + width - 1;
		}
		ESP_LOGD(__FUNCTION__,"_x=%d _w=%d _cols=%d _cole=%d",_x, _w, _cols, _cole);

		int _y;
		int _rows;
		int _rowe;
		if (height >= h) {
			_y = (height - h) / 2;
			_rows = 0;
			_rowe = h -1;
		} else {
			_y = 0;
			_rows = (h - height) / 2;
			_rowe = _rows + height - 1;
		}
		ESP_LOGD(__FUNCTION__,"_y=%d _rows=%d _rowe=%d", _y, _rows, _rowe);

#define BUFFPIXEL 10
		uint8_t sdbuffer[BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
		uint8_t *colors = (uint8_t*)malloc(sizeof(uint8_t) * w);

		for (int row=0; row<h; row++) { // For each scanline...
			if (row < _rows || row > _rowe) continue;
			// Seek to start of scan line.	It might seem labor-
			// intensive to be doing this on every line, but this
			// method covers a lot of gritty details like cropping
			// and scanline padding.	Also, the seek only takes
			// place if the file position actually needs to change
			// (avoids a lot of cluster math in SD library).
			// Bitmap is stored bottom-to-top order (normal BMP)
			int pos = result->header.offset + (h - 1 - row) * rowSize;
			// ESP_LOGD(__FUNCTION__,"_result->header.offset=%d pos=%d rowSize=%d", result->header.offset, pos, rowSize);
			fseek(fp, pos, SEEK_SET);
			int buffidx = sizeof(sdbuffer); // Force buffer reload

			int index = 0;
			uint8_t mask = 0x80;
			for (int col=0; col<w; col++) { // For each pixel...
				if (buffidx >= sizeof(sdbuffer)) { // Indeed
					fread(sdbuffer, sizeof(sdbuffer), 1, fp);
					buffidx = 0; // Set index to beginning
				}
				if (col < _cols || col > _cole) continue;
				// Convert pixel from BMP to TFT format, push to display
				colors[index] = sdbuffer[buffidx] & mask;
				if (colors[index] != 0) colors[index] = BLACK; 
#if 0
				if (row == 0) {
					ESP_LOGI(__FUNCTION__,"dbuffer[%d]=%02x colors[%d]=%d", buffidx, sdbuffer[buffidx], index, colors[index]);
				}
#endif

				index++;
				if (mask == 0x01) {
					buffidx++;
					mask = 0x80;
				} else {
					mask = mask >> 1;
				}
			} // end col
			//lcdDrawMultiPixels(dev, _x, row+_y, _w, colors);
			lcdDrawMultiPixels(dev, _x, _y, _w, colors);
			//lcdWriteBuffer(dev);
			_y++;
		} // end row
		lcdWriteBuffer(dev);
		free(colors);
	} else {
		ESP_LOGW(__FUNCTION__, "Illegal BMP format");
	} // end if 
	free(result);
	fclose(fp);

	endTick = xTaskGetTickCount();
	diffTick = endTick - startTick;
	ESP_LOGI(__FUNCTION__, "elapsed time[ms]:%d",diffTick*portTICK_PERIOD_MS);
	return diffTick;
}

void display_init(TFT_t *dev)
{
	spi_master_init(dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO, CONFIG_BL_GPIO);
	lcdInit(dev, CONFIG_WIDTH, CONFIG_HEIGHT);

#if CONFIG_FLIP
	ESP_LOGI(TAG, "Flip upside down");
	lcdFlipOn(dev);
#endif

#if CONFIG_INVERSION
#endif
	ESP_LOGI(TAG, "Display Inversion");
	lcdInversionOn(dev);
}
