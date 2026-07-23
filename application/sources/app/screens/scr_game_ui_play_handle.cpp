#include "scr_game_ui.h"
#include "game_shooter.h"
#include "game_save.h"
#include "buzzer.h"
#include "timer.h"
#include "task_list.h"

// Forward hardware button events to Game Logic Task during gameplay
void handle_ui_playing_input(ak_msg_t *msg) {
	// Forward hardware button events to Game Logic Task via message passing
	switch (msg->sig) {
		case AC_DISPLAY_BUTTON_UP_PRESSED:
			task_post_pure_msg(AC_TASK_GAME_SHOOTER_ID, AC_GAME_BTN_UP);
			break;
		case AC_DISPLAY_BUTTON_UP_RELEASED:
			task_post_pure_msg(AC_TASK_GAME_SHOOTER_ID, AC_GAME_BTN_UP_RELEASED);
			break;
		case AC_DISPLAY_BUTTON_DOWN_PRESSED:
			task_post_pure_msg(AC_TASK_GAME_SHOOTER_ID, AC_GAME_BTN_DOWN);
			break;
		case AC_DISPLAY_BUTTON_DOWN_RELEASED:
			task_post_pure_msg(AC_TASK_GAME_SHOOTER_ID, AC_GAME_BTN_DOWN_RELEASED);
			break;
		case AC_DISPLAY_BUTTON_MODE_PRESSED:
			task_post_pure_msg(AC_TASK_GAME_SHOOTER_ID, AC_GAME_BTN_MODE);
			break;
	}
}

// Handle input during Game Over animation
void handle_ui_gameover_input(ak_msg_t *msg) {
	if (msg->sig == AC_DISPLAY_BUTTON_MODE_PRESSED) { 
		g_new_high_score_rank = game_update_high_score(g_score);
		g_game_state = GAME_STATE_SHOW_SCORE;
		g_show_score_selected = 0;
		if (g_new_high_score_rank > 0 && g_game_setting.sound_en) {
			BUZZER_PlaySound(BUZZER_SOUND_HIGHSCORE);
		}
	}
}

// Handle input for Score and New High Score screens
void handle_ui_showscore_input(ak_msg_t *msg) {
	switch (msg->sig) {
		case AC_DISPLAY_BUTTON_UP_PRESSED:
			g_show_score_selected = (g_show_score_selected > 0) ? g_show_score_selected - 1 : 2;
			break;
		case AC_DISPLAY_BUTTON_DOWN_PRESSED:
			g_show_score_selected = (g_show_score_selected < 2) ? g_show_score_selected + 1 : 0;
			break;
		case AC_DISPLAY_BUTTON_MODE_PRESSED:
			if (g_show_score_selected == 0) { 
				g_game_state = GAME_STATE_PLAYING; 
				task_post_pure_msg(AC_TASK_GAME_SHOOTER_ID, AC_GAME_START_REQ); 
			} else if (g_show_score_selected == 1) { 
				g_game_state = GAME_STATE_HIGH_SCORE; 
				g_score_selected = 0; 
			} else if (g_show_score_selected == 2) { 
				g_game_state = GAME_STATE_MENU; 
			}
			break;
	}
}
