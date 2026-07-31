#include "game_shooter_render.h"
#include "game_shooter_player_task.h" // For g_tick_count
#include "app.h"
#include "task_list.h"
#include <stdlib.h>

star_t g_stars[MAX_STARS];

// Initialize stars for parallax background
void game_background_init() {
	for (int i = 0; i < MAX_STARS; i++) {
		g_stars[i].x = rand() % 128;
		g_stars[i].y = rand() % 64;
		g_stars[i].speed = (rand() % 2) + 1; // Speed 1 or 2
	}
}

// Update star positions during gameplay loop
void game_background_update() {
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
void game_shooter_update_stars() {
	if (g_stars[0].speed == 0) game_background_init();
	g_tick_count++;
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
