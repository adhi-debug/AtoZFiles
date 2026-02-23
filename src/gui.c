#include "gui.h"
#include "organizer.h"
#include "utils.h"

#include <string.h>
#include <sqlite3.h>
#include <glib.h>

/* widgets */
static GtkWidget *src_entry;
static GtkWidget *dst_entry;
static GtkWidget *log_view;
static GtkWidget *progress_bar;
static GtkWidget *cancel_btn;
static GtkWidget *mode_combo;

/* ---------- progress ---------- */

void ui_set_progress(double fraction)
{
    if (fraction < 0) fraction = 0;
    if (fraction > 1) fraction = 1;

    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), fraction);

    if (fraction >= 1.0) {
        gtk_widget_set_name(progress_bar, "progress_done");
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress_bar), "TRANSFER COMPLETE");
    } else {
        gtk_widget_set_name(progress_bar, "progress_active");
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress_bar), "");
    }
}

/* ---------- live log ---------- */

static gboolean ui_log_idle(gpointer data)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(log_view));
    GtkTextIter end;

    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_insert(buffer, &end, "   LOG // ", -1);
    gtk_text_buffer_insert(buffer, &end, data, -1);
    gtk_text_buffer_insert(buffer, &end, "\n", -1);

    GtkTextMark *mark = gtk_text_buffer_get_insert(buffer);
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(log_view), mark);

    g_free(data);
    return FALSE;
}

void ui_log(const char *msg)
{
    g_idle_add(ui_log_idle, g_strdup(msg));
}

/* ---------- helpers ---------- */

static void browse_folder(GtkButton *btn, gpointer data)
{
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "SELECT FOLDER", NULL,
        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
        "CLOSE", GTK_RESPONSE_CANCEL,
        "SELECT", GTK_RESPONSE_ACCEPT, NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *folder = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_entry_set_text(GTK_ENTRY(data), folder);
        g_free(folder);
    }
    gtk_widget_destroy(dialog);
}

static void navigate_up(GtkButton *btn, gpointer data)
{
    char path[1024];
    strcpy(path, gtk_entry_get_text(GTK_ENTRY(data)));

    char *p = strrchr(path, '/');
    if (p && p != path) {
        *p = 0;
        gtk_entry_set_text(GTK_ENTRY(data), path);
    }
}

/* ---------- DATABASE WINDOWS ---------- */

static void get_db_path(char *out)
{
    const char *base = g_get_user_data_dir();
    sprintf(out, "%s\\AtoZfile\\atozfile.db", base);
}

static void show_history(GtkButton *btn, gpointer win)
{
    GtkWidget *w = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(w), "History Viewer");
    gtk_window_set_default_size(GTK_WINDOW(w), 700, 400);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);

    gtk_container_add(GTK_CONTAINER(scroll), view);
    GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));

    sqlite3 *db;
    char db_path[512];
    get_db_path(db_path);
    sqlite3_open(db_path, &db);

    sqlite3_stmt *st;
    sqlite3_prepare_v2(db, "SELECT time,source,destination,category,operation FROM operations ORDER BY id DESC;", -1, &st, 0);

    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buf, &end);

    while (sqlite3_step(st) == SQLITE_ROW) {
        gtk_text_buffer_insert(buf, &end, (const char*)sqlite3_column_text(st,0), -1);
        gtk_text_buffer_insert(buf, &end, " | ", -1);
        gtk_text_buffer_insert(buf, &end, (const char*)sqlite3_column_text(st,4), -1);
        gtk_text_buffer_insert(buf, &end, " | ", -1);
        gtk_text_buffer_insert(buf, &end, (const char*)sqlite3_column_text(st,3), -1);
        gtk_text_buffer_insert(buf, &end, "\n   FROM: ", -1);
        gtk_text_buffer_insert(buf, &end, (const char*)sqlite3_column_text(st,1), -1);
        gtk_text_buffer_insert(buf, &end, "\n   TO:   ", -1);
        gtk_text_buffer_insert(buf, &end, (const char*)sqlite3_column_text(st,2), -1);
        gtk_text_buffer_insert(buf, &end, "\n\n", -1);
    }

    sqlite3_finalize(st);
    sqlite3_close(db);
    gtk_container_add(GTK_CONTAINER(w), scroll);
    gtk_widget_show_all(w);
}

