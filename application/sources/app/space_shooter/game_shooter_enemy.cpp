#include "game_shooter.h"
#include "game_save.h"
#include <stdlib.h>

int8_t enemy_dir = 1;
uint8_t enemy_move_ticks = 0;

// Handle enemy shooting for normal and spread shooter types
static void handle_enemy_shooting(int e, int ew, int boss_max_hp) {
	int shoot_chance = 0;
	if (g_enemies[e].type == 4) {
		if (g_enemies[e].state == BOSS_STATE_NORMAL) {
			int boss_cycle = g_stage / 3;
			shoot_chance = 9 + (g_game_setting.difficulty * 5) + ((boss_cycle - 1) * 5);
			if (g_enemies[e].hp <= boss_max_hp / 2) shoot_chance += 10; // Enrage bonus
		}
	} else if (g_enemies[e].type == 5) {
		shoot_chance = 5 + g_game_setting.difficulty * 2 + (g_stage / 2);
	} else if (g_enemies[e].type == 6) {
		shoot_chance = 0;
	} else {
		shoot_chance = 3 + g_game_setting.difficulty + (g_stage / 2);
	}
	
	if (shoot_chance > 0 && rand() % 1000 < shoot_chance) {
		if (g_enemies[e].type == 4) {
			game_boss_shoot(e, ew, boss_max_hp);
		} else if (g_enemies[e].type == 5) {
			// Triple shot burst for Spread Shooter
			int bullets_spawned = 0;
			for (int i = 0; i < MAX_BULLETS && bullets_spawned < 3; i++) {
				if (!g_bullets[i].active) {
					g_bullets[i].active = true;
					g_bullets[i].x = g_enemies[e].x + ew / 2;
					g_bullets[i].y = g_enemies[e].y + 8;
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

// Reversal and vertical drop when hitting screen edges
static void handle_edge_reversal(bool hit_edge) {
	if (!hit_edge) return;
	
	enemy_dir = -enemy_dir;
	static uint8_t edge_hit_count = 0;
	edge_hit_count++;
	
	for (int e = 0; e < MAX_ENEMIES; e++) {
		if (g_enemies[e].active) {
			if (g_enemies[e].type == 4) {
				if (g_enemies[e].state == BOSS_STATE_NORMAL) {
					g_enemies[e].x += enemy_dir;
				}
			} else {
				g_enemies[e].x += enemy_dir;
			}
			
			int edge_hits_required = (g_stage > 10) ? 1 : 2;
			int drop_amount = 1 + (g_stage / 5);
			if (drop_amount > 4) drop_amount = 4;
			
			if (edge_hit_count >= edge_hits_required) {
				if (g_enemies[e].type != 6 && g_enemies[e].type != 5 && g_enemies[e].type != 4) { 
					g_enemies[e].y += drop_amount;
				}
			}
		}
	}
	
	int reset_hits = (g_stage > 10) ? 1 : 2;
	if (edge_hit_count >= reset_hits) edge_hit_count = 0;
}

// Main enemy update loop
void game_enemy_update() {
	bool hit_edge = false;
	enemy_move_ticks++;
	int move_threshold = 4 - g_game_setting.difficulty - (g_stage / 5);
	if (g_stage % 3 == 0) move_threshold--;
	if (move_threshold < 1) move_threshold = 1;
	bool do_move = (enemy_move_ticks >= move_threshold);
	
	int boss_max_hp = 10 + ((g_stage / 3) - 1) * 5;

	for (int e = 0; e < MAX_ENEMIES; e++) {
		if (!g_enemies[e].active) continue;
		
		int ew = (g_enemies[e].type >= 4) ? 16 : 8;
		
		if (g_enemies[e].type == 4) {
			game_boss_update_state(e, do_move, move_threshold, boss_max_hp, hit_edge);
		} else {
			if (do_move) {
				g_enemies[e].x += enemy_dir;
				if (g_enemies[e].x <= 0 || g_enemies[e].x + ew >= 128) hit_edge = true;
			}
			if (g_enemies[e].type == 6) {
				game_carrier_update(e);
			}
		}
		
		handle_enemy_shooting(e, ew, boss_max_hp);
	}
	
	if (do_move) {
		enemy_move_ticks = 0;
		handle_edge_reversal(hit_edge);
	}
}
