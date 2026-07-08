#include "scr_game_ui.h"
#include "game_shooter.h"
#include "game_save.h"
#include "view_render.h"
#include "buzzer.h"
#include <stdio.h>
#include <string.h>

// Define 8x8 bitmaps for menu icons and entities
static const uint8_t icon_player[8]   = { 0x18, 0x18, 0x3c, 0x3c, 0x5a, 0x99, 0xff, 0xff };
static const uint8_t icon_enemy1[8]   = { 0x42, 0x24, 0x3c, 0x5a, 0xff, 0xa5, 0x24, 0x42 };
static const uint8_t bmp_explosion[8] = { 0x42, 0x24, 0x18, 0x99, 0x99, 0x18, 0x24, 0x42 };

static const uint8_t icon_play[8] = { 0x10, 0x18, 0x1c, 0x1e, 0x1e, 0x1c, 0x18, 0x10 };
static const uint8_t icon_setting[8] = { 0x24, 0x7e, 0xe7, 0xdb, 0xdb, 0xe7, 0x7e, 0x24 };
static const uint8_t icon_trophy[8] = { 0xff, 0x7e, 0x7e, 0x3c, 0x18, 0x18, 0x3c, 0x7e };
static const uint8_t icon_menu[8] = { 0x00, 0x7e, 0x00, 0x7e, 0x00, 0x7e, 0x00, 0x00 };
static const uint8_t icon_heart[8] = { 0x00, 0x66, 0xff, 0xff, 0x7e, 0x3c, 0x18, 0x00 };

static const uint8_t bmp_boss[32] = {
	0x03, 0xc0, 0x0f, 0xf0, 0x1e, 0x78, 0x39, 0x9c, 0x71, 0x8e, 0x71, 0x8e, 0x71, 0x8e, 0xff, 0xff,
	0xff, 0xff, 0x71, 0x8e, 0x71, 0x8e, 0x39, 0x9c, 0x1c, 0x38, 0x0c, 0x30, 0x04, 0x20, 0x00, 0x00
};

void view_scr_game_ui();

view_dynamic_t dyn_view_idle = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_game_ui
};

view_screen_t scr_game_ui = {
	&dyn_view_idle,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0,
};

static void game_shooter_menu_display() {
	view_render.drawRect(0, 0, 128, 64, WHITE);
	view_render.setTextSize(1);
	view_render.setCursor(26, 6);
	view_render.print("SHOOTER GAME");
	view_render.drawLine(10, 16, 117, 16, WHITE);
	
	view_render.drawBitmap(18, 24, icon_play, 8, 8, WHITE);
	view_render.setCursor(32, 24);
	view_render.print("Play");
	
	view_render.drawBitmap(18, 36, icon_setting, 8, 8, WHITE);
	view_render.setCursor(32, 36);
	view_render.print("Setting");
	
	view_render.drawBitmap(18, 48, icon_trophy, 8, 8, WHITE);
	view_render.setCursor(32, 48);
	view_render.print("High score");
	
	view_render.setCursor(6, 24 + g_menu_selected * 12);
	view_render.print(">");
}

static void game_shooter_setting_display() {
	view_render.drawRect(0, 0, 128, 64, WHITE);
	view_render.setTextSize(1);
	view_render.setCursor(40, 6);
	view_render.print("SETTINGS");
	view_render.drawLine(10, 16, 117, 16, WHITE);
	
	view_render.setCursor(26, 22);
	view_render.print("Sound:");
	view_render.setCursor(80, 22);
	view_render.print(g_game_data.sound_en ? "ON" : "OFF");
	
	view_render.setCursor(26, 34);
	view_render.print("Diff:");
	view_render.setCursor(80, 34);
	if (g_game_data.difficulty == 0) view_render.print("EASY");
	else if (g_game_data.difficulty == 1) view_render.print("MED");
	else view_render.print("HARD");
	
	view_render.setCursor(26, 48);
	view_render.print("Back");
	
	view_render.setCursor(14, 22 + g_setting_selected * 13);
	view_render.print(">");
}

static void game_shooter_highscore_display() {
	view_render.drawRect(0, 0, 128, 64, WHITE);
	view_render.setTextSize(1);
	view_render.setCursor(32, 6);
	view_render.print("HIGH SCORES");
	view_render.drawLine(10, 16, 117, 16, WHITE);
	
	for (int i = 0; i < 3; i++) {
		view_render.setCursor(26, 20 + i * 10);
		if (i == 0) view_render.print("1st: ");
		else if (i == 1) view_render.print("2nd: ");
		else view_render.print("3rd: ");
		
		char temp_str[12];
		sprintf(temp_str, "%u", (unsigned int)g_game_data.top_score[i]);
		view_render.print(temp_str);
	}
	
	view_render.setCursor(24, 52);
	view_render.print("Reset");
	view_render.setCursor(84, 52);
	view_render.print("Back");
	
	if (g_score_selected == 0) view_render.setCursor(14, 52);
	else view_render.setCursor(74, 52);
	
	view_render.print(">");
}

