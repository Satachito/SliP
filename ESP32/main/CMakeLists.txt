#	The interpreter is compiled from C++/ directly.  There is no copy of it here
#	and no ESP32 fork of it: this component is the host, and C++/ is the language.
#
#	Embed.cpp is deliberately absent.  Its JSON contract exists so that the
#	browser and the Mac app cannot drift apart; a serial line wants text, so
#	main.cpp calls Read / Eval the way CLI.cpp does.
#
#	"../../compat" comes first on the include path on purpose — see the headers
#	in it.

idf_component_register(
	SRCS
		"main.cpp"
		"store.cpp"
		"../../C++/SliP.cpp"
		"../../C++/Read.cpp"
		"../../C++/Eval.cpp"
		"../../C++/json.cpp"
	INCLUDE_DIRS
		"../../compat"		#	see compat/stdckdint.h — must come first
		"../../C++"
	REQUIRES
		driver			#	uart
		esp_hw_support	#	esp_random
		vfs
		nvs_flash		#	store.cpp keeps the session in it
)

#	C++23, as C++/CLI.sh and WASM/build.sh use.  Spelled 2b so that the GCC 12
#	that ships with ESP-IDF 5.1 / 5.2 accepts it too; it is the same dialect.
target_compile_options( ${COMPONENT_LIB} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++2b> )

#	picolibc claims eight single-letter macros in C++ that this codebase uses as
#	names.  The header explains which, and why undefining them is safe.
target_compile_options(
	${COMPONENT_LIB} PRIVATE
	$<$<COMPILE_LANGUAGE:CXX>:-include${CMAKE_CURRENT_SOURCE_DIR}/../../compat/undef-libc-ctype.h>
)
