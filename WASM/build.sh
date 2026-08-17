#!/bin/sh
#	Requires em++ ( emscripten ) on PATH.
#	Run from the repository root. Used by WASM.sh ( local docker ) and .github/workflows/pages.yml ( CI ).
#
#	INCOMING_MODULE_JS_API names what the loader is allowed to read off the object
#	handed to createSliP.  Emscripten 6 stopped reading everything by default, and
#	`wasmBinary` is what WASM/smoke.mjs uses to run the web build under node —
#	without it named here the loader ignores the bytes it was given and tries to
#	fetch a file:// URL, which node refuses.

em++ C++/SliP.cpp C++/Read.cpp C++/Eval.cpp C++/json.cpp C++/Embed.cpp WASM/WASM.cpp WASM/BuildJS.cpp \
	-o Web/SliP.js \
	-std=c++23 \
	-O2 \
	-fexceptions \
	-sMODULARIZE \
	-sEXPORT_ES6 \
	-sENVIRONMENT=web \
	-sINCOMING_MODULE_JS_API=wasmBinary,locateFile \
	-sEXPORT_NAME=createSliP \
	--bind \
	--pre-js WASM/pre.js
