#include "game_shooter.h"

// Move the player horizontally within screen boundaries
void game_player_move(int8_t dir) {
	g_player_x += dir;
	if (g_player_x < 0) g_player_x = 0;
	if (g_player_x > 120) g_player_x = 120;
}

// Shoot a player bullet if cooldown allows and slots are available
void game_player_shoot() {
	if (g_shoot_cooldown > 0) return;
	
	if (g_player_dual_shot_timer > 0) {
		int spawned = 0;
		for (int i = 0; i < MAX_BULLETS && spawned < 2; i++) {
			if (!g_bullets[i].active) {
				g_bullets[i].active = true;
				g_bullets[i].is_enemy = false;
				g_bullets[i].x = g_player_x + (spawned == 0 ? 1 : 7);
				g_bullets[i].y = 52;
				g_bullets[i].vx = 0;
				spawned++;
			}
		}
		if (spawned > 0) g_shoot_cooldown = 8;
	} else {
		for (int i = 0; i < MAX_BULLETS; i++) {
			if (!g_bullets[i].active) {
				g_bullets[i].active = true;
				g_bullets[i].is_enemy = false;
				g_bullets[i].x = g_player_x + 4;
				g_bullets[i].y = 52;
				g_bullets[i].vx = 0;
				g_shoot_cooldown = 8; 
				break;
			}
		}
	}
}
