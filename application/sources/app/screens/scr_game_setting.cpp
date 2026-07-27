#include "scr_game_setting.h"
#include "game_shooter.h"
#include "game_save.h"
#include "view_render.h"
#include "buzzer.h"
#include "timer.h"
#include <string.h>

#define CENTER_X(str_len, char_width) ((128 - (str_len) * (char_width)) / 2)

static uint8_t setting_selected = 0;

static void view_scr_game_setting();

view_dynamic_t dyn_view_game_setting = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_game_setting
};

view_screen_t scr_game_setting = {
	&dyn_view_game_setting,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0,
};

static void view_scr_game_setting() {
	view_render.clear();
	view_render.setTextColor(WHITE);

	for (int i = 0; i < MAX_STARS; i++) {
		view_render.drawPixel(g_stars[i].x, g_stars[i].y, WHITE);
	}
	
	view_render.drawRect(0, 0, 128, 64, WHITE);
	view_render.setTextSize(1);
	view_render.setCursor(CENTER_X(8, 6), 6);
	view_render.print("SETTINGS");
	view_render.drawLine(10, 16, 117, 16, WHITE);
	
	view_render.setCursor(26, 22);
	view_render.print("Sound:");
	view_render.setCursor(80, 22);
	view_render.print(g_game_setting.sound_en ? "ON" : "OFF");
	
	view_render.setCursor(26, 34);
	view_render.print("Diff:");
	view_render.setCursor(80, 34);
	if (g_game_setting.difficulty == 0) view_render.print("EASY");
	else if (g_game_setting.difficulty == 1) view_render.print("MED");
	else view_render.print("HARD");
	
	view_render.setCursor(26, 48);
	view_render.print("Back");
	
	view_render.setCursor(14, 22 + setting_selected * 13);
	view_render.print(">");
}

void scr_game_setting_handle(ak_msg_t* msg) {
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
			setting_selected = (setting_selected > 0) ? setting_selected - 1 : 2;
			if (g_game_setting.sound_en) {
				BUZZER_PlaySound(BUZZER_SOUND_CLICK);
			}
			break;

		case AC_DISPLAY_BUTTON_DOWN_PRESSED:
			timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_IDLE_TIMEOUT, 12000, TIMER_ONE_SHOT);
			setting_selected = (setting_selected < 2) ? setting_selected + 1 : 0;
			if (g_game_setting.sound_en) {
				BUZZER_PlaySound(BUZZER_SOUND_CLICK);
			}
			break;

		case AC_DISPLAY_BUTTON_MODE_PRESSED:
			timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_IDLE_TIMEOUT, 12000, TIMER_ONE_SHOT);
			if (g_game_setting.sound_en) {
				BUZZER_PlaySound(BUZZER_SOUND_CLICK);
			}
			if (setting_selected == 0) { 
				g_game_setting.sound_en = !g_game_setting.sound_en; 
				game_save_setting(); 
			} else if (setting_selected == 1) { 
				g_game_setting.difficulty = (g_game_setting.difficulty + 1) % 3; 
				game_save_setting(); 
			} else if (setting_selected == 2) { 
				SCREEN_TRAN(scr_game_menu_handle, &scr_game_menu);
			}
			break;
	}
}
