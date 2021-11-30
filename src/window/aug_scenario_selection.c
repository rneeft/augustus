#include "cck_selection.h"

#include "core/dir.h"
#include "core/encoding.h"
#include "core/file.h"
#include "core/image_group.h"
#include "game/file.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/image.h"
#include "graphics/image_button.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/scrollbar.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "input/input.h"
#include "scenario/criteria.h"
#include "scenario/invasion.h"
#include "scenario/map.h"
#include "scenario/property.h"
#include "sound/music.h"
#include "window/city.h"

#include <string.h>

#define MAX_SCENARIOS 15
static const char* SCENARIOS_FOLDER = "scenarios";

static void button_select_item(int index, int param2);
static void button_start_scenario(int param1, int param2);
static void on_scroll(void);

static image_button start_button =
{ 600, 440, 27, 27, IB_NORMAL, GROUP_SIDEBAR_BUTTONS, 56, button_start_scenario, button_none, 1, 0, 1 };

static generic_button file_buttons[] = {
    {18, 220, 252, 16, button_select_item, button_none, 0, 0},
    {18, 236, 252, 16, button_select_item, button_none, 1, 0},
    {18, 252, 252, 16, button_select_item, button_none, 2, 0},
    {18, 268, 252, 16, button_select_item, button_none, 3, 0},
    {18, 284, 252, 16, button_select_item, button_none, 4, 0},
    {18, 300, 252, 16, button_select_item, button_none, 5, 0},
    {18, 316, 252, 16, button_select_item, button_none, 6, 0},
    {18, 332, 252, 16, button_select_item, button_none, 7, 0},
    {18, 348, 252, 16, button_select_item, button_none, 8, 0},
    {18, 364, 252, 16, button_select_item, button_none, 9, 0},
    {18, 380, 252, 16, button_select_item, button_none, 10, 0},
    {18, 396, 252, 16, button_select_item, button_none, 11, 0},
    {18, 412, 252, 16, button_select_item, button_none, 12, 0},
    {18, 428, 252, 16, button_select_item, button_none, 13, 0},
    {18, 444, 252, 16, button_select_item, button_none, 14, 0},
};

static scrollbar_type scrollbar = { 276, 210, 256, on_scroll, 8, 1 };

static struct {
    int focus_button_id;
    int selected_item;
    char selected_scenario_filename[FILE_NAME_MAX];
    char selected_scenario_map_filename[FILE_NAME_MAX];
    uint8_t selected_scenario_display[FILE_NAME_MAX];

    const dir_listing* aug_scenarios;
} data;

static void init(void)
{
    scenario_set_custom(2);
    const dir_listing *scenario_author_dirs = dir_find_all_subdirectories_for_dir(SCENARIOS_FOLDER, 0);
    char scenario_author_dirs_with_folder[10][FILENAME_MAX];
 
    int num_author_files = scenario_author_dirs->num_files;

    for (int i = 0; i < scenario_author_dirs->num_files; ++i) {
        strcpy(scenario_author_dirs_with_folder[i], scenario_author_dirs->files[i]);
    }

    data.aug_scenarios = dir_find_all_subdirectories_for_dir(scenario_author_dirs_with_folder[0], 0);
    for (int i = 1; i < num_author_files; ++i) {        
        data.aug_scenarios = dir_find_all_subdirectories_for_dir(scenario_author_dirs_with_folder[i], 1);
    }

    data.focus_button_id = 0;
    button_select_item(0, 0);
    scrollbar_init(&scrollbar, 0, data.aug_scenarios->num_files - MAX_SCENARIOS);
}

static void draw_scenario_list(void)
{   
    inner_panel_draw(16, 210, 16, 16);
    char file[FILE_NAME_MAX];
    char *file_name_to_display;
    uint8_t displayable_file[FILE_NAME_MAX];
    for (int i = 0; i < MAX_SCENARIOS; i++) {
        font_t font = FONT_NORMAL_GREEN;
        if (data.focus_button_id == i + 1) {
            font = FONT_NORMAL_WHITE;
        }
        else if (!data.focus_button_id && data.selected_item == i + scrollbar.scroll_position) {
            font = FONT_NORMAL_WHITE;
        }
        if ((i + scrollbar.scroll_position) >= data.aug_scenarios->num_files) {
            break;
        }

        strcpy(file, data.aug_scenarios->files[i + scrollbar.scroll_position]);

        file_name_to_display = strtok(file, "/");
        file_name_to_display = strtok(NULL, "/");
        file_name_to_display = strtok(NULL, "/");

        encoding_from_utf8(file_name_to_display, displayable_file, FILE_NAME_MAX);
        
        text_ellipsize(displayable_file, font, 240);
        text_draw(displayable_file, 24, 220 + 16 * i, font, 0);
        
    }
}

