#include "scr_game_ui.h"
#include "game_shooter.h"
#include "view_render.h"

// Define 8x8 bitmaps for menu icons and entities
static const uint8_t icon_player[8]   = { 0x18, 0x18, 0x3c, 0x3c, 0x5a, 0x99, 0xff, 0xff };
static const uint8_t icon_enemy1[8]   = { 0x42, 0x24, 0x3c, 0x5a, 0xff, 0xa5, 0x24, 0x42 };
static const uint8_t bmp_explosion[8] = { 0x42, 0x24, 0x18, 0x99, 0x99, 0x18, 0x24, 0x42 };

void view_scr_game_ui();

view_dynamic_t dyn_view_idle = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_game_ui
};

view_screen_t scr_game_ui = {
	&dyn_view_idle,
	ITEM_NULL,
	ITEM_NULL,

	.focus_item = 0,
};

void view_scr_game_ui() {
	view_render.clear();
	
	if (g_game_state == GAME_STATE_MENU) {
		view_render.drawRect(0, 0, 128, 64, WHITE);
		view_render.setCursor(26, 6);
		view_render.print("SHOOTER GAME");
	} 
	else if (g_game_state == GAME_STATE_PLAYING) {
		if (g_player_blink % 2 == 0) {
			view_render.drawBitmap(g_player_x, 54, icon_player, 8, 8, WHITE);
		}
		for (int e = 0; e < MAX_ENEMIES; e++) {
			if (g_enemies[e].active) {
				view_render.drawBitmap(g_enemies[e].x, g_enemies[e].y, icon_enemy1, 8, 8, WHITE);
			}
		}
		for (int ex = 0; ex < MAX_EXPLOSIONS; ex++) {
			if (g_explosions[ex].active) {
				view_render.drawBitmap(g_explosions[ex].x, g_explosions[ex].y, bmp_explosion, 8, 8, WHITE);
			}
		}
		if (g_transition_timer > 0) {
			view_render.setCursor(40, 28);
			view_render.print("STAGE ");
			view_render.print(g_stage);
		}
	}
}

#include "buzzer.h"

void scr_game_ui_handle(ak_msg_t *msg) {
	// Game Over handler from Game Task
	if (msg->sig == AC_DISPLAY_GAME_OVER_NEXT) {
		g_game_state = GAME_STATE_GAMEOVER;
		BUZZER_PlaySound(BUZZER_SOUND_LOWSCORE);
	}
	// If the game is currently playing, forward directional inputs to Game Task
	else if (g_game_state == GAME_STATE_PLAYING) {
		if (msg->sig == AC_DISPLAY_BUTON_UP_PRESSED) 
			task_post_pure_msg(AC_TASK_GAME_SHOOTER_ID, AC_GAME_BTN_UP);
		else if (msg->sig == AC_DISPLAY_BUTON_DOWN_PRESSED) 
			task_post_pure_msg(AC_TASK_GAME_SHOOTER_ID, AC_GAME_BTN_DOWN);
		else if (msg->sig == AC_DISPLAY_BUTON_MODE_PRESSED) 
			task_post_pure_msg(AC_TASK_GAME_SHOOTER_ID, AC_GAME_BTN_MODE);
	} 
	// If in menu state, pressing MODE button starts the game
	else if (g_game_state == GAME_STATE_MENU) {
		if (msg->sig == AC_DISPLAY_BUTON_MODE_PRESSED) {
			g_game_state = GAME_STATE_PLAYING;
			task_post_pure_msg(AC_TASK_GAME_SHOOTER_ID, AC_GAME_START_REQ);
		}
	}
}
