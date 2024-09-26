# Physics Engine

An implementation in C of a physics engine from scratch as a team project for 2024 CS 3. Implemented forces (gravity, springs, etc), collision handling, efficient memory handling, and input management. Used unit tests to ensure that code works. Created simulation and game demos like N-Body Simulation, Frogger, Pacman, Space Invaders, and more. Tech stack includes C, Github, Emscripten, and SDL2.

https://github.com/user-attachments/assets/4d4ae16c-2eea-4049-a1cc-b38ed8bfa0e0


## How to Build

**Prerequisites**

Install [SDL2](https://formulae.brew.sh/formula/sdl2) and [Emscripton](https://emscripten.org/docs/getting_started/downloads.html)

**Compilation**

To compile the project, use the following commands:

First, verify that you are in the `Physics-Engine` directory with `pwd`

Then run
```
make NO_ASAN=true
```

and open in browser: `http://localhost:8000/bin/game.html`





