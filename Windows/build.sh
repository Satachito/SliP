#!/bin/sh
#	Cross-compile SliP.exe from macOS or Linux.  Needs mingw-w64:
#
#		brew install mingw-w64        # macOS
#		apt install mingw-w64         # Debian / Ubuntu
#
#	Run from the repository root:  sh Windows/build.sh
#
#	compat/ comes first on the include path: mingw ships <stdckdint.h> but, like
#	picolibc, hides its ckd_* macros from C++, and JP.h includes it on every
#	target that is not Apple.
#
#	-static links libstdc++, libgcc and winpthread in, so the result is one file
#	that runs on a machine with nothing installed on it.

set -e
cd "$( dirname "$0" )/.."

CXX=${CXX:-x86_64-w64-mingw32-g++}
WINDRES=${WINDRES:-x86_64-w64-mingw32-windres}

mkdir -p Windows/build
$WINDRES Windows/slip.rc -O coff -o Windows/build/slip.res
$WINDRES Windows/cli.rc  -O coff -o Windows/build/cli.res

$CXX \
	Windows/main.cpp \
	C++/SliP.cpp C++/Read.cpp C++/Eval.cpp C++/json.cpp C++/Embed.cpp \
	Windows/build/slip.res \
	-o Windows/build/SliP.exe \
	-I compat -I C++ \
	-std=gnu++23 -O2 -municode -mwindows \
	-static -static-libgcc -static-libstdc++ -s \
	-lcomctl32 -lole32 -luuid

#	The console build as well.  It is what the conformance suite can be pointed
#	at, so the language itself can be checked on this target and not just the
#	fact that a window opens.
$CXX \
	C++/CLI.cpp \
	C++/SliP.cpp C++/Read.cpp C++/Eval.cpp C++/json.cpp \
	Windows/build/cli.res \
	-o Windows/build/slip-cli.exe \
	-I compat -I C++ \
	-std=gnu++23 -O2 \
	-static -static-libgcc -static-libstdc++ -s

ls -lh Windows/build/SliP.exe Windows/build/slip-cli.exe
