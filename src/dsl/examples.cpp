
#include "dsl.h"
#include "examples.h"

using namespace DSL;
using namespace Fac;

void Examples::simpleFactory(std::shared_ptr<Factory> const &factory) {
    SET_FACTORY(factory);
    SMALL_STORAGE(plates);
    SMALL_STORAGE(screws);

    plates->manualAdd(100, Resource::IronPlate);
    screws->manualAdd(100, Resource::Screw);

    ASSEMBLER(reinforced_plate, ReinforcedIronPlate);

    LINK_T1(FROM_SLOT0(plates), TO_SLOT0(reinforced_plate));
    LINK_T1(FROM_SLOT0(screws), TO_SLOT1(reinforced_plate));

    SMALL_STORAGE(reinforced_plates_storage);
    LINK_T1(FROM_SLOT0(reinforced_plate), TO_SLOT0(reinforced_plates_storage));

    std::cout << "Setup complete\n";
}


void Examples::complexFactory(std::shared_ptr<Factory> const &factory) {
    SET_FACTORY(factory);
    RESOURCE_NODE(iron_node, IronOre, Normal);
    EXTRACTOR_T1(iron_extractor, iron_node);

    SPLITTER(sp1);
    LINK_T1(FROM_SLOT0(iron_extractor), TO_SLOT0(sp1));

    // Goal is to make around 240 screws per minute
    // We need 2 smelters, 4 rod crafters, and 6 screws crafters

    CRAFTER(smelter1, IronIngot);
    LINK_T1(FROM_SLOT0(sp1), TO_SLOT0(smelter1));

    CRAFTER(smelter2, IronIngot);
    LINK_T1(FROM_SLOT1(sp1), TO_SLOT0(smelter2));

    MERGER(mg1);
    LINK_T1(FROM_SLOT0(smelter1), TO_SLOT0(mg1));
    LINK_T1(FROM_SLOT0(smelter2), TO_SLOT1(mg1));

    // start IRON_ ROD production, 4 rod crafters

    SPLITTER(sp4);
    LINK_T1(FROM_SLOT0(mg1), TO_SLOT0(sp4));
    CRAFTER(rod1, IronRod);
    LINK_T1(FROM_SLOT0(sp4), TO_SLOT0(rod1));

    SPLITTER(sp5);
    LINK_T1(FROM_SLOT1(sp4), TO_SLOT0(sp5));
    CRAFTER(rod2, IronRod);
    LINK_T1(FROM_SLOT0(sp5), TO_SLOT0(rod2));

    SPLITTER(sp6);
    LINK_T1(FROM_SLOT1(sp5), TO_SLOT0(sp6));
    CRAFTER(rod3, IronRod);
    LINK_T1(FROM_SLOT0(sp6), TO_SLOT0(rod3));
    CRAFTER(rod4, IronRod);
    LINK_T1(FROM_SLOT1(sp6), TO_SLOT0(rod4));

    MERGER(mg3);
    LINK_T1(FROM_SLOT0(rod1), TO_SLOT0(mg3));
    LINK_T1(FROM_SLOT0(rod2), TO_SLOT1(mg3));

    MERGER(mg4);
    LINK_T1(FROM_SLOT0(mg3), TO_SLOT0(mg4));
    LINK_T1(FROM_SLOT0(rod3), TO_SLOT1(mg4));

    MERGER(mg5);
    LINK_T1(FROM_SLOT0(mg4), TO_SLOT0(mg5));
    LINK_T1(FROM_SLOT0(rod4), TO_SLOT1(mg5));

    // ROD production complete

    // SMALL_STORAGE(storage2);
    // LINK(FROM_SLOT0(mg5), TO_SLOT0(storage2));

    SPLITTER(screw_sp1);
    LINK_T1(FROM_SLOT0(mg5), TO_SLOT0(screw_sp1));

    CRAFTER(screw1, Screw);
    LINK_T1(FROM_SLOT0(screw_sp1), TO_SLOT0(screw1));

    SPLITTER(screw_sp2);
    LINK_T1(FROM_SLOT1(screw_sp1), TO_SLOT0(screw_sp2));

    CRAFTER(screw2, Screw);
    LINK_T1(FROM_SLOT0(screw_sp2), TO_SLOT0(screw2));

    SPLITTER(screw_sp3);
    LINK_T1(FROM_SLOT1(screw_sp2), TO_SLOT0(screw_sp3));

    CRAFTER(screw3, Screw);
    LINK_T1(FROM_SLOT0(screw_sp3), TO_SLOT0(screw3));

    SPLITTER(screw_sp4);
    LINK_T1(FROM_SLOT1(screw_sp3), TO_SLOT0(screw_sp4));

    CRAFTER(screw4, Screw);
    LINK_T1(FROM_SLOT0(screw_sp4), TO_SLOT0(screw4));

    SPLITTER(screw_sp5);
    LINK_T1(FROM_SLOT1(screw_sp4), TO_SLOT0(screw_sp5));

    CRAFTER(screw5, Screw);
    LINK_T1(FROM_SLOT0(screw_sp5), TO_SLOT0(screw5));

    SPLITTER(screw_sp6);
    LINK_T1(FROM_SLOT1(screw_sp5), TO_SLOT0(screw_sp6));

    MERGER(screw_mg1);
    LINK_T1(FROM_SLOT0(screw1), TO_SLOT0(screw_mg1));
    LINK_T1(FROM_SLOT0(screw2), TO_SLOT1(screw_mg1));

    MERGER(screw_mg2);
    LINK_T3(FROM_SLOT0(screw_mg1), TO_SLOT0(screw_mg2));
    LINK_T1(FROM_SLOT0(screw3), TO_SLOT1(screw_mg2));

    MERGER(screw_mg3);
    LINK_T3(FROM_SLOT0(screw_mg2), TO_SLOT0(screw_mg3));
    LINK_T1(FROM_SLOT0(screw4), TO_SLOT1(screw_mg3));

    MERGER(screw_mg4);
    LINK_T3(FROM_SLOT0(screw_mg3), TO_SLOT0(screw_mg4));
    LINK_T1(FROM_SLOT0(screw5), TO_SLOT1(screw_mg4));

    SMALL_STORAGE(storage3);
    LINK_T3(FROM_SLOT0(screw_mg4), TO_SLOT0(storage3));
}
