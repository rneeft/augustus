#include "farmhouse.h"

#include "building/building.h"
#include "city/resource.h"
#include "map/building.h"
#include "map/grid.h"

#define FARMHOUSE_RANGE 3
#define FARM_CAPACITY 800

int building_farmhouse_is_plot_type(building_type type)
{
	switch (type) {
		case BUILDING_WHEAT_PLOT:
			return 1;
		default:
			return 0;
			break;
	}
}

int building_farmhouse_is_full(building *farm) {
    int total_stored = 0;
    for (int resource = RESOURCE_WHEAT; resource < RESOURCE_MAX_FOOD; resource++) {
        total_stored += farm->data.granary.resource_stored[resource];
    }
    return total_stored >= FARM_CAPACITY;
}

resource_type building_farmhouse_most_food_of_type(building *farm)
{
    int max_stored = 0;
    int max_resource = RESOURCE_NONE;
    for (int resource = RESOURCE_WHEAT; resource < RESOURCE_MAX_FOOD; resource++) {
        if (farm->data.granary.resource_stored[resource] > max_stored) {
            max_stored = farm->data.granary.resource_stored[resource];
            max_resource = resource;
        } 
    }
    return max_resource;
}


building *building_farmhouse_find_workable_field(building *farm)
{
    if (farm->type != BUILDING_FARMHOUSE) {
        return 0;
    }

    for (int radius = 1; radius <= FARMHOUSE_RANGE; radius++) {
        int x_min, y_min, x_max, y_max;
        map_grid_get_area(farm->x, farm->y, farm->size, radius, &x_min, &y_min, &x_max, &y_max);

        for (int yy = y_min; yy <= y_max; yy++) {
            for (int xx = x_min; xx <= x_max; xx++) {
                int grid_offset = map_grid_offset(xx, yy);
                building *b = building_get(map_building_at(grid_offset));

                if (!b || !building_farmhouse_is_plot_type(b->type)) {
                    continue;
                }

                if (b->data.farm_plot.active) {
                    continue;
                }

                if (b->figure_id) {
                    continue;
                }
                return b;
            }
        }
    }
    return 0;
}

building *building_farmhouse_find_harvestable_field(building *farm)
{
    if (farm->type != BUILDING_FARMHOUSE) {
        return 0;
    }

    if (building_farmhouse_is_full(farm)) {
        return 0;
    }

    for (int radius = 1; radius <= FARMHOUSE_RANGE; radius++) {
        int x_min, y_min, x_max, y_max;
        map_grid_get_area(farm->x, farm->y, farm->size, radius, &x_min, &y_min, &x_max, &y_max);

        for (int yy = y_min; yy <= y_max; yy++) {
            for (int xx = x_min; xx <= x_max; xx++) {
                int grid_offset = map_grid_offset(xx, yy);
                building *b = building_get(map_building_at(grid_offset));

                if (!b || !building_farmhouse_is_plot_type(b->type)) {
                    continue;
                }

                if (b->data.farm_plot.progress < FARM_PLOT_TICKS_MAX) {
                    continue;
                }

                if (b->figure_id) {
                    continue;
                }

                return b;
            }
        }
    }
    return 0;
}



