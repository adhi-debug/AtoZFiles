#include "organizer.h"
#include "fileops.h"
#include "utils.h"
#include "gui.h"
#include "database.h"

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <glib.h>

/* progress state */
static int total_files = 0;
static int processed_files = 0;

/* config flags */
static int dry_run = 0;
static int backup_enabled = 0;
static volatile int cancel_flag = 0;
static OperationMode current_op = OP_MOVE;

/* setters */
void set_dry_run(int enable)   { dry_run = enable; }
void set_backup(int enable)    { backup_enabled = enable; }
void set_operation(OperationMode mode) { current_op = mode; }

/* ---------- progress helpers ---------- */

static gboolean ui_progress_cb(gpointer data)
{
    double *fraction = data;
    ui_set_progress(*fraction);
    g_free(fraction);
    return FALSE;
}

static void report_progress(void)
{
    if (total_files == 0) return;

    double *f = g_malloc(sizeof(double));
    *f = (double)processed_files / total_files;
    g_idle_add(ui_progress_cb, f);
}

/* ---------- count files ---------- */

static void count_files(const char *path)
{
    DIR *dir = opendir(path);
    if (!dir) return;

    struct dirent *e;
    while ((e = readdir(dir))) {
        if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
            continue;

        char full[1024];
        snprintf(full, sizeof(full), "%s/%s", path, e->d_name);

        struct stat st;
        if (stat(full, &st) != 0) continue;

        if (S_ISDIR(st.st_mode))
            count_files(full);
        else
            total_files++;
    }
    closedir(dir);
}

/* ---------- main scan ---------- */

static void scan_and_process(const char *src, const char *dst)
{
    DIR *dir = opendir(src);
    if (!dir || cancel_flag) return;

    struct dirent *e;
    while ((e = readdir(dir))) {
        if (cancel_flag) break;
        if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
            continue;

        char full[1024];
        snprintf(full, sizeof(full), "%s/%s", src, e->d_name);

        struct stat st;
        if (stat(full, &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            scan_and_process(full, dst);
        } else {
            const char *cat = get_category(e->d_name);

            char cat_dir[1024];
            snprintf(cat_dir, sizeof(cat_dir), "%s/%s", dst, cat);
            ensure_dir(cat_dir);

            if (dry_run) {
                log_printf("[DRY-RUN] %s -> %s", full, cat_dir);
            } else {
                if (current_op == OP_MOVE) {
                    move_file(full, cat_dir, backup_enabled);
                    db_log_file(full, cat_dir, cat, "MOVE");
                } else {
                    copy_file_only(full, cat_dir);
                    db_log_file(full, cat_dir, cat, "COPY");
                }
            }

            processed_files++;
            report_progress();
        }
    }
    closedir(dir);
}

/* ---------- worker thread ---------- */

static gpointer worker(gpointer data)
{
    char **paths = data;

    cancel_flag = 0;
    total_files = 0;
    processed_files = 0;

    db_init("output/atozfile.db");
    log_printf("Organize started");

    count_files(paths[0]);
    scan_and_process(paths[0], paths[1]);

    log_printf(cancel_flag ? "Organize cancelled" : "Organize completed");

    db_close();

    double *f = g_malloc(sizeof(double));
    *f = 1.0;
    g_idle_add(ui_progress_cb, f);

    g_free(paths[0]);
    g_free(paths[1]);
    g_free(paths);
    return NULL;
}

void organize_files(const char *src, const char *dst)
{
    if (!src || !dst || !*src || !*dst) {
        log_printf("Invalid source or destination");
        return;
    }

    char **paths = g_malloc(sizeof(char*) * 2);
    paths[0] = g_strdup(src);
    paths[1] = g_strdup(dst);

    g_thread_new("organizer-thread", worker, paths);
}

void cancel_organize(void)
{
    cancel_flag = 1;
}

void undo_last_operation(void)
{
    undo_all();
}
