#	The interpreter is compiled from C++/ directly.  There is no copy of it here
#	and no Tab5 fork of it: this component is the host, and C++/ is the language.
#
#	main.cpp and store.cpp read almost exactly like ESP32/main/'s, which is the
#	honest state of things: two ESP-IDF hosts differing in their console and in
#	what is wired to them.  They are kept apart rather than shared because this
#	one has a 720x1280 panel to grow into and the other has no screen at all —
#	the same reason RP2350/main.cpp is its own file.  If they are still this close
#	once the panel is here, that is when to share them.
#
#	"../../compat" comes first on the include path on purpose — see the headers
#	in it.

idf_component_register(
	SRCS
		"main.cpp"
		"store.cpp"
		"board.cpp"
		"screen.cpp"
		"ui.cpp"
		"../../C++/SliP.cpp"
		"../../C++/Read.cpp"
		"../../C++/Eval.cpp"
		"../../C++/json.cpp"
	INCLUDE_DIRS
		"../../compat"		#	see compat/stdckdint.h — must come first
		"../../C++"
	REQUIRES
		esp_driver_usb_serial_jtag	#	the console: the P4 speaks USB itself
		esp_driver_i2c				#	the board's own bus: touch, IO expanders
		esp_lcd						#	the MIPI-DSI panel
		espressif__esp_lcd_touch	#	bsp/touch.h includes its header
		esp_driver_gpio				#	the backlight
		esp_hw_support				#	esp_random
		vfs
		nvs_flash					#	store.cpp keeps the session in it
)

#	C++23, as C++/CLI.sh and WASM/build.sh use.  Spelled 2b so that an older GCC
#	accepts it too; it is the same dialect.
target_compile_options( ${COMPONENT_LIB} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++2b> )

#	picolibc claims eight single-letter macros in C++ that this codebase uses as
#	names.  The header explains which, and why undefining them is safe.
target_compile_options(
	${COMPONENT_LIB} PRIVATE
	$<$<COMPILE_LANGUAGE:CXX>:-include${CMAKE_CURRENT_SOURCE_DIR}/../../compat/undef-libc-ctype.h>
)
