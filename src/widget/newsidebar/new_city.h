#ifndef WIDGET_NEW_SIDEBAR_H
#define WIDGET_NEW_SIDEBAR_H

#include "graphics/tooltip.h"
#include "input/mouse.h"

void widget_new_sidebar_draw_background(void);
void widget_new_sidebar_draw_foreground(void);
int widget_new_sidebar_handle_mouse(const mouse *m);
int widget_new_sidebar_get_tooltip_text(tooltip_context *c);

#endif
