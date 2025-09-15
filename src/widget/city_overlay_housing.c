#include "city_overlay_housing.h"

#include "assets/assets.h"
#include "building/animation.h"
#include "building/building.h"
#include "building/industry.h"
#include "building/model.h"
#include "building/monument.h"
#include "building/roadblock.h"
#include "building/rotation.h"
#include "city/constants.h"
#include "city/finance.h"
#include "core/calc.h"
#include "core/config.h"
#include "core/lang.h"
#include "core/string.h"
#include "game/resource.h"
#include "game/state.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/text.h"
#include "map/bridge.h"
#include "map/building.h"
#include "map/desirability.h"
#include "map/image.h"
#include "map/property.h"
#include "map/random.h"
#include "map/terrain.h"
#include "scenario/property.h"
#include "translation/translation.h"
#include "widget/city_draw_highway.h"

#include <stdio.h>

static int show_figure_none(const figure *f)
{
    return 0;
}

static int get_column_height_none(const building *b)
{
    return NO_COLUMN;
}

static int show_building_small_test(const building *b) {
    return b->type == BUILDING_HOUSE_SMALL_TENT;
}

static int show_building_test(const building *b, int house_type) {
    return b->type == house_type;
}

const city_overlay *city_overlay_for_small_tent(void)
{
    static city_overlay overlay = {
        OVERLAY_HOUSE_SMALL_TENT,
        COLUMN_COLOR_GREEN,
        show_building_small_test,
        show_figure_none,
        get_column_height_none,
        0,
        0,
        0,
        0,
        0
    };
    return &overlay;
}

static int show_building_large_tent(const building *b) {
    return b->type == BUILDING_HOUSE_LARGE_TENT;
}

const city_overlay *city_overlay_for_large_tent(void)
{
    static city_overlay overlay = {
        OVERLAY_HOUSE_LARGE_TENT,
        COLUMN_COLOR_GREEN,
        show_building_large_tent,
        show_figure_none,
        get_column_height_none,
        0,
        0,
        0,
        0,
        0
    };
    return &overlay;
}