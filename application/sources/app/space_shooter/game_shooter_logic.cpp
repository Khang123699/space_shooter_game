#include "game_shooter.h"
#include "timer.h"
#include "task_list.h"
#include "button.h"
#include "app_bsp.h"
#include <stdlib.h>

uint8_t g_new_high_score_rank = 0;
bool g_render_pending = false;

int16_t g_player_x = 60;
uint8_t g_player_blink = 0;
uint16_t g_player_super_bullet_timer = 0;
uint16_t g_player_shield_timer = 0;
uint32_t g_score = 0;
uint8_t g_lives = 3;
enemy_t g_enemies[MAX_ENEMIES];
bullet_t g_bullets[MAX_BULLETS];
explosion_t g_explosions[MAX_EXPLOSIONS];
powerup_t g_powerups[MAX_POWERUPS];
star_t g_stars[MAX_STARS];
uint8_t g_stage = 1;
int8_t g_transition_timer = 0;
uint16_t g_tick_count = 0;
uint8_t g_shoot_cooldown = 0;
bool g_is_moving_left = false;
bool g_is_moving_right = false;

// Initialize game session variables and clear entities
void game_logic_init() {
	g_player_x = 60;
	g_player_blink = 0;
	g_player_super_bullet_timer = 0;
	g_player_shield_timer = 0;
	g_score = 0;
	g_lives = 3;
	g_stage = 1;
	g_transition_timer = 0;
	g_tick_count = 0;
	g_shoot_cooldown = 0;
	g_new_high_score_rank = 0;
	g_render_pending = false;
	g_is_moving_left = false;
	g_is_moving_right = false;
	enemy_dir = 1;
	
	for (int i = 0; i < MAX_ENEMIES; i++) g_enemies[i].active = false;
	for (int i = 0; i < MAX_BULLETS; i++) g_bullets[i].active = false;
	for (int i = 0; i < MAX_EXPLOSIONS; i++) g_explosions[i].active = false;
	for (int i = 0; i < MAX_POWERUPS; i++) g_powerups[i].active = false;
	
	game_background_init();
	game_enemy_spawn();
}

// Update player smooth sliding and cooldown timers
static void update_player_sliding_and_timers() {
	if (g_player_blink > 0) g_player_blink--;
	if (g_player_super_bullet_timer > 0) g_player_super_bullet_timer--;
	if (g_player_shield_timer > 0) g_player_shield_timer--;
	
	// Smooth sliding movement check
	if (g_is_moving_left) {
		if (btn_up.read() == BUTTON_HW_STATE_RELEASED) {
			g_is_moving_left = false;
		} else {
			game_player_move(2); // Left
		}
	}
	if (g_is_moving_right) {
		if (btn_down.read() == BUTTON_HW_STATE_RELEASED) {
			g_is_moving_right = false;
		} else {
			game_player_move(-2); // Right
		}
	}
	
	// Update enemy blink timers
	for (int e = 0; e < MAX_ENEMIES; e++) {
		if (g_enemies[e].active && g_enemies[e].blink_timer > 0) {
			g_enemies[e].blink_timer--;
		}
	}
	
	if (g_shoot_cooldown > 0) g_shoot_cooldown--;
	g_tick_count++;
}

// Main game logic loop coordinator
void game_logic_update() {
	update_player_sliding_and_timers();
	
	game_physics_update();
	game_enemy_update();
	game_powerups_update();
	
	game_stage_update();
	game_check_game_over();
	
	// Force a redraw of the UI frame with new positions if not already rendering
	if (!g_render_pending) {
		g_render_pending = true;
		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_RENDER_SCREEN);
	}
}
