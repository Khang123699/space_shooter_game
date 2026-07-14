#ifndef __SCR_GAME_UI_H__
#define __SCR_GAME_UI_H__

#include "screens.h"

extern uint8_t g_new_high_score_rank;
extern uint8_t g_gameover_anim_frame;

// View Prototypes
void game_shooter_menu_display();
void game_shooter_setting_display();
void game_shooter_highscore_display();
void game_shooter_playing_display();
void game_shooter_gameover_display();
void game_shooter_score_display();

// Handle Prototypes
void handle_ui_menu_input(ak_msg_t *msg);
void handle_ui_setting_input(ak_msg_t *msg);
void handle_ui_highscore_input(ak_msg_t *msg);
void handle_ui_playing_input(ak_msg_t *msg);
void handle_ui_gameover_input(ak_msg_t *msg);
void handle_ui_showscore_input(ak_msg_t *msg);

#endif //__SCR_GAME_UI_H__
