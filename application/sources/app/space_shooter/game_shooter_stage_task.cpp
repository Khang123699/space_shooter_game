#include "game_shooter_stage_task.h"
#include "game_shooter_player_task.h"
#include "game_shooter_enemy_task.h"
#include "game_shooter_bullet_task.h"
#include "game_shooter_render.h"
#include "game_save.h"
#include "timer.h"
#include "task_list.h"
#include "app.h"

uint8_t g_stage = 1;
int8_t g_transition_timer = 0;
uint8_t g_new_high_score_rank = 0;

// Check and handle stage progression when all enemies are destroyed
void game_stage_update() {
	bool all_dead = true;
	for (int e = 0; e < MAX_ENEMIES; e++) {
		if (g_enemies[e].active) {
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
			game_enemy_spawn();
		}
	}
}

// Check for game over condition and trigger UI transition if out of lives
void game_check_game_over() {
	if (g_lives <= 0) {
		// Stop logic timer to pause the game world
		timer_remove_attr(AC_TASK_GAME_PLAYER_ID, AC_GAME_UPDATE_TICK);
		timer_remove_attr(AC_TASK_GAME_ENEMY_ID, AC_GAME_UPDATE_TICK);
		timer_remove_attr(AC_TASK_GAME_BULLET_ID, AC_GAME_UPDATE_TICK);
		timer_remove_attr(AC_TASK_GAME_STAGE_ID, AC_GAME_UPDATE_TICK);
		// Signal UI task to transition to Game Over screen
		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_GAME_OVER_NEXT);
	}
}

void game_stage_task(ak_msg_t* msg) {
	switch (msg->sig) {
		case AC_GAME_UPDATE_TICK:
			game_stage_update();
			game_check_game_over();
			game_shooter_request_render();
			break;
	}
}
