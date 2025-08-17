rm -f _ *.profraw *.profdata index.html control.js style.css 
rm -rf coverage

# カバレッジ計測付きでビルド
clang++ -DDEBUG -fprofile-instr-generate -fcoverage-mapping -std=c++2c -o _ \
	../C++/Read.cpp ../C++/Eval.cpp ../C++/SliP.cpp ../C++/json.cpp \
	TEST.cpp EvalTest.cpp ReadTest.cpp MathTest.cpp

# プロファイル出力を指定して実行
LLVM_PROFILE_FILE="_.profraw" ./_

# .profraw を .profdata に変換
llvm-profdata merge -sparse _.profraw -o _.profdata

# カバレッジレポート（テキスト）
llvm-cov report ./_ -instr-profile=_.profdata

# HTMLレポート出力 / Branch 詳細つき　
llvm-cov show ./_ -instr-profile=_.profdata -format=html -output-dir=. --show-branches=count

# HTMLをブラウザで開く
open index.html

# rm -f _ *.profraw *.profdata index.html control.js style.css 
# rm -rf coverage

