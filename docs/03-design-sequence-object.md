<h1 align="center">Game Object Sequences</h1>

This document describes the runtime sequence of each main object in Space Shooter. Space Shooter centralizes object synchronization inside `game_shooter_logic.cpp`. All objects are evaluated periodically via the `AC_GAME_UPDATE_TICK` signal.

## I. Object Summary

| Object | Data | Routine | Main responsibility |
|---|---|---|---|
| Player | `g_player_x`, `g_lives` | `game_player_move()` | Controls the user-controlled unit and tracks volatile health parameters. |
| Bullet | `g_bullets[]` | `game_bullets_update()` | Handles translation of projectiles and collision validation. |
| Enemy | `g_enemies[]` | `game_enemy_update()` | Handles movement logic, pseudo-random generation, and boss spawning. |
| Powerup | `g_powerups[]` | `game_powerups_update()` | Translates active modifiers (shield, super bullet, nuke) and handles expiration. |
| Explosion | `g_explosions[]` | `update_explosions()` | Renders transient visual particle effects at entity destruction coordinates. |
| Star | `g_stars[]` | `game_background_update()` | Background parallax rendering. |

The main Game task (`game_shooter_task.cpp`) sets up a 50ms periodic timer generating `AC_GAME_UPDATE_TICK`. On each tick, `game_logic_update()` is invoked to synchronously fan out processing to each object routine.

## II. Player Object Sequence

Player owns the coordinate `g_player_x` and state trackers (`g_lives`, `g_score`).

**Setup.** `game_logic_init()` parks the player at `x = 60` and sets `g_lives = 3`.

**Input.** The hardware button driver translates physical presses into `AC_GAME_BTN_*_PRESSED` and `AC_GAME_BTN_*_RELEASED` messages. `game_shooter_task` catches these to toggle boolean flags (`g_is_moving_left`, `g_is_moving_right`).

**Per-tick.** Smooth sliding state is handled via `g_is_moving_left` and `g_is_moving_right` flags:
- If a flag is active (button held) — calls `game_player_move()` to continuously adjust `g_player_x` and clamps to screen bounds.
- Blinking logic: `g_player_blink` is decremented each tick if `> 0`.

**Reset.** Handled inside `game_logic_init()`.

```mermaid
%%{init: {'theme':'base','themeVariables':{'fontSize':'18px','primaryColor':'#1565c0','primaryTextColor':'#ffffff','primaryBorderColor':'#0d47a1','lineColor':'#90a4ae','signalColor':'#ffc107','signalTextColor':'#ffc107','actorBkg':'#1565c0','actorBorder':'#0d47a1','actorTextColor':'#ffffff','actorLineColor':'#90caf9','noteBkgColor':'#fff59d','noteTextColor':'#000000','noteBorderColor':'#f57f17','activationBkgColor':'#66bb6a','activationBorderColor':'#2e7d32','sequenceNumberColor':'#ffffff','loopTextColor':'#ffc107','labelBoxBkgColor':'#37474f','labelBoxBorderColor':'#90a4ae','labelTextColor':'#ffffff'},'sequence':{'actorMargin':120,'messageFontSize':17,'noteFontSize':15,'actorFontSize':17,'boxMargin':15,'boxTextMargin':8,'noteMargin':12,'useMaxWidth':true}}}%%
sequenceDiagram
    autonumber
    actor Btn as Button
    participant OS as OS Timer
    participant Tsk as Game Task
    participant Lgc as Logic Module

    Note over Tsk: AC_GAME_START_REQ
    Tsk->>Lgc: game_logic_init()
    activate Lgc
    Note right of Lgc: init g_player_x = 60, g_lives = 3
    deactivate Lgc
    Note over Tsk: arm 50 ms periodic tick (AC_GAME_UPDATE_TICK)

    Btn->>OS: Hardware Interrupt
    Note right of OS: HW Button buffer updated

    loop Each AC_GAME_UPDATE_TICK
        OS->>Tsk: AC_GAME_UPDATE_TICK
        Tsk->>Lgc: game_logic_update()
        activate Lgc
        alt g_is_moving_left == true
            Note right of Lgc: game_player_move(2)<br/>g_player_x += 2<br/>clamp 0..120
        else g_is_moving_right == true
            Note right of Lgc: game_player_move(-2)<br/>g_player_x += (-2)<br/>clamp 0..120
        end
        deactivate Lgc
    end
```

<p align="center"><strong><em>Figure 1:</em></strong> Player sequence logic</p>

## III. Bullet Object Sequence

