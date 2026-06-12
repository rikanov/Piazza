#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include "imgui.h"
#include "engine.h"

struct DragPayload {
    int r;
    int c;
};

class App {
public:
    App();
    ~App();
    void run();

private:
    void renderUI();
    ImVec4 getDigitColor(unsigned digit);

    void generateGame();
    void drawTicks(int black, int white, bool vertical);

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    bool isRunning = false;

    // Játékállapot változók
    unsigned secret_code[4];
    unsigned guesses[4][4];
    bool marked[4][4];

    int swaps_made = 0;
    int win_state = 0;
    float win_timer = 0.0f;
    bool in_menu = false;
};
