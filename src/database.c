#include "database.h"
#include "utils.h"

#include <sqlite3.h>
#include <stdio.h>
#include <gtk/gtk.h> 
#include <glib.h>      // ✅ for AppData path + mkdir

static sqlite3 *db = NULL;

/* create table */
static const char *CREATE_TABLE_SQL =
    "CREATE TABLE IF NOT EXISTS operations ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
    "source TEXT,"
    "destination TEXT,"
    "category TEXT,"
    "operation TEXT"
    ");";

/* ---------- CORE ---------- */

int db_init(const char *db_path_unused)
{
    (void)db_path_unused;  // prevent unused warning

    const char *base = g_get_user_data_dir();  
    char folder_path[512];
    char db_path[512];

    // C:\Users\Name\AppData\Local\AtoZfile
    sprintf(folder_path, "%s\\AtoZfile", base);
    g_mkdir_with_parents(folder_path, 0700);

    // C:\Users\Name\AppData\Local\AtoZfile\atozfile.db
    sprintf(db_path, "%s\\AtoZfile\\atozfile.db", base);

    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        log_printf("DB open failed: %s", sqlite3_errmsg(db));
        return 0;
    }

    char *err = NULL;
    if (sqlite3_exec(db, CREATE_TABLE_SQL, 0, 0, &err) != SQLITE_OK) {
        log_printf("DB table error: %s", err);
        sqlite3_free(err);
        return 0;
    }

    sqlite3_exec(db, "DELETE FROM operations;", 0, 0, 0);

    log_printf("Database initialized at: %s", db_path);
    return 1;
}

/* ---------- REST OF YOUR FILE IS UNCHANGED ---------- */

void db_log_file(const char *src,
                 const char *dst,
                 const char *category,
                 const char *operation)
{
    if (!db) return;

    const char *sql =
        "INSERT INTO operations(source,destination,category,operation)"
        " VALUES(?,?,?,?);";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK)
        return;

    sqlite3_bind_text(stmt, 1, src, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, dst, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, category, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, operation, -1, SQLITE_TRANSIENT);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void db_close(void)
{
    if (db) {
        sqlite3_close(db);
        db = NULL;
        log_printf("Database closed");
    }
}

/* ---------- HISTORY VIEWER ---------- */

void db_print_history(void)
{
    if (!db) return;

    const char *sql =
        "SELECT id,time,source,destination,category,operation "
        "FROM operations ORDER BY id DESC;";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK)
        return;

    log_printf("---- OPERATION HISTORY ----");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        log_printf("#%d | %s | %s -> %s | %s | %s",
            sqlite3_column_int(stmt, 0),
            sqlite3_column_text(stmt, 1),
            sqlite3_column_text(stmt, 2),
            sqlite3_column_text(stmt, 3),
            sqlite3_column_text(stmt, 4),
            sqlite3_column_text(stmt, 5)
        );
    }

    sqlite3_finalize(stmt);
}

/* ---------- UNDO SUPPORT ---------- */

int db_get_last_operations(int limit)
{
    if (!db) return 0;

    const char *sql =
        "SELECT source,destination,operation "
        "FROM operations ORDER BY id DESC LIMIT ?;";

    sqlite3_stmt *stmt;
    int count = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK)
        return 0;

    sqlite3_bind_int(stmt, 1, limit);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *src = (const char*)sqlite3_column_text(stmt, 0);
        const char *dst = (const char*)sqlite3_column_text(stmt, 1);
        const char *op  = (const char*)sqlite3_column_text(stmt, 2);

        log_printf("UNDO RECORD: %s | %s | %s", src, dst, op);
        count++;
    }

    sqlite3_finalize(stmt);
    return count;
}

/* ---------- STATISTICS ---------- */

void db_print_stats(void)
{
    if (!db) return;    

    const char *sql =
        "SELECT category, COUNT(*) FROM operations GROUP BY category;";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK)
        return;

    log_printf("---- STATISTICS ----");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        log_printf("%s : %d files",
            sqlite3_column_text(stmt, 0),
            sqlite3_column_int(stmt, 1)
        );
    }

    sqlite3_finalize(stmt);
}

/* ---------- MAINTENANCE ---------- */

void db_clear_all(void)
{
    if (!db) return;

    sqlite3_exec(db, "DELETE FROM operations;", 0, 0, 0);
    log_printf("Database cleared");
}

void db_fill_history_buffer(GtkTextBuffer *buffer)
{
    if (!db || !buffer) return;

    const char *sql =
        "SELECT id,time,source,destination,category,operation "
        "FROM operations ORDER BY id DESC;";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK)
        return;

    gtk_text_buffer_set_text(buffer, "", -1);

    char line[2048];

    while (sqlite3_step(stmt) == SQLITE_ROW) {

        snprintf(line, sizeof(line),
            "ID: %d\nTime: %s\nSource: %s\nDestination: %s\nCategory: %s\nOperation: %s\n----------------------------------------\n\n",
            sqlite3_column_int(stmt, 0),
            sqlite3_column_text(stmt, 1),
            sqlite3_column_text(stmt, 2),
            sqlite3_column_text(stmt, 3),
            sqlite3_column_text(stmt, 4),
            sqlite3_column_text(stmt, 5)
        );

        gtk_text_buffer_insert_at_cursor(buffer, line, -1);
    }

    sqlite3_finalize(stmt);
}