Bullet owns the `g_bullets[MAX_BULLETS]` array. It handles shooting from the MODE button.

**Setup.** `game_logic_init()` clears every slot to `0`.

**Input.** On `AC_GAME_BTN_MODE` the Game task calls `game_player_shoot()`. It picks the first free slot in `g_bullets[]`, spawns it at `(g_player_x + 4, 52)`, and sets `is_enemy = false`.

**Per-tick.** Each `AC_GAME_UPDATE_TICK` calls `game_physics_update()`:
- **Collision Check:** Player bullets are checked against active enemies. If `g_player_super_bullet_timer > 0` (Super Bullet active), each hit deals 3 damage instead of 1. Enemy bullets are checked against the player via pixel-perfect collision.
- **Movement:** Visible bullets shift (`y -= speed` for player, `y += speed` for enemies). Bullets leaving the screen (`y < 0` or `y > 64`) are marked `active = false`.

**Cross-task (Enemy).** `game_enemy_update()` may randomly spawn bullets targeting the player with `is_enemy = true`.

```mermaid
%%{init: {'theme':'base','themeVariables':{'fontSize':'18px','primaryColor':'#1565c0','primaryTextColor':'#ffffff','primaryBorderColor':'#0d47a1','lineColor':'#90a4ae','signalColor':'#ffc107','signalTextColor':'#ffc107','actorBkg':'#1565c0','actorBorder':'#0d47a1','actorTextColor':'#ffffff','actorLineColor':'#90caf9','noteBkgColor':'#fff59d','noteTextColor':'#000000','noteBorderColor':'#f57f17','activationBkgColor':'#66bb6a','activationBorderColor':'#2e7d32','sequenceNumberColor':'#ffffff','loopTextColor':'#ffc107','labelBoxBkgColor':'#37474f','labelBoxBorderColor':'#90a4ae','labelTextColor':'#ffffff'},'sequence':{'actorMargin':120,'messageFontSize':17,'noteFontSize':15,'actorFontSize':17,'boxMargin':15,'boxTextMargin':8,'noteMargin':12,'useMaxWidth':true}}}%%
sequenceDiagram
    autonumber
    actor Btn as Button
    participant Tsk as Game Task
    participant Plr as Player Routine
    participant Phy as Physics Routine

    Note over Tsk: AC_GAME_START_REQ
    Tsk->>Plr: game_logic_init()
    Note right of Plr: g_bullets[*]: active=false, x=y=0

    Note over Btn,Tsk: MODE button fires asynchronously
    Btn->>Tsk: AC_GAME_BTN_MODE
    Tsk->>Plr: game_player_shoot()
    activate Plr
    Note right of Plr: pick free slot(s)<br/>active=true, is_enemy=false<br/>g_shoot_cooldown = 8
    deactivate Plr

    loop Each AC_GAME_UPDATE_TICK
        Tsk->>Phy: game_physics_update()
        activate Phy
        Note right of Phy: Phase 1 — Collision
        loop for each active bullet
            opt Intersection detected
                Note right of Phy: active=false, spawn explosion,<br/>deduct HP/lives
            end
        end
        Note right of Phy: Phase 2 — Movement
        loop for each active bullet
            Note right of Phy: adjust y by speed<br/>if offscreen, active=false
        end
        deactivate Phy
    end
```

<p align="center"><strong><em>Figure 2:</em></strong> Bullet sequence logic</p>

## IV. Enemy Object Sequence

Enemy owns the `g_enemies[MAX_ENEMIES]` array.

**Setup.** `game_logic_init()` clears every slot, then invokes `game_enemy_spawn()` to populate the initial wave.

**Per-tick.** Each `AC_GAME_UPDATE_TICK` calls `game_enemy_update()`:
- **Movement & Abilities:** Normal enemies (Type 1–3) move horizontally in sync with `enemy_dir` at a speed that scales with `g_stage`. The Boss (Type 4) utilizes an internal state machine to execute dynamic phases (Dash Charge, Dash Down, Dash Up, Summon) and an Enrage mode when HP < 50%. The Spread Shooter (Type 5) fires 3-way bursts. The Carrier (Type 6) periodically spawns new enemies.
- **Spawn Check:** Handled by `game_stage_update()` — if all enemies are `active = false`, `g_stage` increments and starts a 90-tick (4.5s) transition timer before calling `game_enemy_spawn()`.

**Collision.** `game_physics_update()` handles AABB intersections. For enemy-player collisions, normal enemies are destroyed upon impact, while the Boss survives to prevent instant mechanical bypass. When an enemy's HP drops to 0, the system adds to `g_score`, potentially spawns a Powerup, triggers an Explosion, and marks the slot `active = false`.

