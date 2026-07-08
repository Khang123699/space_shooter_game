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
