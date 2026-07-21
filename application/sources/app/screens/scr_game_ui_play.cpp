#include "scr_game_ui.h"
#include "game_shooter.h"
#include "view_render.h"
#include "xprintf.h"
#include "game_bitmaps.h"
#include <string.h>

// Helper macro to center text horizontally on a 128px screen
#define CENTER_X(str_len, char_width) ((128 - (str_len) * (char_width)) / 2)

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
				int boss_cycle = g_stage / 3;
				int max_hp = 10 + (boss_cycle - 1) * 5;
				int hp_width = (g_enemies[e].hp * 16) / max_hp;
				if (hp_width > 16) hp_width = 16;
				if (hp_width < 0) hp_width = 0;
				view_render.fillRect(g_enemies[e].x, g_enemies[e].y - 3, 16, 2, BLACK);
				view_render.fillRect(g_enemies[e].x, g_enemies[e].y - 3, hp_width, 2, WHITE);
			} else if (g_enemies[e].type == 5) {
				if (draw_sprite) view_render.drawBitmap(g_enemies[e].x, g_enemies[e].y, bmp_enemy_spread, 16, 8, WHITE);
			} else if (g_enemies[e].type == 6) {
				if (draw_sprite) view_render.drawBitmap(g_enemies[e].x, g_enemies[e].y, bmp_enemy_carrier, 16, 8, WHITE);
			} else {
				const uint8_t* icon = icon_enemy1;
				if (g_enemies[e].type == 2) icon = icon_enemy2;
				else if (g_enemies[e].type == 3) icon = icon_enemy3;
				
				if (draw_sprite) view_render.drawBitmap(g_enemies[e].x, g_enemies[e].y, icon, 8, 8, WHITE);
			}
		}
	}
}

// Draw powerups
static void draw_powerups() {
	for (int p = 0; p < MAX_POWERUPS; p++) {
		if (g_powerups[p].active) {
			if (g_powerups[p].type == POWERUP_TYPE_SUPER_BULLET) {
				view_render.drawBitmap(g_powerups[p].x, g_powerups[p].y, icon_item_super, 8, 8, WHITE);
			} else if (g_powerups[p].type == POWERUP_TYPE_SHIELD) {
				view_render.drawBitmap(g_powerups[p].x, g_powerups[p].y, icon_item_shield, 8, 8, WHITE);
			} else if (g_powerups[p].type == POWERUP_TYPE_NUKE) {
				view_render.drawBitmap(g_powerups[p].x, g_powerups[p].y, icon_item_nuke, 8, 8, WHITE);
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
			if (g_bullets[i].is_enemy) {
				view_render.fillRect(g_bullets[i].x, g_bullets[i].y, 2, 4, WHITE);
			} else {
				if (g_player_super_bullet_timer > 0) {
					view_render.fillRect(g_bullets[i].x - 1, g_bullets[i].y, 3, 4, WHITE);
				} else {
					view_render.drawLine(g_bullets[i].x, g_bullets[i].y, g_bullets[i].x, g_bullets[i].y + 3, WHITE);
				}
			}
		}
	}
}

// Render Main Gameplay UI (player ship, UI bar, entities)
void game_shooter_playing_display() {
	// Draw Parallax Stars
	for (int i = 0; i < MAX_STARS; i++) {
		view_render.drawPixel(g_stars[i].x, g_stars[i].y, WHITE);
	}

	if (g_player_blink % 2 == 0) {
		view_render.drawBitmap(g_player_x, 54, icon_player, 8, 8, WHITE);
		// Draw exhaust flame animation
		if ((g_tick_count / 2) % 2 == 0) {
			view_render.drawBitmap(g_player_x, 62, icon_flame1, 8, 8, WHITE);
		} else {
			view_render.drawBitmap(g_player_x, 62, icon_flame2, 8, 8, WHITE);
		}
	}
	
	if (g_player_shield_timer > 0) {
		// Blinking effect when shield is about to expire (< 3 seconds remaining)
		if (g_player_shield_timer > 60 || (g_player_shield_timer % 10 < 5)) {
			view_render.drawCircle(g_player_x + 3, 54 + 4, 6, WHITE);
		}
	}
	
	draw_powerups();
	draw_enemies();
	draw_explosions();
	draw_bullets();
	
	// Draw Top UI Bar
	view_render.fillRect(0, 0, 128, 11, BLACK);
	view_render.drawLine(0, 10, 127, 10, WHITE);
	view_render.setTextSize(1);
	
	view_render.setCursor(0, 2);
	char temp_sc[12];
	xsprintf(temp_sc, "%u", (unsigned int)g_score);
	view_render.print(temp_sc);
	
	view_render.setCursor(30, 2);
	view_render.print("L");
	char temp_lv[12];
	xsprintf(temp_lv, "%u", (unsigned int)g_stage);
	view_render.print(temp_lv);
	
	for (int i = 0; i < g_lives; i++) {
		view_render.drawBitmap(128 - 9 - i * 9, 1, icon_heart, 8, 8, WHITE);
	}
	
	int buff_x = 55;
	if (g_player_super_bullet_timer > 0) {
		bool draw_gun = true;
		if (g_player_super_bullet_timer > 160) draw_gun = (g_player_super_bullet_timer % 6 < 3); // Blink fast (first 2s)
		else if (g_player_super_bullet_timer <= 60) draw_gun = (g_player_super_bullet_timer % 10 < 5); // Blink slow (last 3s)
		
		if (draw_gun) {
			view_render.drawBitmap(buff_x, 1, icon_item_super, 8, 8, WHITE);
			view_render.setCursor(buff_x + 9, 2);
			view_render.print((g_player_super_bullet_timer + 19) / 20); 
		}
		buff_x += 22;
	}
	
	if (g_player_shield_timer > 0) {
		bool draw_shld = true;
		if (g_player_shield_timer > 160) draw_shld = (g_player_shield_timer % 6 < 3);
		else if (g_player_shield_timer <= 60) draw_shld = (g_player_shield_timer % 10 < 5);
		
		if (draw_shld) {
			view_render.drawBitmap(buff_x, 1, icon_item_shield, 8, 8, WHITE);
			view_render.setCursor(buff_x + 9, 2);
			view_render.print((g_player_shield_timer + 19) / 20);
		}
	}
	
	if (g_transition_timer > 0) {
		view_render.setCursor(40, 28);
		view_render.print("STAGE ");
		view_render.print(g_stage);
	}
}

// Render Game Over screen with exploding ship animation
void game_shooter_gameover_display() {
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

// Render standard Score screen after death
void game_shooter_score_display() {
	view_render.setTextSize(1);
	if (g_new_high_score_rank > 0) {
		view_render.setCursor(CENTER_X(15, 6), 6);
		view_render.print("NEW HIGH SCORE!");
		view_render.setCursor(CENTER_X(5, 6), 16);
		view_render.print("TOP ");
		char temp[4];
		xsprintf(temp, "%u", (unsigned int)g_new_high_score_rank);
		view_render.print(temp);
	} else {
		view_render.setCursor(CENTER_X(11, 6), 14);
		view_render.print("YOUR SCORE:");
	}
	
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
	view_render.drawRect(selected_x - 3, 45, 14, 14, WHITE);
}
