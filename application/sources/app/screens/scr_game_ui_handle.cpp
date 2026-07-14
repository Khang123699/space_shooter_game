#include "scr_game_ui.h"
#include "game_shooter.h"
#include "game_save.h"
#include "buzzer.h"
#include "timer.h"
#include "task_list.h"
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
		case GAME_STATE_SHOW_SCORE: handle_ui_showscore_input(msg); break;
	}
}
