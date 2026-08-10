#include "scr_game_play.h"
#include "game_shooter_player_task.h"
#include "game_shooter_enemy_task.h"
#include "game_shooter_bullet_task.h"
#include "game_shooter_stage_task.h"
#include "game_shooter_render.h"
#include "game_save.h"
#include "view_render.h"
#include "buzzer.h"
#include "timer.h"
#include "task_list.h"
#include "xprintf.h"
#include "game_bitmaps.h"
#include <string.h>

#define CENTER_X(str_len, char_width) ((128 - (str_len) * (char_width)) / 2)

static void view_scr_game_play();

view_dynamic_t dyn_view_game_play = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_game_play
};

view_screen_t scr_game_play = {
	&dyn_view_game_play,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0,
};

static void draw_enemies() {
	const enemy_t* enemies = game_get_enemies();
	for (int e = 0; e < MAX_ENEMIES; e++) {
		if (enemies[e].active) {
			bool draw_sprite = true;
			if (enemies[e].blink_timer > 0 && enemies[e].type != 4 && (game_get_tick_count() % 4 < 2)) {
				draw_sprite = false;
			}
			
			switch (enemies[e].type) {
				case ENEMY_TYPE_BOSS:
				{
					if (draw_sprite) {
						view_render.drawBitmap(enemies[e].x, enemies[e].y, bmp_boss, 16, 16, WHITE);
					}
					int boss_cycle = game_get_stage() / 3;
					int max_hp = 10 + (boss_cycle - 1) * 5;
					int hp_width = (enemies[e].hp * 16) / max_hp;
					if (hp_width > 16) hp_width = 16;
					if (hp_width < 0) hp_width = 0;
					view_render.fillRect(enemies[e].x, enemies[e].y - 3, 16, 2, BLACK);
					view_render.fillRect(enemies[e].x, enemies[e].y - 3, hp_width, 2, WHITE);
				}
				break;
				
				case ENEMY_TYPE_SPREAD:
					if (draw_sprite) view_render.drawBitmap(enemies[e].x, enemies[e].y, bmp_enemy_spread, 16, 8, WHITE);
					break;
					
				case ENEMY_TYPE_CARRIER:
					if (draw_sprite) view_render.drawBitmap(enemies[e].x, enemies[e].y, bmp_enemy_carrier, 16, 8, WHITE);
					break;
					
				default:
				{
					const uint8_t* icon = icon_enemy1;
					if (enemies[e].type == ENEMY_TYPE_2) icon = icon_enemy2;
					else if (enemies[e].type == ENEMY_TYPE_3) icon = icon_enemy3;
					
					if (draw_sprite) view_render.drawBitmap(enemies[e].x, enemies[e].y, icon, 8, 8, WHITE);
				}
				break;
			}
		}
	}
}

static void draw_powerups() {
	const powerup_t* powerups = game_get_powerups();
	for (int p = 0; p < MAX_POWERUPS; p++) {
		if (powerups[p].active) {
			switch (powerups[p].type) {
				case POWERUP_TYPE_SUPER_BULLET:
					view_render.drawBitmap(powerups[p].x, powerups[p].y, icon_item_super, 8, 8, WHITE);
					break;
				case POWERUP_TYPE_SHIELD:
					view_render.drawBitmap(powerups[p].x, powerups[p].y, icon_item_shield, 8, 8, WHITE);
					break;
				case POWERUP_TYPE_NUKE:
					view_render.drawBitmap(powerups[p].x, powerups[p].y, icon_item_nuke, 8, 8, WHITE);
					break;
			}
		}
	}
}

