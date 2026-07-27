#include "game_shooter.h"
#include "game_bitmaps.h"
#include "buzzer.h"
#include <stdlib.h>

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

// Handle player being hit by bullet or enemy body
void game_player_hit() {
	if (g_player_shield_timer > 0) {
		g_player_shield_timer = 0;
		g_player_super_bullet_timer = 0;
		g_player_blink = 34;
	} else {
		g_lives--;
		g_player_blink = 34;
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
