#include "game_shooter_bullet_task.h"
#include "game_shooter_player_task.h"
#include "game_shooter_enemy_task.h"
#include "game_shooter_stage_task.h" 
#include "game_shooter_render.h" 
#include "game_save.h"
#include "game_bitmaps.h"
#include "buzzer.h"
#include <stdlib.h>
#include "app_dbg.h"
#include "app.h"
#include "task_list.h"

static bullet_t g_bullets[MAX_BULLETS];
static explosion_t g_explosions[MAX_EXPLOSIONS];

const bullet_t* game_get_bullets() { return g_bullets; }
const explosion_t* game_get_explosions() { return g_explosions; }

// AABB 2D Box collision check helper
bool game_check_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
	return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

// Spawn visual explosion animation
static void game_spawn_explosion(int x, int y) {
	for (int ex = 0; ex < MAX_EXPLOSIONS; ex++) {
		if (!g_explosions[ex].active) {
			g_explosions[ex].active = true;
			g_explosions[ex].x = x;
			g_explosions[ex].y = y;
			g_explosions[ex].timer = 5;
			break;
		}
	}
}

// Pixel-perfect collision check between bullet and player ship
static bool check_player_pixel_collision(int bullet_x, int bullet_y) {
	int player_x = game_get_player_x();
	if (!game_check_collision(bullet_x, bullet_y, 2, 4, player_x, 54, 8, 8)) return false;
	
	int start_x = bullet_x - player_x;
	int end_x = start_x + 2; 
	int start_y = bullet_y - 54;
	int end_y = start_y + 4; 
	
	if (start_x < 0) start_x = 0; 
	if (end_x > 8) end_x = 8;
	if (start_y < 0) start_y = 0; 
	if (end_y > 8) end_y = 8;
	
	for (int y = start_y; y < end_y; y++) {
		uint8_t row_pixels = icon_player[y];
		for (int x = start_x; x < end_x; x++) {
			if (row_pixels & (1 << (7 - x))) {
				return true; 
			}
		}
	}
	return false;
}

// Process collision for bullet index i against player or enemies
static bool process_bullet_collision(int i) {
	if (g_bullets[i].is_enemy) {
		if (game_get_player_blink() == 0 && check_player_pixel_collision(g_bullets[i].x, g_bullets[i].y)) {
			g_bullets[i].active = false;
			task_post_pure_msg(AC_TASK_GAME_PLAYER_ID, AC_GAME_PLAYER_HIT);
			return true;
		}
	} else {
	    const enemy_t* enemies = game_get_enemies();
		for (int e = 0; e < MAX_ENEMIES; e++) {
			if (!enemies[e].active) continue;
			
			int ew = (enemies[e].type >= 4) ? 16 : 8;
			int eh = (enemies[e].type == 4) ? 16 : 8;
			
			// Fast vertical bounding check (AABB rejection) to optimize calculation loop
			if (g_bullets[i].y + 4 < enemies[e].y || g_bullets[i].y > enemies[e].y + eh) continue;
			
			if (game_check_collision(g_bullets[i].x, g_bullets[i].y, 1, 4, enemies[e].x, enemies[e].y, ew, eh)) {
				g_bullets[i].active = false;
				int damage = (game_get_player_super_bullet_timer() > 0) ? 3 : 1;
				
				game_enemy_hit_msg_t hit_msg = {(uint8_t)e, (uint8_t)damage};
				task_post_dynamic_msg(AC_TASK_GAME_ENEMY_ID, AC_GAME_ENEMY_HIT, (uint8_t*)&hit_msg, sizeof(hit_msg));
				
				return true;
			}
		}
	}
	return false;
}

// Update trajectory position of bullet index i
static void update_bullet_movement(int i) {
	g_bullets[i].x += g_bullets[i].vx;
	if (g_bullets[i].is_enemy) {
		int drop = 1;
		if (g_game_setting.difficulty == 2 || game_get_stage() >= 10) drop = 2; // Capped at 2px/frame
		bool move_enemy_bullet = (g_game_setting.difficulty > 0) || (game_get_stage() > 3) || (game_get_tick_count() % 2 == 0);
		if (move_enemy_bullet) {
			g_bullets[i].y += drop;
			if (g_bullets[i].y > 64) g_bullets[i].active = false;
		}
	} else {
		int speed = (game_get_tick_count() % 5 < 2) ? 2 : 1;
		g_bullets[i].y -= speed;
		if (g_bullets[i].y < 0) g_bullets[i].active = false;
	}
}

// Main bullet update loop
static void game_bullets_update() {
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (!g_bullets[i].active) continue;
		if (process_bullet_collision(i)) continue;
		update_bullet_movement(i);
	}
}

// Update explosion animation timers
static void update_explosions() {
	if (game_get_tick_count() % 2 == 0) {
		for (int ex = 0; ex < MAX_EXPLOSIONS; ex++) {
			if (g_explosions[ex].active) {
				g_explosions[ex].timer--;
				if (g_explosions[ex].timer <= 0) g_explosions[ex].active = false;
			}
		}
	}
}

// Main physics update orchestrator
static void game_physics_update(ak_msg_t* msg) {
	game_render_handle(msg);
	game_bullets_update();
	update_explosions();
}

void game_bullet_task(ak_msg_t* msg) {
	switch (msg->sig) {
		case AC_GAME_START_REQ:
		{
			APP_DBG_SIG("AC_GAME_START_REQ\n");
			for (int i = 0; i < MAX_BULLETS; i++) g_bullets[i].active = false;
			for (int i = 0; i < MAX_EXPLOSIONS; i++) g_explosions[i].active = false;
		}
		break;
		
		case AC_GAME_UPDATE_TICK:
		{
			game_physics_update(msg);
		}
		break;
			
		case AC_GAME_SPAWN_BULLET: 
		{
			APP_DBG_SIG("AC_GAME_SPAWN_BULLET\n");
			game_bullet_spawn_msg_t* spawn_msg = (game_bullet_spawn_msg_t*)get_data_common_msg(msg);
			for (int i = 0; i < MAX_BULLETS; i++) {
				if (!g_bullets[i].active) {
					g_bullets[i].active = true;
					g_bullets[i].is_enemy = spawn_msg->is_enemy;
					g_bullets[i].x = spawn_msg->x;
					g_bullets[i].y = spawn_msg->y;
					g_bullets[i].vx = spawn_msg->vx;
					break;
				}
			}
		}
		break;
		
		case AC_GAME_SPAWN_EXPLOSION: 
		{
			APP_DBG_SIG("AC_GAME_SPAWN_EXPLOSION\n");
			game_explosion_msg_t* exp_msg = (game_explosion_msg_t*)get_data_common_msg(msg);
			game_spawn_explosion(exp_msg->x, exp_msg->y);
		}
		break;
	}
}
