CFLAGS      += -I./sources/app/space_shooter
CPPFLAGS    += -I./sources/app/space_shooter
VPATH       += sources/app/space_shooter

SOURCES_CPP += sources/app/space_shooter/game_shooter_logic.cpp
SOURCES_CPP += sources/app/space_shooter/game_shooter_player.cpp
SOURCES_CPP += sources/app/space_shooter/game_shooter_enemy.cpp
SOURCES_CPP += sources/app/space_shooter/game_shooter_enemy_spawn.cpp
SOURCES_CPP += sources/app/space_shooter/game_shooter_boss.cpp
SOURCES_CPP += sources/app/space_shooter/game_shooter_carrier.cpp
SOURCES_CPP += sources/app/space_shooter/game_shooter_bullets.cpp
SOURCES_CPP += sources/app/space_shooter/game_shooter_physics.cpp
SOURCES_CPP += sources/app/space_shooter/game_shooter_stage.cpp
SOURCES_CPP += sources/app/space_shooter/game_shooter_background.cpp
SOURCES_CPP += sources/app/space_shooter/game_shooter_task.cpp
SOURCES_CPP += sources/app/space_shooter/game_save.cpp
