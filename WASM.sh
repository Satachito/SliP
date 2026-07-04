cat > _.sh << EOD
em++ C++/SliP.cpp C++/Read.cpp C++/Eval.cpp C++/json.cpp WASM/WASM.cpp WASM/BuildJS.cpp \
	-o Web/SliP.js \
	-std=c++23 \
	-O2 \
	-sMODULARIZE \
	-sEXPORT_ES6 \
	-sENVIRONMENT=web \
	-fexceptions \
	-sEXPORT_NAME=createSliP \
	--bind \
	--pre-js WASM/pre.js
EOD

docker run --rm -e EM_CONFIG=/opt/emsdk/.emscripten -v "$(pwd)":/mnt -w /mnt emscripten sh _.sh

rm _.sh
