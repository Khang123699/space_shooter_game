#include "game_shooter_enemy_task.h"
#include "game_shooter_player_task.h"
#include "game_shooter_bullet_task.h"
#include "game_shooter_stage_task.h" // For game setting difficulty
#include "game_save.h" // For g_game_setting
#include "app.h"
#include <stdlib.h>

int8_t enemy_dir = 1;
uint8_t enemy_move_ticks = 0;

#define SPAWN_START_X 8
#define SPAWN_START_Y 16
#define SPAWN_OFFSET_X 16
#define SPAWN_OFFSET_Y 12

// Spawn Boss (Type 4) on stage multiples of 3
void game_boss_spawn() {
	g_enemies[0].active = true;
	g_enemies[0].type = 4; // Boss type
	int boss_cycle = g_stage / 3;
	g_enemies[0].hp = 10 + (boss_cycle - 1) * 5;
	g_enemies[0].blink_timer = 0;
	g_enemies[0].x = 56;
	g_enemies[0].y = 16;
	g_enemies[0].state = BOSS_STATE_NORMAL;
	g_enemies[0].timer = 0;
}

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
	
	for (int r = 0; r < rows; r++) {
		for (int c = 0; c < cols; c++) {
			if (rand() % 100 < spawn_chance) {
				g_enemies[e].active = true;
				
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
				
				if (g_enemies[e].type == 5 || g_enemies[e].type == 6) {
					c++; 
				}
				
				e++;
				if (e >= MAX_ENEMIES) break;
			}
		}
		if (e >= MAX_ENEMIES) break;
	}
	
	if (e == 0) {
		g_enemies[0].active = true;
		g_enemies[0].type = 1 + (rand() % 3);
		g_enemies[0].hp = g_enemies[0].type;
		g_enemies[0].blink_timer = 0;
		g_enemies[0].x = 56;
		g_enemies[0].y = 12;
	}
}

// Summon 2 minions to protect the boss
static void spawn_boss_minions(int boss_idx) {
	int minions_spawned = 0;
	for (int ne = 0; ne < MAX_ENEMIES && minions_spawned < 2; ne++) {
		if (!g_enemies[ne].active) {
			g_enemies[ne].active = true;
			g_enemies[ne].type = 1;
			g_enemies[ne].hp = 1;
			g_enemies[ne].blink_timer = 0;
			g_enemies[ne].x = g_enemies[boss_idx].x + (minions_spawned == 0 ? -12 : 20);
			g_enemies[ne].y = g_enemies[boss_idx].y + 8;
			if (g_enemies[ne].x < 0) g_enemies[ne].x = 0;
			if (g_enemies[ne].x > 120) g_enemies[ne].x = 120;
			minions_spawned++;
		}
	}
}

