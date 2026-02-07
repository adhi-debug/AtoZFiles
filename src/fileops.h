#ifndef FILEOPS_H
#define FILEOPS_H

void ensure_dir(const char *path);

/* Move with optional backup */
int move_file(const char *src, const char *dst_dir, int backup);

/* Copy only */
int copy_file_only(const char *src, const char *dst_dir);

/* Undo in-memory moves */
void undo_all(void);

#endif
