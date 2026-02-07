#ifndef DATABASE_H
#define DATABASE_H
#include <gtk/gtk.h>

void db_fill_history_buffer(GtkTextBuffer *buffer);


/* initialize database (create file + table) */
int db_init(const char *db_path);

/* log one file operation */
void db_log_file(const char *src,
                 const char *dst,
                 const char *category,
                 const char *operation);

/* close database */
void db_close(void);

/* ---------- NEW FEATURES ---------- */

/* show full history (for History Viewer window) */
void db_print_history(void);

/* undo last N operations (for Undo from DB) */
int db_get_last_operations(int limit);

/* statistics (for Statistics panel) */
void db_print_stats(void);

/* clear database (optional utility) */
void db_clear_all(void);

#endif
