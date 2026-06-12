# La Piazza

A deliberately under-informed, minimalist mental game. It drifts away from conventional puzzle mechanics, framing itself more accurately as a logical riddle or a "black-box" enigma. 

The player is presented with only a single, explicit objective: **you must assemble a specific color code within a single row or column.** No tutorials are provided. No rules are explained. Every interaction, constraint, and underlying pattern must be deduced by the player through pure observation, deduction, and experimentation.

---

## Developer Guide (How to Build)

The project is built using C++20, leveraging **SDL2** for low-level window management/rendering and **Dear ImGui** for the minimalist user interface. The build system is fully automated via CMake.

### Prerequisites

Ensure you have a C++20 compliant compiler, CMake, and the SDL2 development libraries installed on your system.
On Linux (openSUSE/Ubuntu), you can install them via your package manager:
```bash
# openSUSE
sudo zypper in ncurses-devel libSDL2-devel cmake ninja

# Ubuntu/Debian
sudo apt-get install build-essential libsdl2-dev cmake ninja-build
