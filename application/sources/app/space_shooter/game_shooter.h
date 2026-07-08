#ifndef __GAME_SHOOTER_H__
#define __GAME_SHOOTER_H__

#include "app.h"

#define MAX_ENEMIES 35
#define MAX_BULLETS 20
#define MAX_EXPLOSIONS 5

typedef struct { int8_t x, y; bool active, is_enemy; } bullet_t;
typedef struct { int8_t x, y; int8_t hp; uint8_t type; bool active; } enemy_t;
typedef struct { int8_t x, y; int8_t timer; bool active; } explosion_t;

// Define states for the game menu and play flow
typedef enum {
	GAME_STATE_MENU,
	GAME_STATE_PLAYING,
	GAME_STATE_GAMEOVER
} game_state_t;

// Exported global variables for UI and Logic
extern game_state_t g_game_state;
extern int16_t g_player_x;
extern uint8_t g_player_blink;
extern uint32_t g_score;
extern int8_t g_lives;
extern enemy_t g_enemies[];
extern bullet_t g_bullets[];
extern explosion_t g_explosions[];
extern uint8_t g_stage;
extern int8_t g_transition_timer;

// Logic Interfaces
extern void game_logic_init();
extern void game_player_move(int8_t dir);
extern void game_player_shoot();
extern void game_logic_update();

// UI Interfaces
extern void view_scr_idle();

#endif