# AtoZfile – File Organizer & Backup Utility (C + GTK)

AtoZfile is a desktop application written in C using GTK3.  
It helps automatically organize files into categories (Images, Documents, Audio, Video, etc.), 
with optional backup and dry-run support.

This is a working prototype built for educational and practical use.

---

## 🚀 Features
- Organizes files into categories (Images, Videos, Documents, Code, Audio, Archives, Others)
- GUI built using GTK3 (C language)
- Optional backup before organizing
- Dry-run mode (simulate without moving)
- Progress display and log output
- Handles name collisions automatically
- Cross-volume safe moving (copy+delete fallback)

---

## 📁 Project Structure

AtoZfile/
├── src/
│ ├── main.c
│ ├── gui.c
│ ├── gui.h
│ ├── organizer.c
│ ├── organizer.h
│ ├── fileops.c
│ ├── fileops.h
│ ├── utils.c
│ ├── utils.h
├── assets/
│ └── icon.png
├── output/
│ └── atozfile.log
├── Makefile
├── tasks.json
└── LICENSE


---

## 🛠 Build Instructions (MSYS2 MinGW64 + GTK3)
Install MSYSN2  x86:
LINK: https://www.msys2.org/
OR:
Use the installer in the project directory.


Install dependencies:
pacman -Syu
pacman -S mingw-w64-x86_64-toolchain
pacman -S mingw-w64-x86_64-gtk3
pacman -S mingw-w64-x86_64-make

Add MSYS2 to PATH in system variables:
C:\msys64\mingw64\bin

Navigate to folder:

Build (Debug prototype):
gcc src/*.c -o AtoZfile.exe pkg-config --cflags --libs gtk+-3.0

Run:
./AtoZfile.exe
cd /h/AtoZFiles

Build (Release with embedded .ico icon)(andd database):
windres resources/icon.rc -O coff -o resources/icon.res
gcc -mwindows src/*.c resources/icon.res -o AtoZfile.exe \
`pkg-config --cflags --libs gtk+-3.0 glib-2.0` -lsqlite3





## 🛠 How to use this

Select Source Folder — where files are located
Select Destination Folder — where sorted folders will be created
(Optional) Select Backup Folder — backup original files
Enable Dry-Run if you only want a simulation
Click Scan to preview categorized file.
Click Organize to start actual moving
Watch logs in the log panel or check output/atozfile.log



## 🛠 Testing Checklist

Scan large folder → verify all files listed
Dry-run → no files move, logs show planned moves
Real Organize → categorized folders created correctly
Backup enabled → check backup folder for preserved structure
Cross-disk move → ensure move → copy+delete fallback works
Duplicate filenames → verify auto-renaming (file (1).txt)
Log file updates during operations.

## 🛠 Packaging for Distribution

To run on another computer without MSYS2:
1. Copy AtoZfile.exe
2. Copy these required DLLs from:
    C:\msys64\mingw64\bin\
(GTK3 DLLs, libglib, libcairo, etc.)
3. Place DLLs in the same folder as your EXE
4. Zip and distribute


## 🛠 Technical Architecture

*Modules*
    . GUI Layer — Handles visuals, user events
    . Organizer Layer — Classifies files & coordinates operations
    . FileOps Layer — Performs physical file copy/move
    . Utils Layer — String handling, path helpers, extension mapping

*Threading Model*
    . Long operations (scan/organize) run in background threads
    . GUI updated via g_idle_add() for thread safety.

##  License 📝

AtoZfile is released under the  **MIT License.**
See **LICENSE** for details.


##  Credits 📝

Developed by: V.Chandanadhithyan , 2025
Language: C
Framework: GTK3
Platform: Windows / MSYS2 MinGW64
