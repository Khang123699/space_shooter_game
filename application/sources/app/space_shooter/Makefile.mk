CFLAGS      += -I./sources/app/space_shooter
CPPFLAGS    += -I./sources/app/space_shooter
VPATH       += sources/app/space_shooter

SOURCES_CPP += sources/app/space_shooter/game_shooter_logic.cpp
SOURCES_CPP += sources/app/space_shooter/game_shooter_player.cpp
SOURCES_CPP += sources/app/space_shooter/game_shooter_enemy.cpp
SOURCES_CPP += sources/app/space_shooter/game_shooter_physics.cpp
SOURCES_CPP += sources/app/space_shooter/game_shooter_task.cpp
SOURCES_CPP += sources/app/space_shooter/game_save.cpp
SOURCES_CPP += sources/app/space_shooter/game_bitmaps.cpp
