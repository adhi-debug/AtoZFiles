#include "fileops.h"
#include "utils.h"
#include "database.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

typedef struct {
    char from[1024];
    char to[1024];
} MoveRecord;

static MoveRecord history[10000];
static int history_count = 0;

void ensure_dir(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0) {
        mkdir(path);
        log_printf("Created directory: %s", path);
    }
}

static int exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

/* FAST buffered copy */
static int copy_file_internal(const char *src, const char *dst)
{
    FILE *in = fopen(src, "rb");
    FILE *out = fopen(dst, "wb");
    if (!in || !out) return -1;

    char buf[1024 * 256];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
        fwrite(buf, 1, n, out);

    fclose(in);
    fclose(out);
    return 0;
}

/* ---------- COPY ONLY ---------- */

int copy_file_only(const char *src, const char *dst_dir)
{
    const char *name = strrchr(src, '/');
    name = name ? name + 1 : src;

    char dst[1024];
    snprintf(dst, sizeof(dst), "%s/%s", dst_dir, name);

    int i = 1;
    while (exists(dst))
        snprintf(dst, sizeof(dst), "%s/%d_%s", dst_dir, i++, name);

    if (copy_file_internal(src, dst) == 0) {
        log_printf("Copied: %s -> %s", src, dst);
        db_log_file(src, dst, "UNKNOWN", "COPY");
        return 0;
    }

    log_printf("Copy failed: %s", src);
    return -1;
}

/* ---------- MOVE ---------- */

int move_file(const char *src, const char *dst_dir, int backup)
{
    const char *name = strrchr(src, '/');
    name = name ? name + 1 : src;

    char dst[1024];
    snprintf(dst, sizeof(dst), "%s/%s", dst_dir, name);

    int i = 1;
    while (exists(dst))
        snprintf(dst, sizeof(dst), "%s/%d_%s", dst_dir, i++, name);

    if (rename(src, dst) == 0) {

        strcpy(history[history_count].from, src);
        strcpy(history[history_count].to, dst);
        history_count++;

        log_printf("Moved: %s -> %s", src, dst);
        db_log_file(src, dst, "UNKNOWN", "MOVE");
        return 0;
    }

    log_printf("Move failed: %s", src);
    return -1;
}

/* ---------- UNDO (memory only) ---------- */

void undo_all(void)
{
    for (int i = history_count - 1; i >= 0; i--) {
        rename(history[i].to, history[i].from);
        log_printf("Undo: %s -> %s", history[i].to, history[i].from);
    }

    history_count = 0;
    log_printf("Undo completed");
}
