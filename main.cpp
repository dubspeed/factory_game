#include <iostream>
#include "src/factory.h"
#include "src/tools/generators.h"
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


#include "nlohmann/json.hpp"

using namespace Fac;
using json = nlohmann::json;

volatile sig_atomic_t stop = 0;
auto fac = Factory();
char *save_file;

SDL_Window *window;
SDL_Renderer *renderer;


void saveWorld(Factory &w) {
    std::cout << "Saving world\n";
    std::ofstream current_save_file = std::ofstream(save_file);;
    json j = w;
    auto buffer = std::stringstream();
    buffer << j.dump(4);
    current_save_file << j.dump(4);
    current_save_file.flush();
    current_save_file.close();
    std::cout << "World saved\n";
}

void signal_handler(int signal) {
    stop = 1;
    std::cout << "Signal received: " << signal << std::endl;
    saveWorld(fac);
}


#define LIT(x) #x
#define CONNECTION(a, b) std::make_pair<std::shared_ptr<GameWorldEntity> const&, int>(a, b)
#define FROM_SLOT0(a) CONNECTION(a, 0)
#define FROM_SLOT1(a) CONNECTION(a, 1)
#define TO_SLOT0(a) CONNECTION(a, 0)
#define TO_SLOT1(a) CONNECTION(a, 1)
#define LINK(from_output, to_input) linkWithBelt(from_output, to_input)
#define LINK_T2(from_output, to_input) linkWithBelt(from_output, to_input, 120)
#define LINK_T3(from_output, to_input) linkWithBelt(from_output, to_input, 270)
#define LINK_T4(from_output, to_input) linkWithBelt(from_output, to_input, 480)
#define LINK_T5(from_output, to_input) linkWithBelt(from_output, to_input, 780)
#define LINK_T6(from_output, to_input) linkWithBelt(from_output, to_input, 1200)
#define CREATE(id, type) auto const id = create(fac, type()) ; id->name = "" LIT(id)
#define SMELTER(id, resource) CREATE(id, Machine); id->setRecipe(recipe_##resource)
#define CRAFTER(id, recipe) CREATE(id, Machine); id->setRecipe(recipe_##recipe)
#define ASSEMBLER(id, recipe) auto const id = create(fac, Machine(2, 1)) ; id->name = "" LIT(id) ; id->setRecipe(recipe_##recipe)

// Merger ans Spliiter have no tier, so they operate at each frame
#define SPLITTER(name) CREATE(name, Splitter) ; name->setItemsPerSecond(1000);
#define MERGER(name) CREATE(name, Merger) ; name->setItemsPerSecond(1000);
#define EXTRACTOR_T1(name, node) CREATE(name, Extractor); name->setResourceNode(node) ; name->setDefaultSpeed(60)
#define EXTRACTOR_T2(name, node) CREATE(name, Extractor); name->setResourceNode(node) ; name->setDefaultSpeed(120)
#define EXTRACTOR_T3(name, node) CREATE(name, Extractor); name->setResourceNode(node) ; name->setDefaultSpeed(240)
#define RESOURCE_NODE(name, resource, quality) CREATE(name, ResourceNode) ; name->setResource(Resource::resource); name->setResourceQuality(ResourceQuality::quality)
#define SMALL_STORAGE(name) CREATE(name, Storage) ; name->setMaxItemStacks(12)

typedef std::pair<const std::shared_ptr<GameWorldEntity>, int> Connection;

template<typename T>
    requires std::derived_from<T, GameWorldEntity>
auto create(Factory &w, T const &entity) {
    auto e = std::make_shared<T>(entity);
    w.addEntity(e);
    return e;
}

void connectInput(Connection const &from_input, Connection const &to_output) {
    const auto connector = std::dynamic_pointer_cast<IInputProvider>(from_input.first);
    connector->connectInput(from_input.second, to_output.first, to_output.second);
}

void linkWithBelt(Connection const &from_output, Connection const &to_input, int const rpm = 60) {
    CREATE(belt, Belt);
    belt->setItemsPerSecond(rpm / 60.0);
    connectInput(FROM_SLOT0(belt), from_output);
    connectInput(to_input, TO_SLOT0(belt));
}

void setupGameWorldSimple(Factory &w) {
    SMALL_STORAGE(plates);
    SMALL_STORAGE(screws);

    plates->manualAdd(100, Resource::IronPlate);
    screws->manualAdd(100, Resource::Screw);

    ASSEMBLER(reinforced_plate, ReinforcedIronPlate);

    LINK(FROM_SLOT0(plates), TO_SLOT0(reinforced_plate));
    LINK(FROM_SLOT0(screws), TO_SLOT1(reinforced_plate));

    SMALL_STORAGE(reinforced_plates_storage);
    LINK(FROM_SLOT0(reinforced_plate), TO_SLOT0(reinforced_plates_storage));

    std::cout << "Setup complete\n";
}


