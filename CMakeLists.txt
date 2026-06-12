cmake_minimum_required(VERSION 3.15)
project(Piazza VERSION 0.1.0 LANGUAGES CXX)

# Modern C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 1. Setup path to ImGui sources
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui)

# 2. Collect ImGui & Backend files
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

# 3. Létrehozzuk a futtatható fájlt (Ezt kell legelőször!)
add_executable(${PROJECT_NAME}
    main.cpp
    app.cpp
    engine.cpp
    ${IMGUI_SOURCES}
)

# 4. Include libraries
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# --- 5. PLATFORM SDL2 SETUP ---
if (EMSCRIPTEN)
    # Web Assembly (WASM) 
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
    target_compile_options(${PROJECT_NAME} PRIVATE "-sUSE_SDL=2")
    target_link_options(${PROJECT_NAME} PRIVATE 
        "-sUSE_SDL=2" 
        "-sWASM=1" 
        "-sALLOW_MEMORY_GROWTH=1"
        "-sASYNCIFY=1" # Ez engedi a while(isRunning) futását a böngészőben!
    )
else()
    # Asztali (Linux) fordítás hagyományos SDL2-vel
    find_package(SDL2 REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
endif()

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
)
