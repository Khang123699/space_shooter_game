#include "scr_game_ui.h"
#include "game_shooter.h"
#include "game_save.h"
#include "buzzer.h"
#include "timer.h"
#include "task_list.h"
#include "scr_idle.h"
#include <stdlib.h>
// Main UI input message handler based on current game state
void scr_game_ui_handle(ak_msg_t *msg) {
	switch (msg->sig) {
		case AC_DISPLAY_RENDER_SCREEN:
			g_render_pending = false;
			break;
			
		case SCREEN_ENTRY:
			game_load_data();
			for (int i = 0; i < MAX_STARS; i++) {
				g_stars[i].x = rand() % 128;
				g_stars[i].y = rand() % 64;
				g_stars[i].speed = (rand() % 2) + 1;
			}
			// Init timers for UI animation and idle timeout
			timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_GAME_UI_ANIM_TICK, 100, TIMER_PERIODIC);
			timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_IDLE_TIMEOUT, 12000, TIMER_ONE_SHOT);
			return;

		case AC_DISPLAY_GAME_UI_ANIM_TICK: {
			// Trigger render for active animations
			bool need_render = false;
			if (g_game_state == GAME_STATE_GAMEOVER) {
				if (g_gameover_anim_frame < 250) g_gameover_anim_frame++;
				need_render = true;
			} else if (g_game_state == GAME_STATE_SHOW_SCORE) {
				need_render = true;
			} else if (g_game_state == GAME_STATE_TITLE || g_game_state == GAME_STATE_MENU || 
			           g_game_state == GAME_STATE_SETTING || g_game_state == GAME_STATE_HIGH_SCORE) {
				g_tick_count++;
				for (int i = 0; i < MAX_STARS; i++) {
					g_stars[i].y += g_stars[i].speed;
					if (g_stars[i].y >= 64) {
						g_stars[i].y = 0;
						g_stars[i].x = rand() % 128;
					}
				}
				need_render = true;
			}
			
			if (need_render) {
				task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_RENDER_SCREEN);
			}
			return;
		}

		case AC_DISPLAY_IDLE_TIMEOUT:
			if (g_game_state != GAME_STATE_PLAYING) {
				SCREEN_TRAN(scr_idle_handle, &scr_idle);
			}
			return;

		case AC_DISPLAY_BUTTON_UP_PRESSED:
		case AC_DISPLAY_BUTTON_DOWN_PRESSED:
		case AC_DISPLAY_BUTTON_MODE_PRESSED:
			if (g_game_state != GAME_STATE_PLAYING) {
				// Reset idle timeout on user interaction
				timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_IDLE_TIMEOUT, 12000, TIMER_ONE_SHOT);
				if (g_game_setting.sound_en) {
					BUZZER_PlaySound(BUZZER_SOUND_CLICK);
				}
			}
			break;

		case AC_DISPLAY_GAME_OVER_NEXT:
			g_game_state = GAME_STATE_GAMEOVER;
			g_gameover_anim_frame = 0;
			if(g_game_setting.sound_en) BUZZER_PlaySound(BUZZER_SOUND_LOWSCORE);
			
			timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_IDLE_TIMEOUT, 12000, TIMER_ONE_SHOT);
			return;
	}

	// Dispatch message based on current UI state
	switch (g_game_state) {
		case GAME_STATE_TITLE: handle_ui_title_input(msg); break;
		case GAME_STATE_MENU: handle_ui_menu_input(msg); break;
		case GAME_STATE_SETTING: handle_ui_setting_input(msg); break;
		case GAME_STATE_HIGH_SCORE: handle_ui_highscore_input(msg); break;
		case GAME_STATE_PLAYING: handle_ui_playing_input(msg); break;
		case GAME_STATE_GAMEOVER: handle_ui_gameover_input(msg); break;
		case GAME_STATE_SHOW_SCORE: handle_ui_showscore_input(msg); break;
	}
}
