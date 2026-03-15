# Frogger

A terminal-based implementation of the classic Frogger arcade game, written in C using ncurses.

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
./build/frogger
```

> The game requires a terminal at least **80 columns wide** and **15 rows tall**.

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
2. **Road zone** — dodge cars and trucks. Getting hit costs a life.
3. **River zone** — hop onto logs and turtles to cross. Falling in the water costs a life.
4. Land on all 5 lily pads to complete the level and advance.
5. Each level increases the speed of all vehicles and platforms.

**Scoring:**
- `+10` points for each step forward
- `+50 × level` for reaching a lily pad
- `+200 × level` for filling all 5 pads (level complete)

## Project Structure

```
c_frogger/
├── Makefile
├── include/
│   └── frogger.h      # Types and API declarations
└── src/
    ├── main.c         # Game loop and input handling
    ├── game.c         # Game logic and state management
    └── render.c       # ncurses rendering
```

## Cleaning Up

```sh
make clean
```
