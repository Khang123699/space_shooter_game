#include "scr_startup.h"
#include <string.h>

static void view_scr_startup();

static uint8_t startup_state = 0;

view_dynamic_t dyn_view_startup = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_startup
};

view_screen_t scr_startup = {
	&dyn_view_startup,
	ITEM_NULL,
	ITEM_NULL,

	.focus_item = 0,
};

void view_scr_startup() {
	view_render.clear();
	if (startup_state == 0) {
		#define AK_LOGO_AXIS_X		(23)
		#define AK_LOGO_TEXT		(AK_LOGO_AXIS_X + 4)
		/* ak logo */
		view_render.setTextSize(1);
		view_render.setTextColor(WHITE);
		view_render.setCursor(AK_LOGO_AXIS_X, 3);
		view_render.print("   __    _  _ ");
		view_render.setCursor(AK_LOGO_AXIS_X, 10);
		view_render.print("  /__\\  ( )/ )");
		view_render.setCursor(AK_LOGO_AXIS_X, 20);
		view_render.print(" /(__)\\ (   (");
		view_render.setCursor(AK_LOGO_AXIS_X, 30);
		view_render.print("(__)(__)(_)\\_)");
		view_render.setCursor(AK_LOGO_TEXT, 42);
		view_render.print("Active Kernel");
	} else {
		// "Developed by Khang123699"
		view_render.setTextSize(1);
		view_render.setTextColor(WHITE);
		
		const char* str1 = "Developed by";
		const char* str2 = "Khang123699";
		
		// Centering text (OLED is 128 pixels wide)
		// Font width is 6 pixels per character
		int x1 = (128 - strlen(str1) * 6) / 2;
		int x2 = (128 - strlen(str2) * 6) / 2;
		
		view_render.setCursor(x1, 20);
		view_render.print(str1);
		view_render.setCursor(x2, 35);
		view_render.print(str2);
	}
}

void scr_startup_handle(ak_msg_t *msg) {
	switch (msg->sig) {
	case AC_DISPLAY_INITIAL: {
		APP_DBG_SIG("AC_DISPLAY_INITIAL\n");
		startup_state = 0;
		view_render.initialize();
		view_render_display_on();
		view_render_screen(&scr_startup);
		timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_LOGO, AC_DISPLAY_STARTUP_INTERVAL, TIMER_ONE_SHOT);
	} break;

	case AC_DISPLAY_BUTON_MODE_PRESSED: {
		APP_DBG_SIG("AC_DISPLAY_BUTON_MODE_PRESSED\n");
		timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_LOGO);
		SCREEN_TRAN(scr_game_ui_handle, &scr_game_ui);
	} break;

	case AC_DISPLAY_SHOW_LOGO: {
		APP_DBG_SIG("AC_DISPLAY_SHOW_LOGO\n");
		if (startup_state == 0) {
			startup_state = 1;
			view_render_screen(&scr_startup);
			timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_LOGO, AC_DISPLAY_STARTUP_INTERVAL, TIMER_ONE_SHOT);
		} else {
			SCREEN_TRAN(scr_game_ui_handle, &scr_game_ui);
		}
	} break;

	case AC_DISPLAY_SHOW_IDLE: {
		APP_DBG_SIG("AC_DISPLAY_SHOW_IDLE\n");
		SCREEN_TRAN(scr_game_ui_handle, &scr_game_ui);
	} break;

	default:
		break;
	}
}