// Update Boss state machine
void game_boss_update_state(int e, bool do_move, int move_threshold, int boss_max_hp, bool& hit_edge) {
	int ew = 16;
	bool is_enraged = (g_enemies[e].hp <= boss_max_hp / 2);
	
	if (g_enemies[e].state == BOSS_STATE_NORMAL) {
		if (do_move || (is_enraged && enemy_move_ticks % (move_threshold > 1 ? move_threshold - 1 : 1) == 0)) {
			g_enemies[e].x += enemy_dir;
			if (g_enemies[e].x <= 0) {
				g_enemies[e].x = 0;
				hit_edge = true;
			} else if (g_enemies[e].x + ew >= 128) {
				g_enemies[e].x = 128 - ew;
				hit_edge = true;
			}
		}
		
		if (g_tick_count > 0 && g_tick_count % 60 == 0) {
			int r = rand() % 100;
			if (r < 20) {
				g_enemies[e].state = BOSS_STATE_DASH_CHARGE;
				g_enemies[e].timer = 0;
			} else if (r < 40) {
				g_enemies[e].state = BOSS_STATE_SUMMON;
				g_enemies[e].timer = 0;
			}
		}
	} else if (g_enemies[e].state == BOSS_STATE_DASH_CHARGE) {
		g_enemies[e].timer++;
		g_enemies[e].blink_timer = 2;
		if (g_enemies[e].timer >= 20) {
			g_enemies[e].state = BOSS_STATE_DASH_DOWN;
		}
	} else if (g_enemies[e].state == BOSS_STATE_DASH_DOWN) {
		g_enemies[e].y += 3;
		if (g_enemies[e].y >= 40) {
			g_enemies[e].state = BOSS_STATE_DASH_UP;
		}
	} else if (g_enemies[e].state == BOSS_STATE_DASH_UP) {
		g_enemies[e].y -= 2;
		if (g_enemies[e].y <= 16) {
			g_enemies[e].y = 16;
			g_enemies[e].state = BOSS_STATE_NORMAL;
		}
	} else if (g_enemies[e].state == BOSS_STATE_SUMMON) {
		g_enemies[e].timer++;
		g_enemies[e].blink_timer = 2;
		if (g_enemies[e].timer >= 15) {
			spawn_boss_minions(e);
			g_enemies[e].state = BOSS_STATE_NORMAL;
		}
	}
}

// Execute Boss bullet pattern
void game_boss_shoot(int e, int ew, int boss_max_hp) {
	bool is_enraged = (g_enemies[e].hp <= boss_max_hp / 2);
	int max_b = is_enraged ? 5 : 3;
	int bullets_spawned = 0;
	
	for (int i = 0; i < MAX_BULLETS && bullets_spawned < max_b; i++) {
		if (!g_bullets[i].active) {
			g_bullets[i].active = true;
			g_bullets[i].x = g_enemies[e].x + ew / 2;
			g_bullets[i].y = g_enemies[e].y + 12;
			g_bullets[i].is_enemy = true;
			
			if (bullets_spawned == 0) g_bullets[i].vx = 0;
			else if (bullets_spawned == 1) g_bullets[i].vx = -1;
			else if (bullets_spawned == 2) g_bullets[i].vx = 1;
			else if (bullets_spawned == 3) g_bullets[i].vx = -2;
			else if (bullets_spawned == 4) g_bullets[i].vx = 2;
			
			bullets_spawned++;
		}
	}
}

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

static void spawn_carrier_minion(int spawn_x, int spawn_y) {
	for (int ne = 0; ne < MAX_ENEMIES; ne++) {
		if (!g_enemies[ne].active) {
			g_enemies[ne].active = true;
			g_enemies[ne].type = 1;
			g_enemies[ne].hp = 1;
			g_enemies[ne].blink_timer = 0;
			g_enemies[ne].x = spawn_x;
			g_enemies[ne].y = spawn_y;
			break;
		}
	}
}

void game_carrier_update(int e) {
	if (g_tick_count > 0 && g_tick_count % 120 == 0) {
		int spawn_x = g_enemies[e].x + 4;
		int spawn_y = g_enemies[e].y + 12;
		
		if (is_space_clear_below(e, spawn_x, spawn_y)) {
			spawn_carrier_minion(spawn_x, spawn_y);
		}
	}
}

// Handle enemy shooting
static void handle_enemy_shooting(int e, int ew, int boss_max_hp) {
	int shoot_chance = 0;
	if (g_enemies[e].type == 4) {
		if (g_enemies[e].state == BOSS_STATE_NORMAL) {
			int boss_cycle = g_stage / 3;
			shoot_chance = 9 + (g_game_setting.difficulty * 5) + ((boss_cycle - 1) * 5);
			if (g_enemies[e].hp <= boss_max_hp / 2) shoot_chance += 10;
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

// Reversal and vertical drop
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

void game_enemy_task(ak_msg_t* msg) {
	switch (msg->sig) {
		case AC_GAME_UPDATE_TICK:
			game_enemy_update();
			game_powerups_update();
			break;
	}
}
