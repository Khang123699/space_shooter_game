#include "game_save.h"
#include "flash.h"
#include "app_flash.h"

game_save_data_t g_game_data;

void init_default_save_data() {
	g_game_data.magic = GAME_SAVE_MAGIC;
	g_game_data.top_score[0] = 0;
	g_game_data.top_score[1] = 0;
	g_game_data.top_score[2] = 0;
	g_game_data.sound_en = 1;
	g_game_data.difficulty = 1; 
}

void game_save_data() {
	flash_erase_sector(APP_FLASH_GAME_DATA_SECTOR);
	flash_write(APP_FLASH_GAME_DATA_SECTOR, (uint8_t*)&g_game_data, sizeof(game_save_data_t));
}

void game_load_data() {
	flash_read(APP_FLASH_GAME_DATA_SECTOR, (uint8_t*)&g_game_data, sizeof(game_save_data_t));
	if (g_game_data.magic != GAME_SAVE_MAGIC) {
		init_default_save_data();
		game_save_data();
	}
}

uint8_t game_update_high_score(uint32_t final_score) {
	int index = -1;
	for (int i = 0; i < 3; i++) {
		if (final_score > g_game_data.top_score[i]) {
			index = i;
			break;
		}
	}
	if (index != -1) {
		for (int i = 2; i > index; i--) {
			g_game_data.top_score[i] = g_game_data.top_score[i - 1];
		}
		g_game_data.top_score[index] = final_score;
		game_save_data();
		return index + 1;
	}
	return 0;
}
