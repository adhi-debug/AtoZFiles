# AtoZfile Makefile

CC = gcc
PKG = `pkg-config --cflags --libs gtk+-3.0`
SRC = src/*.c

# Default build (Debug)
all: debug

debug:
	$(CC) $(SRC) -o AtoZfile.exe $(PKG)

# Release build with embedded icon
release: resources/icon.res
	$(CC) $(SRC) resources/icon.res -o AtoZfile.exe $(PKG)

# Compile Windows resource (.ico → .res)
resources/icon.res: resources/icon.rc
	windres resources/icon.rc -O coff -o resources/icon.res

clean:
	del /Q AtoZfile.exe resources\icon.res
