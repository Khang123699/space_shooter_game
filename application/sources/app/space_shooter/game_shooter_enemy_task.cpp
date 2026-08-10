#include "game_shooter_enemy_task.h"
#include "game_shooter_player_task.h"
#include "game_shooter_bullet_task.h"
#include "game_shooter_stage_task.h" 
#include "game_shooter_boss.h"
#include "game_shooter_powerup.h"
#include "game_save.h"
#include "app.h"
#include "task_list.h"
#include "buzzer.h"
#include <stdlib.h>
#include "app_dbg.h"

enemy_t g_enemies[MAX_ENEMIES];
powerup_t g_powerups[MAX_POWERUPS];

int8_t g_enemy_dir = 1;
uint8_t g_enemy_move_ticks = 0;

const enemy_t* game_get_enemies() { return g_enemies; }
const powerup_t* game_get_powerups() { return g_powerups; }

// Handle enemy destruction sequence
void game_enemy_kill(int e) {
	g_enemies[e].active = false;
	uint32_t base_score = (g_enemies[e].type == ENEMY_TYPE_BOSS) ? 100 : 10;
	uint32_t additional_score = base_score + (base_score * g_game_setting.difficulty) / 2;
	
    game_score_update_msg_t score_msg = {additional_score};
    task_post_dynamic_msg(AC_TASK_GAME_PLAYER_ID, AC_GAME_SCORE_UPDATE, (uint8_t*)&score_msg, sizeof(score_msg));
	
	// Drop powerup chance (10%)
	if (g_enemies[e].type != ENEMY_TYPE_BOSS && rand() % 100 < 10) {
		for (int p = 0; p < MAX_POWERUPS; p++) {
			if (!g_powerups[p].active) {
				g_powerups[p].active = true;
				g_powerups[p].x = g_enemies[e].x;
				g_powerups[p].y = g_enemies[e].y;
				g_powerups[p].type = 1 + (rand() % 3);
				break;
			}
		}
	}
	
	int ew = (g_enemies[e].type >= ENEMY_TYPE_BOSS) ? 16 : 8;
	int eh = (g_enemies[e].type == ENEMY_TYPE_BOSS) ? 16 : 8;
	game_explosion_msg_t exp_msg = {(int16_t)(g_enemies[e].x + ew/2 - 4), (int16_t)(g_enemies[e].y + eh/2 - 4)};
	task_post_dynamic_msg(AC_TASK_GAME_BULLET_ID, AC_GAME_SPAWN_EXPLOSION, (uint8_t*)&exp_msg, sizeof(exp_msg));
}

#define SPAWN_START_X 8
#define SPAWN_START_Y 16
#define SPAWN_OFFSET_X 16
#define SPAWN_OFFSET_Y 12

