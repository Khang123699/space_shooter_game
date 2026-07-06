# Space Shooter Game

## Introduction

Space Shooter is a classic arcade game developed for the AK embedded system kit (STM32L151), inspired by the popular Chicken Invaders series. This repository contains the full firmware implementation, demonstrating the application of event-driven architecture to game logic on a resource-constrained microcontroller.

The system architecture highlights the following core concepts of embedded engineering:
- **System Design:** Complex logic flows are modeled utilizing UML and Finite State Machines (FSM).
- **Process Management:** Efficient scheduling and coordination of cooperative OS Tasks.
- **Communication:** Real-time inter-process communication using Signals, hardware Timers, and Messages.
- **Control Logic:** Robust mechanics for asynchronous player input, procedural alien generation, and Boss fight sequences.

### I. Hardware Requirements

[AK Embedded Base Kit](https://epcb.vn/products/ak-embedded-base-kit-lap-trinh-nhung-vi-dieu-khien-mcu) is the target hardware platform for this firmware.

The board integrates a **1.54" OLED LCD**, **3 push buttons**, and a **buzzer** for audio feedback, providing the necessary peripherals to run the event-driven game engine.

**MCU Overview:**

```text
SoC Name : STM32L151CBT6
RAM      : 16 KB

Flash Partitions Layout
----------------------
[ 0x08000000 - 0x08001FFF ] : Bootloader Partition (8 KB)
=> AK Bootloader

[ 0x08002000 - 0x08002FFF ] : BSF Shared Partition (4 KB)
=> Used for data sharing between Bootloader and Application

[ 0x08003000 - 0x0801FFFF ] : Application Partition (116 KB)
=> Space Shooter firmware
```

**MCU Naming Convention:**

| Part | Meaning |
|---|---|
| `STM32` | STMicroelectronics 32-bit MCU family. |
| `L` | Low-power series. |
| `151` | STM32L151 product line. |
| `C` | 48-pin package. |
| `B` | 128 KB Flash memory. |
| `T` | LQFP package. |
| `6` | Industrial temperature grade. |

### II. Game Description and Objects (Concept)

The following section describes the conceptual gameplay and core mechanics of **"Space Shooter"**. It serves as a reference for ongoing game design and firmware development.

#### 1. The Spaceship (Player)
  Controlled via `UP` (Left) and `DOWN` (Right) buttons.
  Fires bullets using the `MODE` button.
  Equipped with an anti-spam mechanism to limit on-screen bullets.
  Starts with 3 lives. Features a temporary invulnerability (blinking) frame upon taking damage.

#### 2. The Alien Invaders
  Spawned in grid formations at the top of the screen.
  3 distinct types of aliens yielding different score values.
  Formations are dynamically randomized (e.g., 30% chance for an empty cell) to ensure unpredictable waves.
  Difficulty automatically scales over time (faster movement, more aggressive attacks).

#### 3. The Epic Boss
  An intimidating Boss entity designed to appear every 5 levels.
  Features advanced movement and attack patterns:
    **Fast Sweep:** Moves horizontally at 3x speed.
    **Triple Shot Burst:** Fires a dangerous 3-bullet spread simultaneously.
    **Slow Descend:** Creeps closer to the player over time, tightening the dodge space.

#### 4. High Score Tracker
  Constantly tracks the player's highest score.
  Saves data persistently to the STM32's non-volatile Flash memory to retain records even after power-off.

