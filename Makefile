CC=cc
CFLAGS=-Wall -Wextra -std=c99 -Iinclude
SOURCES=src/main.c src/fileinfo.c src/directory.c src/compare.c src/report.c
TARGET=filemanager

all: $(TARGET)

$(TARGET): $(SOURCES) include/filemanager.h
$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
rm -f $(TARGET)
