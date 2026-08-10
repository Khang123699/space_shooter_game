#include "scr_game_highscore.h"
#include "game_shooter_player_task.h"
#include "game_shooter_enemy_task.h"
#include "game_shooter_bullet_task.h"
#include "game_shooter_stage_task.h"
#include "game_shooter_render.h"
#include "game_save.h"
#include "view_render.h"
#include "buzzer.h"
#include "timer.h"
#include "xprintf.h"
#include <string.h>

#define CENTER_X(str_len, char_width) ((128 - (str_len) * (char_width)) / 2)

static uint8_t score_selected = 0;

static void view_scr_game_highscore();

view_dynamic_t dyn_view_game_highscore = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_game_highscore
};

view_screen_t scr_game_highscore = {
	&dyn_view_game_highscore,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0,
};

static void view_scr_game_highscore() {
	view_render.clear();
	view_render.setTextColor(WHITE);

	for (int i = 0; i < MAX_STARS; i++) {
		view_render.drawPixel(game_get_stars()[i].x, game_get_stars()[i].y, WHITE);
	}
	
	view_render.drawRect(0, 0, 128, 64, WHITE);
	view_render.setTextSize(1);
	view_render.setCursor(CENTER_X(11, 6), 6);
	view_render.print("HIGH SCORES");
	view_render.drawLine(10, 16, 117, 16, WHITE);
	
	for (int i = 0; i < 3; i++) {
		view_render.setCursor(26, 20 + i * 10);
		const char* prefixes[] = {"1st: ", "2nd: ", "3rd: "};
		view_render.print(prefixes[i]);
		
		char temp_str[12];
		xsprintf(temp_str, "%u", (unsigned int)g_game_score.top_score[i]);
		view_render.print(temp_str);
	}
	
	view_render.setCursor(24, 52);
	view_render.print("Back");
	view_render.setCursor(84, 52);
	view_render.print("Reset");
	
	if (score_selected == 0) view_render.setCursor(14, 52);
	else view_render.setCursor(74, 52);
	
	view_render.print(">");
}

void scr_game_highscore_handle(ak_msg_t* msg) {
	switch (msg->sig) {
		case SCREEN_ENTRY:
			timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_GAME_UI_ANIM_TICK, 50, TIMER_PERIODIC);
			timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_IDLE_TIMEOUT, 12000, TIMER_ONE_SHOT);
			break;

		case AC_DISPLAY_GAME_UI_ANIM_TICK:
			game_render_handle(msg);
			break;

		case AC_DISPLAY_SHOW_IDLE:
		case AC_DISPLAY_IDLE_TIMEOUT:
			SCREEN_TRAN(scr_idle_handle, &scr_idle);
			break;

		case AC_DISPLAY_BUTTON_UP_PRESSED:
		case AC_DISPLAY_BUTTON_DOWN_PRESSED:
			timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_IDLE_TIMEOUT, 12000, TIMER_ONE_SHOT);
			score_selected = !score_selected;
			if (g_game_setting.sound_en) {
				BUZZER_PlaySound(BUZZER_SOUND_CLICK);
			}
			break;

		case AC_DISPLAY_BUTTON_MODE_PRESSED:
			timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_IDLE_TIMEOUT, 12000, TIMER_ONE_SHOT);
			if (g_game_setting.sound_en) {
				BUZZER_PlaySound(BUZZER_SOUND_CLICK);
			}
			if (score_selected == 0) {
				SCREEN_TRAN(scr_game_menu_handle, &scr_game_menu);
			} else {
				reset_high_score_data();
			}
			break;
	}
}
