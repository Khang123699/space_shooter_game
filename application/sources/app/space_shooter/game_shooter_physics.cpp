#include "game_shooter.h"
#include "game_bitmaps.h"
#include "game_save.h"
#include "buzzer.h"

static bool check_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
	return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

// Pixel-perfect collision for player ship
static bool check_player_pixel_collision(int bullet_x, int bullet_y) {
	if (!check_collision(bullet_x, bullet_y, 2, 4, g_player_x, 54, 8, 8)) return false;
	
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

void game_physics_update() {
	// 0. Update Stars (Parallax Background)
	for (int i = 0; i < MAX_STARS; i++) {
		g_stars[i].y += g_stars[i].speed;
		if (g_stars[i].y >= 64) {
			g_stars[i].y = 0;
			g_stars[i].x = rand() % 128;
			g_stars[i].speed = (rand() % 2) + 1;
		}
	}

	// 1. Check collisions FIRST, then update bullet movement
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (g_bullets[i].active) {
			
			// --- A. COLLISION CHECK ---
			if (g_bullets[i].is_enemy) {
				// Hit player: Check Pixel-Perfect collision
				if (g_player_blink == 0 && check_player_pixel_collision(g_bullets[i].x, g_bullets[i].y)) {
					g_bullets[i].active = false;
					if (g_player_shield_timer > 0) {
						g_player_shield_timer = 0;
						g_player_super_bullet_timer = 0;
						g_player_blink = 34;
					} else {
						g_lives--;
						g_player_blink = 34;
					}
					if(g_game_data.sound_en) BUZZER_PlaySound(BUZZER_SOUND_3BEEP);
					continue; // Skip movement if destroyed
				}
			} else {
				for (int e = 0; e < MAX_ENEMIES; e++) {
					if (g_enemies[e].active) {
						// Determine dynamic bounding box based on enemy type (Boss, Carrier, Spread are 16 wide)
						int ew = (g_enemies[e].type >= 4) ? 16 : 8;
						int eh = (g_enemies[e].type == 4) ? 16 : 8; // Boss is 16x16, Carrier/Spread are 16x8
						
						// Check AABB collision between bullet and enemy
						if (check_collision(g_bullets[i].x, g_bullets[i].y, 1, 4, g_enemies[e].x, g_enemies[e].y, ew, eh)) {
							g_bullets[i].active = false;
							int damage = (g_player_super_bullet_timer > 0) ? 3 : 1;
							g_enemies[e].hp -= damage;
							g_enemies[e].blink_timer = 22;
							if(g_game_data.sound_en) BUZZER_PlaySound(BUZZER_SOUND_CLICK);
							if (g_enemies[e].hp <= 0) {
								g_enemies[e].active = false;
								uint32_t base_score = (g_enemies[e].type == 4) ? 100 : 10;
								g_score += base_score + (base_score * g_game_data.difficulty) / 2;
								
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
								
								// Spawn explosion
								for (int ex = 0; ex < MAX_EXPLOSIONS; ex++) {
									if (!g_explosions[ex].active) {
										g_explosions[ex].active = true;
										g_explosions[ex].x = g_enemies[e].x + ew/2 - 4;
										g_explosions[ex].y = g_enemies[e].y + eh/2 - 4;
										g_explosions[ex].timer = 5;
										break;
									}
								}
							}
							break; // Stop checking other enemies for this bullet
						}
					}
				}
				if (!g_bullets[i].active) continue; // Skip movement if destroyed
			}

			// --- B. MOVEMENT UPDATE ---
			g_bullets[i].x += g_bullets[i].vx;
			if (g_bullets[i].is_enemy) {
				int drop = 1;
				if (g_game_data.difficulty == 2 || g_stage >= 10) drop = 2; // Capped at 2px/frame
				
				bool move_enemy_bullet = (g_game_data.difficulty > 0) || (g_stage > 3) || (g_tick_count % 2 == 0);
				
				if (move_enemy_bullet) {
					g_bullets[i].y += drop;
					if (g_bullets[i].y > 64) g_bullets[i].active = false;
				}
			} else {
				// Average speed: 1.4 px/frame (7 px per 5 frames)
				int speed = (g_tick_count % 5 < 2) ? 2 : 1; 
				g_bullets[i].y -= speed;
				if (g_bullets[i].y < 0) g_bullets[i].active = false;
			}
		}
	} 
	
	// 2. Explosion timer update
	if (g_tick_count % 2 == 0) {
		for (int ex = 0; ex < MAX_EXPLOSIONS; ex++) {
			if (g_explosions[ex].active) {
				g_explosions[ex].timer--;
				if (g_explosions[ex].timer <= 0) g_explosions[ex].active = false;
			}
		}
	}
	
	// 3. Enemy body collision with player or bottom edge
	for (int e = 0; e < MAX_ENEMIES; e++) {
		if (g_enemies[e].active) {
			int ew = (g_enemies[e].type >= 4) ? 16 : 8;
			int eh = (g_enemies[e].type == 4) ? 16 : 8;
			
			bool hit_player = (g_player_blink == 0 && check_collision(g_enemies[e].x, g_enemies[e].y, ew, eh, g_player_x, 54, 8, 8));
			
			if (g_enemies[e].y > 60 || hit_player) {
				g_enemies[e].active = false;
				
				if (hit_player && g_player_shield_timer > 0) {
					g_player_shield_timer = 0;
					g_player_super_bullet_timer = 0;
					g_player_blink = 34;
				} else if (hit_player) {
					g_lives--;
					g_player_blink = 34;
				}
				
				if (hit_player) {
					for (int ex = 0; ex < MAX_EXPLOSIONS; ex++) {
						if (!g_explosions[ex].active) {
							g_explosions[ex].x = g_player_x;
							g_explosions[ex].y = 54;
							g_explosions[ex].timer = 5;
							g_explosions[ex].active = true;
							break;
						}
					}
					if(g_game_data.sound_en) BUZZER_PlaySound(BUZZER_SOUND_3BEEP);
				}
			}
		}
	}
	
	// 4. Powerup collision with player
	for (int p = 0; p < MAX_POWERUPS; p++) {
		if (g_powerups[p].active) {
			if (check_collision(g_powerups[p].x, g_powerups[p].y, 8, 8, g_player_x, 54, 8, 8)) {
				g_powerups[p].active = false;
				if(g_game_data.sound_en) BUZZER_PlaySound(BUZZER_SOUND_BANG);
				
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
								g_score += base_score + (base_score * g_game_data.difficulty) / 2;
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
}
