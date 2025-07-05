
# カバレッジ計測付きでビルド
clang++ -fprofile-instr-generate -fcoverage-mapping -std=c++23 -o test TEST.cpp

# プロファイル出力を指定して実行
LLVM_PROFILE_FILE="test.profraw" ./test

# .profraw を .profdata に変換
llvm-profdata merge -sparse test.profraw -o test.profdata

# カバレッジレポート（テキスト）
llvm-cov report ./test -instr-profile=test.profdata

# HTMLレポート出力
# llvm-cov show ./test -instr-profile=test.profdata -format=html -output-dir=.

# HTMLをブラウザで開く
# open index.html

rm -f ./test *.profraw *.profdata index.html control.js style.css 
rm -rf coverage



# rm ./test test-*
# echo "Compiling TEST..."
# c++ -fprofile-arcs -ftest-coverage -std=c++23 TEST.cpp -o test
# ./test
# gcov TEST.cpp

