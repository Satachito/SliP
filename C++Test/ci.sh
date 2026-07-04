#!/bin/sh
#	CI entry point: build and run the C++ test binary.
set -e
cd "$(dirname "$0")"
clang++ -DDEBUG -std=c++23 -o _ \
	../C++/Read.cpp ../C++/Eval.cpp ../C++/SliP.cpp ../C++/json.cpp \
	TEST.cpp EvalTest.cpp ReadTest.cpp MathTest.cpp RegressionTest.cpp
./_
