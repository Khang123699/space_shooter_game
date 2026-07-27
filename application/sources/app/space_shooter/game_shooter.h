#ifndef __GAME_SHOOTER_H__
#define __GAME_SHOOTER_H__

#include "app.h"
#include "game_save.h"

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

// Exported global variables for Logic and UI Screens
extern uint8_t g_new_high_score_rank;
extern bool g_render_pending;

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
extern uint8_t enemy_move_ticks;
extern bool g_is_moving_left;
extern bool g_is_moving_right;

// Core Logic Interfaces
extern void game_logic_init();
extern void game_player_move(int8_t dir);
extern void game_player_shoot();
extern void game_logic_update();
extern void game_shooter_update_stars();

// Background Parallax Module
extern void game_background_init();
extern void game_background_update();

// Stage & Progression Module
extern void game_powerups_update();
extern void game_stage_update();
extern void game_check_game_over();

// Enemy & AI Modules
extern void game_enemy_spawn();
extern void game_enemy_update();
extern void game_boss_spawn();
extern void game_boss_update_state(int e, bool do_move, int move_threshold, int boss_max_hp, bool& hit_edge);
extern void game_boss_shoot(int e, int ew, int boss_max_hp);
extern void game_carrier_update(int e);

// Bullets & Physics Modules
extern void game_bullets_update();
extern void game_physics_update();
extern bool game_check_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
extern void game_spawn_explosion(int x, int y);
extern void game_drop_powerup(int x, int y);
extern void game_enemy_kill(int e);
extern void game_player_hit();

#endif