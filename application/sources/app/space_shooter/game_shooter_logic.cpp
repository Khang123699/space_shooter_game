#include "game_shooter.h"
#include "task_list.h"
#include "timer.h"
#include "buzzer.h"
#include <string.h>
#include <stdlib.h>
#include "app_dbg.h"
#include "game_save.h"
#include "game_bitmaps.h"

// Enemy Spawn Grid & Configuration Constants
#define SPAWN_GRID_ROWS       3
#define SPAWN_GRID_COLS       6
#define SPAWN_START_X         8
#define SPAWN_OFFSET_X        20
#define SPAWN_START_Y         10
#define SPAWN_OFFSET_Y        12
#define SPAWN_BASE_CHANCE     45
#define SPAWN_DIFF_MULTIPLIER 10
#define BOSS_SPAWN_X          56
#define BOSS_SPAWN_Y          16

// Global UI navigation
uint8_t g_menu_selected = 0;
uint8_t g_setting_selected = 0;
uint8_t g_score_selected = 0;
uint8_t g_show_score_selected = 0;
const char* g_encouragement_text = "Good Job!";

// Exported global variables for UI and Logic
game_state_t g_game_state = GAME_STATE_MENU;
int16_t g_player_x = 60;
uint8_t g_player_blink = 0;
bool g_render_pending = false;
uint32_t g_score = 0;
uint8_t g_lives = 3;

static uint8_t g_shoot_cooldown = 0;
uint16_t g_tick_count = 0;

enemy_t g_enemies[MAX_ENEMIES];
bullet_t g_bullets[MAX_BULLETS];
explosion_t g_explosions[MAX_EXPLOSIONS];
uint8_t g_stage = 1;
int8_t g_transition_timer = 0;

static int8_t enemy_dir = 1;
static int enemy_move_ticks = 0;

// Standard AABB collision check
static bool check_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

