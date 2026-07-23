#ifndef __GAME_SHOOTER_H__
#define __GAME_SHOOTER_H__

#include "app.h"

#define MAX_ENEMIES 35
#define MAX_BULLETS 20
#define MAX_EXPLOSIONS 5
#define MAX_POWERUPS 3
#define MAX_STARS 20

#define POWERUP_TYPE_SUPER_BULLET 1
#define POWERUP_TYPE_SHIELD    2
#define POWERUP_TYPE_NUKE      3

typedef struct {
	int16_t x;
	int16_t y;
	int8_t vx;
	bool active;
	bool is_enemy;
} bullet_t;
#define BOSS_STATE_NORMAL 0
#define BOSS_STATE_DASH_CHARGE 1
#define BOSS_STATE_DASH_DOWN 2
#define BOSS_STATE_DASH_UP 3
#define BOSS_STATE_SUMMON 4

typedef struct { int8_t x, y; int8_t hp; uint8_t type; bool active; int8_t blink_timer; uint8_t state; int16_t timer; } enemy_t;
typedef struct { int8_t x, y; int8_t timer; bool active; } explosion_t;
typedef struct { int8_t x, y; uint8_t type; bool active; } powerup_t;

typedef struct {
	int8_t x;
	int8_t y;
	int8_t speed;
} star_t;

// Define states for the game menu and play flow
typedef enum {
	GAME_STATE_TITLE,
	GAME_STATE_MENU,
	GAME_STATE_SETTING,
	GAME_STATE_HIGH_SCORE,
	GAME_STATE_PLAYING,
	GAME_STATE_GAMEOVER,
	GAME_STATE_SHOW_SCORE
} game_state_t;

// Exported global variables for UI and Logic
extern game_state_t g_game_state;
extern uint8_t g_menu_selected;
extern uint8_t g_setting_selected;
extern uint8_t g_score_selected;
extern uint8_t g_show_score_selected;
extern const char* g_encouragement_text;
extern uint8_t g_new_high_score_rank;
extern bool g_render_pending;
extern uint8_t g_gameover_anim_frame;

extern int16_t g_player_x;
extern uint8_t g_player_blink;
extern uint16_t g_player_super_bullet_timer;
extern uint16_t g_player_shield_timer;
extern uint32_t g_score;
extern uint8_t g_lives;
extern enemy_t g_enemies[];
extern bullet_t g_bullets[];
extern explosion_t g_explosions[];
extern powerup_t g_powerups[];
extern star_t g_stars[MAX_STARS];
extern uint8_t g_stage;
extern int8_t g_transition_timer;
extern uint16_t g_tick_count;
extern uint8_t g_shoot_cooldown;
extern int8_t enemy_dir;
extern bool g_is_moving_left;
extern bool g_is_moving_right;

// Logic Interfaces
extern void game_logic_init();
extern void game_player_move(int8_t dir);
extern void game_player_shoot();
extern void game_logic_update();

extern void game_enemy_spawn();
extern void game_enemy_update();
extern void game_physics_update();

// UI Interfaces
extern void view_scr_game_ui();

#endif