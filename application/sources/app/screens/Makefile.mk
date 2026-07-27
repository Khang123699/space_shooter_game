CFLAGS		+= -I./sources/app/screens
CPPFLAGS	+= -I./sources/app/screens

VPATH += sources/app/screens


# CPP source files
SOURCES_CPP += sources/app/screens/scr_game_title.cpp
SOURCES_CPP += sources/app/screens/scr_game_menu.cpp
SOURCES_CPP += sources/app/screens/scr_game_setting.cpp
SOURCES_CPP += sources/app/screens/scr_game_highscore.cpp
SOURCES_CPP += sources/app/screens/scr_game_play.cpp
SOURCES_CPP += sources/app/screens/scr_game_gameover.cpp
SOURCES_CPP += sources/app/screens/scr_game_showscore.cpp
SOURCES_CPP += sources/app/screens/scr_startup.cpp
SOURCES_CPP += sources/app/screens/scr_idle.cpp
SOURCES_CPP += sources/app/screens/game_bitmaps.cpp
