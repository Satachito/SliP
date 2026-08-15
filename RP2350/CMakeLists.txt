#	The interpreter is compiled from C++/ directly.  There is no copy of it here
#	and no RP2350 fork of it: this is a host, and C++/ is the language.
#
#	  export PICO_SDK_PATH=~/pico/pico-sdk
#	  cmake -B build -S . && cmake --build build
#
#	The result is build/slip.uf2, and build/slip.elf beside it.

cmake_minimum_required( VERSION 3.13 )

set( PICO_BOARD waveshare_rp2350_touch_lcd_2_8 )
set( PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR}/boards )

#	Not optional.  Every error in the language is a throw and every type test is
#	a dynamic_pointer_cast, and the SDK turns both off by default — so they are
#	set here, before the SDK is imported, rather than left to a default that
#	would fail at link time with something unhelpful.
set( PICO_CXX_ENABLE_EXCEPTIONS 1 )
set( PICO_CXX_ENABLE_RTTI 1 )

include( pico_sdk_import.cmake )

project( slip CXX C ASM )
set( CMAKE_CXX_STANDARD 23 )

pico_sdk_init()

add_executable(
	slip
	main.cpp
	screen.cpp
	ui.cpp
	../C++/SliP.cpp
	../C++/Read.cpp
	../C++/Eval.cpp
	../C++/json.cpp
)

target_include_directories( slip BEFORE PRIVATE ../compat ../C++ )

#	newlib claims eight single-letter macros that this codebase uses as names,
#	and JP.h has two of them.  The header explains which, and why the order it
#	takes is the only one that works.
target_compile_options(
	slip PRIVATE -O2
	$<$<COMPILE_LANGUAGE:CXX>:-include${CMAKE_CURRENT_SOURCE_DIR}/../compat/undef-libc-ctype.h>
)

#	The default stack is 2 KB, and Eval recurses once per level of nesting.
#	32 KB is deep enough for the conformance suite with room over; past that the
#	guard page faults rather than quietly writing through the heap.
target_compile_definitions(
	slip PRIVATE
	PICO_STACK_SIZE=0x8000
	PICO_CORE1_STACK_SIZE=0x100		#	core 1 is unused; do not reserve for it
	PICO_USE_STACK_GUARDS=1
)

target_link_libraries(
	slip
	pico_stdlib
	pico_rand		#	the hardware RNG, which RandomSeed in SliP.cpp asks for
	hardware_spi	#	the panel
	hardware_i2c	#	the touch controller
)

#	USB only.  The board reaches the Mac over its USB-C socket, and the UART pins
#	are not brought out to anything on this board.
pico_enable_stdio_usb( slip 1 )
pico_enable_stdio_uart( slip 0 )

pico_add_extra_outputs( slip )
