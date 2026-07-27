#include "game_shooter.h"
#include "timer.h"
#include "task_list.h"

// Update powerup falling positions
void game_powerups_update() {
	for (int i = 0; i < MAX_POWERUPS; i++) {
		if (g_powerups[i].active) {
			if (g_tick_count % 2 == 0) { // Fall slowly (1px per 2 ticks)
				g_powerups[i].y++;
			}
			if (g_powerups[i].y > 64) { // Exceeded screen height
				g_powerups[i].active = false;
			}
		}
	}
}

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
		timer_remove_attr(AC_TASK_GAME_SHOOTER_ID, AC_GAME_UPDATE_TICK);
		// Signal UI task to transition to Game Over screen
		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_GAME_OVER_NEXT);
	}
}
