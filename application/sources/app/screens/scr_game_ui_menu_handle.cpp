#include "scr_game_ui.h"
#include "game_shooter.h"
#include "game_save.h"
#include "buzzer.h"
#include "timer.h"
#include "task_list.h"

// Handle input for Title Screen
void handle_ui_title_input(ak_msg_t *msg) {
	if (msg->sig == AC_DISPLAY_BUTTON_MODE_PRESSED) {
		g_game_state = GAME_STATE_MENU;
		g_menu_selected = 0;
	}
}

// Handle input for Main Menu
void handle_ui_menu_input(ak_msg_t *msg) {
	switch (msg->sig) {
		case AC_DISPLAY_BUTTON_UP_PRESSED:
			g_menu_selected = (g_menu_selected > 0) ? g_menu_selected - 1 : 3;
			break;
		case AC_DISPLAY_BUTTON_DOWN_PRESSED:
			g_menu_selected = (g_menu_selected < 3) ? g_menu_selected + 1 : 0;
			break;
		case AC_DISPLAY_BUTTON_MODE_PRESSED:
			if (g_menu_selected == 0) { 
				g_game_state = GAME_STATE_PLAYING; 
				task_post_pure_msg(AC_TASK_GAME_SHOOTER_ID, AC_GAME_START_REQ); 
			} else if (g_menu_selected == 1) { 
				g_game_state = GAME_STATE_SETTING; 
				g_setting_selected = 0; 
			} else if (g_menu_selected == 2) { 
				g_game_state = GAME_STATE_HIGH_SCORE; 
				g_score_selected = 0; 
			} else if (g_menu_selected == 3) {
				g_game_state = GAME_STATE_TITLE;
			}
			break;
	}
}

// Handle input for Settings Menu
void handle_ui_setting_input(ak_msg_t *msg) {
	switch (msg->sig) {
		case AC_DISPLAY_BUTTON_UP_PRESSED:
			g_setting_selected = (g_setting_selected > 0) ? g_setting_selected - 1 : 2;
			break;
		case AC_DISPLAY_BUTTON_DOWN_PRESSED:
			g_setting_selected = (g_setting_selected < 2) ? g_setting_selected + 1 : 0;
			break;
		case AC_DISPLAY_BUTTON_MODE_PRESSED:
			if (g_setting_selected == 0) { 
				g_game_data.sound_en = !g_game_data.sound_en; 
				game_save_data(); 
			} else if (g_setting_selected == 1) { 
				g_game_data.difficulty = (g_game_data.difficulty + 1) % 3; 
				game_save_data(); 
			} else if (g_setting_selected == 2) { 
				g_game_state = GAME_STATE_MENU; 
			}
			break;
	}
}

// Handle input for High Score Menu
void handle_ui_highscore_input(ak_msg_t *msg) {
	switch (msg->sig) {
		case AC_DISPLAY_BUTTON_UP_PRESSED:
		case AC_DISPLAY_BUTTON_DOWN_PRESSED:
			g_score_selected = (g_score_selected == 0) ? 1 : 0;
			break;
		case AC_DISPLAY_BUTTON_MODE_PRESSED:
			if (g_score_selected == 0) {
				g_game_state = GAME_STATE_MENU;
			} else {
				reset_high_score_data();
				game_save_data(); 
			}
			break;
	}
}
