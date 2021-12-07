#ifndef BUILDING_FARMHOUSE_H
#define BUILDING_FARMHOUSE_H

#include "building/building.h"
#include "city/resource.h"

#define FARM_PLOT_TICKS_MAX 10000

int building_farmhouse_is_plot_type(building_type type);
building *building_farmhouse_find_workable_field(building *farm);
building *building_farmhouse_find_harvestable_field(building *farm);
resource_type building_farmhouse_most_food_of_type(building *farm);

#endif // BUILDING_FARMHOUSE_H
