#include "game_shooter.h"
#include "game_save.h"
#include <stdlib.h>

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

// Update Boss state machine (NORMAL, DASH_CHARGE, DASH_DOWN, DASH_UP, SUMMON)
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
		g_enemies[e].blink_timer = 2; // Blink rapidly
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

// Execute Boss bullet pattern (3 or 5 spread bullets when enraged)
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
