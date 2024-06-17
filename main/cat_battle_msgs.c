
#include "cat_battle_msgs.h"

static uint8_t c_rps = RPS_NONE;

void send_ping(uint32_t arbid, uint8_t ping_type, uint8_t my_id)
{
	uint8_t tdata[8];

	tdata[0] = ping_type;
	tdata[1] = my_id;

	can_send(arbid, 0x1, 0x8, (uint8_t*)&tdata);
}

void send_challenge(uint32_t arbid, uint8_t target_id, uint8_t chal_type)
{
	// Place holder for now
	uint8_t tdata[8];

	tdata[0] = target_id;
	tdata[1] = chal_type;

	can_send(arbid, 0x1, 0x8, (uint8_t*)&tdata);
}

// get the local instance of rps
uint8_t get_rps()
{
	c_rps = esp_random() % RPS_NONE;
	return c_rps;
}

void send_rps(uint32_t arbid, uint8_t target_id, uint8_t rps_type, uint8_t value)
{
	uint8_t tdata[8];

	tdata[0] = target_id;
	tdata[1] = rps_type;
	tdata[2] = value;

	can_send(arbid, 0x1, 0x8, (uint8_t*)&tdata);
}

bool didWinRPS(uint8_t my_rps, uint8_t tar_rps)
{
	/*
	0 - 1 = -1 L
	0 - 2 = -2 W
	1 - 0 =  1 W
	1 - 2 = -1 L
	2 - 0 =  0 L
	2 - 1 =  1 W
	*/
	int8_t tmp = (int8_t)my_rps - (int8_t)tar_rps;
	return ((tmp == 1) || (tmp == -2));
}