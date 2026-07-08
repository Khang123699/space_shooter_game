#include "game_shooter.h"
#include "task_list.h"
#include "timer.h"
#include "buzzer.h"
#include <string.h>
#include <stdlib.h>
#include "app_dbg.h"

// Global game state variables
game_state_t g_game_state = GAME_STATE_MENU;
int16_t g_player_x = 60;
uint8_t g_player_blink = 0;
uint32_t g_score = 0;
int8_t g_lives = 3;
enemy_t g_enemies[MAX_ENEMIES];
bullet_t g_bullets[MAX_BULLETS];
explosion_t g_explosions[MAX_EXPLOSIONS];
uint8_t g_stage = 1;
int8_t g_transition_timer = 0;

// Internal function to spawn a grid of enemies
static void spawn_enemies() {
	int idx = 0;
	// Create a 3x6 grid of potential enemies
	for (int r = 0; r < 3; r++) {
		for (int c = 0; c < 6; c++) {
			// 70% chance to spawn an enemy at a grid position
			if (rand() % 100 > 30) { 
				g_enemies[idx].active = true;
				g_enemies[idx].x = 10 + c * 14;
				g_enemies[idx].y = r * 10 + 12;
			} else {
				g_enemies[idx].active = false;
			}
			idx++;
		}
	}
}

// Reset all game variables to start a new game
void game_logic_init() {
	g_player_x = 60;
	g_score = 0;
	g_lives = 3;
	g_stage = 1;
	g_transition_timer = 0;
	memset(g_bullets, 0, sizeof(g_bullets));
	memset(g_explosions, 0, sizeof(g_explosions));
	spawn_enemies();
}

// Move the player horizontally
void game_player_move(int8_t dir) {
	g_player_x += dir;
	if (g_player_x < 0) g_player_x = 0;
	if (g_player_x > 120) g_player_x = 120;
}

// Spawn a bullet from the player's position
void game_player_shoot() {
	for (int i = 0; i < MAX_BULLETS; i++) {
		// Find the first inactive bullet in the array
		if (!g_bullets[i].active) {
			g_bullets[i].active = true;
			g_bullets[i].x = g_player_x + 4;
			g_bullets[i].y = 52;
			break;
		}
	}
}

// Main game logic loop (50ms tick)
void game_logic_update() {
	// 1. Update bullets
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (g_bullets[i].active) {
			g_bullets[i].y -= 3;
			if (g_bullets[i].y < 0) g_bullets[i].active = false;
			
			// Check collision
			for (int e = 0; e < MAX_ENEMIES; e++) {
				if (g_enemies[e].active && g_bullets[i].x >= g_enemies[e].x && g_bullets[i].x <= g_enemies[e].x + 8 
					&& g_bullets[i].y >= g_enemies[e].y && g_bullets[i].y <= g_enemies[e].y + 8) {
					g_bullets[i].active = false;
					g_enemies[e].active = false;
					g_score += 10;
					BUZZER_PlaySound(BUZZER_SOUND_BANG);
					
					// Spawn explosion
					for (int ex = 0; ex < MAX_EXPLOSIONS; ex++) {
						if (!g_explosions[ex].active) {
							g_explosions[ex].x = g_enemies[e].x;
							g_explosions[ex].y = g_enemies[e].y;
							g_explosions[ex].timer = 5;
							g_explosions[ex].active = true;
							break;
						}
					}
				}
			}
		}
	}
	
	// 2. Update explosions
	for (int ex = 0; ex < MAX_EXPLOSIONS; ex++) {
		if (g_explosions[ex].active) {
			g_explosions[ex].timer--;
			if (g_explosions[ex].timer <= 0) g_explosions[ex].active = false;
		}
	}
	
	// 3. Next stage
	bool all_dead = true;
	for (int e = 0; e < MAX_ENEMIES; e++) {
		if (g_enemies[e].active) {
			all_dead = false;
			break;
		}
	}
	
	if (all_dead && g_transition_timer == 0) {
		g_stage++;
		g_transition_timer = 40; // 2s delay
	}
	
	if (g_transition_timer > 0) {
		g_transition_timer--;
		if (g_transition_timer == 0) spawn_enemies();
	}
	
	// 4. Game over
	if (g_lives <= 0) {
		timer_remove_attr(AC_TASK_GAME_SHOOTER_ID, AC_GAME_UPDATE_TICK);
		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_GAME_OVER_NEXT);
	}
	
	// Request render
	task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_RENDER_SCREEN);
}
