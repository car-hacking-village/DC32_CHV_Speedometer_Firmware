#ifndef SPEEDOMETER_H_
#define SPEEDOMETER_H_

#define NUM_MAIN_TASKS 3

// Main Task Priorities
#define CAT_TASK_PRIO                   10
#define SPD_TASK_PRIO                   10
#define DIK_TASK_PRIO                   10

#define SWT_TASK_PRIO                   20
#define CSH_TASK_PRIO                   SWT_TASK_PRIO+1

// Helper Task Priorities
#define SPD_CAN_HLP_PRIO                SPD_TASK_PRIO+1
#define CAT_CAN_HLP_PRIO                CAT_TASK_PRIO+1

// CAN Priorities
#define CTRL_TSK_PRIO                   31              //Control task priority
#define RX_TASK_PRIO                    CTRL_TSK_PRIO-1 //Receiving task priority
#define TX_TASK_PRIO                    CTRL_TSK_PRIO-1 //Sending task priority

enum comm_states {
	BATTL_PSND=0,	// ping send
	BATTL_PRSP,		// ping response
	BATTL_CSND,		// challenge send
	BATTL_CRSP,		// challenge response
	BATTL_RSND,		// rock paper scissor send
	BATTL_RRSP,		// rock paper scissor response
	BATTL_ASND,		// attack send
	BATTL_ARSP,		// attack response
	BATTL_SYNC,		// wait for both sides to be ready
};

// maybe multiple battle sessions can be active at a time and tracked in the heap? heap overflow?

enum cat_states {
	CAT_IDLE=0, // When idle, send ping with id every second until ping response with id
	CAT_CHAL,	// The badge whose ping was responded to sends a key which requires a challenge response to (like seed and key)
	CAT_APRV,	// Sender is happy and starts sending offuscated traffic with (real or placeholder) flag respectively
	CAT_DSPV,	// Sender not happy and both badges go back to idle
	CAT_RSPS,	// The badge who was responded to sends a random rock paper scissor and recieves a random rock paper scissor
				// 		if the sender loses the responder becomes the sender
	CAT_BATL,	// The sender sends a random attack value, then the responder, attacks must be >=1, if attack is zero
				// 		that means that side died
	CAT_VICT,	// Glory to cat, returns to idle after X amount of time
	CAT_DEDD,	// Sad cat day, at least cute cat ghost, stays this way until reboot, sends flag message every second 
				// (placeholder or real)
};

#endif /* SPEEDOMETER_H_ */