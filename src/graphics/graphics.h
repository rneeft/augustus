#ifndef GRAPHICS_GRAPHICS_H
#define GRAPHICS_GRAPHICS_H

#include "graphics/color.h"

void graphics_in_dialog(void);
void graphics_in_dialog_with_size(int width, int height);
void graphics_reset_dialog(void);

void graphics_set_clip_rectangle(int x, int y, int width, int height);
void graphics_reset_clip_rectangle(void);

void graphics_clear_screen(void);

color_t *graphics_get_pixel(int x, int y);

void graphics_clear_city_viewport(void);
void graphics_clear_screens(void);

void graphics_draw_vertical_line(int x, int y1, int y2, color_t color);
void graphics_draw_horizontal_line(int x1, int x2, int y, color_t color);

void graphics_draw_line(int x_start, int x_end, int y_start, int y_end, color_t color);

void graphics_draw_rect(int x, int y, int width, int height, color_t color);
void graphics_draw_inset_rect(int x, int y, int width, int height);

void graphics_fill_rect(int x, int y, int width, int height, color_t color);
void graphics_shade_rect(int x, int y, int width, int height, int darkness);

int graphics_save_to_image(int image_id, int x, int y, int width, int height);
void graphics_draw_from_image(int image_id, int x, int y);

#endif // GRAPHICS_GRAPHICS_H
