#include "province_building.h"

#include "assets/assets.h"
#include "empire/building.h"
#include "empire/build_spot.h"
#include "empire/building_type.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "input/scroll.h"
#include "window/empire/building_selection.h"

static int init()
{
    return 1;
}

static void select_building(int slot, int param2);

static generic_button province_buildings_buttons[] = {
    {90, 130, 160, 64, select_building, button_none, 0, 0},
    {255, 130, 160, 64, select_building, button_none, 1, 0},
    {420, 130, 160, 64, select_building, button_none, 2, 0},
};



static build_spot build_town = { {0,0,0} };

static int previous_button_focus_id = 0;
static int button_focus_id = 0;
static int permitted_buildings[] = { EMPIRE_BUILDING_WHARF,EMPIRE_BUILDING_WOODCUTTER,EMPIRE_BUILDING_WALLS, EMPIRE_BUILDING_HIGHWAY, EMPIRE_BUILDING_NONE };

static void select_building(int slot, int param2)
{
    show_building_selection_window(permitted_buildings, slot, &build_town);
}


static void draw_background(void)
{
    window_draw_underlying_window();
    graphics_in_dialog();
    outer_panel_draw(80, 80, 32, 14);
    inner_panel_draw(90, 130, 10, 4);
	inner_panel_draw(255, 130, 10, 4);
	inner_panel_draw(420, 130, 10, 4);
	for (int i = 0; i < 3; i++) {
		if (build_town.buildings[i]) {
			image_draw(empire_building_image(build_town.buildings[i]), province_buildings_buttons[i].x, province_buildings_buttons[i].y);
		}
	}
    text_draw_centered("Town Name", 80, 100, 480, FONT_LARGE_BLACK, 0);

    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();
    if (previous_button_focus_id != button_focus_id) {
        window_invalidate();

    }
    if (button_focus_id) {
        if (build_town.buildings[button_focus_id - 1]) {
            text_draw_centered(translation_for(empire_building_name(build_town.buildings[button_focus_id - 1])), 80, 250, 480, FONT_NORMAL_BLACK, 0);
            text_draw_centered(translation_for(empire_building_description(build_town.buildings[button_focus_id - 1])), 80, 280, 480, FONT_NORMAL_BLACK, 0);
        }
    }
    for (int i = 0; i < 3; i++) {
        button_border_draw(province_buildings_buttons[i].x, province_buildings_buttons[i].y, province_buildings_buttons[i].width, province_buildings_buttons[i].height, button_focus_id == i + 1);
    }
    graphics_reset_dialog();
}

static int handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);
    if (input_go_back_requested(m_dialog, h)) {
        window_go_back();
        return 1;
    }
    previous_button_focus_id = button_focus_id;
    generic_buttons_handle_mouse(m_dialog, 0, 0, province_buildings_buttons, 3, &button_focus_id);
    return 1;

}


void empire_province_building_show_window()
{
    if (init()) {
        window_type window = {
            WINDOW_BUILDTOWN,
            draw_background,
            draw_foreground,
            handle_input,
        };
        window_show(&window);
    }
}