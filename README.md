# R-Type

> A multiplayer game platform powered by a custom ECS game engine and robust networking layer, featuring two distinct games: a classic side-scrolling shooter and a platform fighting game.

<div align="center">

**[Wiki Documentation](https://github.com/R-Type-ou-rien/R-Type/wiki)** • **[Contributing Guide](https://github.com/R-Type-ou-rien/R-Type/wiki/How-to-contribute)**

</div>

## Overview

This project is a complete game development platform built from scratch, consisting of three main layers:

1. **Game Engine** - A custom Entity Component System (ECS) engine optimized for performance
2. **Network Layer** - Client-server architecture with TCP/UDP protocols, authentication, and voice chat
3. **Games** - Two multiplayer games one R-Type and a Smash game

---

# Part 1: Game Engine

> [!NOTE]
> The custom ECS engine uses **Data-Oriented Design** for cache-efficient performance.

## ECS Architecture

The engine follows the Entity-Component-System pattern, separating data from logic for maximum flexibility and performance.

### Core Concepts

| Concept | Description | Example |
|---------|-------------|---------|
| **Entity** | Unique identifier (just an ID) | Player #1, Enemy #42, Bullet #156 |
| **Component** | Pure data attached to entities | Position, Velocity, Sprite, Health |
| **System** | Logic that processes entities | PhysicsSystem, RenderSystem, CollisionSystem |

### Engine Components

| Component | Description |
|-----------|-------------|
| `Transform2D` | Position, rotation, scale |
| `Velocity` | Movement speed and direction |
| `Sprite2D` | Static sprite rendering |
| `AnimatedSprite2D` | Multi-frame animations (Loop, Once, PingPong modes) |
| `Collider` | Collision detection bounds |
| `Health` | HP and damage handling |
| `Tag` | Entity identification and grouping |

### Engine Systems

| System | Description |
|--------|-------------|
| `RenderSystem` | Sprite and animation rendering |
| `PhysicsSystem` | Movement and velocity integration |
| `CollisionSystem` | Hit detection between entities |
| `AnimationSystem` | Frame-based sprite animation |
| `InputSystem` | Keyboard and gamepad input handling |
| `AudioSystem` | Sound effects and music playback |
| `DestructionSystem` | Entity cleanup and removal |

### Technologies

| Component | Technology |
|-----------|------------|
| Graphics | SFML 3.0 |
| Audio | SFML Audio |
| Build | CMake / vcpkg |

---

# Part 2: Network Layer

## Client-Server Architecture

The platform uses an **authoritative server** model where the server validates all game actions, ensuring fair gameplay and preventing cheating.

```
┌─────────────────────────────────────────────────────────────────┐
│                         CLIENT                                   │
├─────────────────────────┬───────────────────────────────────────┤
│      TCP Channel        │         UDP Channel                   │
│      (Port 8000)        │         (Port 7777)                   │
│                         │                                       │
│  • Authentication       │  • Player Input (60Hz)                │
│  • Lobby Management     │  • Game State Snapshots               │
│  • Chat Messages        │  • Entity Updates                     │
│                         │  • Voice Chat (Port 4243)             │
└─────────────────────────┴───────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────────┐
│                         SERVER                                   │
│                                                                  │
│  TCP Listener (8000)  ←→  Game Logic  ←→  UDP Listener (7777)   │
│                              ↓                                   │
│                         SQLite DB                                │
└─────────────────────────────────────────────────────────────────┘
```

## Network Protocol

| Channel | Port | Use Case |
|---------|------|----------|
| **TCP** | 8000 | Authentication, Lobby, Chat, Critical Events |
| **UDP** | 7777 | Player Input, Game State, Entity Updates |
| **UDP** | 4243 | Voice Chat (Opus-encoded) |

## Network Features

### Authentication System
- User registration with password
- Login with session tokens
- Anonymous login option
- Persistent user database (SQLite)

### Lobby System
- **Lobby browser** - Browse available lobbies or create new ones
- **Room management** - Up to 4 players per room
- **Ready system** - Players signal readiness before game start
- **Host controls** - Host can start/manage the game

### Voice Chat
Full voice communication powered by PortAudio and Opus:
- 48kHz sample rate, 24 kbps compression
- Per-user volume control
- Mute functionality
- Dedicated UDP channel

### Text Chat
- Team chat in lobby and during game
- 8-message history display

### Technologies

| Component | Technology |
|-----------|------------|
| Networking | Asio (standalone) |
| Voice Capture | PortAudio |
| Voice Codec | Opus |
| Database | SQLite |

---

# Part 3: Games

Two complete games built using the Game Engine and Network Layer.

---

## Game 1: R-Type

> A classic side-scrolling shooter where up to 4 players cooperate to fight through waves of enemies and epic boss battles.

### Campaign - 3 Levels same environment but with different ennemies and patterns

| Level | Name | Environment | Boss |
|-------|------|-------------|------|
| 1     | Asteroid Field | Rocky walls, floor & ceiling turrets | boss with tail |

### R-Type Systems

| System | Description |
|--------|-------------|
| `BossSystem` | Boss behavior, phases, and attacks |
| `BossTailSystem` | Tail animation and physics |
| `PodSystem` | Support craft mechanics |
| `PowerupSystem` | Power-up effects and duration |
| `SpawnSystem` | Enemy wave spawning |
| `ScrollSystem` | Level scrolling |
| `LevelTransitionSystem` | Level progression on boss defeat |
| `LeaderboardSystem` | Score tracking and display |

### Multiplayer Features
- Up to 4 players co-op
- Spectator mode for defeated players
- Shared leaderboard

---

## Game 2: Tu smash pas tu rentres pas

> A platform fighting game inspired by Super Smash Bros with arena-style combat for 2 players.

### Gameplay

- 2-player competitive fighting
- Percentage-based damage system (higher % = more knockback)
- Platform-based arenas
- Eject/knockback mechanics

### How to Launch

```bash
# Linux
./launch_project/linux_exec.sh smash

# Then run
./build-client/r-type-client
```

---

# Quick Start

> [!IMPORTANT]
> **Prerequisites:**
> - C++20 compatible compiler (GCC, Clang, or MSVC)
> - CMake 3.17+
> - Git

## Building the Project

1. **Clone the repository**
   ```bash
   git clone https://github.com/R-Type-ou-rien/R-Type.git
   cd R-Type
   ```

2. **Build using launch scripts**

   <table>
   <tr>
   <td><strong>Linux</strong></td>
   <td><strong>Windows</strong></td>
   </tr>
   <tr>
   <td>

   ```bash
   # Build both client and server
   ./launch_project/linux_exec.sh

   # Build client only
   ./launch_project/linux_exec.sh client

   # Build server only
   ./launch_project/linux_exec.sh server

   # Clean builds
   ./launch_project/linux_exec.sh clean
   ```

   </td>
   <td>

   ```powershell
   # Build both client and server
   .\launch_project\win_exec.ps1

   # Build client only
   .\launch_project\win_exec.ps1 client

   # Build server only
   .\launch_project\win_exec.ps1 server

   # Clean builds
   .\launch_project\win_exec.ps1 clean
   ```

   </td>
   </tr>
   </table>

3. **Run the games**

   <table>
   <tr>
   <td><strong>Start Server</strong></td>
   <td><strong>Start Client</strong></td>
   </tr>
   <tr>
   <td>

   ```bash
   # Linux
   ./build-server/r-type-server

   # Windows
   .\r-type-server.exe
   ```

   </td>
   <td>

   ```bash
   # Linux
   ./build-client/r-type-client

   # Windows
   .\r-type-client.exe
   ```

   </td>
   </tr>
   </table>

---

# Project Structure

```
r-type/
├── CMakeLists.txt
├── launch_project/
│   ├── linux_exec.sh              # Linux build script
│   ├── win_exec.ps1               # Windows build script
│   └── units_tests_launch.sh      # Test runner
├── src/
│   ├── Engine/                    # Game Engine (Part 1)
│   │   ├── Core/
│   │   │   ├── ECS/               # Registry, SystemManager, SparseSet
│   │   │   └── Voice/             # VoiceManager (PortAudio/Opus)
│   │   └── Lib/
│   │       ├── Components/        # Engine components
│   │       └── Systems/           # Engine systems
│   ├── Network/                   # Network Layer (Part 2)
│   │   ├── Client/                # Client networking
│   │   ├── Server/                # Server networking
│   │   └── Database/              # SQLite integration
│   └── RType/                     # Games (Part 3)
│       └── Common/
│           ├── Components/        # Game components (Boss, PowerUp, etc.)
│           ├── Systems/           # Game systems
│           ├── Lib/GameManager/   # Menu, Auth, Lobby managers
│           └── content/
│               ├── config/        # Configuration files
│               └── sprites/       # Game assets
├── tests/                         # Unit tests
└── vcpkg/                         # Package manager
```

---

# Configuration

The games are highly configurable through `.cfg` files in `src/RType/Common/content/config/`:

| File | Description |
|------|-------------|
| `game.cfg` | Boss spawn time, scroll speed, wave intervals |
| `player.cfg` | Player speed, HP, fire rate, sprites |
| `boss.cfg` | Boss stats, phases, sub-entities, tail config |
| `enemies.cfg` | Enemy types, HP, damage, movement patterns |
| `ui.cfg` | UI positioning, fonts, colors |
| `levels/*.scene` | Level layouts, walls, turrets, boss selection |

---

# Development

## Commit Convention

All commits **must** follow this format:

```
<type>: <description>
```

| Type | Description |
|------|-------------|
| `feat` | New feature |
| `fix` | Bug fix |
| `docs` | Documentation changes |

## Branch Policy

| Branch | Protection |
|--------|------------|
| `main` | No direct push - PR required |
| `dev` | No direct push - PR required |

> [!CAUTION]
> Direct pushes to `main` and `dev` are blocked by CI.

## CI/CD Pipeline

```
┌─────────────────────┐
│ check-branch-policy │  → Blocks direct push to main/dev
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│  commit-and-style   │  → Validates commits + clang-format
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│   push_to_mirror    │  → Mirrors to external repository
└─────────────────────┘
```

---

# Documentation

| Document | Description |
|----------|-------------|
| [Wiki Home](https://github.com/R-Type-ou-rien/R-Type/wiki) | Project overview and architecture |
| [Technical-ECS](https://github.com/R-Type-ou-rien/R-Type/wiki/Technical-ECS) | ECS engine specification |
| [Technical-Network](https://github.com/R-Type-ou-rien/R-Type/wiki/Technical-Network) | Network protocol specification |
| [How-to-contribute](https://github.com/R-Type-ou-rien/R-Type/wiki/How-to-contribute) | Contribution guidelines |
| [CI-Documentation](https://github.com/R-Type-ou-rien/R-Type/wiki/CI-Documentation) | CI/CD pipeline documentation |
