#!/bin/sh

em++ C++/SliP.cpp C++/Read.cpp C++/Eval.cpp C++/json.cpp WASM/WASM.cpp \
	-o Web/SliP.js \
	-std=c++23 \
	-sMODULARIZE \
	-sEXPORT_ES6 \
	-sENVIRONMENT=web \
	-sEXCEPTION_CATCHING_ALLOWED=all \
	-sEXPORT_NAME=createSliP \
	--bind \
	--pre-js WASM/_.js

