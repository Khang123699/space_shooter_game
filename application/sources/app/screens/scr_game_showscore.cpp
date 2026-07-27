#include "scr_game_showscore.h"
#include "game_shooter.h"
#include "game_save.h"
#include "view_render.h"
#include "buzzer.h"
#include "timer.h"
#include "task_list.h"
#include "xprintf.h"
#include "game_bitmaps.h"
#include <string.h>

#define CENTER_X(str_len, char_width) ((128 - (str_len) * (char_width)) / 2)

static uint8_t show_score_selected = 0;

static void view_scr_game_showscore();

view_dynamic_t dyn_view_game_showscore = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_game_showscore
};

view_screen_t scr_game_showscore = {
	&dyn_view_game_showscore,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0,
};

static void view_scr_game_showscore() {
	for (int i = 0; i < MAX_STARS; i++) {
		view_render.drawPixel(g_stars[i].x, g_stars[i].y, WHITE);
	}

	view_render.setTextSize(1);
	if (g_new_high_score_rank > 0) {
		view_render.setCursor(CENTER_X(15, 6), 6);
		view_render.print("NEW HIGH SCORE!");
		view_render.setCursor(CENTER_X(5, 6), 16);
		view_render.print("TOP ");
		char temp[4];
		xsprintf(temp, "%u", (unsigned int)g_new_high_score_rank);
		view_render.print(temp);
	} else {
		view_render.setCursor(CENTER_X(11, 6), 14);
		view_render.print("YOUR SCORE:");
	}
	
	view_render.setTextSize(2);
	char temp_final[12];
	xsprintf(temp_final, "%u", (unsigned int)g_score);
	int16_t w = strlen(temp_final) * 12;
	view_render.setCursor((128 - w) / 2, 28);
	view_render.print(temp_final);
	
	view_render.drawBitmap(30, 48, icon_play, 8, 8, WHITE);
	view_render.drawBitmap(60, 48, icon_trophy, 8, 8, WHITE);
	view_render.drawBitmap(90, 48, icon_menu, 8, 8, WHITE);
	
	uint8_t selected_x = 30 + (show_score_selected * 30);
	view_render.drawRect(selected_x - 3, 45, 14, 14, WHITE);
}

void scr_game_showscore_handle(ak_msg_t* msg) {
	switch (msg->sig) {
		case SCREEN_ENTRY:
			timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_GAME_UI_ANIM_TICK, 50, TIMER_PERIODIC);
			timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_IDLE_TIMEOUT, 12000, TIMER_ONE_SHOT);
			break;

		case AC_DISPLAY_GAME_UI_ANIM_TICK:
			game_shooter_update_stars();
			break;

		case AC_DISPLAY_SHOW_IDLE:
		case AC_DISPLAY_IDLE_TIMEOUT:
			SCREEN_TRAN(scr_idle_handle, &scr_idle);
			break;

		case AC_DISPLAY_BUTTON_UP_PRESSED:
			timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_IDLE_TIMEOUT, 12000, TIMER_ONE_SHOT);
			show_score_selected = (show_score_selected > 0) ? show_score_selected - 1 : 2;
			if (g_game_setting.sound_en) {
				BUZZER_PlaySound(BUZZER_SOUND_CLICK);
			}
			break;

		case AC_DISPLAY_BUTTON_DOWN_PRESSED:
			timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_IDLE_TIMEOUT, 12000, TIMER_ONE_SHOT);
			show_score_selected = (show_score_selected < 2) ? show_score_selected + 1 : 0;
			if (g_game_setting.sound_en) {
				BUZZER_PlaySound(BUZZER_SOUND_CLICK);
			}
			break;

		case AC_DISPLAY_BUTTON_MODE_PRESSED:
			timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_IDLE_TIMEOUT, 12000, TIMER_ONE_SHOT);
			if (g_game_setting.sound_en) {
				BUZZER_PlaySound(BUZZER_SOUND_CLICK);
			}
			if (show_score_selected == 0) {
				SCREEN_TRAN(scr_game_play_handle, &scr_game_play);
				task_post_pure_msg(AC_TASK_GAME_SHOOTER_ID, AC_GAME_START_REQ);
			} else if (show_score_selected == 1) {
				SCREEN_TRAN(scr_game_highscore_handle, &scr_game_highscore);
			} else if (show_score_selected == 2) {
				SCREEN_TRAN(scr_game_menu_handle, &scr_game_menu);
			}
			break;
	}
}
