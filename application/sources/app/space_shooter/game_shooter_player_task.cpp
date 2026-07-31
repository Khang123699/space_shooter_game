#include "game_shooter_player_task.h"
#include "game_shooter_enemy_task.h"
#include "game_shooter_bullet_task.h"
#include "game_shooter_stage_task.h"
#include "game_shooter_render.h"
#include "task_list.h"
#include "timer.h"
#include "app.h"
#include "button.h"
#include "app_bsp.h"
#include "game_save.h"
#include "buzzer.h"
#include <stdlib.h>

int16_t g_player_x = 60;
uint8_t g_player_blink = 0;
uint16_t g_player_super_bullet_timer = 0;
uint16_t g_player_shield_timer = 0;
uint32_t g_score = 0;
uint8_t g_lives = 3;
uint16_t g_tick_count = 0;
uint8_t g_shoot_cooldown = 0;
bool g_is_moving_left = false;
bool g_is_moving_right = false;

// Move the player horizontally within screen boundaries
void game_player_move(int8_t dir) {
	g_player_x += dir;
	if (g_player_x < 0) g_player_x = 0;
	if (g_player_x > 120) g_player_x = 120;
}

// Shoot a player bullet if cooldown allows and slots are available
void game_player_shoot() {
	if (g_shoot_cooldown > 0) return;
	
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (!g_bullets[i].active) {
			g_bullets[i].active = true;
			g_bullets[i].is_enemy = false;
			g_bullets[i].x = g_player_x + 4;
			g_bullets[i].y = 52;
			g_bullets[i].vx = 0;
			if (g_player_super_bullet_timer > 0) {
				g_shoot_cooldown = 4; // Faster shooting
			} else {
				g_shoot_cooldown = 8; 
			}
			break;
		}
	}
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
void update_player_sliding_and_timers() {
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

void game_player_task(ak_msg_t* msg) {
	switch (msg->sig) {
		case AC_GAME_START_REQ:
			game_logic_init();
			timer_set(AC_TASK_GAME_PLAYER_ID, AC_GAME_UPDATE_TICK, 50, TIMER_PERIODIC);
			timer_set(AC_TASK_GAME_ENEMY_ID, AC_GAME_UPDATE_TICK, 50, TIMER_PERIODIC);
			timer_set(AC_TASK_GAME_BULLET_ID, AC_GAME_UPDATE_TICK, 50, TIMER_PERIODIC);
			timer_set(AC_TASK_GAME_STAGE_ID, AC_GAME_UPDATE_TICK, 50, TIMER_PERIODIC);
			break;
			
		case AC_GAME_BTN_MODE:
			game_player_shoot();
			break;
			
		case AC_GAME_BTN_UP:
			g_is_moving_left = true;
			break;
			
		case AC_GAME_BTN_UP_RELEASED:
			g_is_moving_left = false;
			break;
			
		case AC_GAME_BTN_DOWN:
			g_is_moving_right = true;
			break;
			
		case AC_GAME_BTN_DOWN_RELEASED:
			g_is_moving_right = false;
			break;
			
		case AC_GAME_UPDATE_TICK:
			update_player_sliding_and_timers();
			break;
	}
}