// Pixel-perfect collision for player ship
static bool check_player_pixel_collision(int bullet_x, int bullet_y) {
    if (!check_collision(bullet_x, bullet_y, 2, 4, g_player_x, 54, 8, 8)) return false;
    
    int start_x = bullet_x - g_player_x;
    int end_x = start_x + 2; 
    int start_y = bullet_y - 54;
    int end_y = start_y + 4; 
    
    if (start_x < 0) start_x = 0; 
    if (end_x > 8) end_x = 8;
    if (start_y < 0) start_y = 0; 
    if (end_y > 8) end_y = 8;
    
    for (int y = start_y; y < end_y; y++) {
        uint8_t row_pixels = icon_player[y];
        for (int x = start_x; x < end_x; x++) {
            if (row_pixels & (1 << (7 - x))) {
                return true; 
            }
        }
    }
    return false;
}
// Spawn enemies for the current stage (normal grid or boss)
static void spawn_enemies() {
	if (g_stage % 5 == 0) {
		g_enemies[0].active = true;
		g_enemies[0].type = 4;
		g_enemies[0].x = BOSS_SPAWN_X;
		g_enemies[0].y = BOSS_SPAWN_Y;
		g_enemies[0].hp = 5 + (g_stage / 5) * 5;
		g_enemies[0].blink_timer = 0;
		for (int i = 1; i < MAX_ENEMIES; i++) g_enemies[i].active = false;
		return;
	}
	
	// Normal grid spawn (3x6 Grid)
	// Base chance to spawn at each cell + scaling based on difficulty
	int spawn_chance = SPAWN_BASE_CHANCE + (g_game_data.difficulty * SPAWN_DIFF_MULTIPLIER);
	int e = 0;
	
	for (int i = 0; i < MAX_ENEMIES; i++) g_enemies[i].active = false;
	
	for (int row = 0; row < SPAWN_GRID_ROWS; row++) {
		for (int col = 0; col < SPAWN_GRID_COLS; col++) {
			if (e >= MAX_ENEMIES) break;
			
			if ((rand() % 100) < spawn_chance) {
				g_enemies[e].active = true;
				g_enemies[e].type = 1 + (rand() % 3); // random type 1, 2, or 3
				g_enemies[e].hp = g_enemies[e].type;
				g_enemies[e].blink_timer = 0;
				g_enemies[e].x = SPAWN_START_X + col * SPAWN_OFFSET_X;
				g_enemies[e].y = SPAWN_START_Y + row * SPAWN_OFFSET_Y;
				e++;
			}
		}
	}
	
	// Fallback to avoid empty stage
	if (e == 0) {
		g_enemies[0].active = true;
		g_enemies[0].type = 1 + (rand() % 3);
		g_enemies[0].hp = g_enemies[0].type;
		g_enemies[0].blink_timer = 0;
		g_enemies[0].x = 56;
		g_enemies[0].y = 12;
	}
}
// Initialize or reset the game state variables for a new session
void game_logic_init() {
	g_player_x = 60;
	g_score = 0;
	g_lives = 3;
	g_stage = 1;
	g_transition_timer = 0;
	enemy_dir = 1;
	memset(g_bullets, 0, sizeof(g_bullets));
	memset(g_explosions, 0, sizeof(g_explosions));
	spawn_enemies();
}
// Move the player horizontally within screen boundaries
void game_player_move(int8_t dir) {
	g_player_x += dir;
	if (g_player_x < 0) g_player_x = 0;
	if (g_player_x > 120) g_player_x = 120;
}
// Shoot a player bullet if cooldown allows and slots are available
void game_player_shoot() {
	if (g_shoot_cooldown > 0) return;
	
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (!g_bullets[i].active) {
			g_bullets[i].active = true;
			g_bullets[i].is_enemy = false;
			g_bullets[i].x = g_player_x + 4;
			g_bullets[i].y = 52;
			g_bullets[i].vx = 0;
			g_shoot_cooldown = 8; 
			break;
		}
	}
}
// Main update loop for game logic: moves bullets, checks collisions, moves enemies
void game_logic_update() {
	if (g_player_blink > 0) g_player_blink--;
	
	// Update enemy blink timers
	for (int e = 0; e < MAX_ENEMIES; e++) {
		if (g_enemies[e].active && g_enemies[e].blink_timer > 0) {
			g_enemies[e].blink_timer--;
		}
	}
	
	if (g_shoot_cooldown > 0) g_shoot_cooldown--;
	g_tick_count++;

	// 1. Check collisions FIRST, then update movement
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (g_bullets[i].active) {
			
			// --- A. COLLISION CHECK ---
			if (g_bullets[i].is_enemy) {
				// Hit player: Check Pixel-Perfect collision
				if (g_player_blink == 0 && check_player_pixel_collision(g_bullets[i].x, g_bullets[i].y)) {
					g_bullets[i].active = false;
					g_lives--;
					g_player_blink = 45;
					if(g_game_data.sound_en) BUZZER_PlaySound(BUZZER_SOUND_3BEEP);
					continue; // Skip movement if destroyed
				}
			} else {
				for (int e = 0; e < MAX_ENEMIES; e++) {
					if (g_enemies[e].active) {
						// Determine dynamic bounding box based on enemy type (Boss is 16x16, normal is 8x8)
						int ew = (g_enemies[e].type == 4) ? 16 : 8;
						int eh = (g_enemies[e].type == 4) ? 16 : 8;
						
						// Check AABB collision between bullet and enemy
						if (check_collision(g_bullets[i].x, g_bullets[i].y, 1, 4, g_enemies[e].x, g_enemies[e].y, ew, eh)) {
							g_bullets[i].active = false;
							g_enemies[e].hp--;
							g_enemies[e].blink_timer = 30;
							if(g_game_data.sound_en) BUZZER_PlaySound(BUZZER_SOUND_BANG);
							if (g_enemies[e].hp <= 0) {
								g_enemies[e].active = false;
								g_score += (g_enemies[e].type == 4) ? 100 : 10;
								
								// Spawn explosion
								for (int ex = 0; ex < MAX_EXPLOSIONS; ex++) {
									if (!g_explosions[ex].active) {
										g_explosions[ex].active = true;
										g_explosions[ex].x = g_enemies[e].x + ew/2 - 4;
										g_explosions[ex].y = g_enemies[e].y + eh/2 - 4;
										g_explosions[ex].timer = 5;
										break;
									}
								}
							}
							break; // Stop checking other enemies for this bullet
						}
					}
				}
				if (!g_bullets[i].active) continue; // Skip movement if destroyed
			}

			// --- B. MOVEMENT UPDATE ---
			g_bullets[i].x += g_bullets[i].vx;
			if (g_bullets[i].is_enemy) {
				int drop = 1;
				if (g_game_data.difficulty == 2) drop = 2;
				
				bool move_enemy_bullet = (g_game_data.difficulty > 0) || (g_tick_count % 2 == 0);
				
				if (move_enemy_bullet) {
					g_bullets[i].y += drop;
					if (g_bullets[i].y > 64) g_bullets[i].active = false;
				}
			} else {
				int speed = (g_tick_count % 5 < 2) ? 2 : 1; 
				g_bullets[i].y -= speed;
				if (g_bullets[i].y < 0) g_bullets[i].active = false;
			}
		}
	} 
	if (g_tick_count % 3 == 0) {
		for (int ex = 0; ex < MAX_EXPLOSIONS; ex++) {
			if (g_explosions[ex].active) {
				g_explosions[ex].timer--;
				if (g_explosions[ex].timer <= 0) g_explosions[ex].active = false;
			}
		}
	}
	
	// Enemy movement and shooting
	bool all_dead = true;
	bool hit_edge = false;
	enemy_move_ticks++;
	int move_threshold = (g_stage % 5 == 0) ? 1 : (4 - g_game_data.difficulty);
	bool do_move = (enemy_move_ticks >= move_threshold);
	
	for (int e = 0; e < MAX_ENEMIES; e++) {
		if (g_enemies[e].active) {
			all_dead = false;
			int ew = (g_enemies[e].type == 4) ? 16 : 8;
			if (do_move) {
				g_enemies[e].x += enemy_dir;
				if (g_enemies[e].x <= 0 || g_enemies[e].x + ew >= 128) hit_edge = true;
			}
			
			// Enemy shoot
			int shoot_chance = (g_enemies[e].type == 4) ? (9 + g_game_data.difficulty * 5) : (3 + g_game_data.difficulty);
			if (rand() % 1200 < shoot_chance) {
				if (g_enemies[e].type == 4) {
					// Triple shot burst for Boss
					int bullets_spawned = 0;
					for (int i = 0; i < MAX_BULLETS && bullets_spawned < 3; i++) {
						if (!g_bullets[i].active) {
							g_bullets[i].active = true;
							g_bullets[i].x = g_enemies[e].x + ew / 2;
							g_bullets[i].y = g_enemies[e].y + 12;
							g_bullets[i].is_enemy = true;
							
							if (bullets_spawned == 0) g_bullets[i].vx = 0;
							else if (bullets_spawned == 1) g_bullets[i].vx = -1;
							else if (bullets_spawned == 2) g_bullets[i].vx = 1;
							
							bullets_spawned++;
						}
					}
				} else {
					for (int i = 0; i < MAX_BULLETS; i++) {
						if (!g_bullets[i].active) {
							g_bullets[i].active = true;
							g_bullets[i].x = g_enemies[e].x + ew / 2;
							g_bullets[i].y = g_enemies[e].y + 8;
							g_bullets[i].is_enemy = true;
							g_bullets[i].vx = 0;
							break;
						}
					}
				}
			}
			
			// Collision: Enemy body hits player (AABB) or touches bottom of screen
			int eh = (g_enemies[e].type == 4) ? 16 : 8;
			bool hit_player = (g_player_blink == 0 &&
							   g_enemies[e].x < g_player_x + 8 && g_enemies[e].x + ew > g_player_x &&
							   g_enemies[e].y < 54 + 8 && g_enemies[e].y + eh > 54);
			
			if (g_enemies[e].y > 60 || hit_player) {
				g_lives--;
				g_enemies[e].active = false;
				g_player_blink = 45;
				if (hit_player) {
					for (int ex = 0; ex < MAX_EXPLOSIONS; ex++) {
						if (!g_explosions[ex].active) {
							g_explosions[ex].x = g_player_x;
							g_explosions[ex].y = 54;
							g_explosions[ex].timer = 5;
							g_explosions[ex].active = true;
							break;
						}
					}
				}
				if(g_game_data.sound_en) BUZZER_PlaySound(BUZZER_SOUND_3BEEP);
			}
		}
	}
	
	if (do_move) {
		enemy_move_ticks = 0;
		if (hit_edge) {
			enemy_dir = -enemy_dir;
			static uint8_t edge_hit_count = 0;
			edge_hit_count++;
			
			for (int e = 0; e < MAX_ENEMIES; e++) {
				if (g_enemies[e].active) {
					g_enemies[e].x += enemy_dir;
					if (edge_hit_count >= 2) {
						g_enemies[e].y += 1;
					}
				}
			}
			
			if (edge_hit_count >= 2) edge_hit_count = 0;
		}
	}
	
	if (all_dead && g_transition_timer == 0) {
		g_stage++;
		g_transition_timer = 120;
	}
	
	if (g_transition_timer > 0) {
		g_transition_timer--;
		if (g_transition_timer == 0) spawn_enemies();
	}
	
	// 3. Game Over check
	if (g_lives <= 0) {
		// Stop logic timer to pause the game world
		timer_remove_attr(AC_TASK_GAME_SHOOTER_ID, AC_GAME_UPDATE_TICK);
		// Signal UI task to transition to Game Over screen
		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_GAME_OVER_NEXT);
	}
	
	// Force a redraw of the UI frame with new positions if not already rendering
	if (!g_render_pending) {
		g_render_pending = true;
		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_RENDER_SCREEN);
	}
}
