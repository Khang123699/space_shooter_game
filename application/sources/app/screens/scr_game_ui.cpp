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
uint8_t g_new_high_score_timer = 0;

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
// Render Main Menu UI
static void game_shooter_menu_display() {
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
static void game_shooter_setting_display() {
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
static void game_shooter_highscore_display() {
	view_render.drawRect(0, 0, 128, 64, WHITE);
	view_render.setTextSize(1);
	view_render.setCursor(CENTER_X(11, 6), 6);
	view_render.print("HIGH SCORES");
	view_render.drawLine(10, 16, 117, 16, WHITE);
	
	for (int i = 0; i < 3; i++) {
		view_render.setCursor(26, 20 + i * 10);
		if (i == 0) view_render.print("1st: ");
		else if (i == 1) view_render.print("2nd: ");
		else view_render.print("3rd: ");
		
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
// Draw all active enemies and boss health bar
static void draw_enemies() {
	for (int e = 0; e < MAX_ENEMIES; e++) {
		if (g_enemies[e].active) {
			// Blinking effect when taking damage (except Boss)
			bool draw_sprite = true;
			if (g_enemies[e].blink_timer > 0 && g_enemies[e].type != 4 && (g_tick_count % 4 < 2)) {
				draw_sprite = false;
			}
			
			if (g_enemies[e].type == 4) {
				if (draw_sprite) {
					view_render.drawBitmap(g_enemies[e].x, g_enemies[e].y, bmp_boss, 16, 16, WHITE);
				}
				// HP Bar for Boss
				int max_hp = 5 + (g_stage / 5) * 5;
				int hp_width = (g_enemies[e].hp * 16) / max_hp;
				if (hp_width > 16) hp_width = 16;
				if (hp_width < 0) hp_width = 0;
				view_render.fillRect(g_enemies[e].x, g_enemies[e].y - 3, 16, 2, BLACK);
				view_render.fillRect(g_enemies[e].x, g_enemies[e].y - 3, hp_width, 2, WHITE);
			} else if (g_enemies[e].type == 3) {
				if (draw_sprite) view_render.drawBitmap(g_enemies[e].x, g_enemies[e].y, icon_enemy3, 8, 8, WHITE);
			} else if (g_enemies[e].type == 2) {
				if (draw_sprite) view_render.drawBitmap(g_enemies[e].x, g_enemies[e].y, icon_enemy2, 8, 8, WHITE);
			} else {
				if (draw_sprite) view_render.drawBitmap(g_enemies[e].x, g_enemies[e].y, icon_enemy1, 8, 8, WHITE);
			}
		}
	}
}
// Draw explosion effects for destroyed entities
static void draw_explosions() {
	for (int ex = 0; ex < MAX_EXPLOSIONS; ex++) {
		if (g_explosions[ex].active) {
			int r = 5 - g_explosions[ex].timer;
			int cx = g_explosions[ex].x + 4;
			int cy = g_explosions[ex].y + 4;
			if (r > 0) {
				view_render.drawPixel(cx - r, cy - r, WHITE);
				view_render.drawPixel(cx + r, cy - r, WHITE);
				view_render.drawPixel(cx - r, cy + r, WHITE);
				view_render.drawPixel(cx + r, cy + r, WHITE);
				view_render.drawPixel(cx, cy - r - 1, WHITE);
				view_render.drawPixel(cx, cy + r + 1, WHITE);
				view_render.drawPixel(cx - r - 1, cy, WHITE);
				view_render.drawPixel(cx + r + 1, cy, WHITE);
			} else {
				view_render.fillRect(cx - 1, cy - 1, 3, 3, WHITE);
			}
		}
	}
}
// Draw all active player and enemy bullets
static void draw_bullets() {
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (g_bullets[i].active) {
			if (g_bullets[i].is_enemy) view_render.fillRect(g_bullets[i].x, g_bullets[i].y, 2, 4, WHITE);
			else view_render.drawLine(g_bullets[i].x, g_bullets[i].y, g_bullets[i].x, g_bullets[i].y + 3, WHITE);
		}
	}
}
// Render Main Gameplay UI (player ship, UI bar, entities)
static void game_shooter_playing_display() {
	if (g_player_blink % 2 == 0) {
		view_render.drawBitmap(g_player_x, 54, icon_player, 8, 8, WHITE);
	}
	
	draw_enemies();
	draw_explosions();
	draw_bullets();
	
	// Draw Top UI Bar
	view_render.fillRect(0, 0, 128, 11, BLACK);
	view_render.drawLine(0, 10, 127, 10, WHITE);
	view_render.setTextSize(1);
	
	view_render.setCursor(2, 2);
	view_render.print("S:");
	char temp_sc[12];
	xsprintf(temp_sc, "%u", (unsigned int)g_score);
	view_render.print(temp_sc);
	
	view_render.setCursor(50, 2);
	view_render.print("Lv:");
	char temp_lv[12];
	xsprintf(temp_lv, "%u", (unsigned int)g_stage);
	view_render.print(temp_lv);
	
	for (int i = 0; i < g_lives; i++) {
		view_render.drawBitmap(128 - 10 - i * 10, 1, icon_heart, 8, 8, WHITE);
	}
	
	if (g_transition_timer > 0) {
		view_render.setCursor(40, 28);
		view_render.print("STAGE ");
		view_render.print(g_stage);
	}
}
// Render Game Over screen with exploding ship animation
static void game_shooter_gameover_display() {
	// 1. Exploding Spaceship (Particles)
	if (g_gameover_anim_frame <= 20) {
		int r = g_gameover_anim_frame / 2; // radius of explosion expands 0 to 10
		int cx = g_player_x + 4;
		int cy = 58;
		
		// Draw 8 flying particles
		view_render.drawPixel(cx - r, cy - r, WHITE);
		view_render.drawPixel(cx + r, cy - r, WHITE);
		view_render.drawPixel(cx - r, cy + r, WHITE);
		view_render.drawPixel(cx + r, cy + r, WHITE);
		view_render.drawPixel(cx, cy - r - 2, WHITE);
		view_render.drawPixel(cx, cy + r + 2, WHITE);
		view_render.drawPixel(cx - r - 2, cy, WHITE);
		view_render.drawPixel(cx + r + 2, cy, WHITE);
		
		// Fading center core
		if (g_gameover_anim_frame < 10) {
			view_render.drawPixel(cx, cy, WHITE);
		}
	}

	// 2. GAME OVER Text
	view_render.setTextSize(2);
	int go_y = -16 + (g_gameover_anim_frame * 2);
	if (go_y > 8) go_y = 8;
	view_render.setCursor(CENTER_X(9, 12), go_y);
	view_render.print("GAME OVER");
	
	// 3. Stats (Appears after text settles)
	view_render.setTextSize(1);
	if (g_gameover_anim_frame > 40) {
		char score_str[16];
		xsprintf(score_str, "SCORE: %u", (unsigned int)g_score);
		int score_w = strlen(score_str) * 6;
		view_render.setCursor((128 - score_w) / 2, 28);
		view_render.print(score_str);
		
		char stage_str[16];
		xsprintf(stage_str, "STAGE: %u", (unsigned int)g_stage);
		int stage_w = strlen(stage_str) * 6;
		view_render.setCursor((128 - stage_w) / 2, 38);
		view_render.print(stage_str);
	}
	
	// 4. Press MODE (Blinking text effect)
	if (g_gameover_anim_frame > 60 && ((g_gameover_anim_frame / 5) % 2 == 0)) {
		view_render.setCursor(CENTER_X(18, 6), 52); // (128 - 18 * 6) / 2 = 10
		view_render.print("Press MODE to next");
	}
}
// Render New High Score congratulations screen
static void game_shooter_new_highscore_display() {
	view_render.setTextSize(1);
	view_render.setCursor(CENTER_X(15, 6), 20);
	view_render.print("NEW HIGH SCORE!");
	
	view_render.setCursor(44, 34);
	view_render.print("TOP ");
	char temp[4];
	xsprintf(temp, "%u", (unsigned int)g_new_high_score_rank);
	view_render.print(temp);
}
// Render standard Score screen after death
static void game_shooter_score_display() {
	view_render.setTextSize(1);
	view_render.setCursor(CENTER_X(11, 6), 14);
	view_render.print("YOUR SCORE:");
	view_render.setTextSize(2);
	char temp_final[12];
	xsprintf(temp_final, "%u", (unsigned int)g_score);
	int16_t w = strlen(temp_final) * 12;
	view_render.setCursor((128 - w) / 2, 28);
	view_render.print(temp_final);
	
	view_render.drawBitmap(30, 48, icon_play, 8, 8, WHITE);
	view_render.drawBitmap(60, 48, icon_trophy, 8, 8, WHITE);
	view_render.drawBitmap(90, 48, icon_menu, 8, 8, WHITE);
	
	uint8_t selected_x = 30 + (g_show_score_selected * 30);
	view_render.fillRect(selected_x, 58, 8, 2, WHITE);
}
// Main UI rendering dispatcher based on current game state
void view_scr_game_ui() {
	// Clear the RAM buffer before rendering a new frame (Double Buffering)
	view_render.clear();
	view_render.setTextColor(WHITE);
	switch (g_game_state) {
		case GAME_STATE_MENU: game_shooter_menu_display(); break;
		case GAME_STATE_SETTING: game_shooter_setting_display(); break;
		case GAME_STATE_HIGH_SCORE: game_shooter_highscore_display(); break;
		case GAME_STATE_PLAYING: game_shooter_playing_display(); break;
		case GAME_STATE_GAMEOVER: game_shooter_gameover_display(); break;
		case GAME_STATE_NEW_HIGH_SCORE: game_shooter_new_highscore_display(); break;
		case GAME_STATE_SHOW_SCORE: game_shooter_score_display(); break;
	}
}


