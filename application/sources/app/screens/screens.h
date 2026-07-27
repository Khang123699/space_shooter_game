#ifndef __SCREENS_H__
#define __SCREENS_H__

#include "fsm.h"
#include "port.h"
#include "message.h"
#include "timer.h"

#include "sys_ctrl.h"
#include "sys_dbg.h"

#include "app.h"
#include "app_dbg.h"
#include "task_list.h"
#include "task_display.h"
#include "view_render.h"

#include "buzzer.h"

#include <math.h>
#include <vector>


// scr_game_title
extern view_dynamic_t dyn_view_game_title;
extern view_screen_t scr_game_title;
extern void scr_game_title_handle(ak_msg_t* msg);

// scr_game_menu
extern view_dynamic_t dyn_view_game_menu;
extern view_screen_t scr_game_menu;
extern void scr_game_menu_handle(ak_msg_t* msg);

// scr_game_setting
extern view_dynamic_t dyn_view_game_setting;
extern view_screen_t scr_game_setting;
extern void scr_game_setting_handle(ak_msg_t* msg);

// scr_game_highscore
extern view_dynamic_t dyn_view_game_highscore;
extern view_screen_t scr_game_highscore;
extern void scr_game_highscore_handle(ak_msg_t* msg);

// scr_game_play
extern view_dynamic_t dyn_view_game_play;
extern view_screen_t scr_game_play;
extern void scr_game_play_handle(ak_msg_t* msg);

// scr_game_gameover
extern view_dynamic_t dyn_view_game_gameover;
extern view_screen_t scr_game_gameover;
extern void scr_game_gameover_handle(ak_msg_t* msg);

// scr_game_showscore
extern view_dynamic_t dyn_view_game_showscore;
extern view_screen_t scr_game_showscore;
extern void scr_game_showscore_handle(ak_msg_t* msg);


// scr_startup
extern view_dynamic_t dyn_view_startup;
extern view_screen_t scr_startup;
extern void scr_startup_handle(ak_msg_t* msg);

// scr_idle
extern view_dynamic_t dyn_view_idle;
extern view_screen_t scr_idle;
extern void scr_idle_handle(ak_msg_t* msg);


#endif //__SCREENS_H__
