#!/bin/bash

#rm ./a.out
#echo "Compiling CLI..."
#c++ -std=c++23 CLI.cpp Read.cpp Eval.cpp


echo "Compiling to WebAssembly..."
# Emscripten を使ってコンパイル
# -s EXPORTED_FUNCTIONS="['_processString']": C++の関数をJS側から呼び出せるようにエクスポート
#    通常、emscripten::bind.h を使う場合は不要ですが、念のため記述
# -s ALLOW_MEMORY_GROWTH=1: 必要に応じてメモリを自動的に拡張することを許可
# -s MODULARIZE=1: JSグルーコードをCommonJS/ESモジュールとしてラップする
# -s 'EXPORT_ES6=1': ES6モジュールとしてエクスポート（import文で読み込むため）
# -s 'ENVIRONMENT="web"': ブラウザ環境向けにビルド
# -s 'USE_EXCEPTION_CATCHING=1': C++例外処理を使用する場合 (今回の例では不要だが、一般的な設定として)
# -s 'MALLOC="emmalloc"': メモリマネージャを指定（通常はデフォルトで良いが、明示的に）
# --bind: emscripten::bind.h を使用することを示す
emcc WASM.cpp Read.cpp Eval.cpp \
    -o public/SliP.js \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s MODULARIZE=1 \
    -s EXPORT_ES6=1 \
    -s ENVIRONMENT="web" \
    --bind \
	-Wno-character-conversion \
	-std=c++23 \
    -g

