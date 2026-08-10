#include "game_shooter_render.h"
#include "game_shooter_player_task.h"
#include "app.h"
#include "task_list.h"
#include <stdlib.h>
#include "app_dbg.h"

static star_t g_stars[MAX_STARS];
const star_t* game_get_stars() { return g_stars; }

// Initialize stars for parallax background
static void game_background_init() {
	for (int i = 0; i < MAX_STARS; i++) {
		g_stars[i].x = rand() % 128;
		g_stars[i].y = rand() % 64;
		g_stars[i].speed = (rand() % 2) + 1; // Speed 1 or 2
	}
}

// Update star positions during gameplay loop
static void game_background_update() {
	if (g_stars[0].speed == 0) game_background_init();
	for (int i = 0; i < MAX_STARS; i++) {
		g_stars[i].y += g_stars[i].speed;
		if (g_stars[i].y >= 64) {
			g_stars[i].y = 0;
			g_stars[i].x = rand() % 128;
			g_stars[i].speed = (rand() % 2) + 1;
		}
	}
}

// Update star positions during UI screens (Title, Menu, Highscore animation tick)
static void game_shooter_update_stars() {
	if (g_stars[0].speed == 0) game_background_init();
	for (int i = 0; i < MAX_STARS; i++) {
		g_stars[i].y += g_stars[i].speed;
		if (g_stars[i].y >= 64) {
			g_stars[i].y = 0;
			g_stars[i].x = rand() % 128;
			g_stars[i].speed = (rand() % 2) + 1;
		}
	}
	task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_RENDER_SCREEN);
}

void game_shooter_request_render() {
	task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_RENDER_SCREEN);
}

void game_render_handle(ak_msg_t* msg) {
	switch (msg->sig) {
		case AC_GAME_START_REQ:
			APP_DBG_SIG("AC_GAME_RENDER_START_REQ\n");
			game_background_init();
			break;
		case AC_GAME_UPDATE_TICK:
			game_background_update();
			break;
		case AC_DISPLAY_GAME_UI_ANIM_TICK:
			game_shooter_update_stars();
			break;
	}
}
