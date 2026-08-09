#ifndef __GAME_SHOOTER_STAGE_TASK_H__
#define __GAME_SHOOTER_STAGE_TASK_H__

#include <stdint.h>
#include <stdbool.h>
#include "ak.h"

uint8_t game_get_stage();
int8_t game_get_transition_timer();
uint8_t game_get_new_high_score_rank();
void game_set_new_high_score_rank(uint8_t rank);

extern void game_stage_task(ak_msg_t* msg);

#endif // __GAME_SHOOTER_STAGE_TASK_H__
