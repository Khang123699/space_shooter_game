#ifndef __GAME_SHOOTER_RENDER_H__
#define __GAME_SHOOTER_RENDER_H__

#include <stdint.h>
#include <stdbool.h>
#include "ak.h"

#define MAX_STARS 20

typedef struct {
	int8_t x;
	int8_t y;
	int8_t speed;
} star_t;

extern const star_t* game_get_stars();
extern void game_shooter_request_render();

void game_render_handle(ak_msg_t* msg);

#endif // __GAME_SHOOTER_RENDER_H__
