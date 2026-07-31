#ifndef __GAME_SHOOTER_ENEMY_TASK_H__
#define __GAME_SHOOTER_ENEMY_TASK_H__

#include <stdint.h>
#include <stdbool.h>
#include "ak.h"

#define MAX_ENEMIES 35
#define MAX_POWERUPS 3

#define POWERUP_TYPE_SUPER_BULLET 1
#define POWERUP_TYPE_SHIELD    2
#define POWERUP_TYPE_NUKE      3

#define BOSS_STATE_NORMAL 0
#define BOSS_STATE_DASH_CHARGE 1
#define BOSS_STATE_DASH_DOWN 2
#define BOSS_STATE_DASH_UP 3
#define BOSS_STATE_SUMMON 4

typedef struct { int8_t x, y; int8_t hp; uint8_t type; bool active; int8_t blink_timer; uint8_t state; int16_t timer; } enemy_t;
typedef struct { int8_t x, y; uint8_t type; bool active; } powerup_t;

extern enemy_t g_enemies[];
extern powerup_t g_powerups[];
extern int8_t enemy_dir;
extern uint8_t enemy_move_ticks;

extern void game_enemy_task(ak_msg_t* msg);
extern void game_enemy_spawn();
extern void game_enemy_update();
extern void game_boss_spawn();
extern void game_boss_update_state(int e, bool do_move, int move_threshold, int boss_max_hp, bool& hit_edge);
extern void game_boss_shoot(int e, int ew, int boss_max_hp);
extern void game_carrier_update(int e);
extern void game_powerups_update();

#endif // __GAME_SHOOTER_ENEMY_TASK_H__
