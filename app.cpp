#include "app.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include <iostream>
#include <ctime>
#include <algorithm>
#include <random>
#include <cmath>

App::App() {
    generateGame();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "Hiba az SDL inicializálásakor: " << SDL_GetError() << "\n";
        return;
    }

    // A minimalista rácshoz igazított ablakméret
    window = SDL_CreateWindow("La Piazza", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 600, 500, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    isRunning = true;
}

App::~App() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void App::generateGame() {
    win_state = 0;
    win_timer = 0.0f;
    swaps_made = 0;
    in_menu = false; // Új játéknál mindig visszatérünk a játéktérre

    std::mt19937 rng((unsigned)time(nullptr));
    bool valid = false;

    while (!valid) {
        unsigned pool[16] = {0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7};
        std::shuffle(std::begin(pool), std::end(pool), rng);
        for (int i = 0; i < 4; ++i) secret_code[i] = pool[i];

        std::shuffle(std::begin(pool), std::end(pool), rng);
        int idx = 0;
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                guesses[r][c] = pool[idx++];
                marked[r][c] = false;
            }
        }

        // Csak akkor fogadjuk el a táblát, ha nincs indulásból megoldva
        if (is_solved(code(secret_code), &guesses[0][0]) == 0) valid = true;
    }
}

void App::run() {
    while (isRunning) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);

            if (event.type == SDL_QUIT) isRunning = false;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window)) {
                isRunning = false;
            }

            // Hardveres kilépés (Esc PC-n, Vissza gomb mobilon)
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_AC_BACK) {
                    isRunning = false;
                }
            }
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        renderUI();

        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255); // Sötét atmoszféra
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }
}

