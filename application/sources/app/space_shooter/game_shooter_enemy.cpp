#include "game_shooter.h"
#include "game_save.h"
#include <stdlib.h>

#define SPAWN_START_X 8
#define SPAWN_START_Y 16
#define SPAWN_OFFSET_X 16
#define SPAWN_OFFSET_Y 12

int8_t enemy_dir = 1;
uint8_t enemy_move_ticks = 0;

void game_enemy_spawn() {
	int e = 0;
	if (g_stage % 3 == 0) { // Boss stage every 3 stages
		g_enemies[0].active = true;
		g_enemies[0].type = 4; // Boss type
		int boss_cycle = g_stage / 3;
		g_enemies[0].hp = 10 + (boss_cycle - 1) * 5;
		g_enemies[0].blink_timer = 0;
		g_enemies[0].x = 56;
		g_enemies[0].y = 16;
		e = 1;
	} else {
		int rows = 3;
		int cols = 6;
		int spawn_chance = 40 + (g_game_data.difficulty * 10);
		
		// Center the 6x3 enemy grid (start_x = 20 for 128px screen width)
		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < cols; c++) {
				if (rand() % 100 < spawn_chance) {
					g_enemies[e].active = true;
					g_enemies[e].type = 1 + (rand() % 3);
					g_enemies[e].hp = g_enemies[e].type;
					g_enemies[e].blink_timer = 0;
					g_enemies[e].x = 20 + c * SPAWN_OFFSET_X;
					g_enemies[e].y = SPAWN_START_Y + r * SPAWN_OFFSET_Y;
					e++;
					if (e >= MAX_ENEMIES) break;
				}
			}
			if (e >= MAX_ENEMIES) break;
		}
	}
	
	// Fallback if random spawning resulted in 0 enemies
	if (e == 0) {
		g_enemies[0].active = true;
		g_enemies[0].type = 1 + (rand() % 3);
		g_enemies[0].hp = g_enemies[0].type;
		g_enemies[0].blink_timer = 0;
		g_enemies[0].x = 56;
		g_enemies[0].y = 12;
	}
}

void game_enemy_update() {
	bool hit_edge = false;
	enemy_move_ticks++;
	int move_threshold = (g_stage % 3 == 0) ? 2 : (4 - g_game_data.difficulty);
	bool do_move = (enemy_move_ticks >= move_threshold);
	
	for (int e = 0; e < MAX_ENEMIES; e++) {
		if (g_enemies[e].active) {
			int ew = (g_enemies[e].type == 4) ? 16 : 8;
			if (do_move) {
				g_enemies[e].x += enemy_dir;
				if (g_enemies[e].x <= 0 || g_enemies[e].x + ew >= 128) hit_edge = true;
			}
			
			// Enemy shoot
			int shoot_chance;
			if (g_enemies[e].type == 4) {
				int boss_cycle = g_stage / 3;
				// Base + difficulty + 5 for every boss cycle passed (no limit)
				shoot_chance = 9 + (g_game_data.difficulty * 5) + ((boss_cycle - 1) * 5);
			} else {
				shoot_chance = 3 + g_game_data.difficulty;
			}
			
			if (rand() % 1000 < shoot_chance) {
				if (g_enemies[e].type == 4) {
					// Triple shot burst for Boss
					int bullets_spawned = 0;
					for (int i = 0; i < MAX_BULLETS && bullets_spawned < 3; i++) {
						if (!g_bullets[i].active) {
							g_bullets[i].active = true;
							g_bullets[i].x = g_enemies[e].x + ew / 2;
							g_bullets[i].y = g_enemies[e].y + 12;
							g_bullets[i].is_enemy = true;
							
							if (bullets_spawned == 0) g_bullets[i].vx = 0;
							else if (bullets_spawned == 1) g_bullets[i].vx = -1;
							else if (bullets_spawned == 2) g_bullets[i].vx = 1;
							
							bullets_spawned++;
						}
					}
				} else {
					for (int i = 0; i < MAX_BULLETS; i++) {
						if (!g_bullets[i].active) {
							g_bullets[i].active = true;
							g_bullets[i].x = g_enemies[e].x + ew / 2;
							g_bullets[i].y = g_enemies[e].y + 8;
							g_bullets[i].is_enemy = true;
							g_bullets[i].vx = 0;
							break;
						}
					}
				}
			}
		}
	}
	
	if (do_move) {
		enemy_move_ticks = 0;
		if (hit_edge) {
			enemy_dir = -enemy_dir;
			static uint8_t edge_hit_count = 0;
			edge_hit_count++;
			
			for (int e = 0; e < MAX_ENEMIES; e++) {
				if (g_enemies[e].active) {
					g_enemies[e].x += enemy_dir;
					if (edge_hit_count >= 2) {
						g_enemies[e].y += 1;
					}
				}
			}
			
			if (edge_hit_count >= 2) edge_hit_count = 0;
		}
	}
}
