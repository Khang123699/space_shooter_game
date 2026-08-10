#include "game_shooter_powerup.h"
#include "game_shooter_enemy_task.h"
#include "game_shooter_player_task.h"
#include "game_shooter_bullet_task.h"
#include "game_save.h"
#include "buzzer.h"
#include "timer.h"
#include "app.h"
#include "task_list.h"
#include "app_dbg.h"

// Update powerup item positions
static void game_powerups_update() {
	for (int i = 0; i < MAX_POWERUPS; i++) {
		if (g_powerups[i].active) {
			if (game_get_tick_count() % 2 == 0) { // Fall slowly (1px per 2 ticks)
				g_powerups[i].y++;
			}
			if (g_powerups[i].y > 64) { // Exceeded screen height
				g_powerups[i].active = false;
			}
		}
	}
}

// Check collisions between player ship and powerup items
static void update_powerup_collisions() {
	int player_x = game_get_player_x();
	for (int p = 0; p < MAX_POWERUPS; p++) {
		if (!g_powerups[p].active) continue;
		
		if (game_check_collision(g_powerups[p].x, g_powerups[p].y, 8, 8, player_x, 54, 8, 8)) {
			g_powerups[p].active = false;
			if (g_game_setting.sound_en) BUZZER_PlaySound(BUZZER_SOUND_BANG);
			
			game_powerup_msg_t pmsg = {(uint8_t)p, g_powerups[p].type};
			task_post_dynamic_msg(AC_TASK_GAME_PLAYER_ID, AC_GAME_POWERUP_PICKUP, (uint8_t*)&pmsg, sizeof(pmsg));
			
			// Nuke powerup instantly damages all enemies
			switch (g_powerups[p].type) {
				case POWERUP_TYPE_NUKE:
					for (int e = 0; e < MAX_ENEMIES; e++) {
						if (g_enemies[e].active) {
							g_enemies[e].hp--;
							g_enemies[e].blink_timer = 22;
							
							if (g_enemies[e].hp <= 0) {
								game_enemy_kill(e); // Kills enemy and drops score
							}
						}
					}
					break;
			}
		}
	}
}

void game_powerup_handle(ak_msg_t* msg) {
	switch (msg->sig) {
		case AC_GAME_START_REQ:
		{
			APP_DBG_SIG("AC_GAME_POWERUP_START_REQ\n");
			for (int i = 0; i < MAX_POWERUPS; i++) g_powerups[i].active = false;
		}
		break;
			
		case AC_GAME_UPDATE_TICK:
		{
			game_powerups_update();
			update_powerup_collisions();
		}
		break;
	}
}
