#include "scr_game_ui.h"
#include "game_shooter.h"
#include "game_save.h"
#include "view_render.h"
#include "buzzer.h"
#include "xprintf.h"
#include <string.h>
#include "game_bitmaps.h"

// Helper macro to center text horizontally on a 128px screen
#define CENTER_X(str_len, char_width) ((128 - (str_len) * (char_width)) / 2)

uint8_t g_new_high_score_rank = 0;
uint8_t g_gameover_anim_frame = 0;

void view_scr_game_ui();

view_dynamic_t dyn_view_game_ui = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_game_ui
};

view_screen_t scr_game_ui = {
	&dyn_view_game_ui,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0,
};
// Main UI rendering dispatcher based on current game state
void view_scr_game_ui() {
	// Clear the RAM buffer before rendering a new frame (Double Buffering)
	view_render.clear();
	view_render.setTextColor(WHITE);
	switch (g_game_state) {
		case GAME_STATE_TITLE: game_shooter_title_display(); break;
		case GAME_STATE_MENU: game_shooter_menu_display(); break;
		case GAME_STATE_SETTING: game_shooter_setting_display(); break;
		case GAME_STATE_HIGH_SCORE: game_shooter_highscore_display(); break;
		case GAME_STATE_PLAYING: game_shooter_playing_display(); break;
		case GAME_STATE_GAMEOVER: game_shooter_gameover_display(); break;
		case GAME_STATE_SHOW_SCORE: game_shooter_score_display(); break;
	}
}


