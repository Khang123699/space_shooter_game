<h1 align="center">Game Object Sequences</h1>

This document describes the runtime sequence of each main object in Space Shooter. Space Shooter separates objects into independent tasks: Player, Enemy, Bullet, Stage, and a Render module. All objects are periodically evaluated via the `AC_GAME_UPDATE_TICK` signal routed through the task pipeline.

## I. Object Summary

| Task / Module | Main Data | Routine | Main responsibility |
|---|---|---|---|
| `PLAYER` | `g_player` | `update_player_sliding_and_timers()` | Controls the user-controlled unit, tracks volatile health parameters, and cooldowns. |
| `BULLET` | `g_bullets[]` | `game_physics_update()` | Handles translation of projectiles, checks collisions (AABB / pixel-perfect), and spawns explosions. |
| `ENEMY` | `g_enemies[]` | `game_enemy_update()` | Handles movement logic, enemy projectile generation, and boss spawning. Manages powerup drops. |
| `STAGE` | `g_stage` | `game_stage_update()` | Checks wave transitions (when enemies are cleared) and routes to Game Over when lives are exhausted. |
| Render Module | `g_stars[]` | `game_background_update()` | Moves background space effects (parallax) and coordinates screen rendering. (Not a separate task) |

The Stage task receives a 50ms periodic timer which dispatches `AC_GAME_UPDATE_TICK`. On each tick, it sequentially pushes the tick signal down the pipeline to the other tasks via the AKOS message queue.

## II. Player Object Sequence

The Player task owns the `g_player` struct (coordinate, lives, score, state).

**Setup.** `game_logic_init()` parks the player at `x = 60` and sets `lives = 3`.

**Input.** Hardware button interrupts route asynchronous signals directly from the Display Task to `AC_TASK_GAME_PLAYER_ID`.

**Per-tick.** Smooth sliding state is handled via `g_is_moving_left` and `g_is_moving_right` flags:
- If a flag is active (button held), continuously adjusts `g_player.x` and clamps to screen bounds.
- Blinking logic: `g_player.blink_timer` is decremented each tick if `> 0`.

```mermaid
%%{init: {'theme':'base','themeVariables':{'fontSize':'18px','primaryColor':'#1565c0','primaryTextColor':'#ffffff','primaryBorderColor':'#0d47a1','lineColor':'#90a4ae','signalColor':'#ffc107','signalTextColor':'#ffc107','actorBkg':'#1565c0','actorBorder':'#0d47a1','actorTextColor':'#ffffff','actorLineColor':'#90caf9','noteBkgColor':'#fff59d','noteTextColor':'#000000','noteBorderColor':'#f57f17','activationBkgColor':'#66bb6a','activationBorderColor':'#2e7d32','sequenceNumberColor':'#ffffff','loopTextColor':'#ffc107','labelBoxBkgColor':'#37474f','labelBoxBorderColor':'#90a4ae','labelTextColor':'#ffffff'},'sequence':{'actorMargin':120,'messageFontSize':17,'noteFontSize':15,'actorFontSize':17,'boxMargin':15,'boxTextMargin':8,'noteMargin':12,'useMaxWidth':true}}}%%
sequenceDiagram
    autonumber
    actor Btn as Button
    participant Q as AKOS Message Queue
    participant Tsk as Player Task
    
    Note over Tsk: AC_GAME_START_REQ
    Tsk->>Tsk: game_logic_init()
    Note right of Tsk: init g_player state
    Note over Tsk: Send start request to other tasks
    
    Btn->>Tsk: UI Signal (e.g., AC_GAME_BTN_UP)
    Note right of Tsk: Set flag g_is_moving_left = true
    
    loop Each AC_GAME_UPDATE_TICK
        Q-)Tsk: AC_GAME_UPDATE_TICK
        activate Tsk
        alt g_is_moving_left == true
            Note right of Tsk: game_player_move(2)<br/>g_player.x += 2<br/>clamp 0..120
        end
        deactivate Tsk
    end
```

<p align="center"><strong><em>Figure 1:</em></strong> Player sequence logic</p>

## III. Bullet Object Sequence

Bullet task owns the `g_bullets[MAX_BULLETS]` array.

**Shooting.** On `AC_GAME_BTN_MODE`, the Player Task calls `game_player_shoot()` to spawn a new bullet. The Enemy Task can also spawn enemy bullets via `game_boss_shoot()`.

**Per-tick.** Upon receiving `AC_GAME_UPDATE_TICK` from the queue, it calls `game_physics_update()`:
- **Collision Check:** Player bullets are checked against active enemies. Enemy bullets are checked against the player (pixel-perfect). If hit, triggers an Explosion.
- **Movement:** Visible bullets translate (`y -= speed` or `y += speed`).