ImVec4 App::getDigitColor(unsigned digit) {
    switch(digit) {
        case 0: return ImVec4(0.9f, 0.2f, 0.2f, 1.0f);
        case 1: return ImVec4(0.2f, 0.8f, 0.2f, 1.0f);
        case 2: return ImVec4(0.2f, 0.4f, 1.0f, 1.0f);
        case 3: return ImVec4(0.9f, 0.9f, 0.1f, 1.0f);
        case 4: return ImVec4(0.8f, 0.2f, 0.8f, 1.0f);
        case 5: return ImVec4(0.2f, 0.8f, 0.8f, 1.0f);
        case 6: return ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
        case 7: return ImVec4(0.9f, 0.6f, 0.7f, 1.0f);
        default: return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

void App::drawTicks(int black, int white, bool vertical) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();

    ImU32 cBlack = IM_COL32(10, 10, 10, 255);
    ImU32 cWhite = IM_COL32(240, 240, 240, 255);
    ImU32 cGray  = IM_COL32(80, 80, 80, 255);

    float thickness = 6.0f;
    float length = 30.0f;
    float spacing = 12.0f;

    for (int i = 0; i < 4; ++i) {
        ImU32 col = (i < black) ? cBlack : (i < black + white) ? cWhite : cGray;
        ImVec2 pMin, pMax;

        if (vertical) {
            pMin = ImVec2(p.x + static_cast<float>(i) * spacing, p.y);
            pMax = ImVec2(p.x + static_cast<float>(i) * spacing + thickness, p.y + length);
        } else {
            pMin = ImVec2(p.x, p.y + static_cast<float>(i) * spacing);
            pMax = ImVec2(p.x + length, p.y + static_cast<float>(i) * spacing + thickness);
        }
        draw_list->AddRectFilled(pMin, pMax, col, thickness * 0.5f);
    }

    if (vertical) ImGui::Dummy(ImVec2(4 * spacing, length));
    else ImGui::Dummy(ImVec2(length, 4 * spacing));
}

void App::renderUI() {
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    // Extrém minimalizmus beállítása
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground |
    ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("La Piazza Grid", nullptr, window_flags);

    // 1. Állapot kiértékelése
    int current_win = is_solved(code(secret_code), &guesses[0][0]);
    if (current_win > 0 && win_state == 0) {
        win_state = current_win;
        win_timer = ImGui::GetTime(); // Animáció indítása
    }

    // 2. Animáció lejárata és játék újraindítása
    if (win_state > 0) {
        float elapsed = static_cast<float>(ImGui::GetTime()) - win_timer;
        if (elapsed > 3.0f) { // 3 másodperc villogás után
            generateGame();
        }
    }

    ImGuiTableFlags table_flags = ImGuiTableFlags_SizingFixedFit;
    if (ImGui::BeginTable("PiazzaGrid", 5, table_flags)) {

        for (int row = 0; row < 4; ++row) {
            ImGui::TableNextRow();
            ImGui::PushID(row);

            for (int col = 0; col < 4; ++col) {
                ImGui::TableNextColumn();
                ImGui::PushID(col);

                if (in_menu) {
                    // --- TITKOS MENÜ NÉZET ---
                    if (row == 3 && col == 0) {
                        // BAL ALSÓ: PIROS (Kilépés)
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.1f, 0.1f, 1.0f));
                        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

                        if (ImGui::Button("##exit", ImVec2(50, 50))) isRunning = false;

                        ImGui::PopStyleVar();
                        ImGui::PopStyleColor(3);
                    }
                    else if (row == 3 && col == 3) {
                        // JOBB ALSÓ: ZÖLD (Új játék)
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.5f, 0.1f, 1.0f));
                        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

                        if (ImGui::Button("##play", ImVec2(50, 50))) {
                            generateGame(); // Ez automatikusan visszavált in_menu = false-ra
                        }

                        ImGui::PopStyleVar();
                        ImGui::PopStyleColor(3);
                    }
                    else {
                        // Üres hely a menüben
                        ImGui::Dummy(ImVec2(50, 50));
                    }
                }
                else {
                    // --- NORMÁL JÁTÉK NÉZET ---
                    unsigned val = guesses[row][col];
                    ImVec4 color = getDigitColor(val);

                    // VILLOGÁS LOGIKÁJA GYŐZELEMKOR
                    if (win_state > 0) {
                        bool is_winning_cell = false;
                        if (win_state >= 1 && win_state <= 4 && row == (win_state - 1)) is_winning_cell = true;
                        if (win_state >= 5 && win_state <= 8 && col == (win_state - 5)) is_winning_cell = true;

                        if (is_winning_cell) {
                            float elapsed = static_cast<float>(ImGui::GetTime()) - win_timer;
                            float flash = (std::sin(elapsed * 6.28318f * 1.5f) + 1.0f) * 0.5f;
                            color.x = color.x + (1.0f - color.x) * flash;
                            color.y = color.y + (1.0f - color.y) * flash;
                            color.z = color.z + (1.0f - color.z) * flash;
                        }
                    }

                    ImGui::PushStyleColor(ImGuiCol_Button, color);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.x*1.1f, color.y*1.1f, color.z*1.1f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(color.x*0.9f, color.y*0.9f, color.z*0.9f, 1.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

                    ImGui::Button("##hidden_val", ImVec2(50, 50));

                    // REJTETT JELÖLÉS (Middle Click vagy Ctrl+Click)
                    if (ImGui::IsItemHovered() &&
                        (ImGui::IsMouseClicked(ImGuiMouseButton_Middle) ||
                        (ImGui::GetIO().KeyCtrl && ImGui::IsMouseClicked(ImGuiMouseButton_Left)))) {
                        marked[row][col] = !marked[row][col];
                        }

                        // Jelölő keret rajzolása
                        if (marked[row][col]) {
                            ImVec2 pMin = ImGui::GetItemRectMin();
                            ImVec2 pMax = ImGui::GetItemRectMax();
                            ImDrawList* draw_list = ImGui::GetWindowDrawList();
                            draw_list->AddRect(
                                ImVec2(pMin.x + 8.0f, pMin.y + 8.0f), ImVec2(pMax.x - 8.0f, pMax.y - 8.0f),
                                               IM_COL32(255, 255, 255, 120), 4.0f, 0, 2.0f
                            );
                        }

                        // DRAG & DROP LOGIKA (Csak ha nincs győzelmi animáció)
                        if (win_state == 0) {
                            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                                DragPayload payload = { row, col };
                                ImGui::SetDragDropPayload("DIGIT_CELL", &payload, sizeof(payload));

                                ImGui::PushStyleColor(ImGuiCol_Button, color);
                                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
                                ImGui::Button("##tooltip_btn", ImVec2(25, 25));
                                ImGui::PopStyleVar();
                                ImGui::PopStyleColor();

                                ImGui::EndDragDropSource();
                            }

                            if (ImGui::BeginDragDropTarget()) {
                                if (const ImGuiPayload* payload_data = ImGui::AcceptDragDropPayload("DIGIT_CELL")) {
                                    const DragPayload* source = (const DragPayload*)payload_data->Data;

                                    // Cserék végrehajtása
                                    unsigned temp = guesses[row][col];
                                    guesses[row][col] = guesses[source->r][source->c];
                                    guesses[source->r][source->c] = temp;

                                    bool temp_mark = marked[row][col];
                                    marked[row][col] = marked[source->r][source->c];
                                    marked[source->r][source->c] = temp_mark;

                                    swaps_made++; // Lépés levonása

                                    // Szigorú 5 lépéses határ ellenőrzése
                                    if (swaps_made >= 5 && is_solved(code(secret_code), &guesses[0][0]) == 0) {
                                        generateGame();
                                    }
                                }
                                ImGui::EndDragDropTarget();
                            }
                        }

                        ImGui::PopStyleVar();
                        ImGui::PopStyleColor(3);
                }

                ImGui::PopID();
            }

            // Függőleges pálcikák kirajzolása (csak játék közben)
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            if (!in_menu) {
                response res = row_value(code(secret_code), row, &guesses[0][0]);
                drawTicks(res.black, res.white, true);
            } else {
                ImGui::Dummy(ImVec2(4 * 12.0f, 30.0f));
            }

            ImGui::PopID();
        }

        // --- 5. SOR: Vízszintes pálcikák és a Titkos Kapcsoló ---
        ImGui::TableNextRow();
        for (int col = 0; col < 4; ++col) {
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            if (!in_menu) {
                response res = col_value(code(secret_code), col, &guesses[0][0]);
                drawTicks(res.black, res.white, false);
            } else {
                ImGui::Dummy(ImVec2(30.0f, 4 * 12.0f));
            }
        }

        // Jobb alsó (5,5) cella
        ImGui::TableNextColumn();
        ImVec2 p = ImGui::GetCursorScreenPos();

        if (!in_menu) {
            // Lépésszámláló rács (2x2) rajzolása
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            int white_count = std::max(0, 4 - swaps_made);

            for (int i = 0; i < 4; ++i) {
                float dx = static_cast<float>(i % 2) * 14.0f;
                float dy = static_cast<float>(i / 2) * 14.0f;
                ImU32 square_col = (i < white_count) ? IM_COL32(240, 240, 240, 255) : IM_COL32(80, 80, 80, 255);

                draw_list->AddRectFilled(
                    ImVec2(p.x + dx + 10.0f, p.y + dy + 6.0f),
                                         ImVec2(p.x + dx + 20.0f, p.y + dy + 16.0f),
                                         square_col, 2.0f
                );
            }
        }

        // Láthatatlan Gomb a 2x2 mátrix (vagy üres hely) felett
        ImGui::InvisibleButton("MenuTrigger", ImVec2(40, 40));

        // Hosszú nyomás a menü eléréséhez
        if (ImGui::IsItemActive() && ImGui::GetIO().MouseDownDuration[0] > 0.6f) {
            in_menu = true;
        }

        // Dupla kattintás a visszalépéshez
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            in_menu = !in_menu;
        }

        ImGui::EndTable();
    }
    ImGui::End();
}