static void game_shooter_playing_display() {
	if (g_player_blink % 2 == 0) {
		view_render.drawBitmap(g_player_x, 54, icon_player, 8, 8, WHITE);
	}
	
	for (int e = 0; e < MAX_ENEMIES; e++) {
		if (g_enemies[e].active) {
			if (g_enemies[e].type == 4) {
				view_render.drawBitmap(g_enemies[e].x, g_enemies[e].y, bmp_boss, 16, 16, WHITE);
				// HP Bar for Boss
				int max_hp = 20 + (g_stage / 5) * 10;
				int hp_width = (g_enemies[e].hp * 16) / max_hp;
				if (hp_width > 16) hp_width = 16;
				if (hp_width < 0) hp_width = 0;
				view_render.fillRect(g_enemies[e].x, g_enemies[e].y - 3, 16, 2, BLACK);
				view_render.fillRect(g_enemies[e].x, g_enemies[e].y - 3, hp_width, 2, WHITE);
			} else {
				view_render.drawBitmap(g_enemies[e].x, g_enemies[e].y, icon_enemy1, 8, 8, WHITE);
			}
		}
	}
	
	for (int ex = 0; ex < MAX_EXPLOSIONS; ex++) {
		if (g_explosions[ex].active) {
			view_render.drawBitmap(g_explosions[ex].x, g_explosions[ex].y, bmp_explosion, 8, 8, WHITE);
		}
	}
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (g_bullets[i].active) {
			if (g_bullets[i].is_enemy) view_render.fillRect(g_bullets[i].x, g_bullets[i].y, 2, 4, WHITE);
			else view_render.drawLine(g_bullets[i].x, g_bullets[i].y, g_bullets[i].x, g_bullets[i].y + 3, WHITE);
		}
	}
	
	// Draw Top UI Bar
	view_render.fillRect(0, 0, 128, 11, BLACK);
	view_render.drawLine(0, 10, 127, 10, WHITE);
	view_render.setTextSize(1);
	
	view_render.setCursor(2, 2);
	view_render.print("S:");
	char temp_sc[12];
	sprintf(temp_sc, "%u", (unsigned int)g_score);
	view_render.print(temp_sc);
	
	view_render.setCursor(50, 2);
	view_render.print("L:");
	char temp_lv[12];
	sprintf(temp_lv, "%u", (unsigned int)g_stage);
	view_render.print(temp_lv);
	
	for (int i = 0; i < g_lives; i++) {
		view_render.drawBitmap(128 - 10 - i * 10, 1, icon_heart, 8, 8, WHITE);
	}
	
	if (g_transition_timer > 0) {
		view_render.setCursor(40, 28);
		view_render.print("STAGE ");
		view_render.print(g_stage);
	}
}

static void game_shooter_gameover_display() {
	view_render.setTextSize(2);
	view_render.setCursor(10, 16);
	view_render.print("GAME OVER");
	view_render.setTextSize(1);
	view_render.setCursor(34, 40);
	view_render.print(g_encouragement_text);
}

static void game_shooter_score_display() {
	view_render.setTextSize(1);
	view_render.setCursor(31, 14);
	view_render.print("YOUR SCORE:");
	view_render.setTextSize(2);
	char temp_final[12];
	sprintf(temp_final, "%u", (unsigned int)g_score);
	int16_t w = strlen(temp_final) * 12;
	view_render.setCursor((128 - w) / 2, 28);
	view_render.print(temp_final);
	
	view_render.drawBitmap(30, 48, icon_play, 8, 8, WHITE);
	view_render.drawBitmap(60, 48, icon_trophy, 8, 8, WHITE);
	view_render.drawBitmap(90, 48, icon_menu, 8, 8, WHITE);
	
	uint8_t selected_x = 30 + (g_show_score_selected * 30);
	view_render.fillRect(selected_x, 58, 8, 2, WHITE);
}

void view_scr_game_ui() {
	view_render.clear();
	view_render.setTextColor(WHITE);
	switch (g_game_state) {
		case GAME_STATE_MENU: game_shooter_menu_display(); break;
		case GAME_STATE_SETTING: game_shooter_setting_display(); break;
		case GAME_STATE_HIGH_SCORE: game_shooter_highscore_display(); break;
		case GAME_STATE_PLAYING: game_shooter_playing_display(); break;
		case GAME_STATE_GAMEOVER: game_shooter_gameover_display(); break;
		case GAME_STATE_SHOW_SCORE: game_shooter_score_display(); break;
	}
}

