rm _
rm -f _ *.profraw *.profdata index.html control.js style.css 
rm -rf coverage

# カバレッジ計測付きでビルド
clang++ -fprofile-instr-generate -fcoverage-mapping -std=c++23 -o _ TEST.cpp

# プロファイル出力を指定して実行
LLVM_PROFILE_FILE="_.profraw" ./_

# .profraw を .profdata に変換
llvm-profdata merge -sparse _.profraw -o _.profdata

# カバレッジレポート（テキスト）
llvm-cov report ./_ -instr-profile=_.profdata

# HTMLレポート出力
llvm-cov show ./_ -instr-profile=_.profdata -format=html -output-dir=.

# HTMLをブラウザで開く
open index.html

# rm -f _ *.profraw *.profdata index.html control.js style.css 
# rm -rf coverage



# rm ./_ _-*
# echo "Compiling TEST..."
# c++ -fprofile-arcs -ftest-coverage -std=c++23 TEST.cpp
# ./test
# gcov TEST.cpp

