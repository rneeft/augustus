#include "building_selection.h"

#include "assets/assets.h"
#include "empire/building.h"
#include "empire/building_type.h"
#include "empire/build_spot.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/screen.h"
#include "graphics/scrollbar.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "input/scroll.h"
#include "input/scroll.h"

#define MAX_BUILDINGS 10

static void build_building(int button_number, int param2);


static generic_button building_selection_buttons[] = {
    {90, 130, 160, 64, build_building, button_none, 0, 0},
    {255, 130, 160, 64, build_building, button_none, 1, 0},
    {420, 130, 160, 64, build_building, button_none, 2, 0},
};

static int permitted_buildings[10] = { 0 };
static int building_types = 0;
static int slot_number = 0;
static int previous_button_focus_id = 0;
static int button_focus_id = 0;
static build_spot *build_town;

static void on_scroll(void);

static scrollbar_type scrollbar = { 578, 88, 208, on_scroll };

static void build_building(int button_number, int param2)
{
    build_town->buildings[slot_number] = permitted_buildings[button_number+scrollbar.scroll_position];
    window_go_back();
    window_invalidate();
}

static void on_scroll(void)
{
    window_invalidate();
}

static void clear_permitted_buildings(void)
{
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        permitted_buildings[i] = EMPIRE_BUILDING_NONE;
    }
    building_types = 0;
}

static int init(int allowed_buildings[], int slot, build_spot *spot)
{
    clear_permitted_buildings();
    slot_number = slot;
    build_town = spot;
    building_types = MAX_BUILDINGS;
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        permitted_buildings[i] = allowed_buildings[i];
        if (allowed_buildings[i] == EMPIRE_BUILDING_NONE) {
            building_types = i;
            break;
        }
    }
    scrollbar_init(&scrollbar, 0, building_types - 3);
    return 1;
}


static void draw_background(void)
{
    window_draw_underlying_window();
    graphics_in_dialog();
    outer_panel_draw(80, 80, 34, 14);
    inner_panel_draw(90, 130, 10, 4);
    inner_panel_draw(255, 130, 10, 4);
    inner_panel_draw(420, 130, 10, 4);

    text_draw_centered("Select a building", 80, 100, 480, FONT_LARGE_BLACK, 0);
    for (int i = 0; i < building_types && i < 3 ; i++) {
        image_draw(empire_building_image(permitted_buildings[i+scrollbar.scroll_position]), 95+165*i, 135);
    }
    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    graphics_in_dialog();
    if (previous_button_focus_id != button_focus_id) {
        window_invalidate();
    }
    scrollbar_draw(&scrollbar);
    if (button_focus_id) {
        int building_selected = permitted_buildings[button_focus_id + scrollbar.scroll_position - 1];
        if (building_selected) {
            text_draw_centered(translation_for(empire_building_name(building_selected)), 80, 250, 480, FONT_NORMAL_BLACK, 0);
            text_draw_centered(translation_for(empire_building_description(building_selected)), 80, 280, 480, FONT_NORMAL_BLACK, 0);
        }
    }
    for (int i = 0; i < 3; i++) {
        button_border_draw(building_selection_buttons[i].x, building_selection_buttons[i].y, building_selection_buttons[i].width, building_selection_buttons[i].height, button_focus_id == i + 1);
    }
    graphics_reset_dialog();
}

static int handle_input(const mouse *m, const hotkeys *h)
{
    const mouse *m_dialog = mouse_in_dialog(m);

    if (scrollbar_handle_mouse(&scrollbar, m_dialog)) {
        return 1;
    }
    if (input_go_back_requested(m_dialog, h)) {
        window_go_back();
        return 1;
    }
    previous_button_focus_id = button_focus_id;
    generic_buttons_handle_mouse(m_dialog, 0, 0, building_selection_buttons, 3, &button_focus_id);
    return 0;
}


void show_building_selection_window(int allowed_buildings[], int slot, build_spot *spot)
{
    if (init(allowed_buildings, slot, spot)) {
        window_type window = {
            WINDOW_BUILDTOWN,
            draw_background,
            draw_foreground,
            handle_input,
        };
        window_show(&window);
    }
}