void scr_game_ui_handle(ak_msg_t *msg) {
	if (msg->sig == SCREEN_ENTRY) {
		game_load_data();
		timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_IDLE, 50, TIMER_ONE_SHOT);
		return;
	}
	if (msg->sig == AC_DISPLAY_SHOW_IDLE) {
		timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_IDLE, 50, TIMER_ONE_SHOT);
		return;
	}

	if (msg->sig == AC_DISPLAY_GAME_OVER_NEXT) {
		game_update_high_score(g_score);
		g_game_state = GAME_STATE_GAMEOVER;
		if(g_game_data.sound_en) BUZZER_PlaySound(BUZZER_SOUND_LOWSCORE);
		// Automatically transition to SCORE screen after a delay
		// Or wait for user input
		return;
	}

	switch (g_game_state) {
		case GAME_STATE_MENU:
			if (msg->sig == AC_DISPLAY_BUTON_UP_PRESSED) { g_menu_selected = (g_menu_selected > 0) ? g_menu_selected - 1 : 2; }
			else if (msg->sig == AC_DISPLAY_BUTON_DOWN_PRESSED) { g_menu_selected = (g_menu_selected < 2) ? g_menu_selected + 1 : 0; }
			else if (msg->sig == AC_DISPLAY_BUTON_MODE_PRESSED) {
				if (g_menu_selected == 0) { g_game_state = GAME_STATE_PLAYING; task_post_pure_msg(AC_TASK_GAME_SHOOTER_ID, AC_GAME_START_REQ); }
				else if (g_menu_selected == 1) { g_game_state = GAME_STATE_SETTING; g_setting_selected = 0; }
				else if (g_menu_selected == 2) { g_game_state = GAME_STATE_HIGH_SCORE; g_score_selected = 0; }
			}
			break;
		case GAME_STATE_SETTING:
			if (msg->sig == AC_DISPLAY_BUTON_UP_PRESSED) { g_setting_selected = (g_setting_selected > 0) ? g_setting_selected - 1 : 2; }
			else if (msg->sig == AC_DISPLAY_BUTON_DOWN_PRESSED) { g_setting_selected = (g_setting_selected < 2) ? g_setting_selected + 1 : 0; }
			else if (msg->sig == AC_DISPLAY_BUTON_MODE_PRESSED) {
				if (g_setting_selected == 0) { g_game_data.sound_en = !g_game_data.sound_en; game_save_data(); }
				else if (g_setting_selected == 1) { g_game_data.difficulty = (g_game_data.difficulty + 1) % 3; game_save_data(); }
				else if (g_setting_selected == 2) { g_game_state = GAME_STATE_MENU; }
			}
			break;
		case GAME_STATE_HIGH_SCORE:
			if (msg->sig == AC_DISPLAY_BUTON_UP_PRESSED || msg->sig == AC_DISPLAY_BUTON_DOWN_PRESSED) {
				g_score_selected = (g_score_selected == 0) ? 1 : 0;
			}
			else if (msg->sig == AC_DISPLAY_BUTON_MODE_PRESSED) {
				if (g_score_selected == 0) { init_default_save_data(); game_save_data(); }
				else if (g_score_selected == 1) { g_game_state = GAME_STATE_MENU; }
			}
			break;
		case GAME_STATE_PLAYING:
			if (msg->sig == AC_DISPLAY_BUTON_UP_PRESSED) task_post_pure_msg(AC_TASK_GAME_SHOOTER_ID, AC_GAME_BTN_UP);
			else if (msg->sig == AC_DISPLAY_BUTON_DOWN_PRESSED) task_post_pure_msg(AC_TASK_GAME_SHOOTER_ID, AC_GAME_BTN_DOWN);
			else if (msg->sig == AC_DISPLAY_BUTON_MODE_PRESSED) task_post_pure_msg(AC_TASK_GAME_SHOOTER_ID, AC_GAME_BTN_MODE);
			break;
		case GAME_STATE_GAMEOVER:
			if (msg->sig == AC_DISPLAY_BUTON_MODE_PRESSED) { g_game_state = GAME_STATE_SHOW_SCORE; g_show_score_selected = 0; }
			break;
		case GAME_STATE_SHOW_SCORE:
			if (msg->sig == AC_DISPLAY_BUTON_UP_PRESSED) { g_show_score_selected = (g_show_score_selected > 0) ? g_show_score_selected - 1 : 2; }
			else if (msg->sig == AC_DISPLAY_BUTON_DOWN_PRESSED) { g_show_score_selected = (g_show_score_selected < 2) ? g_show_score_selected + 1 : 0; }
			else if (msg->sig == AC_DISPLAY_BUTON_MODE_PRESSED) {
				if (g_show_score_selected == 0) { g_game_state = GAME_STATE_PLAYING; task_post_pure_msg(AC_TASK_GAME_SHOOTER_ID, AC_GAME_START_REQ); }
				else if (g_show_score_selected == 1) { g_game_state = GAME_STATE_HIGH_SCORE; g_score_selected = 1; }
				else if (g_show_score_selected == 2) { g_game_state = GAME_STATE_MENU; }
			}
			break;
	}
}
