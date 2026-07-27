#include "game_shooter.h"
#include <stdlib.h>

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
	g_tick_count++;
	for (int i = 0; i < MAX_STARS; i++) {
		g_stars[i].y += g_stars[i].speed;
		if (g_stars[i].y >= 64) {
			g_stars[i].y = 0;
			g_stars[i].x = rand() % 128;
			g_stars[i].speed = (rand() % 2) + 1;
		}
	}
}
