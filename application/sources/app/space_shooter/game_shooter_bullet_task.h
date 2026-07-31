#ifndef __GAME_SHOOTER_BULLET_TASK_H__
#define __GAME_SHOOTER_BULLET_TASK_H__

#include <stdint.h>
#include <stdbool.h>
#include "ak.h"

#define MAX_BULLETS 20
#define MAX_EXPLOSIONS 5

typedef struct {
	int16_t x;
	int16_t y;
	int8_t vx;
	bool active;
	bool is_enemy;
} bullet_t;

typedef struct { int8_t x, y; int8_t timer; bool active; } explosion_t;

extern bullet_t g_bullets[];
extern explosion_t g_explosions[];

extern void game_bullet_task(ak_msg_t* msg);
extern void game_physics_update();
extern void game_bullets_update();
extern bool game_check_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
extern void game_spawn_explosion(int x, int y);
extern void game_enemy_kill(int e);

#endif // __GAME_SHOOTER_BULLET_TASK_H__
