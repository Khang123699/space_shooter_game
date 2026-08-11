#ifndef __GAME_SHOOTER_PLAYER_TASK_H__
#define __GAME_SHOOTER_PLAYER_TASK_H__

#include <stdint.h>
#include <stdbool.h>
#include "ak.h"

typedef enum {
    PLAYER_STATE_DEAD = 0,
    PLAYER_STATE_ALIVE,
    PLAYER_STATE_INVULNERABLE
} player_state_e;

typedef struct {
    int16_t x;
    uint8_t blink_timer;
    uint16_t shield_timer;
    uint16_t super_bullet_timer;
    uint8_t lives;
    uint32_t score;
    player_state_e state;
} player_t;

int16_t game_get_player_x();
uint8_t game_get_player_blink();
uint16_t game_get_player_shield_timer();
uint16_t game_get_player_super_bullet_timer();
uint32_t game_get_score();
uint8_t game_get_lives();
uint16_t game_get_tick_count();

extern void game_player_task(ak_msg_t* msg);

#endif // __GAME_SHOOTER_PLAYER_TASK_H__
