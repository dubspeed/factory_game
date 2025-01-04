#include <iostream>
#include "src/factory.h"
#include "src/tools/generators.h"
#include "src/game/game.h"
#include <csignal>
#include <thread>
#include <unistd.h>
#include <chrono>
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#include <stdio.h>
#include <SDL3/SDL.h>
#include <string>
#include "src/tools/defer.h"
#include "nlohmann/json.hpp"
#include "src/dsl/examples.h"

using namespace Fac;
using json = nlohmann::json;

volatile sig_atomic_t stop = 0;
auto gameState = GameState();
char *save_file;

SDL_Window *window;
SDL_Renderer *renderer;


void saveWorld() {
    std::cout << "Saving world\n";
    auto current_save_file = std::ofstream(save_file);;
    auto buffer = std::stringstream();

    json j = gameState;
    buffer << j.dump(4);
    std::cout << buffer.str() << std::endl;
    current_save_file << buffer.str() << std::endl;
    current_save_file.flush();
    current_save_file.close();
    std::cout << "World saved\n";
}

void signal_handler(int const signal) {
    stop = 1;
    std::cout << "Signal received: " << signal << std::endl;
    saveWorld();
}


int setupImGui() {
    // Setup SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return -1;
    }

    // Create window with SDL_Renderer graphics context
    Uint32 window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
    window = SDL_CreateWindow("Dear ImGui SDL3+SDL_Renderer example", 1280, 720, window_flags);
    if (window == nullptr) {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }
    renderer = SDL_CreateRenderer(window, nullptr);
    SDL_SetRenderVSync(renderer, 1);
    if (renderer == nullptr) {
        SDL_Log("Error: SDL_CreateRenderer(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    ImFont *font = io.Fonts->AddFontFromFileTTF("../fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    IM_ASSERT(font != nullptr);
    return 0;
}


std::string camelCaseToSpaced(std::string text) {
    std::string result;
    result.reserve(text.length() + 5); // Reserve some extra space for potential spaces

    for (size_t i = 0; i < text.length(); ++i) {
        if (i > 0 && std::isupper(text[i]) && !std::isupper(text[i - 1])) {
            result += ' ';
        }
        result += text[i];
    }

    return result;
}


int main(int argc, char *argv[]) {
    std::signal(SIGINT, signal_handler); // CTRL-C
    std::signal(SIGTERM, signal_handler); // Termination request

    // first argument must be a filename for the save file
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    save_file = argv[1];
    std::cout << "Save file: " << save_file << std::endl;

    // try to read the world as json from the safe file, if not continue
    if (std::ifstream i(save_file); i) {
        json j;
        i >> j;
        gameState = j.get<GameState>();
        std::cout << "Loaded world from file\n";
        i.close();
    } else {
        // setupGameWorldSimple();
        gameState.factories.push_back(std::make_shared<Factory>());
        // Examples::complexFactory(gameState.factories.front());
        Examples::simpleFactory(gameState.factories.front());
        std::cout << "Created new world\n";
    }

    std::cout << "Running... (Press CTRL-D or CTRL-C to exit)\n";

    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    auto const startTime = Clock::now();

    if (setupImGui() != 0) {
        return 1;
    }

    constexpr auto clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    auto gameWindowViewModel = GameWindowViewModel(gameState, gameState.factories.front());
    auto gameWindow = GameWindow(gameWindowViewModel);

    while (!stop) {
        auto const currentTime = Clock::now();
        std::chrono::duration<double> const elapsed = currentTime - startTime;

        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                stop = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
                stop = true;
        }
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
            SDL_Delay(10);
            continue;
        }

        // Factory logic
        for (const auto &factory : gameState.factories) {
            factory->processWorldStep();
        }

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();


        ImGui::ShowDemoWindow();

        gameWindow.render();

        auto showFactoryList = []() {
            if (ImGui::BeginTable("table1", 7)) {
                for (auto &entity: gameState.factories.front()->getEntities()) {
                    // if (auto gameEntity = std::dynamic_pointer_cast<GameWorldEntity>(entity)) {
                    //     ImGui::Text(std::format("Entity: {0} {1}", entity->getId(), entity->name).c_str());
                    //     ImGui::Separator();
                    // }
                    ImGui::TableNextRow();
                    if (const auto m = std::dynamic_pointer_cast<Machine>(entity); m) {
                        ImGui::TableNextColumn();
                        ImGui::Checkbox(std::format("##{0}", m->getId()).c_str(), &m->processing);
                        ImGui::TableNextColumn();
                        ImGui::ProgressBar(m->processing_progress / (m->getRecipe().value().processing_time_s * 1000));
                        ImGui::TableNextColumn();
                        auto resourceName = camelCaseToSpaced(
                            resourceToString(m->getRecipe().value().products.front().resource).data());
                        ImGui::Text(resourceName.c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text(std::format("Mach: {0} {1}", m->getId(), m->name).c_str());
                        for (int i = 0; i < m->getInputSlots(); i++) {
                            ImGui::TableNextColumn();
                            ImGui::Text(std::format("I{0}: {1}", i, m->getInputStack(i)->getAmount()).c_str());
                        }
                        for (int i = 0; i < m->getOutputSlots(); i++) {
                            ImGui::TableNextColumn();
                            ImGui::Text(std::format("O{0}: {1}", i, m->getOutputStack(i)->getAmount()).c_str());
                        }
                        ImGui::TableNextColumn();
                        // ImGui::Text(std::format("P: {0}", m->processing).c_str());
                        ImGui::Text(std::format("PPM: {0}", m->getInputRpm()).c_str());

                        // ImGui::Text(std::format("T: {0}", m->getRecipe().value().processing_time_s * 1000 - m->processing_progress).c_str());
                    }
                }
                ImGui::EndTable();
            }
        };


        ImGui::Begin("Factory Overview");

        ImGui::Text(std::format("Time: {0}s", elapsed.count()).c_str());

        if (ImGui::BeginTabBar("##tabs")) {
            if (ImGui::BeginTabItem("All")) {
                showFactoryList();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Machines")) {
                ImGui::Text("Machines");
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        };


        ImGui::End();

        //     if (auto m = std::dynamic_pointer_cast<Machine>(entity); m) {
        //         std::cout << "Mach:" << std::setw(2) << m->getId() << " " << m->name;
        //         for (int i = 0; i < m->getInputSlots(); i++) {
        //             std::cout << "/I" << i << ":" << m->getInputStack(i)->getAmount();
        //         }
        //         for (int i = 0; i < m->getOutputSlots(); i++) {
        //             std::cout << "/O" << i << ":" << m->getOutputStack(i)->getAmount();
        //         }
        //         std::cout << "/P:" << m->processing;
        //         std::cout << "/PPM:" << m->getInputRpm();
        //         std::cout << "/T:" << m->getRecipe().value().processing_time_s * 1000 - m->processing_progress;
        //         std::cout << std::endl;
        //     }
        //     if (auto m = std::dynamic_pointer_cast<Belt>(entity); m) {
        //         std::cout << "Belt:" << std::setw(2) << m->getId() << " " << m->name;
        //         std::cout << "/TR:" << m->_in_transit_stack.size();
        //         std::cout << "/I0:" << m->getInputStack(0)->getAmount();
        //         std::cout << "/O0:" << m->getOutputStack(0)->getAmount();
        //         std::cout << "/J:" << m->getJammed();
        //         std::cout << "/PPM:" << m->getOutputRpm();
        //         std::cout << std::endl;
        //     }
        // if (auto m = std::dynamic_pointer_cast<Splitter>(entity); m) {
        //     std::cout << "Spli:" << std::setw(2) << m->getId() << " " << m->name;
        //     std::cout << "/TR:" << m->_in_transit_stack.size();
        //     std::cout << "/I0:" << m->getInputStack(0)->getAmount();
        //     std::cout << "/O0:" << m->getOutputStack(0)->getAmount();
        //     std::cout << "/O1:" << m->getOutputStack(1)->getAmount();
        //     std::cout << "/PPM:" << m->getOutputRpm();
        //     std::cout << "/J:" << m->getJammed();
        //     std::cout << std::endl;
        // }
        // if (auto m = std::dynamic_pointer_cast<Merger>(entity); m) {
        //     std::cout << "Merg:" << std::setw(2) << m->getId() << " " << m->name;
        //     std::cout << "/TR:" << m->_in_transit_stack.size();
        //     std::cout << "/I0:" << m->getInputStack(0)->getAmount();
        //     std::cout << "/I1:" << m->getInputStack(1)->getAmount();
        //     std::cout << "/O0:" << m->getOutputStack(0)->getAmount();
        //     std::cout << "/PPM:" << m->getOutputRpm();
        //     std::cout << "/J:" << m->getJammed();
        //     std::cout << std::endl;
        // }
        // if (auto m = std::dynamic_pointer_cast<Extractor>(entity); m) {
        //     std::cout << "Extr:" << std::setw(2) << m->getId() << " " << m->name;
        //     std::cout << "/O0:" << m->getOutputStack(0)->getAmount();
        //     std::cout << "/EX:" << m->extracting;
        //     std::cout << "/PPM:" << m->getOutputRpm();
        //     std::cout << std::endl;
        // }
        // if (auto m = std::dynamic_pointer_cast<Storage>(entity); m) {
        //     std::cout << "Stor:" << std::setw(2) << m->getId() << " " << m->name;
        //     std::cout << "/Ore:" << m->getAmount(Resource::IronOre);
        //     std::cout << "/Iron:" << m->getAmount(Resource::IronIngot);
        //     std::cout << "/Rods:" << m->getAmount(Resource::IronRod);
        //     std::cout << "/Screw:" << m->getAmount(Resource::Screw);
        //     std::cout << "/Plates:" << m->getAmount(Resource::IronPlate);
        //     std::cout << "/RePlates:" << m->getAmount(Resource::ReinforcedIronPlate);
        //     // std::cout << "/Cop:" << m->getAmount(Resource::CopperOre);
        //     std::cout << std::endl;
        // }
        // }

        // test deletion of things, by deleting something random every second or so (it the doubles match up)
        // int count = static_cast<int>(elapsed.count() * 1000.0);
        // if (count % 1000 == 0) {
        //     // if (elapsed.count() == 10 || elapsed.count() == 20 || elapsed.count() == 30) {
        //     if (fac.getEntities().size() > 0) {
        //         auto random_entity = fac.getEntities()[rand() % fac.getEntities().size()];
        //
        //         fac.removeEntity(random_entity);
        //     }
        // }
        //
        // if (count > 10000) {
        //     saveWorld(fac);
        //     break;
        // }
        // std::this_thread::sleep_for(std::chrono::milliseconds(10));
        // Rendering
        ImGui::Render();
        //SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);

        // This will execute all queued calls outside frame rendering
        deferCall(nullptr);
    }

    // Cleanup
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    saveWorld();

    return 0;
}
