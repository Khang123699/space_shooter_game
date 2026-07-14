#include "game_shooter.h"
#include "task_list.h"
#include <string.h>

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
bool g_render_pending = false;
uint32_t g_score = 0;
uint8_t g_lives = 3;

uint8_t g_shoot_cooldown = 0;
uint16_t g_tick_count = 0;

enemy_t g_enemies[MAX_ENEMIES];
bullet_t g_bullets[MAX_BULLETS];
explosion_t g_explosions[MAX_EXPLOSIONS];
uint8_t g_stage = 1;
int8_t g_transition_timer = 0;

// Initialize or reset the game state variables for a new session
void game_logic_init() {
	g_player_x = 60;
	g_score = 0;
	g_lives = 3;
	g_stage = 1;
	g_transition_timer = 0;
	enemy_dir = 1;
	memset(g_bullets, 0, sizeof(g_bullets));
	memset(g_explosions, 0, sizeof(g_explosions));
	game_enemy_spawn();
}

// Main update loop for game logic
void game_logic_update() {
	if (g_player_blink > 0) g_player_blink--;
	
	// Update enemy blink timers
	for (int e = 0; e < MAX_ENEMIES; e++) {
		if (g_enemies[e].active && g_enemies[e].blink_timer > 0) {
			g_enemies[e].blink_timer--;
		}
	}
	
	if (g_shoot_cooldown > 0) g_shoot_cooldown--;
	g_tick_count++;

	// Delegate to specific modules
	game_physics_update();
	game_enemy_update();
	
	// Stage transition logic
	bool all_dead = true;
	for (int e = 0; e < MAX_ENEMIES; e++) {
		if (g_enemies[e].active) {
			all_dead = false;
			break;
		}
	}
	
	if (all_dead && g_transition_timer == 0) {
		g_stage++;
		g_transition_timer = 90;
	}
	
	if (g_transition_timer > 0) {
		g_transition_timer--;
		if (g_transition_timer == 0) game_enemy_spawn();
	}
	
	// Game Over check
	if (g_lives <= 0) {
		// Stop logic timer to pause the game world
		timer_remove_attr(AC_TASK_GAME_SHOOTER_ID, AC_GAME_UPDATE_TICK);
		// Signal UI task to transition to Game Over screen
		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_GAME_OVER_NEXT);
	}
	
	// Force a redraw of the UI frame with new positions if not already rendering
	if (!g_render_pending) {
		g_render_pending = true;
		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_RENDER_SCREEN);
	}
}
