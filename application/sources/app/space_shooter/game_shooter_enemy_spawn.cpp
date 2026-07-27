#include "game_shooter.h"
#include "game_save.h"
#include <stdlib.h>

#define SPAWN_START_X 8
#define SPAWN_START_Y 16
#define SPAWN_OFFSET_X 16
#define SPAWN_OFFSET_Y 12

// Generate a 6x3 grid of enemies or trigger boss spawn on boss stages
void game_enemy_spawn() {
	if (g_stage % 3 == 0) { // Boss stage every 3 stages
		game_boss_spawn();
		return;
	}
	
	int e = 0;
	int rows = 3;
	int cols = 6;
	int spawn_chance = 40 + (g_game_setting.difficulty * 10) + (g_stage * 3);
	if (spawn_chance > 90) spawn_chance = 90;
	bool spawned_type5 = false;
	bool spawned_type6 = false;
	
	// Center the 6x3 enemy grid (start_x = 20 for 128px screen width)
	for (int r = 0; r < rows; r++) {
		for (int c = 0; c < cols; c++) {
			if (rand() % 100 < spawn_chance) {
				g_enemies[e].active = true;
				
				// Randomize enemy type based on restrictions
				int r_val = rand() % 100;
				int type6_chance = 15 + (g_stage / 2);
				if (type6_chance > 30) type6_chance = 30;
				
				if (r == 0 && r_val < type6_chance && !spawned_type6) {
					g_enemies[e].type = 6; // Carrier
					spawned_type6 = true;
					g_enemies[e].hp = 4;
				} else if (r == 0 && r_val >= type6_chance && r_val < type6_chance + 20 && !spawned_type5) {
					g_enemies[e].type = 5; // Spread Shooter
					spawned_type5 = true;
					g_enemies[e].hp = 3;
				} else {
					g_enemies[e].type = 1 + (rand() % 3);
					g_enemies[e].hp = g_enemies[e].type;
				}
				
				g_enemies[e].blink_timer = 0;
				g_enemies[e].x = 20 + c * SPAWN_OFFSET_X;
				g_enemies[e].y = SPAWN_START_Y + r * SPAWN_OFFSET_Y;
				
				// If this is a wide enemy, skip the next column so they don't touch
				if (g_enemies[e].type == 5 || g_enemies[e].type == 6) {
					c++; 
				}
				
				e++;
				if (e >= MAX_ENEMIES) break;
			}
		}
		if (e >= MAX_ENEMIES) break;
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
