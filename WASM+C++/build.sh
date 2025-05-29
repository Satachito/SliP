#!/bin/bash

# 出力ディレクトリ
OUTPUT_DIR="public"
mkdir -p "$OUTPUT_DIR"

# C++ ソースファイル
CPP_FILE="src/main.cpp"

# 出力ファイル名 (WASMとJSグルーコード)
OUTPUT_NAME="SliP"

echo "Compiling C++ to WebAssembly..."

# Emscripten を使ってコンパイル
# -o: 出力ファイル名 (JSグルーコードとWASMファイルが生成される)
# -s EXPORTED_FUNCTIONS="['_processString']": C++の関数をJS側から呼び出せるようにエクスポート
#    通常、emscripten::bind.h を使う場合は不要ですが、念のため記述
# -s ALLOW_MEMORY_GROWTH=1: 必要に応じてメモリを自動的に拡張することを許可
# -s MODULARIZE=1: JSグルーコードをCommonJS/ESモジュールとしてラップする
# -s 'EXPORT_ES6=1': ES6モジュールとしてエクスポート（import文で読み込むため）
# -s 'ENVIRONMENT="web"': ブラウザ環境向けにビルド
# -s 'USE_EXCEPTION_CATCHING=1': C++例外処理を使用する場合 (今回の例では不要だが、一般的な設定として)
# -s 'MALLOC="emmalloc"': メモリマネージャを指定（通常はデフォルトで良いが、明示的に）
# --bind: emscripten::bind.h を使用することを示す
emcc "$CPP_FILE" \
    -o "$OUTPUT_DIR/$OUTPUT_NAME.js" \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s MODULARIZE=1 \
    -s EXPORT_ES6=1 \
    -s ENVIRONMENT="web" \
    --bind \
    -g # デバッグ情報を含める（開発用）

echo "Compilation complete! Check the '$OUTPUT_DIR' directory."
