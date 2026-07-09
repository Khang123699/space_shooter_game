#include "game_shooter.h"
#include "task_list.h"
#include "timer.h"
#include "buzzer.h"
#include <string.h>
#include <stdlib.h>
#include "app_dbg.h"
#include "game_save.h"

// Global UI navigation
uint8_t g_menu_selected = 0;
uint8_t g_setting_selected = 0;
uint8_t g_score_selected = 0;
uint8_t g_show_score_selected = 0;
const char* g_encouragement_text = "Good Job!";

// Exported global variables for UI and Logic
game_state_t g_game_state = GAME_STATE_MENU;
int16_t g_player_x = 60;
uint8_t g_player_blink = 0;
uint32_t g_score = 0;
uint8_t g_lives = 3;

static uint8_t g_shoot_cooldown = 0;
static uint16_t g_tick_count = 0;

enemy_t g_enemies[MAX_ENEMIES];
bullet_t g_bullets[MAX_BULLETS];
explosion_t g_explosions[MAX_EXPLOSIONS];
uint8_t g_stage = 1;
int8_t g_transition_timer = 0;

static int8_t enemy_dir = 1;
static int enemy_move_ticks = 0;

static void spawn_enemies() {
	if (g_stage % 5 == 0) {
		g_enemies[0].active = true;
		g_enemies[0].type = 4;
		g_enemies[0].x = 56;
		g_enemies[0].y = 12;
		g_enemies[0].hp = 5 + (g_stage / 5) * 5;
		for (int i = 1; i < MAX_ENEMIES; i++) g_enemies[i].active = false;
		return;
	}
	
	// Normal: 3 rows, 6 columns
	int spawn_chance = 45 + (g_game_data.difficulty * 10);
	int e = 0;
	
	for (int i = 0; i < MAX_ENEMIES; i++) g_enemies[i].active = false;
	
	for (int row = 0; row < 3; row++) {
		for (int col = 0; col < 6; col++) {
			if (e >= MAX_ENEMIES) break;
			
			if ((rand() % 100) < spawn_chance) {
				g_enemies[e].active = true;
				g_enemies[e].type = 1 + (rand() % 3); // random type 1, 2, or 3
				g_enemies[e].hp = g_enemies[e].type;
				g_enemies[e].x = 8 + col * 20;
				g_enemies[e].y = 10 + row * 12;
				e++;
			}
		}
	}
	
	// Fallback to avoid empty stage
	if (e == 0) {
		g_enemies[0].active = true;
		g_enemies[0].type = 1 + (rand() % 3);
		g_enemies[0].hp = g_enemies[0].type;
		g_enemies[0].x = 56;
		g_enemies[0].y = 12;
	}
}

void game_logic_init() {
	g_player_x = 60;
	g_score = 0;
	g_lives = 3;
	g_stage = 1;
	g_transition_timer = 0;
	enemy_dir = 1;
	memset(g_bullets, 0, sizeof(g_bullets));
	memset(g_explosions, 0, sizeof(g_explosions));
	spawn_enemies();
}

void game_player_move(int8_t dir) {
	g_player_x += dir;
	if (g_player_x < 0) g_player_x = 0;
	if (g_player_x > 120) g_player_x = 120;
}

void game_player_shoot() {
	if (g_shoot_cooldown > 0) return;
	
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (!g_bullets[i].active) {
			g_bullets[i].active = true;
			g_bullets[i].is_enemy = false;
			g_bullets[i].x = g_player_x + 4;
			g_bullets[i].y = 52;
			g_bullets[i].vx = 0;
			g_shoot_cooldown = 3; // Cooldown of 5 ticks (faster fire rate)
			break;
		}
	}
}

