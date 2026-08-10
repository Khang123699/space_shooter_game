CFLAGS      += -I./sources/app/space_shooter
CPPFLAGS    += -I./sources/app/space_shooter
VPATH       += sources/app/space_shooter

SOURCES_CPP += sources/app/space_shooter/game_save.cpp
SOURCES_CPP += sources/app/space_shooter/game_shooter_player_task.cpp
SOURCES_CPP += sources/app/space_shooter/game_shooter_enemy_task.cpp
SOURCES_CPP += sources/app/space_shooter/game_shooter_boss.cpp
SOURCES_CPP += sources/app/space_shooter/game_shooter_powerup.cpp
SOURCES_CPP += sources/app/space_shooter/game_shooter_bullet_task.cpp
SOURCES_CPP += sources/app/space_shooter/game_shooter_stage_task.cpp
SOURCES_CPP += sources/app/space_shooter/game_shooter_render.cpp
