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

#define ENEMY_TYPE_1       1
#define ENEMY_TYPE_2       2
#define ENEMY_TYPE_3       3
#define ENEMY_TYPE_BOSS    4
#define ENEMY_TYPE_SPREAD  5
#define ENEMY_TYPE_CARRIER 6

#define BOSS_STATE_NORMAL 0
#define BOSS_STATE_DASH_CHARGE 1
#define BOSS_STATE_DASH_DOWN 2
#define BOSS_STATE_DASH_UP 3
#define BOSS_STATE_SUMMON 4

typedef struct { int8_t x, y; int8_t hp; uint8_t type; bool active; int8_t blink_timer; uint8_t state; int16_t timer; } enemy_t;
typedef struct { int8_t x, y; uint8_t type; bool active; } powerup_t;

const enemy_t* game_get_enemies();
const powerup_t* game_get_powerups();

extern enemy_t g_enemies[MAX_ENEMIES];
extern powerup_t g_powerups[MAX_POWERUPS];
extern int8_t g_enemy_dir;
extern uint8_t g_enemy_move_ticks;

void game_enemy_kill(int e); // Exported for powerup nuke

extern void game_enemy_task(ak_msg_t* msg);

#endif // __GAME_SHOOTER_ENEMY_TASK_H__