```mermaid
%%{init: {'theme':'base','themeVariables':{'fontSize':'18px','primaryColor':'#1565c0','primaryTextColor':'#ffffff','primaryBorderColor':'#0d47a1','lineColor':'#90a4ae','signalColor':'#ffc107','signalTextColor':'#ffc107','actorBkg':'#1565c0','actorBorder':'#0d47a1','actorTextColor':'#ffffff','actorLineColor':'#90caf9','noteBkgColor':'#fff59d','noteTextColor':'#000000','noteBorderColor':'#f57f17','activationBkgColor':'#66bb6a','activationBorderColor':'#2e7d32','sequenceNumberColor':'#ffffff','loopTextColor':'#ffc107','labelBoxBkgColor':'#37474f','labelBoxBorderColor':'#90a4ae','labelTextColor':'#ffffff'},'sequence':{'actorMargin':120,'messageFontSize':17,'noteFontSize':15,'actorFontSize':17,'boxMargin':15,'boxTextMargin':8,'noteMargin':12,'useMaxWidth':true}}}%%
sequenceDiagram
    autonumber
    participant Tsk as Game Task
    participant Lgc as Logic Module
    participant Phy as Physics Routine
    participant Enm as Enemy Routine

    Note over Tsk: AC_GAME_START_REQ
    Tsk->>Lgc: game_logic_init()
    Lgc->>Enm: game_enemy_spawn()
    activate Enm
    Note right of Enm: initialize enemy wave based on g_stage
    deactivate Enm

    loop Each AC_GAME_UPDATE_TICK
        Tsk->>Phy: game_physics_update()
        activate Phy
        Note right of Phy: Check bullet AABB vs enemies.<br/>On HP ≤ 0: active=false, add score,<br/>roll powerup chance, trigger explosion.
        deactivate Phy

        Tsk->>Enm: game_enemy_update()
        activate Enm
        loop for each active enemy
            Note right of Enm: Apply movement vectors<br/>Probabilistic projectile generation (is_enemy=true)
        end
        deactivate Enm
        
        opt all_dead == true
            Tsk->>Lgc: g_stage++
            Note right of Lgc: wait 90 ticks (4.5s) transition
            Lgc->>Enm: game_enemy_spawn()
        end
    end
```

<p align="center"><strong><em>Figure 3:</em></strong> Enemy sequence logic</p>

## V. Per-Tick Execution Order

On every `AC_GAME_UPDATE_TICK` (50ms interval), `game_shooter_task` invokes `game_logic_update()` which processes logic in the following sequence:

1. `update_player_sliding_and_timers()`: Updates smooth movement flags and countdown active timers (blink, shield, cooldown).
2. `game_physics_update()`: Checks AABB collisions, handles bullet movement and damage.
3. `game_enemy_update()`: Handles enemy translation and hostile bullet generation.
4. `game_powerups_update()`: Translates active powerups.
5. `game_stage_update()`: Wave validation, advances stage if all enemies are dead.
6. `game_check_game_over()`: Transitions state if the player runs out of lives.
7. Post `AC_DISPLAY_RENDER_SCREEN` to the UI task to update the interface.

## VI. Code References

| Object | Source file | Header file |
|---|---|---|
| Main Task | `game_shooter_task.cpp` | `game_shooter.h` |
| Logic & Core Loop | `game_shooter_logic.cpp` | `game_shooter.h` |
| Player (Move/Shoot) | `game_shooter_player.cpp` | `game_shooter.h` |
| Bullet & Collision | `game_shooter_bullets.cpp` | `game_shooter.h` |
| Physics Orchestrator | `game_shooter_physics.cpp` | `game_shooter.h` |
| Background (Parallax) | `game_shooter_background.cpp` | `game_shooter.h` |
| Enemy (Movement/AI) | `game_shooter_enemy.cpp` | `game_shooter.h` |
| Enemy Spawn | `game_shooter_enemy_spawn.cpp` | `game_shooter.h` |
| Boss | `game_shooter_boss.cpp` | `game_shooter.h` |
| Carrier | `game_shooter_carrier.cpp` | `game_shooter.h` |
| Stage & Powerup | `game_shooter_stage.cpp` | `game_shooter.h` |
| Save Game | `game_save.cpp` | `game_save.h` |
| UI & Screens | `screens/scr_game_*.cpp` | `screens/screens.h` |

> **Note:** All files above are located in `application/sources/app/space_shooter/`, except UI & Screens which are in `application/sources/app/screens/`.
