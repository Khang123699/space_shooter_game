#include "scr_game_ui.h"
#include "game_shooter.h"
#include "game_save.h"
#include "buzzer.h"
#include "timer.h"
#include "task_list.h"
// Handle input for Main Menu
static void handle_ui_menu_input(ak_msg_t *msg) {
	switch (msg->sig) {
		case AC_DISPLAY_BUTTON_UP_PRESSED:
			g_menu_selected = (g_menu_selected > 0) ? g_menu_selected - 1 : 2;
			break;
		case AC_DISPLAY_BUTTON_DOWN_PRESSED:
			g_menu_selected = (g_menu_selected < 2) ? g_menu_selected + 1 : 0;
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
			}
			break;
	}
}
// Handle input for Settings Menu
static void handle_ui_setting_input(ak_msg_t *msg) {
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
static void handle_ui_highscore_input(ak_msg_t *msg) {
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
// Forward hardware button events to Game Logic Task during gameplay
static void handle_ui_playing_input(ak_msg_t *msg) {
	// Forward hardware button events to Game Logic Task via message passing
	if (msg->sig == AC_DISPLAY_BUTTON_UP_PRESSED) task_post_pure_msg(AC_TASK_GAME_SHOOTER_ID, AC_GAME_BTN_UP);
	else if (msg->sig == AC_DISPLAY_BUTTON_DOWN_PRESSED) task_post_pure_msg(AC_TASK_GAME_SHOOTER_ID, AC_GAME_BTN_DOWN);
	else if (msg->sig == AC_DISPLAY_BUTTON_MODE_PRESSED) task_post_pure_msg(AC_TASK_GAME_SHOOTER_ID, AC_GAME_BTN_MODE);
}
// Handle input during Game Over animation
static void handle_ui_gameover_input(ak_msg_t *msg) {
	if (msg->sig == AC_DISPLAY_BUTTON_MODE_PRESSED) { 
		g_new_high_score_rank = game_update_high_score(g_score);
		if (g_new_high_score_rank > 0) {
			g_game_state = GAME_STATE_NEW_HIGH_SCORE;
			g_new_high_score_timer = 40; // 2.0s
			if (g_game_data.sound_en) BUZZER_PlaySound(BUZZER_SOUND_HIGHSCORE);
		} else {
			g_game_state = GAME_STATE_SHOW_SCORE; 
			g_show_score_selected = 0; 
		}
	}
}
// Handle input for Score and New High Score screens
static void handle_ui_showscore_input(ak_msg_t *msg) {
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
				g_score_selected = 1; 
			} else if (g_show_score_selected == 2) { 
				g_game_state = GAME_STATE_MENU; 
			}
			break;
	}
}
// Main UI input message handler based on current game state
void scr_game_ui_handle(ak_msg_t *msg) {
	if (msg->sig == AC_DISPLAY_RENDER_SCREEN) {
		g_render_pending = false;
		// return; // Let it fall through, so screen_manager will render it
	}
	
	if (msg->sig == SCREEN_ENTRY) {
		game_load_data();
		// Start UI animation timer (100ms interval) for things like blinking or explosions
		timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_GAME_UI_ANIM_TICK, 100, TIMER_PERIODIC);
		return;
	}
	if (msg->sig == AC_DISPLAY_GAME_UI_ANIM_TICK) {
		bool need_render = false;
		if (g_game_state == GAME_STATE_GAMEOVER) {
			if (g_gameover_anim_frame < 250) g_gameover_anim_frame++;
			need_render = true;
		} else if (g_game_state == GAME_STATE_NEW_HIGH_SCORE) {
			if (g_new_high_score_timer > 0) {
				g_new_high_score_timer--;
				if (g_new_high_score_timer == 0) {
					g_game_state = GAME_STATE_SHOW_SCORE;
					g_show_score_selected = 0;
				}
			}
			need_render = true;
		} else if (g_game_state == GAME_STATE_SHOW_SCORE) {
			need_render = true;
		}
		
		if (need_render) {
			task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_RENDER_SCREEN);
		}
		
		return;
	}

	if (msg->sig == AC_DISPLAY_BUTTON_UP_PRESSED || 
	    msg->sig == AC_DISPLAY_BUTTON_DOWN_PRESSED || 
	    msg->sig == AC_DISPLAY_BUTTON_MODE_PRESSED) {
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
		case GAME_STATE_MENU: handle_ui_menu_input(msg); break;
		case GAME_STATE_SETTING: handle_ui_setting_input(msg); break;
		case GAME_STATE_HIGH_SCORE: handle_ui_highscore_input(msg); break;
		case GAME_STATE_PLAYING: handle_ui_playing_input(msg); break;
		case GAME_STATE_GAMEOVER: handle_ui_gameover_input(msg); break;
		case GAME_STATE_NEW_HIGH_SCORE: break;
		case GAME_STATE_SHOW_SCORE: handle_ui_showscore_input(msg); break;
	}
}
