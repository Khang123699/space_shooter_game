#include "game_shooter_bullet_task.h"
#include "game_shooter_player_task.h"
#include "game_shooter_enemy_task.h"
#include "game_shooter_stage_task.h" // For g_game_setting
#include "game_shooter_render.h" // For game_background_update()
#include "game_save.h"
#include "game_bitmaps.h"
#include "buzzer.h"
#include <stdlib.h>

bullet_t g_bullets[MAX_BULLETS];
explosion_t g_explosions[MAX_EXPLOSIONS];

// AABB 2D Box collision check helper
bool game_check_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
	return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

// Spawn visual explosion animation
void game_spawn_explosion(int x, int y) {
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

// Handle enemy destruction sequence
void game_enemy_kill(int e) {
	g_enemies[e].active = false;
	uint32_t base_score = (g_enemies[e].type == 4) ? 100 : 10;
	g_score += base_score + (base_score * g_game_setting.difficulty) / 2;
	
	// Drop powerup chance (10%)
	if (g_enemies[e].type != 4 && rand() % 100 < 10) {
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
	
	int ew = (g_enemies[e].type >= 4) ? 16 : 8;
	int eh = (g_enemies[e].type == 4) ? 16 : 8;
	game_spawn_explosion(g_enemies[e].x + ew/2 - 4, g_enemies[e].y + eh/2 - 4);
}

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

// Update explosion animation timers
static void update_explosions() {
	if (g_tick_count % 2 == 0) {
		for (int ex = 0; ex < MAX_EXPLOSIONS; ex++) {
			if (g_explosions[ex].active) {
				g_explosions[ex].timer--;
				if (g_explosions[ex].timer <= 0) g_explosions[ex].active = false;
			}
		}
	}
}

// Check physical body collisions between enemy ships and player ship
static void update_enemy_body_collisions() {
	for (int e = 0; e < MAX_ENEMIES; e++) {
		if (!g_enemies[e].active) continue;
		
		int ew = (g_enemies[e].type >= 4) ? 16 : 8;
		int eh = (g_enemies[e].type == 4) ? 16 : 8;
		
		bool hit_player = (g_player_blink == 0 && game_check_collision(g_enemies[e].x, g_enemies[e].y, ew, eh, g_player_x, 54, 8, 8));
		
		if (g_enemies[e].y > 60 || hit_player) {
			if (g_enemies[e].y > 60 || g_enemies[e].type != 4) {
				g_enemies[e].active = false;
			}
			
			if (hit_player) {
				game_player_hit();
				game_spawn_explosion(g_player_x, 54);
				if (g_game_setting.sound_en) BUZZER_PlaySound(BUZZER_SOUND_3BEEP);
			}
		}
	}
}

// Check collisions between player ship and powerup items
static void update_powerup_collisions() {
	for (int p = 0; p < MAX_POWERUPS; p++) {
		if (!g_powerups[p].active) continue;
		
		if (game_check_collision(g_powerups[p].x, g_powerups[p].y, 8, 8, g_player_x, 54, 8, 8)) {
			g_powerups[p].active = false;
			if (g_game_setting.sound_en) BUZZER_PlaySound(BUZZER_SOUND_BANG);
			
			if (g_powerups[p].type == POWERUP_TYPE_SUPER_BULLET) {
				g_player_super_bullet_timer = 200; // 10 seconds (20 ticks per second)
			} else if (g_powerups[p].type == POWERUP_TYPE_SHIELD) {
				g_player_shield_timer = 200; // 10 seconds
			} else if (g_powerups[p].type == POWERUP_TYPE_NUKE) {
				// Deal 1 damage to all enemies on screen
				for (int e = 0; e < MAX_ENEMIES; e++) {
					if (g_enemies[e].active) {
						g_enemies[e].hp--;
						g_enemies[e].blink_timer = 22;
						
						if (g_enemies[e].hp <= 0) {
							g_enemies[e].active = false;
							uint32_t base_score = (g_enemies[e].type == 4) ? 100 : 10;
							g_score += base_score + (base_score * g_game_setting.difficulty) / 2;
							for (int ex = 0; ex < MAX_EXPLOSIONS; ex++) {
								if (!g_explosions[ex].active) {
									g_explosions[ex].active = true;
									g_explosions[ex].x = g_enemies[e].x + (g_enemies[e].type >= 4 ? 4 : 0);
									g_explosions[ex].y = g_enemies[e].y;
									g_explosions[ex].timer = 5;
									break;
								}
							}
						}
					}
				}
			}
		}
	}
}

// Main physics update orchestrator
void game_physics_update() {
	game_background_update();
	game_bullets_update();
	update_explosions();
	update_enemy_body_collisions();
	update_powerup_collisions();
}

void game_bullet_task(ak_msg_t* msg) {
	switch (msg->sig) {
		case AC_GAME_UPDATE_TICK:
			game_physics_update();
			break;
	}
}
