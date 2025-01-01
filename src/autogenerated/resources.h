#ifndef RESOURCES_H
#define RESOURCES_H

#include <string_view>
namespace Fac {
enum class Resource {
	None,
	CopperOre,
	CopperIngot,
	IronOre,
	IronIngot,
	IronPlate,
	IronRod,
	Wire,
	Cable,
	Concrete,
	Screw,
	ReinforcedIronPlate,
	ModularFrame,
	Rotor,
	SmartPlating,
	SteelIngot,
	SteelPipe,
	VersatileFramework,
	SolidBiofuel,
	Leaves,
	Wood,
	Biomass,
	Limestone,
	CopperSheet,
	Stator,
	AutomatedWiring,
	SteelBeam,
	EncasedIndustrialBeam,
	Motor,
	Coal,
};
constexpr std::string_view resourceToString(Resource const r) {
	switch (r) {
		case Resource::None:
			return "None";
		case Resource::CopperOre:
			return "CopperOre";
		case Resource::CopperIngot:
			return "CopperIngot";
		case Resource::IronOre:
			return "IronOre";
		case Resource::IronIngot:
			return "IronIngot";
		case Resource::IronPlate:
			return "IronPlate";
		case Resource::IronRod:
			return "IronRod";
		case Resource::Wire:
			return "Wire";
		case Resource::Cable:
			return "Cable";
		case Resource::Concrete:
			return "Concrete";
		case Resource::Screw:
			return "Screw";
		case Resource::ReinforcedIronPlate:
			return "ReinforcedIronPlate";
		case Resource::ModularFrame:
			return "ModularFrame";
		case Resource::Rotor:
			return "Rotor";
		case Resource::SmartPlating:
			return "SmartPlating";
		case Resource::SteelIngot:
			return "SteelIngot";
		case Resource::SteelPipe:
			return "SteelPipe";
		case Resource::VersatileFramework:
			return "VersatileFramework";
		case Resource::SolidBiofuel:
			return "SolidBiofuel";
		case Resource::Leaves:
			return "Leaves";
		case Resource::Wood:
			return "Wood";
		case Resource::Biomass:
			return "Biomass";
		case Resource::Limestone:
			return "Limestone";
		case Resource::CopperSheet:
			return "CopperSheet";
		case Resource::Stator:
			return "Stator";
		case Resource::AutomatedWiring:
			return "AutomatedWiring";
		case Resource::SteelBeam:
			return "SteelBeam";
		case Resource::EncasedIndustrialBeam:
			return "EncasedIndustrialBeam";
		case Resource::Motor:
			return "Motor";
		case Resource::Coal:
			return "Coal";
	}
	return "None";
}
} // Fac
#endif //RESOURCES_H
