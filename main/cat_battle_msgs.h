
#include "esp_random.h"
#include "esp_log.h"

#include "can_helper.h"
#include "speedometer.h"

#define CMG_TAG			"CMG"


/*
If i run out of time, 
*/

enum rps_values {
	RPS_ROCK = 0,
	RPS_PAPR,
	RPS_SICS,
	RPS_NONE,
};

char * rps_str[] = {
	"Rock!",
	"Paper!",
	"Sicsors!",
};

void send_ping(uint32_t arbid, uint8_t ping_type, uint8_t my_id);
void send_challenge(uint32_t arbid, uint8_t target_id, uint8_t chal_type);
uint8_t get_rps();
void send_rps(uint32_t arbid, uint8_t target_id, uint8_t rps_type, uint8_t value);
bool didWinRPS(uint8_t my_rps, uint8_t tar_rps);