```mermaid
%%{init: {'theme':'base','themeVariables':{'fontSize':'18px','primaryColor':'#1565c0','primaryTextColor':'#ffffff','primaryBorderColor':'#0d47a1','lineColor':'#90a4ae','signalColor':'#ffc107','signalTextColor':'#ffc107','actorBkg':'#1565c0','actorBorder':'#0d47a1','actorTextColor':'#ffffff','actorLineColor':'#90caf9','noteBkgColor':'#fff59d','noteTextColor':'#000000','noteBorderColor':'#f57f17','activationBkgColor':'#66bb6a','activationBorderColor':'#2e7d32','sequenceNumberColor':'#ffffff','loopTextColor':'#ffc107','labelBoxBkgColor':'#37474f','labelBoxBorderColor':'#90a4ae','labelTextColor':'#ffffff'},'sequence':{'actorMargin':120,'messageFontSize':17,'noteFontSize':15,'actorFontSize':17,'boxMargin':15,'boxTextMargin':8,'noteMargin':12,'useMaxWidth':true}}}%%
sequenceDiagram
    autonumber
    participant Q as AKOS Message Queue
    participant Plr as Player Task
    participant Bul as Bullet Task
    
    Plr->>Plr: Receive AC_GAME_BTN_MODE
    Note right of Plr: game_player_shoot(): spawn new bullet
    
    loop Each AC_GAME_UPDATE_TICK
        Q-)Bul: AC_GAME_UPDATE_TICK
        activate Bul
        Note right of Bul: Phase 1 — Collision
        loop for each active bullet
            opt Intersection detected
                Note right of Bul: active=false, spawn explosion,<br/>call game_player_hit() / deduct enemy HP
            end
        end
        Note right of Bul: Phase 2 — Movement
        loop for each active bullet
            Note right of Bul: adjust y by speed<br/>if offscreen, active=false
        end
        deactivate Bul
    end
```

<p align="center"><strong><em>Figure 2:</em></strong> Bullet sequence logic</p>

## IV. Enemy Object Sequence

The Enemy Task manages the `g_enemies[MAX_ENEMIES]` and `g_powerups[MAX_POWERUPS]` arrays.

**Setup & Spawn.** `game_enemy_spawn()` creates a wave or Boss based on `g_stage`. This is triggered at the start of the game or when the Stage Task detects the screen is clear.

**Per-tick.** Each `AC_GAME_UPDATE_TICK` calls `game_enemy_update()`:
- **Movement & Abilities:** Enemies move according to their internal states. The Boss executes dynamic phases. Carriers spawn minions.
- **Powerups:** Translates active powerups falling down the screen.

```mermaid
%%{init: {'theme':'base','themeVariables':{'fontSize':'18px','primaryColor':'#1565c0','primaryTextColor':'#ffffff','primaryBorderColor':'#0d47a1','lineColor':'#90a4ae','signalColor':'#ffc107','signalTextColor':'#ffc107','actorBkg':'#1565c0','actorBorder':'#0d47a1','actorTextColor':'#ffffff','actorLineColor':'#90caf9','noteBkgColor':'#fff59d','noteTextColor':'#000000','noteBorderColor':'#f57f17','activationBkgColor':'#66bb6a','activationBorderColor':'#2e7d32','sequenceNumberColor':'#ffffff','loopTextColor':'#ffc107','labelBoxBkgColor':'#37474f','labelBoxBorderColor':'#90a4ae','labelTextColor':'#ffffff'},'sequence':{'actorMargin':120,'messageFontSize':17,'noteFontSize':15,'actorFontSize':17,'boxMargin':15,'boxTextMargin':8,'noteMargin':12,'useMaxWidth':true}}}%%
sequenceDiagram
    autonumber
    participant Q as AKOS Message Queue
    participant Enm as Enemy Task
    participant Stg as Stage Task

    Stg-)Q: Call spawn request (if clear)
    Q-)Enm: AC_GAME_UPDATE_TICK (from pipeline)
    activate Enm
    Note right of Enm: initialize enemy wave based on g_stage
    deactivate Enm

    loop Each AC_GAME_UPDATE_TICK
        Q-)Enm: AC_GAME_UPDATE_TICK
        activate Enm
        loop for each active enemy
            Note right of Enm: Apply movement vectors<br/>Generate hostile bullets (is_enemy=true)
        end
        Note right of Enm: Move active powerups
        deactivate Enm
    end
```

<p align="center"><strong><em>Figure 3:</em></strong> Enemy sequence logic</p>
