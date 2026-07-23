#ifndef __GAME_SAVE_H__
#define __GAME_SAVE_H__

#include <stdint.h>
#include <stdbool.h>

#define GAME_SAVE_MAGIC          ((uint32_t)0x53504143) 

#define APP_EEPROM_SCORE_ADDR    (0x0010)
#define APP_EEPROM_SETTING_ADDR  (0x0100)

typedef struct {
	uint8_t sound_en;   // (0: off, 1: on)
	uint8_t difficulty; // (0: EASY, 1: MED, 2: HARD)
} game_setting_t;

typedef struct {
	uint32_t top_score[3]; // 1st, 2nd, 3rd high scores
} game_score_t;

typedef struct {
	uint32_t magic_number;
	game_setting_t data;
	uint8_t check_sum;
} game_setting_eeprom_t;

typedef struct {
	uint32_t magic_number;
	game_score_t data;
	uint8_t check_sum;
} game_score_eeprom_t;

extern game_setting_t g_game_setting;
extern game_score_t g_game_score;

extern void init_default_save_data();
extern void reset_high_score_data();
extern void game_save_setting();
extern void game_save_score();
extern void game_save_data();
extern void game_load_data();
extern uint8_t game_update_high_score(uint32_t final_score);

#endif // __GAME_SAVE_H__
