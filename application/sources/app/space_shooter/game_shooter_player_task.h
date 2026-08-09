#ifndef __GAME_SHOOTER_PLAYER_TASK_H__
#define __GAME_SHOOTER_PLAYER_TASK_H__

#include <stdint.h>
#include <stdbool.h>
#include "ak.h"

int16_t game_get_player_x();
uint8_t game_get_player_blink();
uint16_t game_get_player_shield_timer();
uint16_t game_get_player_super_bullet_timer();
uint32_t game_get_score();
uint8_t game_get_lives();
uint16_t game_get_tick_count();

extern void game_player_task(ak_msg_t* msg);

#endif // __GAME_SHOOTER_PLAYER_TASK_H__
