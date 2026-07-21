<div align="center">
  
![Repo Traffic](https://komarev.com/ghpvc/?username=space-shooter-game&label=Repo+Traffic&color=blue&style=flat-square)

</div>

# Space Shooter Game on AK Embedded Base Kit

<hr>

## System Documentation

| Document | Purpose |
|---|---|
| [README.md](README.md) | High-level system overview, target hardware, and game mechanics. |
| [docs/01-guide-getting-started.md](docs/01-guide-getting-started.md) | Step-by-step instructions for environment setup and project initialization. |
| [docs/02-guide-coding-rules.md](docs/02-guide-coding-rules.md) | Standardized coding conventions and commit message formats. |
| [docs/03-design-sequence-object.md](docs/03-design-sequence-object.md) | Detailed sequence diagrams defining the lifecycle of in-game entities. |
| [docs/04-design-sequence-runtime.md](docs/04-design-sequence-runtime.md) | System architecture detailing signal processing and task scheduling. |

## 1. Overview

Space Shooter is an embedded software project developed for the AK Embedded Base Kit. It demonstrates the implementation of a classic arcade game using a cooperative event-driven OS.

This repository provides practical examples of:

- **State Machine Integration:** Managing game states through discrete logic.
- **Event-Driven Architecture:** Processing inputs asynchronously via signals and messages.
- **Resource Management:** Scheduling UI rendering and physics calculations under strict hardware constraints.

## 2. Hardware Platform

<table align="center">
  <tr>
    <td align="center"><img src="hardware/images/ak-embedded-base-kit-version-3.jpg" alt="AK Embedded Base Kit - STM32L151 - v3.0" width="480"/></td>
  </tr>
</table>
<p align="center"><strong><em>Figure 1:</em></strong> AK Embedded Base Kit - STM32L151</p>

The game is designed to run on the [AK Embedded Base Kit](https://epcb.vn/products/ak-embedded-base-kit-lap-trinh-nhung-vi-dieu-khien-mcu). It utilizes a **1.54" OLED display** for graphics rendering, **three push buttons** for user input, and a **buzzer** for audio feedback.

**Microcontroller Specifications:**

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
=> Space Shooter Firmware
```

## 3. Game Mechanics and Objects

The application boots into a **Title Screen**, progressing to a **Main Menu** containing the following options:

- **Play:** Initialize a new game session.
- **Setting:** Adjust system parameters (e.g., Audio).
- **Rank:** Display the highest recorded score.
- **Exit:** Terminate the application.

### Defined Entities:

| Entity | Functional Description |
| :--- |:--- |
| **Player** | The user-controlled unit. Supports horizontal translation (Left/Right) and weapon discharge. |
| **Bullet** | Projectiles instantiated by either the Player or the Boss. Features simple vertical translation and AABB collision detection. |
| **Enemy** | Computer-controlled units spawned at the top boundary. They traverse downward using predefined algorithms (Linear, Zigzag, Sine). |
| **Boss** | A high-HP entity that spawns based on score thresholds. Features advanced horizontal movement and multi-projectile attacks. |
| **Powerup** | Dropped conditionally upon Enemy destruction. Applies temporary state modifiers to the Player (Shield, Multi-shot, Nuke). |
| **Explosion** | A transient visual effect rendered at the coordinates of a destroyed entity. |

### Gameplay Mechanics:

- **Controls:** Navigate the Player unit using the **[Up]** and **[Down]** hardware buttons. Trigger the primary weapon using the **[Mode]** button.
- **Objective:** Maximize the score integer by destroying Enemy units while avoiding collision with hostile entities or projectiles.
- **Scaling Difficulty:** The spawn rate and translation speed of Enemy units increment dynamically as the session persists.
- **Termination:** The game session terminates when the Player's life counter reaches zero.

## 4. Technical Architecture

> **Reference:** For comprehensive documentation on system execution flows and object interaction, refer to [Runtime Signal Processing](docs/04-design-sequence-runtime.md) and [Game Object Sequences](docs/03-design-sequence-object.md).

## Support

``` Note
Thank you for reviewing the Space Shooter project.
For technical inquiries or bug reports, please open an issue in the repository.
```
