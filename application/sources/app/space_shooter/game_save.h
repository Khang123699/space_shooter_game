#ifndef __GAME_SAVE_H__
#define __GAME_SAVE_H__

#include <stdint.h>
#include <stdbool.h>

#define GAME_SAVE_MAGIC 0x53544143

typedef struct {
	uint32_t magic;
	uint32_t top_score[3];
	uint8_t sound_en; // (0: off, 1: on)
	uint8_t difficulty; // (0,1,2)
} game_save_data_t;

extern game_save_data_t g_game_data;

extern void init_default_save_data();
extern void game_save_data();
extern void game_load_data();
extern uint8_t game_update_high_score(uint32_t final_score);

#endif // __GAME_SAVE_H__
