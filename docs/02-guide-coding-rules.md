<h1 align="center">Coding rules and style guide</h1>

This document defines the naming conventions, code style, commit message format, and document naming conventions used in the project. The goal is to make sure code contributed by team members stays consistent in form and easy to track through search tools, code review, and version control.

---

## Table of contents

- [I. Naming conventions](#i-naming-conventions)
  - [1. Folder](#1-folder)
  - [2. Source and header files](#2-source-and-header-files)
  - [3. Header guard](#3-header-guard)
  - [4. Macros and compile-time constants](#4-macros-and-compile-time-constants)
  - [5. Signal (enum values)](#5-signal-enum-values)
  - [6. Task ID](#6-task-id)
  - [7. Data types and typedef](#7-data-types-and-typedef)
  - [8. Functions](#8-functions)
  - [9. Variables](#9-variables)
- [II. Commit message convention](#ii-commit-message-convention)
- [III. Document file naming convention](#iii-document-file-naming-convention)
- [IV. References](#iv-references)

---

## I. Naming conventions

The conventions below are drawn directly from the existing source code. You can follow these conventions to develop your coding so that tooling, search, and reviewers all work consistently.

**Case styles used in this document:**

| Style | Description | Example in project | Applied to |
|---|---|---|---|
| `lower_snake_case` | Lowercase letters, words separated by underscore `_` | `game_shooter_logic`, `g_score` | Variables, functions, typedefs, source file names, folder names |
| `UPPER_SNAKE_CASE` | Uppercase letters, words separated by underscore `_` | `MAX_ENEMIES`, `AC_GAME_BTN_UP`, `AC_TASK_DISPLAY_ID` | `#define` constants, signal enums, task IDs, macros |
| `kebab-case` | Lowercase letters, words separated by hyphen `-` | `02-guide-coding-rules.md` | Documentation file names under `docs/` |

### 1. Folder

Use `lower_snake_case` for folder names. Organize by feature (feature-based), not by file type.

```
application/sources/app/
  space_shooter/          # folder holding the source code of all objects of a game,
                          # e.g. space_shooter
  screens/                # folder holding the source code of all display screens
  ...
```

### 2. Source and header files

Source and header files always carry a module prefix so you can identify the module right from the file name:

| Prefix | Meaning | Example |
|---|---|---|
| `scr_*` | Handler of a screen | `scr_game_ui_play.cpp`, `scr_game_ui.h` |
| `game_shooter_*` | Object belonging to the Space Shooter game | `game_shooter_enemy.cpp`, `game_shooter.h` |

Each game defines its own short prefix (for example `game_shooter_*` for Space Shooter) and applies it consistently to every file in that game's folder.

File extensions: `.h` for headers, `.cpp` for implementation (the project is built as C++).

### 3. Header guard

Use the pattern `__<FILE_NAME>_H__`, fully uppercased, matching the file name exactly:

```cpp
#ifndef __GAME_SHOOTER_ENEMY_H__
#define __GAME_SHOOTER_ENEMY_H__
...
#endif //__GAME_SHOOTER_ENEMY_H__
```

### 4. Macros and compile-time constants

Use `UPPER_SNAKE_CASE` for names. Always wrap numeric values in parentheses to avoid errors when the macro gets expanded.

**Mandatory rule: a macro that belongs to an object MUST carry that object's name as a prefix.**

Pattern: `<OBJECT>_<PROPERTY>` or `<OBJECT>_<ACTION>` — the object always comes first. Reading the macro name tells you immediately which module it belongs to, and grepping by object name returns every constant of that module.

| Constant type | Correct form |
|---|---|
| Count | `MAX_ENEMIES`, `MAX_BULLETS` |
| Coordinate | `PLAYER_START_X`, `PLAYER_START_Y` |
| Movement step | `PLAYER_MOVE_STEP`, `BULLET_SPEED` |
| Behavior / limit | `SCREEN_WIDTH`, `SCREEN_HEIGHT` |

Examples done right:

```cpp
// game_shooter.h
#define MAX_BULLETS            (20)
#define MAX_ENEMIES            (35)
#define MAX_POWERUPS           (3)
```

**Exception — system / project level macros:** macros belonging to the whole game (not tied to any specific object) use the project prefix `GAME_STATE_*`:

```cpp
#define GAME_STATE_TITLE (0)
#define GAME_STATE_PLAY  (1)
```

Group related constants in the right module header (`game_shooter.h` holds game constants, etc.). Never leave magic numbers scattered across `.cpp` files.

### 5. Signal (enum values)

Signals are the **public contract** between tasks. Always use the full prefix — no abbreviations, not in comments, documentation, or sequence diagrams.

| Pattern | Applied to | Example |
|---|---|---|
| `<PREFIX>_<TASK>_<ACTION>` | Task signals | `AC_DISPLAY_GAME_UI_INIT` |

Declare each task's signal set in `app.h` as its own enum block, anchored to `AK_USER_DEFINE_SIG` (or `AK_SYS_DEFINE_SIG` for system signals):

```cpp
/*****************************************************************************/
/*  DISPLAY task define
 */
/*****************************************************************************/
enum {
    AC_DISPLAY_RENDER_SCREEN = AK_SYS_DEFINE_SIG,
    AC_DISPLAY_INITIAL = AK_USER_DEFINE_SIG,
    AC_DISPLAY_BUTTON_MODE_PRESSED,
    AC_DISPLAY_BUTTON_UP_PRESSED,
    AC_DISPLAY_BUTTON_DOWN_PRESSED,
};
```

### 6. Task ID

Use the pattern `<PREFIX>_<NAME>_ID`, fully uppercased, registered in `task_list.h`:

```cpp
AC_TASK_DISPLAY_ID
```

The corresponding handler in `task_list.cpp` keeps the same name, replacing the `_ID` suffix with `_handle` and lowercased:

```cpp
{AC_TASK_DISPLAY_ID, TASK_PRI_LEVEL_8, ac_task_display_handle},
```

### 7. Data types and typedef

Use `lower_snake_case` with the `_t` suffix. The struct stays anonymous; the typedef is the public name:

```cpp
typedef struct {
    float x;
    float y;
    int type;
    bool active;
    uint8_t hp;
} game_enemy_t;
```

Types provided by the framework follow the same pattern (`ak_msg_t`, `view_screen_t`).

### 8. Functions

Use `lower_snake_case` with the module name as prefix, so that grepping the prefix returns every entry point of that module:

```cpp
void game_shooter_update();
void game_shooter_update_player();
void game_shooter_spawn_enemy();
void game_shooter_update_bullets();
```

### 9. Variables

Use `lower_snake_case`. Do not start names with an underscore.

- **Globals shared between modules:** declare `extern` in the header, define exactly once in the `.cpp` of the owning module.

  ```cpp
  // game_shooter_player.h
  extern int16_t g_player_x;
  extern uint8_t g_lives;
  extern uint16_t g_score;
  ```

- **Module-internal variables:** declare `static` in the `.cpp`.

  ```cpp
  // game_shooter_logic.cpp
  static uint8_t s_boss_stage = 0;
  ```

- **Local variables:** short, describe the role accurately. Loop counters can use `i`, `j`, `k` when the scope is clear.

---

## II. Commit message convention

Every commit must follow the format `[ACTION] short description` so the git history stays easy to read and easy to filter.

### 1. Workflow

```bash
git add .                                     # stage every change
git commit -m "[ACTION] short description"    # tag is mandatory, keep description short
git push                                      # push to remote
```

When you only need to stage specific files, replace `git add .` with `git add <path>` to avoid committing junk files by mistake.

### 2. Action tags

| Tag | When to use |
|---|---|
| `[ADD]` | Adding a new file, feature, asset, or document |
| `[UPDATE]` | Updating existing code — refactor, rename, tweak logic, bump version |
| `[FIX]` | Fixing an existing bug, build error, or formatting error |
| `[REMOVE]` | Removing a file, feature, or dead code |
| `[DOC]` | Documentation-only changes (`docs/`, `README.md`, large comment blocks) 

### 3. Description style

- Tag fully uppercased inside `[]`, followed by exactly one space, then the description.
- Description in lowercase, imperative mood (`add`, `fix`, `rename`, `move`...), no trailing period.
- Keep the length around 70 characters — if longer, shorten it or move the details into the commit body.
- When the change touches a specific module / signal / file, name it directly so the history is easy to grep.

### 4. Good examples

```text
[ADD] player movement mechanic
[ADD] sequence diagram for enemy logic
[UPDATE] rename variables to match standard
[UPDATE] split logic processing into multiple functions
[UPDATE] increase max enemy speed
[FIX] overlap when drawing player
[REMOVE] unused redundant state
[DOC] coding rules
```

### 5. Examples to avoid

```text
update                          # missing tag
[update] fix something          # tag must be uppercase
[ADD] Added new file.           # no past tense and no trailing period
[FIX] fix bug                   # too vague, which bug?
[ADD] game_shooter_enemy.cpp + scr_game_ui_play.cpp + ... # too long, group by topic
```

---

## III. Document file naming convention

Files in `docs/` follow the format `<NN>-<category>-<topic>.md`:

| Component | Convention | Example |
|---|---|---|
| `NN` | A 2-digit sequence number, starting from `01`. Reflects reading order — guides come first, design docs come after. | `01`, `02`, `03` |
| `category` | Document category. Only use predefined values; do not add new categories. | `guide`, `design` |
| `topic` | Main topic, written in `kebab-case` (lowercase, words separated by `-`). | `getting-started`, `coding-rules`, `sequence-object` |

Categories currently in use:

| Category | Purpose | Typical content |
|---|---|---|
| `guide` | Workflow, setup, and process guides for contributors | Getting started, coding rules |
| `design` | Describes architecture and runtime behavior of the system | Sequence diagrams |

Example of files currently in the repo:

```
docs/
├── 01-guide-getting-started.md
├── 02-guide-coding-rules.md
├── 03-design-sequence-object.md
├── 04-design-sequence-runtime.md
└── 05-design-data-storage.md
```

A few important notes:

- Documentation files (`.md`) use `kebab-case` (hyphens). Source files and folders use `snake_case` (underscores). The difference is intentional: `kebab-case` is the standard convention for Markdown slugs and URLs.
- Images that go with a document live under `resources/images/<topic_dir>/`, where `<topic_dir>` follows the folder convention (snake_case). For example: `resources/images/guide-coding-rules/`.
- When adding a new document, continue the sequence number from the current maximum and keep the same category together to preserve a natural reading order.
- Renaming a documentation file must be done with `git mv` so that the rename history is tracked properly.

---

## IV. References

- [Naming convention — Multi-word identifiers (Wikipedia)](https://en.wikipedia.org/wiki/Naming_convention_(programming)#Multiple-word_identifiers) — definitions of `snake_case`, `SCREAMING_SNAKE_CASE`, and `kebab-case` used in Sections I and III.
- [Pro Git — Recording Changes to the Repository](https://git-scm.com/book/en/v2/Git-Basics-Recording-Changes-to-the-Repository) — the `git add` / `git commit` / `git push` workflow used in Section II.

