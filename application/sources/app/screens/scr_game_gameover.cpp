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
	for (int i = 0; i < MAX_STARS; i++) {
		view_render.drawPixel(g_stars[i].x, g_stars[i].y, WHITE);
	}

	// 1. Exploding Spaceship (Particles)
	if (gameover_anim_frame <= 20) {
		int r = gameover_anim_frame / 2;
		int cx = g_player_x + 4;
		int cy = 58;
		
		view_render.drawPixel(cx - r, cy - r, WHITE);
		view_render.drawPixel(cx + r, cy - r, WHITE);
		view_render.drawPixel(cx - r, cy + r, WHITE);
		view_render.drawPixel(cx + r, cy + r, WHITE);
		view_render.drawPixel(cx, cy - r - 2, WHITE);
		view_render.drawPixel(cx, cy + r + 2, WHITE);
		view_render.drawPixel(cx - r - 2, cy, WHITE);
		view_render.drawPixel(cx + r + 2, cy, WHITE);
		
		if (gameover_anim_frame < 10) {
			view_render.drawPixel(cx, cy, WHITE);
		}
	}

	// 2. GAME OVER Text
	view_render.setTextSize(2);
	view_render.setCursor(CENTER_X(9, 12), 24);
	view_render.print("GAME OVER");
}

void scr_game_gameover_handle(ak_msg_t* msg) {
	switch (msg->sig) {
		case SCREEN_ENTRY:
			gameover_anim_frame = 0;
			timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_GAME_UI_ANIM_TICK, 50, TIMER_PERIODIC);
			timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_IDLE_TIMEOUT, 12000, TIMER_ONE_SHOT);
			break;

		case AC_DISPLAY_GAME_UI_ANIM_TICK:
			game_shooter_update_stars();
			if (gameover_anim_frame < 60) {
				gameover_anim_frame++;
				if (gameover_anim_frame == 60) {
					SCREEN_TRAN(scr_game_showscore_handle, &scr_game_showscore);
				}
			}
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
