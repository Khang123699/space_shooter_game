#ifndef __GAME_SHOOTER_PLAYER_TASK_H__
#define __GAME_SHOOTER_PLAYER_TASK_H__

#include <stdint.h>
#include <stdbool.h>
#include "ak.h"

extern int16_t g_player_x;
extern uint8_t g_player_blink;
extern uint16_t g_player_super_bullet_timer;
extern uint16_t g_player_shield_timer;
extern uint32_t g_score;
extern uint8_t g_lives;
extern uint16_t g_tick_count;
extern uint8_t g_shoot_cooldown;
extern bool g_is_moving_left;
extern bool g_is_moving_right;

extern void game_logic_init();
extern void game_player_move(int8_t dir);
extern void game_player_shoot();
extern void update_player_sliding_and_timers();
extern void game_player_hit();

extern void game_player_task(ak_msg_t* msg);

#endif // __GAME_SHOOTER_PLAYER_TASK_H__
