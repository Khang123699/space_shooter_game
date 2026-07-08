#include "scr_game_ui.h"
#include "game_shooter.h"
#include "game_save.h"
#include "view_render.h"
#include "buzzer.h"
#include "xprintf.h"
#include <string.h>
#include "game_bitmaps.h"

uint8_t g_new_high_score_rank = 0;
uint8_t g_gameover_anim_frame = 0;
static uint8_t g_new_high_score_timer = 0;

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
		xsprintf(temp_str, "%u", (unsigned int)g_game_data.top_score[i]);
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
			int r = 5 - g_explosions[ex].timer;
			int cx = g_explosions[ex].x + 4;
			int cy = g_explosions[ex].y + 4;
			if (r > 0) {
				view_render.drawPixel(cx - r, cy - r, WHITE);
				view_render.drawPixel(cx + r, cy - r, WHITE);
				view_render.drawPixel(cx - r, cy + r, WHITE);
				view_render.drawPixel(cx + r, cy + r, WHITE);
				view_render.drawPixel(cx, cy - r - 1, WHITE);
				view_render.drawPixel(cx, cy + r + 1, WHITE);
				view_render.drawPixel(cx - r - 1, cy, WHITE);
				view_render.drawPixel(cx + r + 1, cy, WHITE);
			} else {
				view_render.fillRect(cx - 1, cy - 1, 3, 3, WHITE);
			}
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
	xsprintf(temp_sc, "%u", (unsigned int)g_score);
	view_render.print(temp_sc);
	
	view_render.setCursor(50, 2);
	view_render.print("L:");
	char temp_lv[12];
	xsprintf(temp_lv, "%u", (unsigned int)g_stage);
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
	
	// Draw animated broken spaceship
	int anim_x = (g_gameover_anim_frame * 2) % 128; // moves across screen
	view_render.drawBitmap(anim_x, 34, icon_player, 8, 8, WHITE);
	if ((g_gameover_anim_frame / 2) % 2 == 0) {
		view_render.drawBitmap(anim_x - 8, 34, icon_flame1, 8, 8, WHITE);
	} else {
		view_render.drawBitmap(anim_x - 8, 34, icon_flame2, 8, 8, WHITE);
	}
	
	view_render.setTextSize(1);
	if ((g_gameover_anim_frame / 5) % 2 == 0) {
		view_render.setCursor(0, 50);
		view_render.print("Press Mode to continue");
	}
}

static void game_shooter_new_highscore_display() {
	view_render.setTextSize(1);
	view_render.setCursor(20, 20);
	view_render.print("NEW HIGH SCORE!");
	
	view_render.setCursor(44, 34);
	view_render.print("TOP ");
	char temp[4];
	xsprintf(temp, "%u", (unsigned int)g_new_high_score_rank);
	view_render.print(temp);
}

static void game_shooter_score_display() {
	view_render.setTextSize(1);
	view_render.setCursor(31, 14);
	view_render.print("YOUR SCORE:");
	view_render.setTextSize(2);
	char temp_final[12];
	xsprintf(temp_final, "%u", (unsigned int)g_score);
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
		case GAME_STATE_NEW_HIGH_SCORE: game_shooter_new_highscore_display(); break;
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
		if (g_game_state == GAME_STATE_GAMEOVER) {
			g_gameover_anim_frame++;
		} else if (g_game_state == GAME_STATE_NEW_HIGH_SCORE) {
			if (g_new_high_score_timer > 0) {
				g_new_high_score_timer--;
				if (g_new_high_score_timer == 0) {
					g_game_state = GAME_STATE_SHOW_SCORE;
					g_show_score_selected = 0;
				}
			}
		}
		timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_IDLE, 50, TIMER_ONE_SHOT);
		return;
	}

	if (msg->sig == AC_DISPLAY_BUTON_UP_PRESSED || 
	    msg->sig == AC_DISPLAY_BUTON_DOWN_PRESSED || 
	    msg->sig == AC_DISPLAY_BUTON_MODE_PRESSED) {
		if (g_game_state != GAME_STATE_PLAYING && g_game_data.sound_en) {
			BUZZER_PlaySound(BUZZER_SOUND_CLICK);
		}
	}

	if (msg->sig == AC_DISPLAY_GAME_OVER_NEXT) {
		g_game_state = GAME_STATE_GAMEOVER;
		g_gameover_anim_frame = 0;
		if(g_game_data.sound_en) BUZZER_PlaySound(BUZZER_SOUND_LOWSCORE);
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
			if (msg->sig == AC_DISPLAY_BUTON_MODE_PRESSED) { 
				g_new_high_score_rank = game_update_high_score(g_score);
				if (g_new_high_score_rank > 0) {
					g_game_state = GAME_STATE_NEW_HIGH_SCORE;
					g_new_high_score_timer = 30; // 1.5s
					if (g_game_data.sound_en) BUZZER_PlaySound(BUZZER_SOUND_HIGHSCORE);
				} else {
					g_game_state = GAME_STATE_SHOW_SCORE; 
					g_show_score_selected = 0; 
				}
			}
			break;
		case GAME_STATE_NEW_HIGH_SCORE:
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
