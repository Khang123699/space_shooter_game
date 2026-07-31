#ifndef __GAME_SHOOTER_STAGE_TASK_H__
#define __GAME_SHOOTER_STAGE_TASK_H__

#include <stdint.h>
#include <stdbool.h>
#include "ak.h"

extern uint8_t g_stage;
extern int8_t g_transition_timer;
extern uint8_t g_new_high_score_rank;

extern void game_stage_task(ak_msg_t* msg);
extern void game_stage_update();
extern void game_check_game_over();

#endif // __GAME_SHOOTER_STAGE_TASK_H__
