#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>

void gui_init(GtkWidget *window);
void ui_set_progress(double fraction);
void ui_log(const char *msg);

#endif
