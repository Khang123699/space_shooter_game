#include "game_save.h"
#include "eeprom.h"

game_setting_t g_game_setting;
game_score_t g_game_score;

static uint8_t game_eeprom_checksum(uint8_t* data, uint32_t size) {
	uint8_t check_sum = 0;
	for (uint32_t i = 0; i < size; i++) {
		check_sum += data[i];
	}
	return check_sum;
}

// Initialize default settings and clear high scores
void init_default_save_data() {
	g_game_setting.sound_en = 1;
	g_game_setting.difficulty = 1;
	
	g_game_score.top_score[0] = 0;
	g_game_score.top_score[1] = 0;
	g_game_score.top_score[2] = 0;
}

// Reset all high score data to 0 and save to EEPROM
void reset_high_score_data() {
	g_game_score.top_score[0] = 0;
	g_game_score.top_score[1] = 0;
	g_game_score.top_score[2] = 0;
	game_save_score();
}

// Save game configuration settings to EEPROM
void game_save_setting() {
	game_setting_eeprom_t setting_pkt;
	setting_pkt.magic_number = GAME_SAVE_MAGIC;
	setting_pkt.data = g_game_setting;
	setting_pkt.check_sum = game_eeprom_checksum((uint8_t*)&setting_pkt, sizeof(uint32_t) + sizeof(game_setting_t));
	
	eeprom_write(APP_EEPROM_SETTING_ADDR, (uint8_t*)&setting_pkt, sizeof(game_setting_eeprom_t));
}

// Save high scores to EEPROM
void game_save_score() {
	game_score_eeprom_t score_pkt;
	score_pkt.magic_number = GAME_SAVE_MAGIC;
	score_pkt.data = g_game_score;
	score_pkt.check_sum = game_eeprom_checksum((uint8_t*)&score_pkt, sizeof(uint32_t) + sizeof(game_score_t));
	
	eeprom_write(APP_EEPROM_SCORE_ADDR, (uint8_t*)&score_pkt, sizeof(game_score_eeprom_t));
}

// Save both settings and score data
void game_save_data() {
	game_save_setting();
	game_save_score();
}

// Load game settings and high scores from EEPROM with Magic Number & Checksum verification
void game_load_data() {
	// 1. Load Settings
	game_setting_eeprom_t setting_pkt;
	eeprom_read(APP_EEPROM_SETTING_ADDR, (uint8_t*)&setting_pkt, sizeof(game_setting_eeprom_t));
	uint8_t setting_expected_checksum = game_eeprom_checksum((uint8_t*)&setting_pkt, sizeof(uint32_t) + sizeof(game_setting_t));
	
	if (setting_pkt.magic_number != GAME_SAVE_MAGIC || setting_pkt.check_sum != setting_expected_checksum) {
		g_game_setting.sound_en = 1;
		g_game_setting.difficulty = 1;
		game_save_setting();
	} else {
		g_game_setting = setting_pkt.data;
	}

	// 2. Load Scores
	game_score_eeprom_t score_pkt;
	eeprom_read(APP_EEPROM_SCORE_ADDR, (uint8_t*)&score_pkt, sizeof(game_score_eeprom_t));
	uint8_t score_expected_checksum = game_eeprom_checksum((uint8_t*)&score_pkt, sizeof(uint32_t) + sizeof(game_score_t));
	
	if (score_pkt.magic_number != GAME_SAVE_MAGIC || score_pkt.check_sum != score_expected_checksum) {
		g_game_score.top_score[0] = 0;
		g_game_score.top_score[1] = 0;
		g_game_score.top_score[2] = 0;
		game_save_score();
	} else {
		g_game_score = score_pkt.data;
	}
}

// Update high score list with final_score, returns rank (1-3) or 0 if not ranked
uint8_t game_update_high_score(uint32_t final_score) {
	int index = -1;
	for (int i = 0; i < 3; i++) {
		if (final_score > g_game_score.top_score[i]) {
			index = i;
			break;
		}
	}
	if (index != -1) {
		for (int i = 2; i > index; i--) {
			g_game_score.top_score[i] = g_game_score.top_score[i - 1];
		}
		g_game_score.top_score[index] = final_score;
		game_save_score();
		return index + 1;
	}
	return 0;
}
