CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2 -Iinclude

CC ?= gcc
CFLAGS ?= -std=c99 -Wall -Wextra -O2 -Iinclude

CPP_SOURCES = src/main.cpp src/FileInfo.cpp src/DirScanner.cpp src/FileOps.cpp src/ReportExporter.cpp src/TerminalUI.cpp
C_SOURCES = src/main.c src/fileinfo.c src/directory.c src/compare.c src/report.c

TARGET = filemanager
TARGET_C = filemanager_c

ifeq ($(OS),Windows_NT)
    TARGET_BIN = $(TARGET).exe
    TARGET_C_BIN = $(TARGET_C).exe
    RM = del /Q /F
else
    TARGET_BIN = $(TARGET)
    TARGET_C_BIN = $(TARGET_C)
    RM = rm -f
endif

all: $(TARGET_BIN)

$(TARGET_BIN): $(CPP_SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET_BIN) $(CPP_SOURCES)

legacy_c: $(TARGET_C_BIN)

$(TARGET_C_BIN): $(C_SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET_C_BIN) $(C_SOURCES)

clean:
	$(RM) $(TARGET_BIN) $(TARGET_C_BIN) *.o
