#include "scr_game_ui.h"
#include "game_shooter.h"
#include "game_save.h"
#include "view_render.h"
#include "xprintf.h"
#include "game_bitmaps.h"
#include <string.h>

// Helper macro to center text horizontally on a 128px screen
#define CENTER_X(str_len, char_width) ((128 - (str_len) * (char_width)) / 2)

// Render Title Screen
void game_shooter_title_display() {
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
	if ((g_tick_count / 5) % 2 == 0) { // Blink every ~0.5s
		view_render.setCursor(CENTER_X(18, 6), 56);
		view_render.print("PRESS MODE TO PLAY");
	}
}

// Render Main Menu UI
void game_shooter_menu_display() {
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
	
	view_render.setCursor(6, 18 + g_menu_selected * 11);
	view_render.print(">");
}

// Render Settings Menu UI
void game_shooter_setting_display() {
	// Draw Parallax Stars in background
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
	view_render.print(g_game_data.sound_en ? "ON" : "OFF");
	
	view_render.setCursor(26, 34);
	view_render.print("Diff:");
	view_render.setCursor(80, 34);
	const char* diff_str[] = {"EASY", "MED", "HARD"};
	view_render.print(diff_str[g_game_data.difficulty]);
	
	view_render.setCursor(26, 48);
	view_render.print("Back");
	
	view_render.setCursor(14, 22 + g_setting_selected * 13);
	view_render.print(">");
}

// Render High Score Menu UI
void game_shooter_highscore_display() {
	// Draw Parallax Stars in background
	for (int i = 0; i < MAX_STARS; i++) {
		view_render.drawPixel(g_stars[i].x, g_stars[i].y, WHITE);
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
