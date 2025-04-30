# === CONFIGURATION ===
CXX = g++
CFLAGS = -Wall -Wextra -std=c++17
LIBS = -lGL -lglad -lglfw -lm

# Source files
SRCS = main.cpp mapa.cpp dibujo.c

# Executable name
TARGET = mfs

# === RULES ===

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CFLAGS) -o $@ $^ $(LIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean run
