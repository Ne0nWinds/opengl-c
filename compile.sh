#!/bin/sh
g++ main.c -o main.exec -I/usr/include `pkg-config --libs glfw3 glew --cflags cglm`
