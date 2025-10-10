cat > _.sh << EOD
clang++ -I/usr/include -I/usr/include/aarch64-linux-gnu -c C++/SliP.cpp \
	--std=c++23 \
	--target=wasm32 \
	-nostdlib \
	-o SliP.o

clang++ -I/usr/include -I/usr/include/aarch64-linux-gnu -c C++/Read.cpp \
	--std=c++23 \
	--target=wasm32 \
	-nostdlib \
	-o Read.o

clang++ -I/usr/include -I/usr/include/aarch64-linux-gnu -c C++/Eval.cpp \
	--std=c++23 \
	--target=wasm32 \
	-nostdlib \
	-o Eval.o

wasm-ld SliP.o Read.o Eval.o \
	--no-entry \
	--export-all \
	--allow-undefined \
	--initial-memory=131072 \
	-o Web/SliP.wasm

rm *.o
EOD

docker run --rm -v "$(pwd)":/mnt -w /mnt clang-wasm-ld sh _.sh

rm _.sh

