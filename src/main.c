#include <gtk/gtk.h>
#include "database.h"   // ✅ NEW

/* Implemented in gui.c */
void gui_init(GtkWidget *window);

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    /* ✅ NEW: initialize database */
    db_init("output/atozfile.db");

    /* Create main window */
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "AtoZfile");
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 560);

    /* Load window icon if available */
    if (g_file_test("assets/icon.png", G_FILE_TEST_EXISTS))
        gtk_window_set_icon_from_file(GTK_WINDOW(window), "assets/atoz.png", NULL);

    /* Build GUI */
    gui_init(window);

    /* Close app when window is destroyed */
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    /* ✅ NEW: close database safely */
    db_close();

    return 0;
}
