
#include "cat_battle_msgs.h"

static uint8_t c_rps = RPS_NONE;

void send_ping(uint32_t arbid, uint8_t comm_type, uint8_t my_id)
{
	uint8_t tdata[8];

	tdata[0] = comm_type;
	tdata[1] = my_id;

	can_send(arbid, 0x1, 0x8, (uint8_t*)&tdata);
}

void send_challenge(uint32_t arbid, uint8_t target_id, uint8_t comm_type)
{
	// Place holder for now
	uint8_t tdata[8];

	tdata[0] = comm_type;
	tdata[1] = target_id;

	can_send(arbid, 0x1, 0x8, (uint8_t*)&tdata);
}

// get the local instance of rps
uint8_t get_rps()
{
	// Over kill
	do {
		c_rps = esp_random() % RPS_NONE;
	} while (c_rps >= RPS_NONE);

	return c_rps;
}

void send_rps(uint32_t arbid, uint8_t target_id, uint8_t comm_type, int16_t value)
{
	uint8_t tdata[8];

	tdata[0] = comm_type;
	tdata[1] = target_id;
	tdata[2] = (uint8_t) (value >> 4) & 0xff;
	tdata[3] = (uint8_t)  value & 0xff;

	can_send(arbid, 0x1, 0x8, (uint8_t*)&tdata);
}

// woid send_sync()

bool didWinRPS(int16_t my_rps, int16_t tar_rps)
{
	/*
	0 - 1 = -1 L
	0 - 2 = -2 W
	1 - 0 =  1 W
	1 - 2 = -1 L
	2 - 0 =  0 L
	2 - 1 =  1 W
	*/

	int16_t tmp = my_rps - tar_rps;
	ESP_LOGI(CMG_TAG, "%d - %d = %d", my_rps, tar_rps, tmp);
	return ((tmp == 1) || (tmp == -2));
}