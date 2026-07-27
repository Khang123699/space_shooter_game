#include "game_shooter.h"
#include "game_bitmaps.h"
#include "buzzer.h"

// Pixel-perfect collision check between bullet and player ship
static bool check_player_pixel_collision(int bullet_x, int bullet_y) {
	if (!game_check_collision(bullet_x, bullet_y, 2, 4, g_player_x, 54, 8, 8)) return false;
	
	int start_x = bullet_x - g_player_x;
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
		if (g_player_blink == 0 && check_player_pixel_collision(g_bullets[i].x, g_bullets[i].y)) {
			g_bullets[i].active = false;
			game_player_hit();
			if (g_game_setting.sound_en) BUZZER_PlaySound(BUZZER_SOUND_3BEEP);
			return true;
		}
	} else {
		for (int e = 0; e < MAX_ENEMIES; e++) {
			if (!g_enemies[e].active) continue;
			
			int ew = (g_enemies[e].type >= 4) ? 16 : 8;
			int eh = (g_enemies[e].type == 4) ? 16 : 8;
			
			// Fast vertical bounding check (AABB rejection) to optimize calculation loop
			if (g_bullets[i].y + 4 < g_enemies[e].y || g_bullets[i].y > g_enemies[e].y + eh) continue;
			
			if (game_check_collision(g_bullets[i].x, g_bullets[i].y, 1, 4, g_enemies[e].x, g_enemies[e].y, ew, eh)) {
				g_bullets[i].active = false;
				int damage = (g_player_super_bullet_timer > 0) ? 3 : 1;
				g_enemies[e].hp -= damage;
				g_enemies[e].blink_timer = 22;
				if (g_game_setting.sound_en) BUZZER_PlaySound(BUZZER_SOUND_CLICK);
				
				if (g_enemies[e].hp <= 0) {
					game_enemy_kill(e);
				}
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
		if (g_game_setting.difficulty == 2 || g_stage >= 10) drop = 2; // Capped at 2px/frame
		bool move_enemy_bullet = (g_game_setting.difficulty > 0) || (g_stage > 3) || (g_tick_count % 2 == 0);
		if (move_enemy_bullet) {
			g_bullets[i].y += drop;
			if (g_bullets[i].y > 64) g_bullets[i].active = false;
		}
	} else {
		int speed = (g_tick_count % 5 < 2) ? 2 : 1;
		g_bullets[i].y -= speed;
		if (g_bullets[i].y < 0) g_bullets[i].active = false;
	}
}

// Main bullet update loop
void game_bullets_update() {
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (!g_bullets[i].active) continue;
		if (process_bullet_collision(i)) continue;
		update_bullet_movement(i);
	}
}
