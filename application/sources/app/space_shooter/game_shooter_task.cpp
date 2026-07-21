#include "game_shooter.h"
#include "task_list.h"
#include "timer.h"

// Task handler for Game Shooter OS signals (Main Controller for Game Task)
// Handles Message Passing events asynchronously
void game_shooter_task(ak_msg_t* msg) {
	switch (msg->sig) {
		case AC_GAME_START_REQ:
			// Initialize the physics and logic variables
			game_logic_init();
			// Start game tick timer (50ms interval = 20 FPS)
			timer_set(AC_TASK_GAME_SHOOTER_ID, AC_GAME_UPDATE_TICK, 50, TIMER_PERIODIC);
			break;
			
		case AC_GAME_BTN_MODE:
			// Player shoots a bullet
			game_player_shoot();
			break;
			
		case AC_GAME_UPDATE_TICK:
			// Main physics and logic tick event triggered by timer
			game_logic_update();
			break;
	}
}
