# OpenCV
# The following lines were added in the .zprofile
# Adapt them to fit your opencv install version
#
# PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig
# export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/opt/opencv/lib/pkgconfig/opencv4.pc
#
# OpenCV 4 requires -std=c++11 option
#

CC=clang++
CFLAGS=$(shell pkg-config --cflags --libs opencv4)

PROJECT=StarFilter

all: build

build: main.cpp
	$(CC) -o $(PROJECT) -std=c++11 -g -fdiagnostics-color=always -Wall $? $(CFLAGS)

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(PROJECT)