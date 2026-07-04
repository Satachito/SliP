#!/bin/sh
#	Requires em++ ( emscripten ) on PATH.
#	Run from the repository root. Used by WASM.sh ( local docker ) and .github/workflows/pages.yml ( CI ).

em++ C++/SliP.cpp C++/Read.cpp C++/Eval.cpp C++/json.cpp WASM/WASM.cpp WASM/BuildJS.cpp \
	-o Web/SliP.js \
	-std=c++23 \
	-O2 \
	-fexceptions \
	-sMODULARIZE \
	-sEXPORT_ES6 \
	-sENVIRONMENT=web \
	-sEXPORT_NAME=createSliP \
	--bind \
	--pre-js WASM/pre.js
