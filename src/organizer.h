#ifndef ORGANIZER_H
#define ORGANIZER_H

/* Operation mode */
typedef enum {
    OP_MOVE,
    OP_COPY
} OperationMode;

/* Configuration */
void set_dry_run(int enable);
void set_backup(int enable);
void set_operation(OperationMode mode);

/* Control */
void organize_files(const char *src, const char *dst);
void cancel_organize(void);
void undo_last_operation(void);

#endif