static void draw_explosions() {
	const explosion_t* explosions = game_get_explosions();
	for (int ex = 0; ex < MAX_EXPLOSIONS; ex++) {
		if (explosions[ex].active) {
			int r = 5 - explosions[ex].timer;
			int cx = explosions[ex].x + 4;
			int cy = explosions[ex].y + 4;
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

static void draw_bullets() {
	const bullet_t* bullets = game_get_bullets();
	uint16_t super_timer = game_get_player_super_bullet_timer();
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (bullets[i].active) {
			if (bullets[i].is_enemy) {
				view_render.fillRect(bullets[i].x, bullets[i].y, 2, 4, WHITE);
			} else {
				if (super_timer > 0) {
					view_render.fillRect(bullets[i].x - 1, bullets[i].y, 3, 4, WHITE);
				} else {
					view_render.drawLine(bullets[i].x, bullets[i].y, bullets[i].x, bullets[i].y + 3, WHITE);
				}
			}
		}
	}
}

static void view_scr_game_play() {
	int player_x = game_get_player_x();
	uint8_t player_blink = game_get_player_blink();
	uint16_t player_shield = game_get_player_shield_timer();
	uint16_t tick_count = game_get_tick_count();
	
	for (int i = 0; i < MAX_STARS; i++) {
		view_render.drawPixel(game_get_stars()[i].x, game_get_stars()[i].y, WHITE);
	}

	if (player_blink % 2 == 0) {
		view_render.drawBitmap(player_x, 54, icon_player, 8, 8, WHITE);
		if ((tick_count / 2) % 2 == 0) {
			view_render.drawBitmap(player_x, 62, icon_flame1, 8, 8, WHITE);
		} else {
			view_render.drawBitmap(player_x, 62, icon_flame2, 8, 8, WHITE);
		}
	}
	
	if (player_shield > 0) {
		if (player_shield > 60 || (player_shield % 10 < 5)) {
			view_render.drawCircle(player_x + 3, 54 + 4, 6, WHITE);
		}
	}
	
	draw_powerups();
	draw_enemies();
	draw_explosions();
	draw_bullets();
	
	view_render.fillRect(0, 0, 128, 11, BLACK);
	view_render.drawLine(0, 10, 127, 10, WHITE);
	view_render.setTextSize(1);
	
	view_render.setCursor(0, 2);
	char temp_sc[12];
	xsprintf(temp_sc, "%u", (unsigned int)game_get_score());
	view_render.print(temp_sc);
	
	view_render.setCursor(30, 2);
	view_render.print("L");
	char temp_lv[12];
	xsprintf(temp_lv, "%u", (unsigned int)game_get_stage());
	view_render.print(temp_lv);
	
	for (int i = 0; i < game_get_lives(); i++) {
		view_render.drawBitmap(128 - 9 - i * 9, 1, icon_heart, 8, 8, WHITE);
	}
	
	int buff_x = 55;
	uint16_t player_super_timer = game_get_player_super_bullet_timer();
	if (player_super_timer > 0) {
		bool draw_gun = true;
		if (player_super_timer > 160) draw_gun = (player_super_timer % 6 < 3);
		else if (player_super_timer <= 60) draw_gun = (player_super_timer % 10 < 5);
		
		if (draw_gun) {
			view_render.drawBitmap(buff_x, 1, icon_item_super, 8, 8, WHITE);
			view_render.setCursor(buff_x + 9, 2);
			view_render.print((player_super_timer + 19) / 20); 
		}
		buff_x += 22;
	}
	
	player_shield = game_get_player_shield_timer();
	if (player_shield > 0) {
		bool draw_shld = true;
		if (player_shield > 160) draw_shld = (player_shield % 6 < 3);
		else if (player_shield <= 60) draw_shld = (player_shield % 10 < 5);
		
		if (draw_shld) {
			view_render.drawBitmap(buff_x, 1, icon_item_shield, 8, 8, WHITE);
			view_render.setCursor(buff_x + 9, 2);
			view_render.print((player_shield + 19) / 20);
		}
	}
	
	if (game_get_transition_timer() > 0) {
		view_render.setCursor(40, 28);
		view_render.print("STAGE ");
		view_render.print(game_get_stage());
	}
}

void scr_game_play_handle(ak_msg_t* msg) {
	switch (msg->sig) {
		case SCREEN_ENTRY:
			timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_IDLE_TIMEOUT);
			timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_IDLE);
			timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_GAME_UI_ANIM_TICK);
			break;

		case AC_DISPLAY_BUTTON_UP_PRESSED:
			task_post_pure_msg(AC_TASK_GAME_PLAYER_ID, AC_GAME_BTN_UP);
			break;

		case AC_DISPLAY_BUTTON_UP_RELEASED:
			task_post_pure_msg(AC_TASK_GAME_PLAYER_ID, AC_GAME_BTN_UP_RELEASED);
			break;

		case AC_DISPLAY_BUTTON_DOWN_PRESSED:
			task_post_pure_msg(AC_TASK_GAME_PLAYER_ID, AC_GAME_BTN_DOWN);
			break;

		case AC_DISPLAY_BUTTON_DOWN_RELEASED:
			task_post_pure_msg(AC_TASK_GAME_PLAYER_ID, AC_GAME_BTN_DOWN_RELEASED);
			break;

		case AC_DISPLAY_BUTTON_MODE_PRESSED:
			task_post_pure_msg(AC_TASK_GAME_PLAYER_ID, AC_GAME_BTN_MODE);
			break;

		case AC_DISPLAY_GAME_OVER_NEXT:
			game_set_new_high_score_rank(game_update_high_score(game_get_score()));
			if (game_get_new_high_score_rank() > 0 && g_game_setting.sound_en) {
				BUZZER_PlaySound(BUZZER_SOUND_HIGHSCORE);
			} else if (g_game_setting.sound_en) {
				BUZZER_PlaySound(BUZZER_SOUND_LOWSCORE);
			}
			SCREEN_TRAN(scr_game_gameover_handle, &scr_game_gameover);
			break;
	}
}
