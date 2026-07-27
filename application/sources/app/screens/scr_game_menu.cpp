#include "scr_game_menu.h"
#include "game_shooter.h"
#include "game_save.h"
#include "view_render.h"
#include "buzzer.h"
#include "timer.h"
#include "task_list.h"
#include "game_bitmaps.h"
#include <string.h>

#define CENTER_X(str_len, char_width) ((128 - (str_len) * (char_width)) / 2)

static uint8_t current_location = 0;

static void view_scr_game_menu();

view_dynamic_t dyn_view_game_menu = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_game_menu
};

view_screen_t scr_game_menu = {
	&dyn_view_game_menu,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0,
};

static void view_scr_game_menu() {
	view_render.clear();
	view_render.setTextColor(WHITE);

	// Draw Parallax Stars in background
	for (int i = 0; i < MAX_STARS; i++) {
		view_render.drawPixel(g_stars[i].x, g_stars[i].y, WHITE);
	}
	
	view_render.drawRect(0, 0, 128, 64, WHITE);
	view_render.setTextSize(1);
	view_render.setCursor(CENTER_X(12, 6), 4);
	view_render.print("SPACE SHOOTER");
	view_render.drawLine(10, 14, 117, 14, WHITE);
	
	view_render.drawBitmap(18, 18, icon_play, 8, 8, WHITE);
	view_render.setCursor(32, 18);
	view_render.print("Play");
	
	view_render.drawBitmap(18, 29, icon_setting, 8, 8, WHITE);
	view_render.setCursor(32, 29);
	view_render.print("Setting");
	
	view_render.drawBitmap(18, 40, icon_trophy, 8, 8, WHITE);
	view_render.setCursor(32, 40);
	view_render.print("High score");
	
	view_render.drawBitmap(18, 51, icon_exit, 8, 8, WHITE);
	view_render.setCursor(32, 51);
	view_render.print("Exit");
	
	view_render.setCursor(6, 18 + current_location * 11);
	view_render.print(">");
}

void scr_game_menu_handle(ak_msg_t* msg) {
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
			current_location = (current_location > 0) ? current_location - 1 : 3;
			if (g_game_setting.sound_en) {
				BUZZER_PlaySound(BUZZER_SOUND_CLICK);
			}
			break;

		case AC_DISPLAY_BUTTON_DOWN_PRESSED:
			timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_IDLE_TIMEOUT, 12000, TIMER_ONE_SHOT);
			current_location = (current_location < 3) ? current_location + 1 : 0;
			if (g_game_setting.sound_en) {
				BUZZER_PlaySound(BUZZER_SOUND_CLICK);
			}
			break;

		case AC_DISPLAY_BUTTON_MODE_PRESSED:
			timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_IDLE_TIMEOUT, 12000, TIMER_ONE_SHOT);
			if (g_game_setting.sound_en) {
				BUZZER_PlaySound(BUZZER_SOUND_CLICK);
			}
			switch (current_location) {
				case 0:
					SCREEN_TRAN(scr_game_play_handle, &scr_game_play);
					task_post_pure_msg(AC_TASK_GAME_SHOOTER_ID, AC_GAME_START_REQ);
					break;
				case 1:
					SCREEN_TRAN(scr_game_setting_handle, &scr_game_setting);
					break;
				case 2:
					SCREEN_TRAN(scr_game_highscore_handle, &scr_game_highscore);
					break;
				case 3:
					SCREEN_TRAN(scr_game_title_handle, &scr_game_title);
					break;
			}
			break;
	}
}
