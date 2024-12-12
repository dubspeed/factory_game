#ifndef RECIPES_H
#define RECIPES_H

#include "resources.h"
#include "../core.h"
namespace Fac {
inline Recipe recipe_IronIngot = {
	.inputs = {
		{Resource::IronOre, 1},
	},
	.products = {
		{Resource::IronIngot, 1},
	},
	.processing_time_s = 2
};
inline Recipe recipe_IronPlate = {
	.inputs = {
		{Resource::IronIngot, 3},
	},
	.products = {
		{Resource::IronPlate, 2},
	},
	.processing_time_s = 6
};
inline Recipe recipe_IronRod = {
	.inputs = {
		{Resource::IronIngot, 1},
	},
	.products = {
		{Resource::IronRod, 1},
	},
	.processing_time_s = 4
};
inline Recipe recipe_Cable = {
	.inputs = {
		{Resource::Wire, 2},
	},
	.products = {
		{Resource::Cable, 1},
	},
	.processing_time_s = 2
};
inline Recipe recipe_Concrete = {
	.inputs = {
		{Resource::Limestone, 3},
	},
	.products = {
		{Resource::Concrete, 1},
	},
	.processing_time_s = 4
};
inline Recipe recipe_CopperIngot = {
	.inputs = {
		{Resource::CopperOre, 1},
	},
	.products = {
		{Resource::CopperIngot, 1},
	},
	.processing_time_s = 2
};
inline Recipe recipe_ReinforcedIronPlate = {
	.inputs = {
		{Resource::IronPlate, 6},
		{Resource::Screw, 12},
	},
	.products = {
		{Resource::ReinforcedIronPlate, 1},
	},
	.processing_time_s = 12
};
inline Recipe recipe_Screw = {
	.inputs = {
		{Resource::IronRod, 1},
	},
	.products = {
		{Resource::Screw, 4},
	},
	.processing_time_s = 6
};
inline Recipe recipe_CopperSheet = {
	.inputs = {
		{Resource::CopperIngot, 2},
	},
	.products = {
		{Resource::CopperSheet, 1},
	},
	.processing_time_s = 6
};
inline Recipe recipe_ModularFrame = {
	.inputs = {
		{Resource::ReinforcedIronPlate, 3},
		{Resource::IronRod, 12},
	},
	.products = {
		{Resource::ModularFrame, 2},
	},
	.processing_time_s = 60
};
inline Recipe recipe_Rotor = {
	.inputs = {
		{Resource::IronRod, 5},
		{Resource::Screw, 25},
	},
	.products = {
		{Resource::Rotor, 1},
	},
	.processing_time_s = 15
};
inline Recipe recipe_SolidBiofuel = {
	.inputs = {
		{Resource::Biomass, 8},
	},
	.products = {
		{Resource::SolidBiofuel, 4},
	},
	.processing_time_s = 4
};
inline Recipe recipe_AutomatedWiring = {
	.inputs = {
		{Resource::Stator, 1},
		{Resource::Cable, 20},
	},
	.products = {
		{Resource::AutomatedWiring, 1},
	},
	.processing_time_s = 24
};
inline Recipe recipe_EncasedIndustrialBeam = {
	.inputs = {
		{Resource::SteelBeam, 3},
		{Resource::Concrete, 6},
	},
	.products = {
		{Resource::EncasedIndustrialBeam, 1},
	},
	.processing_time_s = 10
};
inline Recipe recipe_Motor = {
	.inputs = {
		{Resource::Rotor, 2},
		{Resource::Stator, 2},
	},
	.products = {
		{Resource::Motor, 1},
	},
	.processing_time_s = 12
};
inline Recipe recipe_Stator = {
	.inputs = {
		{Resource::SteelPipe, 3},
		{Resource::Wire, 8},
	},
	.products = {
		{Resource::Stator, 1},
	},
	.processing_time_s = 12
};
} // Fac
#endif //RECIPES_H
