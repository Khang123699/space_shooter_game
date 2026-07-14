CFLAGS		+= -I./sources/app/screens
CPPFLAGS	+= -I./sources/app/screens

VPATH += sources/app/screens


# CPP source files
SOURCES_CPP += sources/app/screens/scr_game_ui.cpp
SOURCES_CPP += sources/app/screens/scr_game_ui_handle.cpp
SOURCES_CPP += sources/app/screens/scr_game_ui_menu.cpp
SOURCES_CPP += sources/app/screens/scr_game_ui_menu_handle.cpp
SOURCES_CPP += sources/app/screens/scr_game_ui_play.cpp
SOURCES_CPP += sources/app/screens/scr_game_ui_play_handle.cpp
SOURCES_CPP += sources/app/screens/scr_startup.cpp
SOURCES_CPP += sources/app/screens/scr_idle.cpp
