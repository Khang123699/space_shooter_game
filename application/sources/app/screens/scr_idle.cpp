#include "scr_idle.h"
#include "scr_game_ui.h"
#include "game_shooter.h"

#define MAX_BALL_DISPLAY (5)
#define BALL_MOVE_STEP	 (2)

class ball {
	// rand from a to b
	// (rand() % (b - a + 1)) + a
public:
	static int total;
	int id, x, y, slope, axis_x, axis_y, radius;

	ball() {
		axis_x = 1;
		axis_y = 1;
		slope  = (rand() % (31)) - 15;
		radius = (rand() % (7)) + 6;
		x	   = radius + (rand() % (LCD_WIDTH - 2 * radius));
		y	   = radius + (rand() % (LCD_HEIGHT - 2 * radius));
	}

	int distance(ball &__ball) {
		uint8_t dx, dy;
		dx = abs(x - __ball.x);
		dy = abs(y - __ball.y);
		return sqrt(dx * dx + dy * dy);
	}

	bool is_hit_to_other(ball &__ball) {
		if ((radius + __ball.radius) <= distance(__ball)) {
			return true;
		}
		else {
			return false;
		}
	}

	void moving() {
		if (axis_x > 0) {
			x = x + BALL_MOVE_STEP;
		}
		else {
			x = x - BALL_MOVE_STEP;
		}

		if (axis_y > 0) {
			y += BALL_MOVE_STEP * atan(slope);
		}
		else {
			y -= BALL_MOVE_STEP * atan(slope);
		}

		if (x > ((LCD_WIDTH - 1) - radius) || x < radius) {
			axis_x = -axis_x;
			if (x < radius) {
				x = radius;
			}
			else if (x > ((LCD_WIDTH - 1) - radius)) {
				x = (LCD_WIDTH - 1) - radius;
			}
		}

		if (y > ((LCD_HEIGHT - 1) - radius) || y < radius) {
			axis_y = -axis_y;
			if (y < radius) {
				y = radius;
			}
			else if (y > ((LCD_HEIGHT - 1) - radius)) {
				y = (LCD_HEIGHT - 1) - radius;
			}
		}
	}
};

static void view_scr_idle();

view_dynamic_t dyn_view_idle = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_idle
};

view_screen_t scr_idle = {
	&dyn_view_idle,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0,
};

ball v_idle_ball[MAX_BALL_DISPLAY];
int active_balls = 0;
int ball::total;

static void scr_idle_return_screen() {
	timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_IDLE_BALL_MOVING_UPDATE);
	if (g_game_state == GAME_STATE_PLAYING) {
		timer_set(AC_TASK_GAME_SHOOTER_ID, AC_GAME_UPDATE_TICK, 50, TIMER_PERIODIC);
	}
	timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_IDLE_TIMEOUT, 12000, TIMER_ONE_SHOT);
	SCREEN_TRAN(scr_game_ui_handle, &scr_game_ui);
}

void view_scr_idle() {
    view_render.clear(); // Important: Clear the screen explicitly
	for (int i = 0; i < active_balls; i++) {
		view_render.drawCircle(v_idle_ball[i].x, v_idle_ball[i].y, v_idle_ball[i].radius, WHITE);
	}
}

void scr_idle_handle(ak_msg_t *msg) {
	switch (msg->sig) {
	case SCREEN_ENTRY: {
		APP_DBG_SIG("SCREEN_ENTRY\n");
		if (active_balls == 0) {
            for(int i = 0; i < 5; i++) { // Spawn 5 balls by default
			    v_idle_ball[i] = ball();
			    v_idle_ball[i].id = ball::total++;
            }
            active_balls = 5;
		}

        // Set to 100ms interval for smooth but safe queueing (10 fps)
		timer_set(AC_TASK_DISPLAY_ID, \
				  AC_DISPLAY_SHOW_IDLE_BALL_MOVING_UPDATE, \
				  100, \
				  TIMER_PERIODIC);
	} break;

	case AC_DISPLAY_SHOW_IDLE_BALL_MOVING_UPDATE: {
		for (int i = 0; i < active_balls; i++) {
			v_idle_ball[i].moving();
		}
        // Force screen re-render for animation
        view_render_screen(&scr_idle);
	} break;

	case AC_DISPLAY_BUTTON_MODE_PRESSED:
	case AC_DISPLAY_BUTTON_UP_PRESSED:
	case AC_DISPLAY_BUTTON_DOWN_PRESSED: {
		APP_DBG_SIG("AC_DISPLAY_BUTTON_PRESSED\n");
		scr_idle_return_screen();
	} break;

	default:
		break;
	}
}
