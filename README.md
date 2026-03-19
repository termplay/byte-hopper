# Byte Hopper

A terminal-based arcade game inspired by the classic Frogger, written in C using ncurses.

```
 _______________________________________________
| SCORE: 0    LIVES: 3    LEVEL: 1    P=pause  |
|  ___   ___   ___   ___   ___                 |
| | 1 | | 2 | | 3 | | 4 | | 5 |               |
|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ river ~~~~~|
|   [===log===]        [===log===]             |
|        [turtle]   [turtle]                   |
|   [========log========]                      |
|        [turtle]   [turtle]  [turtle]         |
|        [========log========]                 |
|______________ median ________________________|
| <car>    <truck-->       <car>    road lanes |
|     <--car>       <truck-->                  |
|  <--car>    <car>      <truck-->             |
|       <car>      <--truck-->    <car>        |
|  <--truck-->          <--car>               |
|_________________^____________________________|
```

## Requirements

- **C compiler:** clang (or gcc)
- **ncurses** development library
- **make**

### Installing ncurses

**macOS:**
```sh
brew install ncurses
```

**Debian / Ubuntu:**
```sh
sudo apt install libncurses-dev
```

**Fedora / RHEL:**
```sh
sudo dnf install ncurses-devel
```

**Arch Linux:**
```sh
sudo pacman -S ncurses
```

## Building

Clone or download the project, then from the project root:

```sh
# Debug build (default)
make

# Optimized release build
make release

# Build with AddressSanitizer + UBSan (for development)
make asan
```

The compiled binary is placed in `build/`.

## Running

```sh
# Build and run in one step
make run

# Or run the binary directly after building
./build/byte-hopper
```

> The game requires a terminal at least **80 columns wide** and **15 rows tall**.

## Installing

```sh
# Install to /usr/local/bin (may need sudo)
sudo make install

# Install to a custom prefix
make install PREFIX=$HOME/.local

# Uninstall
sudo make uninstall
```

## Controls

| Key | Action |
|-----|--------|
| `W` / `UP` | Move up |
| `S` / `DOWN` | Move down |
| `A` / `LEFT` | Move left |
| `D` / `RIGHT` | Move right |
| `P` | Pause / Resume |
| `R` | Restart (after game over) |
| `Q` | Quit |

## How to Play

1. Guide the frog from the bottom of the screen to one of the **5 lily pads** at the top.
2. **Road zone** -- dodge cars and trucks. Getting hit costs a life.
3. **River zone** -- hop onto logs and turtles to cross. Falling in the water costs a life.
4. Land on all 5 lily pads to complete the level and advance.
5. Each level gets progressively harder -- vehicles and platforms move faster, and more obstacles appear.

**Scoring:**
- `+10` points for each step forward
- `+50 x level` for reaching a lily pad
- `+200 x level` for filling all 5 pads (level complete)

## Features

- **Start screen** with ASCII art title, instructions, and high score table
- **Name entry** for high scores -- prompted when you earn a top-10 spot
- **Level complete screen** shown between levels
- **Progressive difficulty** -- speed increases 12% per level (capped at 2.5x) and extra obstacles spawn every 3 levels
- **Persistent high scores** -- top 10 scores saved to `~/.local/share/byte_hopper/highscores.dat` (respects `XDG_DATA_HOME`)
- **Death animation** with flashing sprite
- **Colorful ncurses rendering** with distinct visuals for each object type

## Project Structure

```
c_frogger/
├── Makefile
├── include/
│   └── frogger.h        # Types and API declarations
└── src/
    ├── main.c           # Game loop, input handling, start screen
    ├── game.c           # Game logic, state management, difficulty scaling
    ├── render.c         # ncurses rendering (gameplay, overlays, start screen)
    └── highscore.c      # High score persistence (~/.local/share/byte_hopper/)
```

## Cleaning Up

```sh
make clean
```
