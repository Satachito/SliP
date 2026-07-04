docker run --rm -e EM_CONFIG=/opt/emsdk/.emscripten -v "$(pwd)":/mnt -w /mnt emscripten sh WASM/build.sh
