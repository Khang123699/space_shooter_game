#include "scr_game_ui.h"
#include "game_shooter.h"
#include "game_save.h"
#include "view_render.h"
#include "xprintf.h"
#include "game_bitmaps.h"
#include <string.h>

// Helper macro to center text horizontally on a 128px screen
#define CENTER_X(str_len, char_width) ((128 - (str_len) * (char_width)) / 2)

// Render Main Menu UI
void game_shooter_menu_display() {
	view_render.drawRect(0, 0, 128, 64, WHITE);
	view_render.setTextSize(1);
	view_render.setCursor(CENTER_X(12, 6), 6);
	view_render.print("SPACE SHOOTER");
	view_render.drawLine(10, 16, 117, 16, WHITE);
	
	view_render.drawBitmap(18, 24, icon_play, 8, 8, WHITE);
	view_render.setCursor(32, 24);
	view_render.print("Play");
	
	view_render.drawBitmap(18, 36, icon_setting, 8, 8, WHITE);
	view_render.setCursor(32, 36);
	view_render.print("Setting");
	
	view_render.drawBitmap(18, 48, icon_trophy, 8, 8, WHITE);
	view_render.setCursor(32, 48);
	view_render.print("High score");
	
	view_render.setCursor(6, 24 + g_menu_selected * 12);
	view_render.print(">");
}

// Render Settings Menu UI
void game_shooter_setting_display() {
	view_render.drawRect(0, 0, 128, 64, WHITE);
	view_render.setTextSize(1);
	view_render.setCursor(CENTER_X(8, 6), 6);
	view_render.print("SETTINGS");
	view_render.drawLine(10, 16, 117, 16, WHITE);
	
	view_render.setCursor(26, 22);
	view_render.print("Sound:");
	view_render.setCursor(80, 22);
	view_render.print(g_game_data.sound_en ? "ON" : "OFF");
	
	view_render.setCursor(26, 34);
	view_render.print("Diff:");
	view_render.setCursor(80, 34);
	if (g_game_data.difficulty == 0) view_render.print("EASY");
	else if (g_game_data.difficulty == 1) view_render.print("MED");
	else view_render.print("HARD");
	
	view_render.setCursor(26, 48);
	view_render.print("Back");
	
	view_render.setCursor(14, 22 + g_setting_selected * 13);
	view_render.print(">");
}

// Render High Score Menu UI
void game_shooter_highscore_display() {
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
		xsprintf(temp_str, "%u", (unsigned int)g_game_data.top_score[i]);
		view_render.print(temp_str);
	}
	
	view_render.setCursor(24, 52);
	view_render.print("Back");
	view_render.setCursor(84, 52);
	view_render.print("Reset");
	
	if (g_score_selected == 0) view_render.setCursor(14, 52);
	else view_render.setCursor(74, 52);
	
	view_render.print(">");
}
