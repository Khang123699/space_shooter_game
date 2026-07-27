#include "scr_game_gameover.h"
#include "game_shooter.h"
#include "game_save.h"
#include "view_render.h"
#include "buzzer.h"
#include "timer.h"
#include <string.h>

#define CENTER_X(str_len, char_width) ((128 - (str_len) * (char_width)) / 2)

static uint8_t gameover_anim_frame = 0;

static void view_scr_game_gameover();

view_dynamic_t dyn_view_game_gameover = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_game_gameover
};

view_screen_t scr_game_gameover = {
	&dyn_view_game_gameover,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0,
};

static void view_scr_game_gameover() {
	// 1. GAME OVER Text falling from top
	int y_pos = -12 + (int)gameover_anim_frame;
	if (y_pos > 8) {
		y_pos = 8;
	}
	view_render.setTextSize(2);
	view_render.setCursor(CENTER_X(9, 12), y_pos);
	view_render.print("GAME OVER");

	// 2. SCORE & STAGE
	if (gameover_anim_frame >= 20) {
		view_render.setTextSize(1);
		char str_score[20];
		xsprintf(str_score, "SCORE: %u", (unsigned int)g_score);
		view_render.setCursor(CENTER_X(strlen(str_score), 6), 28);
		view_render.print(str_score);

		char str_stage[20];
		xsprintf(str_stage, "STAGE: %u", (unsigned int)g_stage);
		view_render.setCursor(CENTER_X(strlen(str_stage), 6), 38);
		view_render.print(str_stage);
	}

	// 3. Blinking Instruction 
	if (gameover_anim_frame >= 30) {
		if ((gameover_anim_frame / 5) % 2 == 0) {
			view_render.setCursor(CENTER_X(18, 6), 52);
			view_render.print("Press MODE to next");
		}
	}
}

void scr_game_gameover_handle(ak_msg_t* msg) {
	switch (msg->sig) {
		case SCREEN_ENTRY:
			gameover_anim_frame = 0;
			timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_GAME_UI_ANIM_TICK, 50, TIMER_PERIODIC);
			timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_IDLE_TIMEOUT, 12000, TIMER_ONE_SHOT);
			break;

		case AC_DISPLAY_GAME_UI_ANIM_TICK:
			gameover_anim_frame++;
			task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_RENDER_SCREEN);
			break;

		case AC_DISPLAY_SHOW_IDLE:
		case AC_DISPLAY_IDLE_TIMEOUT:
			SCREEN_TRAN(scr_idle_handle, &scr_idle);
			break;

		case AC_DISPLAY_BUTTON_UP_PRESSED:
		case AC_DISPLAY_BUTTON_DOWN_PRESSED:
		case AC_DISPLAY_BUTTON_MODE_PRESSED:
			timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_IDLE_TIMEOUT, 12000, TIMER_ONE_SHOT);
			SCREEN_TRAN(scr_game_showscore_handle, &scr_game_showscore);
			if (g_game_setting.sound_en) {
				BUZZER_PlaySound(BUZZER_SOUND_CLICK);
			}
			break;
	}
}
