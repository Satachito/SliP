#!/bin/sh
#	Build Web/SliP.js and Web/SliP.wasm from C++/ through Emscripten.
#
#		sh WASM.sh
#
#	`latest` on purpose: the build should be done with whatever Emscripten is
#	current, so that a version that breaks it is found here rather than later.
#	`docker pull emscripten/emsdk` to move it forward.

docker run --rm \
	-e EM_CONFIG=/opt/emsdk/.emscripten \
	-v "$(pwd)":/mnt \
	-w /mnt \
	emscripten/emsdk:latest \
	sh WASM/build.sh