static void show_stats(GtkButton *btn, gpointer win)
{
    sqlite3 *db;
    char db_path[512];
    get_db_path(db_path);
    sqlite3_open(db_path, &db);

    int total = 0, moves = 0, copies = 0;
    sqlite3_stmt *st;

    sqlite3_prepare_v2(db,"SELECT COUNT(*) FROM operations;",-1,&st,0);
    if (sqlite3_step(st) == SQLITE_ROW) total = sqlite3_column_int(st, 0);
    sqlite3_finalize(st);

    sqlite3_prepare_v2(db,"SELECT COUNT(*) FROM operations WHERE operation='MOVE';",-1,&st,0);
    if (sqlite3_step(st) == SQLITE_ROW) moves = sqlite3_column_int(st, 0);
    sqlite3_finalize(st);

    sqlite3_prepare_v2(db,"SELECT COUNT(*) FROM operations WHERE operation='COPY';",-1,&st,0);
    if (sqlite3_step(st) == SQLITE_ROW) copies = sqlite3_column_int(st, 0);
    sqlite3_finalize(st);

    char categories[2048] = "";
    sqlite3_prepare_v2(db,"SELECT category, COUNT(*) FROM operations GROUP BY category;",-1,&st,0);

    while (sqlite3_step(st) == SQLITE_ROW) {
        strcat(categories, (const char*)sqlite3_column_text(st, 0));
        strcat(categories, " : ");
        char num[32];
        sprintf(num, "%d", sqlite3_column_int(st, 1));
        strcat(categories, num);
        strcat(categories, "\n");
    }
    sqlite3_finalize(st);
    sqlite3_close(db);

    char msg[4096];
    snprintf(msg, sizeof(msg), "📊 AtoZfile Statistics\n\nTotal files: %d\nMoves: %d\nCopies: %d\n\nBy category:\n%s",
        total, moves, copies, categories[0] ? categories : "No data.\n");

    GtkWidget *d = gtk_message_dialog_new(GTK_WINDOW(win), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
    gtk_dialog_run(GTK_DIALOG(d));
    gtk_widget_destroy(d);
}

/* ---------- operation ---------- */

static void apply_operation_mode(void)
{
    int active = gtk_combo_box_get_active(GTK_COMBO_BOX(mode_combo));
    set_operation(active == 0 ? OP_MOVE : OP_COPY);
}

static void on_scan_clicked(GtkButton *btn, gpointer data)
{
    ui_log("SCANNING DIRECTORIES...");
}

static void on_organize_clicked(GtkButton *btn, gpointer data)
{
    apply_operation_mode();
    set_backup(1);
    ui_set_progress(0);
    ui_log("INITIALIZING TASK...");
    organize_files(gtk_entry_get_text(GTK_ENTRY(src_entry)), gtk_entry_get_text(GTK_ENTRY(dst_entry)));
}

/* ---------- GUI INIT ---------- */

void gui_init(GtkWidget *window)
{
    // 1. Setup CSS
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "assets/style.css", NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    // 2. Cinematic Overlay (For the background glow)
    GtkWidget *overlay = gtk_overlay_new();
    gtk_container_add(GTK_CONTAINER(window), overlay);

    GtkWidget *glow_bg = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(glow_bg, "animated_glow");
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), glow_bg);

    // 3. Main Layout Container
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 25);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 45);
    gtk_widget_set_name(vbox, "main_glass_vbox");
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), vbox);

    // Header Bar
    GtkWidget *header = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(header), "ATOZFILE");
    gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header), "v1.0 PREMIUM TERMINAL");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
    gtk_window_set_titlebar(GTK_WINDOW(window), header);

    /* SOURCE */
    GtkWidget *src_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_name(src_box, "input_group");
    GtkWidget *src_up = gtk_button_new_with_label("UP");
    GtkWidget *src_btn = gtk_button_new_with_label("SOURCE");
    src_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(src_entry), "ENTER SOURCE PATH...");
    gtk_box_pack_start(GTK_BOX(src_box), src_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(src_box), src_up, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(src_box), src_btn, FALSE, FALSE, 0);

    /* DESTINATION */
    GtkWidget *dst_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_name(dst_box, "input_group");
    GtkWidget *dst_up = gtk_button_new_with_label("UP");
    GtkWidget *dst_btn = gtk_button_new_with_label("TARGET");
    dst_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(dst_entry), "ENTER TARGET PATH...");
    gtk_box_pack_start(GTK_BOX(dst_box), dst_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(dst_box), dst_up, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(dst_box), dst_btn, FALSE, FALSE, 0);

    /* MODE */
    mode_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(mode_combo), "PROTOCOL: SECURE MOVE");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(mode_combo), "PROTOCOL: SECURE COPY");
    gtk_combo_box_set_active(GTK_COMBO_BOX(mode_combo), 0);
    gtk_widget_set_name(mode_combo, "glass_combo");

    /* ACTIONS */
    GtkWidget *action_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    GtkWidget *scan_btn = gtk_button_new_with_label("SCAN");
    GtkWidget *org_btn = gtk_button_new_with_label("EXECUTE");
    GtkWidget *history = gtk_button_new_with_label("HISTORY");
    GtkWidget *stats = gtk_button_new_with_label("STATS");
    cancel_btn = gtk_button_new_with_label("EXIT");

    gtk_widget_set_name(org_btn, "organize_btn");
    gtk_widget_set_name(cancel_btn, "cancel_btn");

    gtk_box_pack_start(GTK_BOX(action_box), history, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(action_box), stats, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(action_box), org_btn, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(action_box), scan_btn, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(action_box), cancel_btn, FALSE, FALSE, 0);

    /* PROGRESS */
    progress_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress_bar), TRUE);
    gtk_widget_set_name(progress_bar, "progress_active");

    /* LOG - Transmorphic Bottom Part */
    GtkWidget *log_frame = gtk_frame_new(NULL);
    gtk_widget_set_name(log_frame, "transmorphic_log_frame");
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scroll, -1, 140);
    log_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(log_view), FALSE);
    gtk_widget_set_name(log_view, "cinematic_log_view");

    gtk_container_add(GTK_CONTAINER(scroll), log_view);
    gtk_container_add(GTK_CONTAINER(log_frame), scroll);

    /* PACK ALL */
    gtk_box_pack_start(GTK_BOX(vbox), src_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), dst_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), mode_combo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), action_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), progress_bar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), log_frame, TRUE, TRUE, 0);

    /* SIGNALS */
    g_signal_connect(src_btn,"clicked",G_CALLBACK(browse_folder),src_entry);
    g_signal_connect(dst_btn,"clicked",G_CALLBACK(browse_folder),dst_entry);
    g_signal_connect(src_up,"clicked",G_CALLBACK(navigate_up),src_entry);
    g_signal_connect(dst_up,"clicked",G_CALLBACK(navigate_up),dst_entry);
    g_signal_connect(scan_btn,"clicked",G_CALLBACK(on_scan_clicked),NULL);
    g_signal_connect(org_btn,"clicked",G_CALLBACK(on_organize_clicked),NULL);
    g_signal_connect(history,"clicked",G_CALLBACK(show_history),window);
    g_signal_connect(stats,"clicked",G_CALLBACK(show_stats),window);
    g_signal_connect(cancel_btn,"clicked",G_CALLBACK(cancel_organize),NULL);

    logger_init("output/atozfile.log");
    gtk_widget_show_all(window);
}