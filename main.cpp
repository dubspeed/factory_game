#include <iostream>
#include "src/factory.h"
#include "src/tools/generators.h"
#include <csignal>
#include <thread>
#include <unistd.h>
#include <chrono>

#include "nlohmann/json.hpp"

using namespace Fac;
using json = nlohmann::json;

volatile sig_atomic_t stop = 0;
auto fac = Factory();
char *save_file;



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
#define CRAFTER(name, recipe) CREATE(name, Machine); name->setRecipe(recipe_##recipe)
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
    RESOURCE_NODE(iron_node, IronOre, Pure);
    EXTRACTOR_T3(iron_extractor, iron_node);

    SPLITTER(sp1);
    LINK_T4(FROM_SLOT0(iron_extractor), TO_SLOT0(sp1));

    MERGER(mg1);
    LINK_T4(FROM_SLOT0(sp1), TO_SLOT0(mg1));

    SMALL_STORAGE(storage1);
    LINK_T4(FROM_SLOT0(mg1), TO_SLOT0(storage1));
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
        // setupGameWorldComplex(w);
        setupGameWorldComplex(fac);
        std::cout << "Created new world\n";
    }

    std::cout << "Running... (Press CTRL-D or CTRL-C to exit)\n";

    std::string CSI = "\u001B[";
    auto clear = CSI + "2J";
    std::cout << CSI + "s";
    std::cout << std::setprecision(0) << std::fixed;

    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    auto const startTime = Clock::now();


    // auto entities = w.getEntities();
    while (!stop) {
        std::cout << CSI + "2J";
        auto const currentTime = Clock::now();
        std::chrono::duration<double> const elapsed = currentTime - startTime;
        std::cout << "Time: " << elapsed.count() << "s\n";
        fac.processWorldStep();
        for (auto &entity: fac.getEntities()) {
            if (auto m = std::dynamic_pointer_cast<Machine>(entity); m) {
                std::cout << "Mach:" << std::setw(2) << m->getId() << " " << m->name;
                std::cout << "/I1:" << m->getInputStack(1)->getAmount();
                std::cout << "/I0:" << m->getInputStack(0)->getAmount();
                std::cout << "/O0:" << m->getOutputStack(0)->getAmount();
                std::cout << "/P:" << m->processing;
                std::cout << "/PPM:" << m->getInputRpm();
                std::cout << "/T:" << m->getRecipe().value().processing_time_s * 1000 - m->processing_progress;
                std::cout << std::endl;
            }
            // if (auto m = std::dynamic_pointer_cast<Belt>(entity); m) {
            //     std::cout << "Belt:" << std::setw(2) << m->getId() << " " << m->name;
            //     std::cout << "/TR:" << m->_in_transit_stack.size();
            //     std::cout << "/I0:" << m->getInputStack(0)->getAmount();
            //     std::cout << "/O0:" << m->getOutputStack(0)->getAmount();
            //     std::cout << "/J:" << m->getJammed();
            //     std::cout << "/PPM:" << m->getOutputRpm();
            //     std::cout << std::endl;
            // }
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
            if (auto m = std::dynamic_pointer_cast<Extractor>(entity); m) {
                std::cout << "Extr:" << std::setw(2) << m->getId() << " " << m->name;
                std::cout << "/O0:" << m->getOutputStack(0)->getAmount();
                std::cout << "/EX:" << m->extracting;
                std::cout << "/PPM:" << m->getOutputRpm();
                std::cout << std::endl;
            }
            if (auto m = std::dynamic_pointer_cast<Storage>(entity); m) {
                std::cout << "Stor:" << std::setw(2) << m->getId() << " " << m->name;
                std::cout << "/Ore:" << m->getAmount(Resource::IronOre);
                std::cout << "/Iron:" << m->getAmount(Resource::IronIngot);
                std::cout << "/Rods:" << m->getAmount(Resource::IronRod);
                std::cout << "/Screw:" << m->getAmount(Resource::Screw);
                // std::cout << "/Cop:" << m->getAmount(Resource::CopperOre);
                std::cout << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }


    return 0;
}
