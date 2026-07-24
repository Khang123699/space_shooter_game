<h1 align="center">Project Setup and Compilation Guide</h1>

This document outlines the standard procedures for initializing the Space Shooter project repository, setting up the development environment, and deploying code modifications.

---

## Table of Contents

- [1. Repository Initialization (Forking)](#1-repository-initialization-forking)
- [2. Development Environment Setup](#2-development-environment-setup)
- [3. Standard Development Workflow](#3-standard-development-workflow)
  - [3.1 Workspace Configuration](#31-workspace-configuration)
  - [3.2 Cloning the Repository](#32-cloning-the-repository)
  - [3.3 Modifying the Source Code](#33-modifying-the-source-code)
  - [3.4 Hardware Version Configuration (Ver 2.1 vs Ver 3.0)](#34-hardware-version-configuration-ver-21-vs-ver-30)
  - [3.5 Version Control Operations](#35-version-control-operations)

---

## 1. Repository Initialization (Forking)

To maintain version control integrity, developers are required to work on a personal fork of the repository:

### 1.1 Access the Source Repository

**Source URL:** [https://github.com/the-ak-foundation/ak-base-kit-stm32l151](https://github.com/the-ak-foundation/ak-base-kit-stm32l151) (Or the applicable upstream repository).

### 1.2 Fork Generation

Execute the **Fork** operation on the GitHub interface to clone the repository to your personal account.

> **Note:**
> - Name the repository after your game's name.
> - Add a brief description of your game in the Description field.

---

## 2. Development Environment Setup

Firmware compilation and flashing require a Linux-based environment (preferably Ubuntu). Detailed configuration steps for the toolchain are documented here:

**[AK Embedded Base Kit STM32L151 — Configuration Guide](https://epcb.vn/blogs/ak-embedded-software/ak-embedded-base-kit-stm32l151-getting-started)**

### 2.1 Configuring Makefile Paths for Different Linux Machines

By default, the `Makefile`s in this project (`application/Makefile` and `boot/Makefile`) contain hardcoded toolchain paths that might not match every developer's environment. To make the build process work on your specific Linux machine, you should update the toolchain variables inside these Makefiles.

1. Open `application/Makefile` and `boot/Makefile`.
2. Locate the path configuration section (usually near the top).
3. Modify them to match your local installation paths, or use the `?=` operator to allow environment variable overrides:

```makefile
# Allow overriding via environment variables
GCC_PATH            ?= /usr
PROGRAMER_PATH      ?= /usr/local/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin
OPENOCD_CFG_PATH    ?= /usr/share/openocd/scripts/board/stm32ldiscovery.cfg
```

If you installed `gcc-arm-none-eabi` globally via `apt`, it's recommended to remove the `$(GCC_PATH)/bin/` prefix from the compiler definitions (`CC`, `CPP`, etc.) so the OS can automatically locate them via the `$PATH` variable:

```makefile
CC  = arm-none-eabi-gcc
CPP = arm-none-eabi-g++
```

---

## 3. Standard Development Workflow

> **Requirement:** All `make` and `flash` operations must be executed within a Linux Terminal to ensure toolchain compatibility.

### 3.1 Workspace Configuration

Create a designated root directory (e.g., `Workspace`) within the user's `Home` directory to isolate project dependencies.

Within the `Workspace` directory, provision two subdirectories:

| Directory | Function |
| --------- | --------- |
| `Sources` | Designated location for git repositories. |
| `Tools`   | Designated location for compiler binaries and flashing utilities. |

---

### 3.2 Cloning the Repository

> **Note:** This operation is executed once during initial setup.

Navigate to the `Sources` directory via the Terminal and execute the clone command (substitute the URL with your forked repository):

```bash
git clone https://github.com/<username>/<repository-name>.git
```

---

### 3.3 Modifying the Source Code

It is recommended to use Visual Studio Code for Linux for source code editing.

The core application logic is located within the `application/sources/app` directory.

#### Procedure for Modifying UI Components (Example: Title Screen)

**Step 3.3.1 —** Inspect the header definitions in `screens/scr_game_ui.h` to understand the UI rendering API.

**Step 3.3.2 —** Modify the implementation in `scr_game_ui_menu.cpp`. This file processes bitmap data and updates entity coordinates for the Title Screen.

**Step 3.3.3 —** If new source files are introduced (e.g., `screens_bitmap.cpp`), they must be appended to the `Makefile.mk` list within the `screens/` directory to be included in the compilation target.

**Step 3.3.4 —** Execute the build script and flash the resulting binary to the MCU.

---

### 3.4 Hardware Version Configuration (Ver 2.1 vs Ver 3.0)

The AK Base Kit has different hardware versions with distinct OLED display drivers:
- **Ver 2.1**: Uses a 1.3" OLED screen (SH1106 driver).
- **Ver 3.0**: Uses a 1.54" OLED screen (SSD1309 driver).

To switch between these versions, modify `application/Makefile` in the `Hardware option` section:

**For Ver 2.1 (Default):**
```makefile
HARDWARE_OPTION += \
		-USSD1309_DRIVER_EN	\
		-DSH1106_DRIVER_EN
```

**For Ver 3.0:**
```makefile
HARDWARE_OPTION += \
		-DSSD1309_DRIVER_EN	\
		-USH1106_DRIVER_EN
```

> **Important:** After modifying the Makefile, you must run `make clean` before rebuilding to apply the changes.

#### Fixing OLED Offset Deviation (SH1106)
If the display on Ver 2.1 (SH1106) appears shifted or cut off on the sides (typically a 2-pixel deviation), you may need to adjust the column offset.
1. Open `application/sources/driver/Adafruit_oled_drv/Adafruit_oled_drv.h`.
2. Locate the `SH1106_DRIVER_EN` definition (around line 8) and change `OLED_COL_OFFSET` from `0` to `2`:
```cpp
#if defined (SH1106_DRIVER_EN)
#define OLED_COL_OFFSET 2
```
3. Run `make clean` and `make` to compile the firmware with the new offset.

---

### 3.5 Version Control Operations

Upon completing a feature implementation, commit the changes to the remote repository. Execute the following commands from the repository root:

```bash
git add .
git commit -m "[UPDATE] Modified Title Screen rendering logic"
git push origin main
```

---

## References

- Hardware & Embedded Systems Documentation: [AK Embedded Software](https://epcb.vn/blogs/ak-embedded-software)
