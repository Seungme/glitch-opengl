CC = g++
CPP_FILES = src/*.cc
HXX_FILES=
OBJ_FILES = $(CPP_FILES:.cpp=.o)

CXX_FLAGS += -Wall -Wextra -g -std=c++11 -Iinclude
LDXX_FLAGS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lassimp -fPIC -lGLEW

DIST = glitch

all: build

build: $(OBJ_FILES)
	gcc -c src/glad.c -Iinclude
	$(CC) -o $(DIST) $(OBJ_FILES) include/imgui/*.cpp glad.o $(CXX_FLAGS) $(LDXX_FLAGS)
	rm glad.o

.PHONY: all clean build

clean:
	rm -rf $(DIST)
