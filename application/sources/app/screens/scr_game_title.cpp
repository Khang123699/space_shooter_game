#include "scr_game_title.h"
#include "game_shooter.h"
#include "game_save.h"
#include "view_render.h"
#include "buzzer.h"
#include "timer.h"
#include "game_bitmaps.h"
#include <string.h>

#define CENTER_X(str_len, char_width) ((128 - (str_len) * (char_width)) / 2)

static void view_scr_game_title();

view_dynamic_t dyn_view_game_title = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_game_title
};

view_screen_t scr_game_title = {
	&dyn_view_game_title,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0,
};

static void view_scr_game_title() {
	view_render.clear();
	view_render.setTextColor(WHITE);

	// Draw Parallax Stars
	for (int i = 0; i < MAX_STARS; i++) {
		view_render.drawPixel(g_stars[i].x, g_stars[i].y, WHITE);
	}

	// Enemy stationary at top right
	view_render.drawBitmap(110, 5, icon_enemy2, 8, 8, WHITE);
	
	// Boss hovering at top left
	view_render.drawBitmap(5, 5, bmp_boss, 16, 16, WHITE);
	
	view_render.setTextSize(2);
	view_render.setCursor(CENTER_X(5, 12), 6); // 5 letters: SPACE
	view_render.print("SPACE");
	view_render.setCursor(CENTER_X(7, 12), 22); // 7 letters: SHOOTER
	view_render.print("SHOOTER");
	
	view_render.setTextSize(1);
	if ((g_tick_count / 10) % 2 == 0) { // Blink every ~0.5s
		view_render.setCursor(CENTER_X(18, 6), 56);
		view_render.print("PRESS MODE TO PLAY");
	}
}

void scr_game_title_handle(ak_msg_t* msg) {
	switch (msg->sig) {
		case SCREEN_ENTRY:
			game_load_data();
			if (g_stars[0].speed == 0) game_background_init();
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
		case AC_DISPLAY_BUTTON_DOWN_PRESSED:
		case AC_DISPLAY_BUTTON_MODE_PRESSED:
			timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_IDLE_TIMEOUT, 12000, TIMER_ONE_SHOT);
			if (msg->sig == AC_DISPLAY_BUTTON_MODE_PRESSED) {
				SCREEN_TRAN(scr_game_menu_handle, &scr_game_menu);
				if (g_game_setting.sound_en) {
					BUZZER_PlaySound(BUZZER_SOUND_CLICK);
				}
			}
			break;
	}
}
