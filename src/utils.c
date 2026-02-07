#include "utils.h"
#include "gui.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <time.h>

static FILE *logf = NULL;

/* extract extension */
static const char *ext(const char *name)
{
    const char *dot = strrchr(name, '.');
    return dot ? dot + 1 : "";
}

/* categorize file */
const char *get_category(const char *filename)
{
    const char *e = ext(filename);

    /* ---------------- Images ---------------- */
    if (!strcasecmp(e,"jpg")  || !strcasecmp(e,"jpeg") ||
        !strcasecmp(e,"png")  || !strcasecmp(e,"gif")  ||
        !strcasecmp(e,"bmp")  || !strcasecmp(e,"webp") ||
        !strcasecmp(e,"svg")  || !strcasecmp(e,"ico")  ||
        !strcasecmp(e,"heic") || !strcasecmp(e,"heif") ||
        !strcasecmp(e,"raw")  || !strcasecmp(e,"cr2")  ||
        !strcasecmp(e,"cr3")  || !strcasecmp(e,"nef")  ||
        !strcasecmp(e,"arw")  || !strcasecmp(e,"dng"))
        return "Images";

    /* ---------------- Videos ---------------- */
    if (!strcasecmp(e,"mp4") || !strcasecmp(e,"mkv") ||
        !strcasecmp(e,"avi") || !strcasecmp(e,"mov") ||
        !strcasecmp(e,"flv") || !strcasecmp(e,"webm")||
        !strcasecmp(e,"wmv") || !strcasecmp(e,"mpg") ||
        !strcasecmp(e,"mpeg")|| !strcasecmp(e,"3gp"))
        return "Videos";

    /* ---------------- Audio ---------------- */
    if (!strcasecmp(e,"mp3") || !strcasecmp(e,"wav") ||
        !strcasecmp(e,"flac")|| !strcasecmp(e,"aac") ||
        !strcasecmp(e,"ogg") || !strcasecmp(e,"opus")||
        !strcasecmp(e,"m4a") || !strcasecmp(e,"wma"))
        return "Audio";

    /* ---------------- Documents ---------------- */
    if (!strcasecmp(e,"pdf")  || !strcasecmp(e,"txt")  ||
        !strcasecmp(e,"doc")  || !strcasecmp(e,"docx") ||
        !strcasecmp(e,"rtf")  || !strcasecmp(e,"odt")  ||
        !strcasecmp(e,"ppt")  || !strcasecmp(e,"pptx") ||
        !strcasecmp(e,"xls")  || !strcasecmp(e,"xlsx") ||
        !strcasecmp(e,"ods"))
        return "Documents";

    /* ---------------- Code / Scripts ---------------- */
    if (!strcasecmp(e,"c")   || !strcasecmp(e,"h")   ||
        !strcasecmp(e,"cpp") || !strcasecmp(e,"hpp") ||
        !strcasecmp(e,"py")  || !strcasecmp(e,"java")||
        !strcasecmp(e,"js")  || !strcasecmp(e,"ts")  ||
        !strcasecmp(e,"html")|| !strcasecmp(e,"css") ||
        !strcasecmp(e,"php") || !strcasecmp(e,"json")||
        !strcasecmp(e,"xml") || !strcasecmp(e,"yaml")||
        !strcasecmp(e,"yml") || !strcasecmp(e,"sh")  ||
        !strcasecmp(e,"bat") || !strcasecmp(e,"ps1"))
        return "Code";

    /* ---------------- Executables / Installers ---------------- */
    if (!strcasecmp(e,"exe") || !strcasecmp(e,"dll") ||
        !strcasecmp(e,"msi") || !strcasecmp(e,"cmd"))
        return "Executables";

    /* ---------------- Archives ---------------- */
    if (!strcasecmp(e,"zip") || !strcasecmp(e,"rar") ||
        !strcasecmp(e,"7z")  || !strcasecmp(e,"tar") ||
        !strcasecmp(e,"gz")  || !strcasecmp(e,"bz2")||
        !strcasecmp(e,"xz"))
        return "Archives";

    /* ---------------- Databases ---------------- */
    if (!strcasecmp(e,"db")     || !strcasecmp(e,"sqlite") ||
        !strcasecmp(e,"sqlite3")|| !strcasecmp(e,"sql")    ||
        !strcasecmp(e,"mdb")    || !strcasecmp(e,"accdb"))
        return "Databases";

    /* ---------------- Disk / System Images ---------------- */
    if (!strcasecmp(e,"iso") || !strcasecmp(e,"img") ||
        !strcasecmp(e,"dmg") || !strcasecmp(e,"vhd") ||
        !strcasecmp(e,"vhdx"))
        return "DiskImages";

    /* ---------------- Default ---------------- */
    return "Others";
}

/* logging */
void log_printf(const char *fmt, ...)
{
    char buffer[1024];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    /* always log to UI */
    ui_log(buffer);

    /* try file logging (optional) */
    if (!logf)
        logf = fopen("output/atozfile.log", "a");

    if (logf) {
        time_t t = time(NULL);
        fprintf(logf, "[%ld] %s\n", t, buffer);
        fflush(logf);
    }
}


void logger_init(const char *path)
{
    if (logf) {
        fclose(logf);
        logf = NULL;
    }
    logf = fopen(path, "a");
}
