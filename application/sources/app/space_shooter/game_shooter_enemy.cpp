#include "game_shooter.h"
#include <stdlib.h>

#define SPAWN_START_X 8
#define SPAWN_START_Y 16
#define SPAWN_OFFSET_X 16
#define SPAWN_OFFSET_Y 12

int8_t enemy_dir = 1;
uint8_t enemy_move_ticks = 0;

void game_enemy_spawn() {
	int e = 0;
	if (g_stage % 5 == 0) { // Boss stage every 5 stages
		g_enemies[0].active = true;
		g_enemies[0].type = 4; // Boss type
		g_enemies[0].hp = 15 + g_stage;
		g_enemies[0].blink_timer = 0;
		g_enemies[0].x = 56;
		g_enemies[0].y = 16;
		e = 1;
	} else {
		int rows = 2 + (g_stage / 2);
		if (rows > 4) rows = 4;
		int cols = 4 + (g_stage / 3);
		if (cols > 7) cols = 7;
		
		for (int row = 0; row < rows; row++) {
			for (int col = 0; col < cols; col++) {
				if (e >= MAX_ENEMIES) break;
				g_enemies[e].active = true;
				g_enemies[e].type = 1 + (rand() % 3); // random type 1, 2, or 3
				g_enemies[e].hp = g_enemies[e].type;
				g_enemies[e].blink_timer = 0;
				g_enemies[e].x = SPAWN_START_X + col * SPAWN_OFFSET_X;
				g_enemies[e].y = SPAWN_START_Y + row * SPAWN_OFFSET_Y;
				e++;
			}
		}
	}
	
	// Fallback to avoid empty stage
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
	int move_threshold = (g_stage % 5 == 0) ? 1 : (4 - g_game_data.difficulty);
	bool do_move = (enemy_move_ticks >= move_threshold);
	
	for (int e = 0; e < MAX_ENEMIES; e++) {
		if (g_enemies[e].active) {
			int ew = (g_enemies[e].type == 4) ? 16 : 8;
			if (do_move) {
				g_enemies[e].x += enemy_dir;
				if (g_enemies[e].x <= 0 || g_enemies[e].x + ew >= 128) hit_edge = true;
			}
			
			// Enemy shoot
			int shoot_chance = (g_enemies[e].type == 4) ? (9 + g_game_data.difficulty * 5) : (3 + g_game_data.difficulty);
			if (rand() % 1200 < shoot_chance) {
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
