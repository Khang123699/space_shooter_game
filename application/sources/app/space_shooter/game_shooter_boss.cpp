#include "game_shooter_boss.h"
#include "game_shooter_enemy_task.h"
#include "game_shooter_stage_task.h"
#include "game_shooter_bullet_task.h"
#include "game_shooter_player_task.h"
#include "task_list.h"
#include "task_list.h"
#include "app.h"
#include "timer.h"
#include <stdlib.h>
#include "app_dbg.h"
#include "game_save.h"

// Boss spawn function
static void game_boss_spawn() {

	g_enemies[0].type = ENEMY_TYPE_BOSS; // Boss type
	int boss_cycle = game_get_stage() / 3;
	g_enemies[0].hp = 10 + (boss_cycle - 1) * 5;
	g_enemies[0].blink_timer = 0;
	g_enemies[0].x = 56;
	g_enemies[0].y = 16;
	g_enemies[0].state = BOSS_STATE_NORMAL;
	g_enemies[0].timer = 0;
}

// Summon 2 minions to protect the boss
static void spawn_boss_minions(int boss_idx) {
	int minions_spawned = 0;
	for (int ne = 0; ne < MAX_ENEMIES && minions_spawned < 2; ne++) {
		if (g_enemies[ne].state == ENEMY_STATE_INACTIVE) {
			g_enemies[ne].state = ENEMY_STATE_IDLE;
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

// Complex state machine for boss movement and attacks
static void game_boss_update_state(int e, bool do_move, int move_threshold, int boss_max_hp, bool& hit_edge) {
	int ew = 16;
	bool is_enraged = (g_enemies[e].hp <= boss_max_hp / 2);
	
	if (g_enemies[e].state == BOSS_STATE_NORMAL) {
		if (do_move || (is_enraged && g_enemy_move_ticks % (move_threshold > 1 ? move_threshold - 1 : 1) == 0)) {
			g_enemies[e].x += g_enemy_dir;
			if (g_enemies[e].x <= 0) {
				g_enemies[e].x = 0;
				hit_edge = true;
			} else if (g_enemies[e].x + ew >= 128) {
				g_enemies[e].x = 128 - ew;
				hit_edge = true;
			}
		}
		
		if (game_get_tick_count() > 0 && game_get_tick_count() % 60 == 0) {
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

// Execute Boss bullet pattern (sends messages to Bullet Task)
static void game_boss_shoot() {
	int e = 0;
	int ew = 16;
	int boss_max_hp = 10 + ((game_get_stage() / 3) - 1) * 5;
	bool is_enraged = (g_enemies[e].hp <= boss_max_hp / 2);
	int max_b = is_enraged ? 5 : 3;
	
	int8_t vx_pattern[] = {0, -1, 1, -2, 2};
	for (int b = 0; b < max_b; b++) {
		game_bullet_spawn_msg_t spawn_msg;
		spawn_msg.x = g_enemies[e].x + ew / 2;
		spawn_msg.y = g_enemies[e].y + 12;
		spawn_msg.is_enemy = true;
		spawn_msg.vx = vx_pattern[b];
		task_post_common_msg(AC_TASK_GAME_BULLET_ID, AC_GAME_SPAWN_BULLET, (uint8_t*)&spawn_msg, sizeof(spawn_msg));
	}
}

void game_boss_handle(ak_msg_t* msg) {
	switch (msg->sig) {
		case AC_GAME_START_REQ:
		case AC_GAME_SPAWN_ENEMY:
		{
			APP_DBG_SIG("AC_GAME_BOSS_START_REQ/SPAWN_ENEMY\n");
			if (game_get_stage() % 3 == 0) {
				game_boss_spawn();
			}
		}
		break;
			
		case AC_GAME_UPDATE_TICK:
		{
			if (g_enemies[0].state == ENEMY_STATE_INACTIVE || g_enemies[0].type != 4) break;
			
			int move_threshold = 4 - g_game_setting.difficulty - (game_get_stage() / 5);
			if (game_get_stage() % 3 == 0) move_threshold--;
			if (move_threshold < 1) move_threshold = 1;
			bool do_move = (g_enemy_move_ticks >= move_threshold);
			int boss_max_hp = 10 + ((game_get_stage() / 3) - 1) * 5;
			
			bool hit_edge = false;
			game_boss_update_state(0, do_move, move_threshold, boss_max_hp, hit_edge);
		}
		break;
			
		case AC_GAME_BOSS_SHOOT:
		{
			APP_DBG_SIG("AC_GAME_BOSS_SHOOT\n");
			game_boss_shoot();
		}
		break;
	}
}