void setupGameWorldComplex(Factory &w) {
    RESOURCE_NODE(iron_node, IronOre, Normal);
    EXTRACTOR_T1(iron_extractor, iron_node);

    SPLITTER(sp1);
    LINK(FROM_SLOT0(iron_extractor), TO_SLOT0(sp1));

    // Goal is to make around 240 screws per minute
    // We need 2 smelters, 4 rod crafters, and 6 screws crafters

    CRAFTER(smelter1, IronIngot);
    LINK(FROM_SLOT0(sp1), TO_SLOT0(smelter1));

    CRAFTER(smelter2, IronIngot);
    LINK(FROM_SLOT1(sp1), TO_SLOT0(smelter2));

    MERGER(mg1);
    LINK(FROM_SLOT0(smelter1), TO_SLOT0(mg1));
    LINK(FROM_SLOT0(smelter2), TO_SLOT1(mg1));

    // start IRON_ ROD production, 4 rod crafters

    SPLITTER(sp4);
    LINK(FROM_SLOT0(mg1), TO_SLOT0(sp4));
    CRAFTER(rod1, IronRod);
    LINK(FROM_SLOT0(sp4), TO_SLOT0(rod1));

    SPLITTER(sp5);
    LINK(FROM_SLOT1(sp4), TO_SLOT0(sp5));
    CRAFTER(rod2, IronRod);
    LINK(FROM_SLOT0(sp5), TO_SLOT0(rod2));

    SPLITTER(sp6);
    LINK(FROM_SLOT1(sp5), TO_SLOT0(sp6));
    CRAFTER(rod3, IronRod);
    LINK(FROM_SLOT0(sp6), TO_SLOT0(rod3));
    CRAFTER(rod4, IronRod);
    LINK(FROM_SLOT1(sp6), TO_SLOT0(rod4));

    MERGER(mg3);
    LINK(FROM_SLOT0(rod1), TO_SLOT0(mg3));
    LINK(FROM_SLOT0(rod2), TO_SLOT1(mg3));

    MERGER(mg4);
    LINK(FROM_SLOT0(mg3), TO_SLOT0(mg4));
    LINK(FROM_SLOT0(rod3), TO_SLOT1(mg4));

    MERGER(mg5);
    LINK(FROM_SLOT0(mg4), TO_SLOT0(mg5));
    LINK(FROM_SLOT0(rod4), TO_SLOT1(mg5));

    // ROD production complete

    // SMALL_STORAGE(storage2);
    // LINK(FROM_SLOT0(mg5), TO_SLOT0(storage2));

    SPLITTER(screw_sp1);
    LINK(FROM_SLOT0(mg5), TO_SLOT0(screw_sp1));

    CRAFTER(screw1, Screw);
    LINK(FROM_SLOT0(screw_sp1), TO_SLOT0(screw1));

    SPLITTER(screw_sp2);
    LINK(FROM_SLOT1(screw_sp1), TO_SLOT0(screw_sp2));

    CRAFTER(screw2, Screw);
    LINK(FROM_SLOT0(screw_sp2), TO_SLOT0(screw2));

    SPLITTER(screw_sp3);
    LINK(FROM_SLOT1(screw_sp2), TO_SLOT0(screw_sp3));

    CRAFTER(screw3, Screw);
    LINK(FROM_SLOT0(screw_sp3), TO_SLOT0(screw3));

    SPLITTER(screw_sp4);
    LINK(FROM_SLOT1(screw_sp3), TO_SLOT0(screw_sp4));

    CRAFTER(screw4, Screw);
    LINK(FROM_SLOT0(screw_sp4), TO_SLOT0(screw4));

    SPLITTER(screw_sp5);
    LINK(FROM_SLOT1(screw_sp4), TO_SLOT0(screw_sp5));

    CRAFTER(screw5, Screw);
    LINK(FROM_SLOT0(screw_sp5), TO_SLOT0(screw5));

    SPLITTER(screw_sp6);
    LINK(FROM_SLOT1(screw_sp5), TO_SLOT0(screw_sp6));

    MERGER(screw_mg1);
    LINK(FROM_SLOT0(screw1), TO_SLOT0(screw_mg1));
    LINK(FROM_SLOT0(screw2), TO_SLOT1(screw_mg1));

    MERGER(screw_mg2);
    LINK_T3(FROM_SLOT0(screw_mg1), TO_SLOT0(screw_mg2));
    LINK(FROM_SLOT0(screw3), TO_SLOT1(screw_mg2));

    MERGER(screw_mg3);
    LINK_T3(FROM_SLOT0(screw_mg2), TO_SLOT0(screw_mg3));
    LINK(FROM_SLOT0(screw4), TO_SLOT1(screw_mg3));

    MERGER(screw_mg4);
    LINK_T3(FROM_SLOT0(screw_mg3), TO_SLOT0(screw_mg4));
    LINK(FROM_SLOT0(screw5), TO_SLOT1(screw_mg4));

    SMALL_STORAGE(storage3);
    LINK_T3(FROM_SLOT0(screw_mg4), TO_SLOT0(storage3));
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
        fac = j.get<Factory>();
        std::cout << "Loaded world from file\n";
        i.close();
    } else {
        // setupGameWorldSimple(fac);
        setupGameWorldComplex(fac);
        std::cout << "Created new world\n";
    }

    std::cout << "Running... (Press CTRL-D or CTRL-C to exit)\n";

    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    auto const startTime = Clock::now();

    if (setupImGui() != 0) {
        return 1;
    }

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // auto entities = w.getEntities();
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
        fac.processWorldStep();

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();


        ImGui::ShowDemoWindow();


        auto showFactoryList = []() {
            if (ImGui::BeginTable("table1", 7)) {
                for (auto &entity: fac.getEntities()) {
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
    }

    // Cleanup
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
