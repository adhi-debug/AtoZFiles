#ifndef UTILS_H
#define UTILS_H

void logger_init(const char *path);
void log_printf(const char *fmt, ...);

/* Returns category name based on file extension */
const char *get_category(const char *filename);

#endif
