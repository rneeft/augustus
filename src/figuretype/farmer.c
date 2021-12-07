#include "farmer.h"

#include "building/building.h"
#include "building/farmhouse.h"
#include "building/image.h"
#include "city/resource.h"
#include "core/image.h"
#include "core/image_group.h"
#include "figure/combat.h"
#include "figure/image.h"
#include "figure/movement.h"
#include "figure/route.h"
#include "map/building_tiles.h"
#include "map/terrain.h"

#define FOOD_PER_PLOT 135

void figure_farmer_action(figure *f)
{
    f->terrain_usage = TERRAIN_USAGE_ANY;
    figure_image_increase_offset(f, 12);

    building *b = building_get(f->building_id);
    if (b->figure_id4 != f->id || b->state != BUILDING_STATE_IN_USE) {
        f->state = FIGURE_STATE_DEAD;
    }

    switch (f->action_state) {
        case FIGURE_ACTION_150_ATTACK:
            figure_combat_handle_attack(f);
            break;
        case FIGURE_ACTION_149_CORPSE:
            figure_combat_handle_corpse(f);
            break;
        case FIGURE_ACTION_231_FARMER_SPAWNED:
        {
            f->wait_ticks++;
            if (f->wait_ticks > 4) {

                building *field = building_farmhouse_find_harvestable_field(b);

                if (!field) {
                    field = building_farmhouse_find_workable_field(b);
                }

                if (field) {
                    f->destination_building_id = field->id;
                    f->destination_x = field->x;
                    f->destination_y = field->y;
                    f->action_state = FIGURE_ACTION_232_FARMER_GOING_TO_PLOT;
                    field->figure_id = f->id;
                    f->wait_ticks = 0;
                } else {
                    f->state = FIGURE_STATE_DEAD;
                }
            }
            break;
        }
        case FIGURE_ACTION_232_FARMER_GOING_TO_PLOT:
            figure_movement_move_ticks(f, 1);
            building *field = building_get(f->destination_building_id);
            if (!building_farmhouse_is_plot_type(field->type)) {
                f->state = FIGURE_STATE_DEAD;
            } else if (f->direction == DIR_FIGURE_AT_DESTINATION) {
                f->action_state = FIGURE_ACTION_233_FARMER_FARMING;
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            } else if (f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
            }
            break;

        case FIGURE_ACTION_233_FARMER_FARMING:
            f->wait_ticks++;
            if (f->wait_ticks >= 75) {
                building *field = building_get(f->destination_building_id);
                if (field->data.farm_plot.active) {
                    field->data.farm_plot.active = 0;
                    field->data.farm_plot.progress = 0;
                    f->resource_id = field->output_resource_id;
                    map_building_tiles_add(field->id, field->x, field->y, field->size, building_image_get(field), TERRAIN_BUILDING);
                } else {
                    field->data.farm_plot.active = 1;
                }
                field->figure_id = 0;
                f->action_state = FIGURE_ACTION_234_FARMER_RETURNING_TO_FARMHOUSE;
                f->wait_ticks = 0;
                f->destination_x = f->source_x;
                f->destination_y = f->source_y;
            }
            break;

        case FIGURE_ACTION_234_FARMER_RETURNING_TO_FARMHOUSE:
            figure_movement_move_ticks(f, 1);
            if (f->direction == DIR_FIGURE_AT_DESTINATION || f->direction == DIR_FIGURE_LOST) {
                f->state = FIGURE_STATE_DEAD;
                if (f->resource_id) {
                    b->data.granary.resource_stored[f->resource_id] += FOOD_PER_PLOT;
                }
            } else if (f->direction == DIR_FIGURE_REROUTE) {
                figure_route_remove(f);
            }
            break;
    }
    figure_image_update(f, image_group(GROUP_FIGURE_LABOR_SEEKER));
}
