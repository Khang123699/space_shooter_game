#include "game_shooter.h"

// Check if space below carrier is free of other enemies
static bool is_space_clear_below(int carrier_idx, int spawn_x, int spawn_y) {
	for (int c_e = 0; c_e < MAX_ENEMIES; c_e++) {
		if (g_enemies[c_e].active && c_e != carrier_idx) {
			int ew2 = (g_enemies[c_e].type >= 4) ? 16 : 8;
			if (spawn_x < g_enemies[c_e].x + ew2 && spawn_x + 8 > g_enemies[c_e].x &&
				spawn_y < g_enemies[c_e].y + 8 && spawn_y + 8 > g_enemies[c_e].y) {
				return false;
			}
		}
	}
	return true;
}

// Spawn a basic 1-HP minion directly below the carrier
static void spawn_carrier_minion(int spawn_x, int spawn_y) {
	for (int ne = 0; ne < MAX_ENEMIES; ne++) {
		if (!g_enemies[ne].active) {
			g_enemies[ne].active = true;
			g_enemies[ne].type = 1; // Basic 1-HP enemy
			g_enemies[ne].hp = 1;
			g_enemies[ne].blink_timer = 0;
			g_enemies[ne].x = spawn_x;
			g_enemies[ne].y = spawn_y;
			break;
		}
	}
}

// Update Carrier AI (Type 6): Spawns a minion every 6 seconds (120 ticks)
void game_carrier_update(int e) {
	if (g_tick_count > 0 && g_tick_count % 120 == 0) {
		int spawn_x = g_enemies[e].x + 4; // Center the spawn
		int spawn_y = g_enemies[e].y + 12; // Next row down
		
		if (is_space_clear_below(e, spawn_x, spawn_y)) {
			spawn_carrier_minion(spawn_x, spawn_y);
		}
	}
}
