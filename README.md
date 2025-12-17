# R-Type

> A networked multiplayer game platform featuring a classic side-scrolling shooter and a platform fighting game, built with a custom ECS game engine.

<div align="center">

**[Wiki Documentation](https://github.com/R-Type-ou-rien/R-Type/wiki)** • **[Contributing Guide](https://github.com/R-Type-ou-rien/R-Type/wiki/How-to-contribute)**

</div>

> [!NOTE]
> Built with a custom Entity Component System (ECS) engine for high-performance gameplay

## What it does

R-Type is a multiplayer game platform that lets you play two different games with friends over the network. The platform uses a client-server architecture where the server is authoritative, ensuring fair gameplay and preventing cheating.

**Games included:**
- **R-Type** - A classic side-scrolling shooter where players cooperate to fight through waves of enemies and bosses
- **Tu smash pas tu rentres pas** - A platform fighting game inspired by Super Smash Bros with arena-style combat

**Features:**
- Real-time multiplayer with up to 4 players per room
- Custom ECS game engine optimized for performance
- Cross-platform support (Linux & Windows)
- Room-based lobby system
- Text and voice chat

## Architecture

The project follows a **client-server architecture** with a shared **Entity Component System (ECS)** game engine.

### Components

| Component | Technology | Description |
|-----------|------------|-------------|
| **Client** | C++ / SFML 3.0 | Game rendering, input handling, and network sync |
| **Server** | C++ / Asio | Authoritative game logic, room management |
| **Engine** | Custom ECS | Shared entity-component-system for game logic |
| **Build** | CMake / vcpkg | Cross-platform build system |

### How it works

```
┌─────────────────────────────────────────────────────────────────┐
│                         CLIENT                                   │
├─────────────────────────────┬───────────────────────────────────┤
│      TCP Channel            │         UDP Channel               │
│      (Port 8000)            │         (Port 7777)               │
│                             │                                   │
│  • Authentication           │  • Player Input                   │
│  • Lobby Management         │  • Game State Snapshots           │
│  • Chat Messages            │  • Entity Updates                 │
│                             │  • Voice Chat (VoIP)              │
└─────────────────────────────┴───────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                         SERVER                                   │
│                                                                  │
│  TCP Listener (8000)  ←→  Game Logic  ←→  UDP Listener (7777)   │
└─────────────────────────────────────────────────────────────────┘
```

1. **Client** sends player inputs via UDP at 60Hz
2. **Server** processes inputs, runs game logic, detects collisions
3. **Server** broadcasts authoritative game state to all clients
4. **Client** reconciles state and renders the game

## Quick Start

> [!IMPORTANT]
> **Prerequisites:**
> - C++20 compatible compiler (GCC, Clang, or MSVC)
> - CMake 3.17+
> - Git

### Building the Project

1. **Clone the repository with submodules**
   ```bash
   git clone https://github.com/R-Type-ou-rien/R-Type.git
   cd R-Type
   ```

2. **Configure and build**
   ```bash
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

3. **Run the application**

   <table>
   <tr>
   <td><strong>Start Server</strong></td>
   <td><strong>Start Client</strong></td>
   </tr>
   <tr>
   <td>

   ```bash
   ./r-type_server [port]
   ```

   </td>
   <td>

   ```bash
   ./r-type_client [host] [port]
   ```

   </td>
   </tr>
   </table>

## Project Structure

```
r-type/
├── CMakeLists.txt
├── src/
│   ├── Client/                    # Client application
│   ├── Server/                    # Server application
│   └── Engine/                    # Shared ECS engine
│       ├── Core/
│       │   ├── ECS/               # Registry, SystemManager, SparseSet
│       │   ├── Components/        # Component definitions
│       │   └── Systems/           # System implementations
│       ├── Input/                 # Input management
│       └── Resources/             # Resource management
├── assets/                        # Game assets (sprites, sounds, fonts)
├── tests/                         # Unit tests
└── docs/                          # Documentation
```

## ECS Engine

> [!TIP]
> The custom ECS engine uses **Data-Oriented Design** for cache-efficient performance.

### Core Concepts

| Concept | Description | Example |
|---------|-------------|---------|
| **Entity** | Unique identifier (just an ID) | Player #1, Enemy #42, Bullet #156 |
| **Component** | Pure data attached to entities | Position, Velocity, Sprite, Health |
| **System** | Logic that processes entities | PhysicsSystem, RenderSystem, CollisionSystem |

## Network Protocol

The game uses a hybrid TCP/UDP protocol:

| Channel | Port | Use Case |
|---------|------|----------|
| **TCP** | 8000 | Authentication, Lobby, Chat, Critical Events |
| **UDP** | 7777 | Player Input, Game State, Entity Updates |

## Development

### Commit Convention

All commits **must** follow this format:

```
<type>: <description>
```

| Type | Description |
|------|-------------|
| `feat` | New feature |
| `fix` | Bug fix |
| `docs` | Documentation changes |

### Branch Policy

| Branch | Protection |
|--------|------------|
| `main` | No direct push - PR required |
| `dev` | No direct push - PR required |

> [!CAUTION]
> Direct pushes to `main` and `dev` are blocked by CI.

## CI/CD Pipeline

The project uses **GitHub Actions** for continuous integration:

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

## Documentation

| Document | Description |
|----------|-------------|
| [Wiki Home](https://github.com/R-Type-ou-rien/R-Type/wiki) | Project overview and architecture |
| [Technical-ECS](https://github.com/R-Type-ou-rien/R-Type/wiki/Technical-ECS) | ECS engine specification |
| [Technical-Network](https://github.com/R-Type-ou-rien/R-Type/wiki/Technical-Network) | Network protocol specification |
| [How-to-contribute](https://github.com/R-Type-ou-rien/R-Type/wiki/How-to-contribute) | Contribution guidelines |
| [CI-Documentation](https://github.com/R-Type-ou-rien/R-Type/wiki/CI-Documentation) | CI/CD pipeline documentation |

## Troubleshooting

<details>
<summary><strong>Build fails with missing dependencies</strong></summary>

Then reconfigure:
```bash
rm -rf build
mkdir build && cd build
cmake ..
cmake --build .
```

</details>

<details>
<summary><strong>Client can't connect to server</strong></summary>

> [!TIP]
> Check that:
> - Server is running and listening on the correct port
> - Firewall allows connections on ports 8000 (TCP) and 7777 (UDP)
> - Client is using the correct host IP and port

</details>

<details>
<summary><strong>Game stutters or lags</strong></summary>

- Check network latency between client and server
- Ensure server isn't overloaded (check CPU usage)
- Verify UDP packets aren't being dropped by firewall

</details>

<details>
<summary><strong>CI fails on commit message</strong></summary>

Ensure your commit follows the format:
```bash
# Good
git commit -m "feat: add multiplayer lobby"
git commit -m "fix: resolve collision bug"
git commit -m "docs: update README"

# Bad
git commit -m "added feature"      # Missing type
git commit -m "feature: add lobby" # Wrong type (use "feat")
```

</details>

## License

This is an Epitech school project (3rd year - 2025).

## Contributors

| Name | Role |
|------|------|
| See Git history | For contributor information |
