#include "game_shooter_stage_task.h"
#include "game_shooter_player_task.h"
#include "game_shooter_enemy_task.h"
#include "game_shooter_bullet_task.h"
#include "game_shooter_render.h"
#include "game_save.h"
#include "timer.h"
#include "task_list.h"
#include "app.h"
#include "app_dbg.h"

static uint8_t g_stage = 1;
static int8_t g_transition_timer = 0;
static uint8_t g_new_high_score_rank = 0;

uint8_t game_get_stage() { return g_stage; }
int8_t game_get_transition_timer() { return g_transition_timer; }
uint8_t game_get_new_high_score_rank() { return g_new_high_score_rank; }
void game_set_new_high_score_rank(uint8_t rank) { g_new_high_score_rank = rank; }

// Check and handle stage progression when all enemies are destroyed
static void game_stage_update() {
	bool all_dead = true;
	const enemy_t* enemies = game_get_enemies();
	for (int e = 0; e < MAX_ENEMIES; e++) {
		if (enemies[e].active) {
			all_dead = false;
			break;
		}
	}
	
	if (all_dead && g_transition_timer == 0) {
		g_stage++;
		g_transition_timer = 90;
	}
	
	if (g_transition_timer > 0) {
		g_transition_timer--;
		if (g_transition_timer == 0) {
			task_post_pure_msg(AC_TASK_GAME_ENEMY_ID, AC_GAME_SPAWN_ENEMY);
		}
	}
}

// Check for game over condition and trigger UI transition if out of lives
static void game_check_game_over() {
	if (game_get_lives() <= 0) {
		// Stop logic timer to pause the game world
		timer_remove_attr(AC_TASK_GAME_STAGE_ID, AC_GAME_UPDATE_TICK);
		// Signal UI task to transition to Game Over screen
		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_GAME_OVER_NEXT);
	}
}

void game_stage_task(ak_msg_t* msg) {
	switch (msg->sig) {
		case AC_GAME_START_REQ:
		{
			APP_DBG_SIG("AC_GAME_START_REQ\n");
			g_stage = 1;
			g_transition_timer = 0;
			g_new_high_score_rank = 0;
			timer_set(AC_TASK_GAME_STAGE_ID, AC_GAME_UPDATE_TICK, 50, TIMER_PERIODIC);
		}
		break;
			
		case AC_GAME_UPDATE_TICK:
		{
			task_post_pure_msg(AC_TASK_GAME_PLAYER_ID, AC_GAME_UPDATE_TICK);
			task_post_pure_msg(AC_TASK_GAME_ENEMY_ID, AC_GAME_UPDATE_TICK);
			task_post_pure_msg(AC_TASK_GAME_BULLET_ID, AC_GAME_UPDATE_TICK);
			game_stage_update();
			game_check_game_over();
			game_shooter_request_render();
		}
		break;
	}
}
