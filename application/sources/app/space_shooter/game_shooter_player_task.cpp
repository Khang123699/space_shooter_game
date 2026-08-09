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

static int16_t g_player_x = 60;
static uint8_t g_player_blink = 0;
static uint16_t g_player_super_bullet_timer = 0;
static uint16_t g_player_shield_timer = 0;
static uint32_t g_score = 0;
static uint8_t g_lives = 3;
static uint16_t g_tick_count = 0;
static uint8_t g_shoot_cooldown = 0;
static bool g_is_moving_left = false;
static bool g_is_moving_right = false;

int16_t game_get_player_x() { return g_player_x; }
uint8_t game_get_player_blink() { return g_player_blink; }
uint16_t game_get_player_shield_timer() { return g_player_shield_timer; }
uint16_t game_get_player_super_bullet_timer() { return g_player_super_bullet_timer; }
uint32_t game_get_score() { return g_score; }
uint8_t game_get_lives() { return g_lives; }
uint16_t game_get_tick_count() { return g_tick_count; }

// Move the player horizontally within screen boundaries
static void game_player_move(int8_t dir) {
	g_player_x += dir;
	if (g_player_x < 0) g_player_x = 0;
	if (g_player_x > 120) g_player_x = 120;
}

// Shoot a player bullet if cooldown allows and slots are available
static void game_player_shoot() {
	if (g_shoot_cooldown > 0) return;
	
    game_bullet_spawn_msg_t spawn_msg;
    spawn_msg.x = g_player_x + 4;
    spawn_msg.y = 52;
    spawn_msg.is_enemy = false;
    spawn_msg.vx = 0;
    task_post(AC_TASK_GAME_BULLET_ID, AC_GAME_SPAWN_BULLET, (uint8_t*)&spawn_msg, sizeof(spawn_msg));
    
	if (g_player_super_bullet_timer > 0) {
		g_shoot_cooldown = 4; // Faster shooting
	} else {
		g_shoot_cooldown = 8; 
	}
}

// Handle player being hit by bullet or enemy body
static void game_player_hit() {
	if (g_player_shield_timer > 0) {
		g_player_shield_timer = 0;
		g_player_super_bullet_timer = 0;
		g_player_blink = 34;
	} else {
		g_lives--;
		g_player_blink = 34;
	}
}

// Initialize game session variables and notify other tasks to reset
static void game_logic_init() {
	// Reset Player Task's own data
	g_player_x = 60;
	g_player_blink = 0;
	g_player_super_bullet_timer = 0;
	g_player_shield_timer = 0;
	g_score = 0;
	g_lives = 3;
	g_tick_count = 0;
	g_shoot_cooldown = 0;
	g_is_moving_left = false;
	g_is_moving_right = false;
	
	game_background_init();
	
	// Send AC_GAME_START_REQ to other tasks so they reset their own data
	task_post_pure_msg(AC_TASK_GAME_ENEMY_ID, AC_GAME_START_REQ);
	task_post_pure_msg(AC_TASK_GAME_BULLET_ID, AC_GAME_START_REQ);
	task_post_pure_msg(AC_TASK_GAME_STAGE_ID, AC_GAME_START_REQ);
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
	
	// Enemy blink timers are now managed in game_shooter_enemy_task.cpp
	
	if (g_shoot_cooldown > 0) g_shoot_cooldown--;
	g_tick_count++;
}

void game_player_task(ak_msg_t* msg) {
	switch (msg->sig) {
		case AC_GAME_START_REQ:
			game_logic_init();
			timer_set(AC_TASK_GAME_PLAYER_ID, AC_GAME_UPDATE_TICK, 50, TIMER_PERIODIC);
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
			task_post_pure_msg(AC_TASK_GAME_ENEMY_ID, AC_GAME_UPDATE_TICK);
			task_post_pure_msg(AC_TASK_GAME_BULLET_ID, AC_GAME_UPDATE_TICK);
			task_post_pure_msg(AC_TASK_GAME_STAGE_ID, AC_GAME_UPDATE_TICK);
			break;
			
		case AC_GAME_PLAYER_HIT:
			game_player_hit();
			break;
			
		case AC_GAME_SCORE_UPDATE: {
			game_score_update_msg_t* score_msg = (game_score_update_msg_t*)get_data_common_msg(msg);
			g_score += score_msg->additional_score;
			break;
		}
		
		case AC_GAME_POWERUP_PICKUP: {
			game_powerup_msg_t* pmsg = (game_powerup_msg_t*)get_data_common_msg(msg);
			if (pmsg->type == 1) {
				g_player_super_bullet_timer = 200;
			} else if (pmsg->type == 2) {
				g_player_shield_timer = 200;
			}
			break;
		}
	}
}
