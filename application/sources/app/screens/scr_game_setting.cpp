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
	view_render.setCursor(CENTER_X(8, 6), 4);
	view_render.print("SETTINGS");
	view_render.drawLine(10, 14, 117, 14, WHITE);
	
	const char* diff_names[] = {"EASY", "NORMAL", "HARD"};
	char opt_str[32];
	
	for (int i = 0; i < 3; i++) {
		if (i == 0) {
			strcpy(opt_str, "SOUND: ");
			strcat(opt_str, g_game_setting.sound_en ? "ON" : "OFF");
		} else if (i == 1) {
			strcpy(opt_str, "DIFF: ");
			strcat(opt_str, diff_names[g_game_setting.difficulty % 3]);
		} else {
			strcpy(opt_str, "BACK");
		}
		
		if (i == setting_selected) {
			view_render.fillRect(15, 20 + i * 13, 98, 11, WHITE);
			view_render.setTextColor(BLACK);
		} else {
			view_render.setTextColor(WHITE);
		}
		view_render.setCursor(CENTER_X(strlen(opt_str), 6), 22 + i * 13);
		view_render.print(opt_str);
	}
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