void game_logic_update() {
	if (g_player_blink > 0) g_player_blink--;
	if (g_shoot_cooldown > 0) g_shoot_cooldown--;
	g_tick_count++;

	// 1. Update bullets
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (g_bullets[i].active) {
			g_bullets[i].x += g_bullets[i].vx;
			
			if (g_bullets[i].is_enemy) {
				int drop = 0;
				if (g_game_data.difficulty == 0) drop = 1;
				else if (g_game_data.difficulty == 1) drop = 2;
				else if (g_game_data.difficulty == 2) drop = 3;
				
				g_bullets[i].y += drop;
				if (g_bullets[i].y > 64) g_bullets[i].active = false;
				
				// Hit player
				if (g_player_blink == 0 && g_bullets[i].x >= g_player_x && g_bullets[i].x <= g_player_x + 8 && g_bullets[i].y >= 54) {
					g_bullets[i].active = false;
					g_lives--;
					g_player_blink = 15;
					if(g_game_data.sound_en) BUZZER_PlaySound(BUZZER_SOUND_3BEEP);
				}
			} else {
				g_bullets[i].y -= 2;
				if (g_bullets[i].y < 0) g_bullets[i].active = false;
				
				for (int e = 0; e < MAX_ENEMIES; e++) {
					if (g_enemies[e].active) {
						int ew = (g_enemies[e].type == 4) ? 16 : 8;
						int eh = (g_enemies[e].type == 4) ? 16 : 8;
						if (g_bullets[i].x >= g_enemies[e].x && g_bullets[i].x <= g_enemies[e].x + ew 
							&& g_bullets[i].y >= g_enemies[e].y && g_bullets[i].y <= g_enemies[e].y + eh) {
							g_bullets[i].active = false;
							g_enemies[e].hp--;
							if(g_game_data.sound_en) BUZZER_PlaySound(BUZZER_SOUND_BANG);
							if (g_enemies[e].hp <= 0) {
								g_enemies[e].active = false;
								g_score += (g_enemies[e].type == 4) ? 100 : 10;
								for (int ex = 0; ex < MAX_EXPLOSIONS; ex++) {
									if (!g_explosions[ex].active) {
										g_explosions[ex].x = g_enemies[e].x + ew/2 - 4;
										g_explosions[ex].y = g_enemies[e].y + eh/2 - 4;
										g_explosions[ex].timer = 5;
										g_explosions[ex].active = true;
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
	
	for (int ex = 0; ex < MAX_EXPLOSIONS; ex++) {
		if (g_explosions[ex].active) {
			g_explosions[ex].timer--;
			if (g_explosions[ex].timer <= 0) g_explosions[ex].active = false;
		}
	}
	
	// Enemy movement and shooting
	bool all_dead = true;
	bool hit_edge = false;
	enemy_move_ticks++;
	int move_threshold = (g_stage % 5 == 0) ? 2 : (5 - g_game_data.difficulty);
	bool do_move = (enemy_move_ticks >= move_threshold);
	
	for (int e = 0; e < MAX_ENEMIES; e++) {
		if (g_enemies[e].active) {
			all_dead = false;
			int ew = (g_enemies[e].type == 4) ? 16 : 8;
			if (do_move) {
				g_enemies[e].x += (g_enemies[e].type == 4) ? (enemy_dir * 3) : enemy_dir;
				if (g_enemies[e].x <= 0 || g_enemies[e].x + ew >= 128) hit_edge = true;
			}
			
			// Shoot
			int shoot_chance = (g_enemies[e].type == 4) ? (10 + g_game_data.difficulty * 5) : (1 + g_game_data.difficulty);
			if (rand() % 300 < shoot_chance) {
				if (g_enemies[e].type == 4) {
					// Triple shot burst for Boss
					int bullets_spawned = 0;
					for (int i = 0; i < MAX_BULLETS && bullets_spawned < 3; i++) {
						if (!g_bullets[i].active) {
							g_bullets[i].active = true;
							g_bullets[i].x = g_enemies[e].x + ew / 2;
							g_bullets[i].y = g_enemies[e].y + 12;
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
			
			// Touch bottom
			if (g_enemies[e].y > 60) {
				g_lives--;
				g_enemies[e].active = false;
				g_player_blink = 15;
				if(g_game_data.sound_en) BUZZER_PlaySound(BUZZER_SOUND_3BEEP);
			}
		}
	}
	
	if (do_move) {
		enemy_move_ticks = 0;
		if (hit_edge) {
			enemy_dir = -enemy_dir;
			for (int e = 0; e < MAX_ENEMIES; e++) {
				if (g_enemies[e].active) {
					g_enemies[e].x += enemy_dir * 2;
					g_enemies[e].y += 4;
				}
			}
		}
	}
	
	if (all_dead && g_transition_timer == 0) {
		g_stage++;
		g_transition_timer = 40;
	}
	
	if (g_transition_timer > 0) {
		g_transition_timer--;
		if (g_transition_timer == 0) spawn_enemies();
	}
	
	if (g_lives <= 0) {
		timer_remove_attr(AC_TASK_GAME_SHOOTER_ID, AC_GAME_UPDATE_TICK);
		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_GAME_OVER_NEXT);
	}
	
	task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_RENDER_SCREEN);
}