static void draw_scenario_info(void)
{
    const int scenario_info_x = 335;
    const int scenario_info_width = 280;
    const int scenario_criteria_x = 420;
    const int scenario_goals_x = 78;
    int scenario_goals_y = 36;
 
    //image_draw(image_group(GROUP_SCENARIO_IMAGE) + scenario_image_id(), 78, 36);

    text_ellipsize(data.selected_scenario_display, FONT_LARGE_BLACK, scenario_info_width + 10);
    text_draw_centered(data.selected_scenario_display, scenario_info_x, 30, scenario_info_width + 10, FONT_LARGE_BLACK, 0);
    text_draw_centered(scenario_author(), scenario_info_x, 60, scenario_info_width + 10, FONT_NORMAL_BLACK, 0);
    text_draw_centered(scenario_brief_description(), scenario_info_x, 80, scenario_info_width, FONT_NORMAL_WHITE, 0);
    lang_text_draw_year(scenario_property_start_year(), scenario_criteria_x, 100, FONT_LARGE_BLACK);
    text_draw_multiline(scenario_briefing(), scenario_info_x, 140, scenario_info_width, FONT_NORMAL_BLACK, 0);

    lang_text_draw_centered(44, 127, scenario_goals_x, scenario_goals_y, 180, FONT_NORMAL_BLACK);
    scenario_goals_y += 36;
    
    int width;
    if (scenario_criteria_culture_enabled()) {
        width = text_draw_number(scenario_criteria_culture(), '@', " ", scenario_goals_x, scenario_goals_y, FONT_NORMAL_BLACK, 0);
        lang_text_draw(44, 129, scenario_goals_x + width, scenario_goals_y, FONT_NORMAL_BLACK);
        scenario_goals_y += 18;
    }
    if (scenario_criteria_prosperity_enabled()) {
        width = text_draw_number(scenario_criteria_prosperity(), '@', " ", scenario_goals_x, scenario_goals_y, FONT_NORMAL_BLACK, 0);
        lang_text_draw(44, 130, scenario_goals_x + width, scenario_goals_y, FONT_NORMAL_BLACK);
        scenario_goals_y += 18;
    }
    if (scenario_criteria_peace_enabled()) {
        width = text_draw_number(scenario_criteria_peace(), '@', " ", scenario_goals_x, scenario_goals_y, FONT_NORMAL_BLACK, 0);
        lang_text_draw(44, 131, scenario_goals_x + width, scenario_goals_y, FONT_NORMAL_BLACK);
        scenario_goals_y += 18;
    }
    if (scenario_criteria_favor_enabled()) {
        width = text_draw_number(scenario_criteria_favor(), '@', " ", scenario_goals_x, scenario_goals_y, FONT_NORMAL_BLACK, 0);
        lang_text_draw(44, 132, scenario_goals_x + width, scenario_goals_y, FONT_NORMAL_BLACK);
        scenario_goals_y += 18;
    }
    if (scenario_criteria_population_enabled()) {
        width = text_draw_number(scenario_criteria_population(), '@', " ", scenario_goals_x, scenario_goals_y, FONT_NORMAL_BLACK, 0);
        lang_text_draw(44, 133, scenario_goals_x + width, scenario_goals_y, FONT_NORMAL_BLACK);
        scenario_goals_y += 18;
    }

}

//import 
static void draw_background(void)
{
    image_draw_fullscreen_background(image_group(GROUP_CCK_BACKGROUND));
    graphics_in_dialog();
    inner_panel_draw(280, 242, 2, 12);
    draw_scenario_list();
    draw_scenario_info();
    graphics_reset_dialog();
}

//import
static void draw_foreground(void)
{
    graphics_in_dialog();
    image_buttons_draw(0, 0, &start_button, 1);
    scrollbar_draw(&scrollbar);
    draw_scenario_list();
    graphics_reset_dialog();
}

static void handle_input(const mouse* m, const hotkeys* h)
{
    const mouse* m_dialog = mouse_in_dialog(m);
    if (scrollbar_handle_mouse(&scrollbar, m_dialog)) {
        return;
    }
    if (image_buttons_handle_mouse(m_dialog, 0, 0, &start_button, 1, 0)) {
        return;
    }
    if (generic_buttons_handle_mouse(m_dialog, 0, 0, file_buttons, MAX_SCENARIOS, &data.focus_button_id)) {
        return;
    }
    if (h->enter_pressed) {
        button_start_scenario(0, 0);
        return;
    }
    if (input_go_back_requested(m, h)) {
        window_go_back();
    }
}

static void button_select_item(int index, int param2)
{
    char file_name[FILENAME_MAX];
    char *file_name_to_display;
    
    if (index >= data.aug_scenarios->num_files) {
        return;
    }
    data.selected_item = scrollbar.scroll_position + index;
    strcpy(data.selected_scenario_filename, data.aug_scenarios->files[data.selected_item]);
    strcpy(file_name, data.selected_scenario_filename);

    file_name_to_display = strtok(file_name, "/");
    file_name_to_display = strtok(NULL, "/");
    file_name_to_display = strtok(NULL, "/");

    strcpy((char *)data.selected_scenario_display, file_name_to_display);

    char map_filepath[FILE_NAME_MAX];
    strcpy(map_filepath, data.selected_scenario_filename);
    strcat(map_filepath, "/");
    strcat(map_filepath, (char*) data.selected_scenario_display);
    strcat(map_filepath, ".map");

    
    strcpy(data.selected_scenario_map_filename, map_filepath);

    game_file_load_scenario_data_from_xml(data.selected_scenario_map_filename);
    window_invalidate();
}

static void button_start_scenario(int param1, int param2)
{
    if (game_file_start_scenario(data.selected_scenario_map_filename, 1)) {
        sound_music_update(1);
        window_city_show();
    }
}

static void on_scroll(void)
{
    window_invalidate();
}

void window_augustus_scenarios_show(void)
{
    window_type window = {
        WINDOW_AUG_SCENARIO_SELECTION,
        draw_background,
        draw_foreground,
        handle_input
    };
    init();
    window_show(&window);
}
