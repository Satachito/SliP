#!/bin/sh

rm ./a.out
echo "Compiling CLI..."
c++ CLI.cpp SliP.cpp Read.cpp Eval.cpp json.cpp \
	-o SliP \
	-std=c++23

