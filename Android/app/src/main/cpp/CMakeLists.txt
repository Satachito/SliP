#	The interpreter is compiled from C++/ directly.  There is no copy of it here
#	and no Android fork of it: this is a host, and C++/ is the language.
#
#	compat/ comes first on the include path: the NDK sysroot has no <stdckdint.h>
#	at all, and JP.h includes it on every target that is not Apple.

cmake_minimum_required( VERSION 3.22.1 )
project( slip CXX )

set( SLIP "${CMAKE_CURRENT_SOURCE_DIR}/../../../../.." )

add_library(
	slip SHARED
	jni.cpp
	${SLIP}/C++/SliP.cpp
	${SLIP}/C++/Read.cpp
	${SLIP}/C++/Eval.cpp
	${SLIP}/C++/json.cpp
	${SLIP}/C++/Embed.cpp
)

target_include_directories( slip BEFORE PRIVATE ${SLIP}/compat ${SLIP}/C++ )
target_compile_options( slip PRIVATE -O2 )
target_link_libraries( slip log )