// Initial wave generation
static void game_enemy_spawn() {
	if (game_get_stage() % 3 == 0) { // Boss stage every 3 stages
		return;
	}
	
	int e = 0;
	int rows = 3;
	int cols = 6;
	int spawn_chance = 40 + (g_game_setting.difficulty * 10) + (game_get_stage() * 3);
	if (spawn_chance > 90) spawn_chance = 90;
	bool spawned_type5 = false;
	bool spawned_type6 = false;
	
	for (int r = 0; r < rows; r++) {
		for (int c = 0; c < cols; c++) {
			if (rand() % 100 < spawn_chance) {
				g_enemies[e].active = true;
				
				int r_val = rand() % 100;
				int type6_chance = 15 + (game_get_stage() / 2);
				if (type6_chance > 30) type6_chance = 30;
				
				if (r == 0 && r_val < type6_chance && !spawned_type6) {
					g_enemies[e].type = ENEMY_TYPE_CARRIER; // Carrier
					spawned_type6 = true;
					g_enemies[e].hp = 4;
				} else if (r == 0 && r_val >= type6_chance && r_val < type6_chance + 20 && !spawned_type5) {
					g_enemies[e].type = ENEMY_TYPE_SPREAD; // Spread Shooter
					spawned_type5 = true;
					g_enemies[e].hp = 3;
				} else {
					g_enemies[e].type = 1 + (rand() % 3);
					g_enemies[e].hp = g_enemies[e].type;
				}
				
				g_enemies[e].blink_timer = 0;
				g_enemies[e].x = 20 + c * SPAWN_OFFSET_X;
				g_enemies[e].y = SPAWN_START_Y + r * SPAWN_OFFSET_Y;
				
				if (g_enemies[e].type == ENEMY_TYPE_SPREAD || g_enemies[e].type == ENEMY_TYPE_CARRIER) {
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

static bool is_space_clear_below(int carrier_idx, int spawn_x, int spawn_y) {
	for (int c_e = 0; c_e < MAX_ENEMIES; c_e++) {
		if (g_enemies[c_e].active && c_e != carrier_idx) {
			int ew2 = (g_enemies[c_e].type >= ENEMY_TYPE_BOSS) ? 16 : 8;
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

static void game_carrier_update(int e) {
	uint16_t tick = game_get_tick_count();
	if (tick > 0 && tick % 120 == 0) {
		int spawn_x = g_enemies[e].x + 4;
		int spawn_y = g_enemies[e].y + 12;
		
		if (is_space_clear_below(e, spawn_x, spawn_y)) {
			spawn_carrier_minion(spawn_x, spawn_y);
		}
	}
}

// Handle enemy shooting
static void handle_enemy_shooting(int e, int ew, int boss_max_hp) {
	uint8_t stage = game_get_stage();
	int shoot_chance = 0;
	switch (g_enemies[e].type) {
		case ENEMY_TYPE_BOSS:
			if (g_enemies[e].state == BOSS_STATE_NORMAL) {
				int boss_cycle = stage / 3;
				shoot_chance = 9 + (g_game_setting.difficulty * 5) + ((boss_cycle - 1) * 5);
				if (g_enemies[e].hp <= boss_max_hp / 2) shoot_chance += 10;
			}
			break;
		case ENEMY_TYPE_SPREAD:
			shoot_chance = 5 + g_game_setting.difficulty * 2 + (stage / 2);
			break;
		case ENEMY_TYPE_CARRIER:
			shoot_chance = 0;
			break;
		default:
			shoot_chance = 3 + g_game_setting.difficulty + (stage / 2);
			break;
	}
	
	if (shoot_chance > 0 && rand() % 1000 < shoot_chance) {
		switch (g_enemies[e].type) {
			case ENEMY_TYPE_BOSS:
				task_post_pure_msg(AC_TASK_GAME_ENEMY_ID, AC_GAME_BOSS_SHOOT);
				break;
			case ENEMY_TYPE_SPREAD:
			{
				int8_t vx_spread[] = {0, -1, 1};
				for (int b = 0; b < 3; b++) {
					game_bullet_spawn_msg_t spawn_msg;
					spawn_msg.x = g_enemies[e].x + ew / 2;
					spawn_msg.y = g_enemies[e].y + 8;
					spawn_msg.is_enemy = true;
					spawn_msg.vx = vx_spread[b];
					task_post_dynamic_msg(AC_TASK_GAME_BULLET_ID, AC_GAME_SPAWN_BULLET, (uint8_t*)&spawn_msg, sizeof(spawn_msg));
				}
			}
			break;
			default:
			{
				game_bullet_spawn_msg_t spawn_msg;
				spawn_msg.x = g_enemies[e].x + ew / 2;
				spawn_msg.y = g_enemies[e].y + 8;
				spawn_msg.is_enemy = true;
				spawn_msg.vx = 0;
				task_post_dynamic_msg(AC_TASK_GAME_BULLET_ID, AC_GAME_SPAWN_BULLET, (uint8_t*)&spawn_msg, sizeof(spawn_msg));
			}
			break;
		}
	}
}

// Reversal and vertical drop
static void handle_edge_reversal(bool hit_edge) {
	if (!hit_edge) return;
	
	g_enemy_dir = -g_enemy_dir;
	static uint8_t edge_hit_count = 0;
	edge_hit_count++;
	
	for (int e = 0; e < MAX_ENEMIES; e++) {
		if (g_enemies[e].active) {
			switch (g_enemies[e].type) {
				case ENEMY_TYPE_BOSS:
					if (g_enemies[e].state == BOSS_STATE_NORMAL) {
						g_enemies[e].x += g_enemy_dir;
					}
					break;
				default:
					g_enemies[e].x += g_enemy_dir;
					break;
			}
			
			int edge_hits_required = (game_get_stage() > 10) ? 1 : 2;
			int drop_amount = 1 + (game_get_stage() / 5);
			if (drop_amount > 4) drop_amount = 4;
			
			if (edge_hit_count >= edge_hits_required) {
				switch (g_enemies[e].type) {
					case ENEMY_TYPE_BOSS:
					case ENEMY_TYPE_SPREAD:
					case ENEMY_TYPE_CARRIER:
						break;
					default:
						g_enemies[e].y += drop_amount;
						break;
				}
			}
		}
	}
	
	int reset_hits = (game_get_stage() > 10) ? 1 : 2;
	if (edge_hit_count >= reset_hits) edge_hit_count = 0;
}

// Main logic for moving enemies
static void game_enemy_update() {
	bool hit_edge = false;
	g_enemy_move_ticks++;
	
	// Update enemy blink timers
	for (int e = 0; e < MAX_ENEMIES; e++) {
		if (g_enemies[e].active && g_enemies[e].blink_timer > 0) {
			g_enemies[e].blink_timer--;
		}
	}
	
	int move_threshold = 4 - g_game_setting.difficulty - (game_get_stage() / 5);
	if (game_get_stage() % 3 == 0) move_threshold--;
	if (move_threshold < 1) move_threshold = 1;
	bool do_move = (g_enemy_move_ticks >= move_threshold);
	
	int boss_max_hp = 10 + ((game_get_stage() / 3) - 1) * 5;

	for (int e = 0; e < MAX_ENEMIES; e++) {
		if (!g_enemies[e].active) continue;
		
		int ew = (g_enemies[e].type >= ENEMY_TYPE_BOSS) ? 16 : 8;
		
		switch (g_enemies[e].type) {
			case ENEMY_TYPE_BOSS:
				if (do_move) {
					if (g_enemies[e].x <= 0 || g_enemies[e].x + ew >= 128) hit_edge = true;
				}
				break;
			case ENEMY_TYPE_CARRIER:
				if (do_move) {
					g_enemies[e].x += g_enemy_dir;
					if (g_enemies[e].x <= 0 || g_enemies[e].x + ew >= 128) hit_edge = true;
				}
				game_carrier_update(e);
				break;
			default:
				if (do_move) {
					g_enemies[e].x += g_enemy_dir;
					if (g_enemies[e].x <= 0 || g_enemies[e].x + ew >= 128) hit_edge = true;
				}
				break;
		}
		
		handle_enemy_shooting(e, ew, boss_max_hp);
	}
	
	if (do_move) {
		g_enemy_move_ticks = 0;
		handle_edge_reversal(hit_edge);
	}
}

// Check physical body collisions between enemy ships and player ship
static void update_enemy_body_collisions() {
	int player_x = game_get_player_x();
	int player_blink = game_get_player_blink();
	for (int e = 0; e < MAX_ENEMIES; e++) {
		if (!g_enemies[e].active) continue;
		
		int ew = (g_enemies[e].type >= ENEMY_TYPE_BOSS) ? 16 : 8;
		int eh = (g_enemies[e].type == ENEMY_TYPE_BOSS) ? 16 : 8;
		
		bool hit_player = (player_blink == 0 && game_check_collision(g_enemies[e].x, g_enemies[e].y, ew, eh, player_x, 54, 8, 8));
		
		if (g_enemies[e].y > 60 || hit_player) {
			if (g_enemies[e].y > 60 || g_enemies[e].type != ENEMY_TYPE_BOSS) {
				g_enemies[e].active = false; // Out of bounds or destroyed on impact
			}
			
			if (hit_player) {
				task_post_pure_msg(AC_TASK_GAME_PLAYER_ID, AC_GAME_PLAYER_HIT);
				game_explosion_msg_t exp_msg = {(int16_t)player_x, 54};
				task_post_dynamic_msg(AC_TASK_GAME_BULLET_ID, AC_GAME_SPAWN_EXPLOSION, (uint8_t*)&exp_msg, sizeof(exp_msg));
				if (g_game_setting.sound_en) BUZZER_PlaySound(BUZZER_SOUND_3BEEP);
			}
		}
	}
}

void game_enemy_task(ak_msg_t* msg) {
	switch (msg->sig) {
		case AC_GAME_START_REQ:
		{
			APP_DBG_SIG("AC_GAME_START_REQ\n");
			for (int i = 0; i < MAX_ENEMIES; i++) g_enemies[i].active = false;
			game_powerup_handle(msg);
			g_enemy_dir = 1;
			g_enemy_move_ticks = 0;
			game_enemy_spawn();
			game_boss_handle(msg);
		}
		break;
			
		case AC_GAME_SPAWN_ENEMY:
		{
			APP_DBG_SIG("AC_GAME_SPAWN_ENEMY\n");
			game_enemy_spawn();
			game_boss_handle(msg);
		}
		break;
			
		case AC_GAME_UPDATE_TICK:
		{
			game_boss_handle(msg);
			game_enemy_update();
			update_enemy_body_collisions();
			game_powerup_handle(msg);
		}
		break;
			
		case AC_GAME_BOSS_SHOOT:
		{
			game_boss_handle(msg);
		}
		break;
			
		case AC_GAME_ENEMY_HIT: 
		{
			APP_DBG_SIG("AC_GAME_ENEMY_HIT\n");
			game_enemy_hit_msg_t* hit_msg = (game_enemy_hit_msg_t*)get_data_common_msg(msg);
			int e = hit_msg->enemy_index;
			if (g_enemies[e].active) {
				g_enemies[e].hp -= hit_msg->damage;
				g_enemies[e].blink_timer = 22;
				if (g_enemies[e].hp <= 0) {
					game_enemy_kill(e);
				}
			}
		}
		break;
	}
}
