//	SliP on an ESP32, as a serial REPL.
//
//	The board has no screen, no DOM and no file system, so this host is the
//	language and nothing else: a line in on the console UART, a value or an error
//	out.  C++/ is compiled in unchanged — the only two places that needed to know
//	about this chip say so there, at SLIP_NO_THREADS in SliP.hpp and RandomSeed
//	in SliP.cpp.
//
//	The one thing this host must do that the CLI need not: catch everything.  The
//	CLI can let an exception reach main and exit with a diagnostic.  Here the
//	frame above the interpreter is a FreeRTOS task function, and an exception that
//	escapes it reaches std::terminate, which panics and reboots the chip.  So
//	every entry into the interpreter goes through Guarded, including a catch-all
//	for the bare `string` that the UTF-8 decoder in JP.h still throws.

#include "SliP.hpp"

#include <cstdio>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_heap_caps.h"
#include "esp_system.h"

#include "driver/uart.h"

//	The same three calls, renamed in ESP-IDF 5.3.
#if __has_include( "driver/uart_vfs.h" )
	#include "driver/uart_vfs.h"
	#define	SLIP_UART_USE_DRIVER	uart_vfs_dev_use_driver
	#define	SLIP_UART_RX_ENDINGS	uart_vfs_dev_port_set_rx_line_endings
	#define	SLIP_UART_TX_ENDINGS	uart_vfs_dev_port_set_tx_line_endings
#else
	#include "esp_vfs_dev.h"
	#define	SLIP_UART_USE_DRIVER	esp_vfs_dev_uart_use_driver
	#define	SLIP_UART_RX_ENDINGS	esp_vfs_dev_uart_port_set_rx_line_endings
	#define	SLIP_UART_TX_ENDINGS	esp_vfs_dev_uart_port_set_tx_line_endings
#endif

extern void				Build();
extern SP< SliP >		Read( iReader&, char32_t );
extern V< SP< SliP > >	ReadList( iReader&, char32_t );
extern SP< SliP >		Eval( SP< Context >, SP< SliP > );
extern void				ClearStack();

//	Eval recurses once per level of nesting, and the FreeRTOS default of a few
//	kilobytes is nowhere near enough.  Deep enough for the conformance suite with
//	room to spare; a runaway recursion still overflows, and the stack canary
//	reports it as such rather than corrupting the heap.
static constexpr auto	STACK_BYTES	= 32 * 1024;

//	The REPL runs on the APP CPU so that a long evaluation starves only the idle
//	task of core 1, which sdkconfig.defaults takes off the task watchdog.  Core 0
//	keeps its idle task, and its watchdog.
static constexpr auto	CORE		= 1;

static SP< Context >	theContext		= MS< Context >();
static auto				calculatorMode	= true;

static void
Print( string const& _ ) {
	fputs( _.c_str(), stdout );
	fputc( '\n', stdout );
	fflush( stdout );
}

//	Errors are reported, never propagated.  Returns whether the body finished, so
//	a caller that must stop at the first failure can tell.
template< typename F > static bool
Guarded( F _ ) {
	try {
		_();
		return true;
	} catch( exception const& e ) {
		Print( string( "! " ) + e.what() );
	} catch( string const& e ) {	//	JP.h's UTF-8 decoder, on a malformed byte
		Print( "! " + e );
	} catch( ... ) {
		Print( "! Unknown error" );
	}
	return false;
}

//	Calculator mode, the way the web calculator has always read a line: the line
//	is the contents of a sentence, so `2πr` behaves as `( 2πr )`.  The comment is
//	stripped before the synthetic close paren is appended, or the reader — which
//	understands `//` — swallows the paren along with the comment.
//
//	One guard per line, because the lines are largely independent: a typo on the
//	first line must not hide the answer on the sixth.
static void
RunLine( string const& _ ) {
	auto	expression = _.substr( 0, _.find( "//" ) );
	if( expression.find_first_not_of( " \t\r" ) == string::npos ) return;
	Guarded(
		[ & ] {
			StringReader	R( expression + ')' );
			auto			form = MS< Sentence >( ReadList( R, U')' ) );
			Print( "= " + Eval( theContext, form )->REPR() );
		}
	);
}

//	Programming mode.  One guard around the whole source, which is exactly the
//	stopping rule: a later form usually depends on an earlier one, so the first
//	failure ends the run rather than reporting its own consequences.  The reader
//	is constructed inside the guard because decoding the source can throw too.
static void
RunProgram( string const& _ ) {
	Guarded(
		[ & ] {
			StringReader	R( _ );
			while( true ) {
				auto form = Read( R, -1 );
				if( !form ) break;
				Print( "= " + Eval( theContext, form )->REPR() );
			}
		}
	);
}

static void
Run( string const& _ ) {
	if( calculatorMode ) for( auto const& line: Split( _ ) ) RunLine( line );
	else RunProgram( _ );
}

static void
Help() {
	Print( "  <expr>     evaluate it" );
	Print( "  :calc      a line is one sentence — 2πr is ( 2πr )   [default]" );
	Print( "  :prog      a line is toplevel forms, stopping at the first error" );
	Print( "  :{  :}     collect lines, then run them as one" );
	Print( "  :reset     forget every binding" );
	Print( "  :free      free heap" );
	Print( "  :version   the language version this build implements" );
	Print( "  :help      this" );
}

//	Returns whether the line was a command.
static bool
Command( string const& _ ) {
	if( _.empty() || _[ 0 ] != ':' ) return false;

	if( _ == ":help"    ) { Help(); return true; }
	if( _ == ":version" ) { Print( SLIP_VERSION ); return true; }
	if( _ == ":calc"    ) { calculatorMode = true ; Print( "calculator mode"  ); return true; }
	if( _ == ":prog"    ) { calculatorMode = false; Print( "programming mode" ); return true; }
	if( _ == ":reset"   ) {
		theContext = MS< Context >();
		ClearStack();
		Print( "reset" );
		return true;
	}
	if( _ == ":free" ) {
		Print(
			"heap "
		+	to_string( esp_get_free_heap_size() )
		+	" free, largest block "
		+	to_string( heap_caps_get_largest_free_block( MALLOC_CAP_8BIT ) )
		+	", low water "
		+	to_string( esp_get_minimum_free_heap_size() )
		);
		return true;
	}
	Print( "! unknown command: " + _ + "   ( :help )" );
	return true;
}

//	Blocking, byte-oriented stdin.  Without the driver installed and wired to the
//	VFS, reads return immediately and the REPL spins at full speed instead of
//	waiting for a keystroke.
static void
StartConsole() {
	setvbuf( stdin, nullptr, _IONBF, 0 );

	SLIP_UART_RX_ENDINGS( (uart_port_t)CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CR   );
	SLIP_UART_TX_ENDINGS( (uart_port_t)CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CRLF );

	uart_config_t	config = {
		.baud_rate				= CONFIG_ESP_CONSOLE_UART_BAUDRATE
	,	.data_bits				= UART_DATA_8_BITS
	,	.parity					= UART_PARITY_DISABLE
	,	.stop_bits				= UART_STOP_BITS_1
	,	.flow_ctrl				= UART_HW_FLOWCTRL_DISABLE
	,	.rx_flow_ctrl_thresh	= 0		//	unused: no flow control
	,	.source_clk				= UART_SCLK_DEFAULT
	,	.flags					= {}
	};
	ESP_ERROR_CHECK( uart_driver_install( (uart_port_t)CONFIG_ESP_CONSOLE_UART_NUM, 1024, 0, 0, nullptr, 0 ) );
	ESP_ERROR_CHECK( uart_param_config(   (uart_port_t)CONFIG_ESP_CONSOLE_UART_NUM, &config ) );
	SLIP_UART_USE_DRIVER( CONFIG_ESP_CONSOLE_UART_NUM );
}

//	Not linenoise, which ESP-IDF has right here and which this used at first.
//	Every line it returns has been through its sanitize(), which keeps only the
//	bytes that satisfy isprint() — and in the C locale no byte of a multi-byte
//	character does.  The stripping happens after the echo, so the character
//	appears on the screen and is gone from the buffer: `2π` reached the reader as
//	`2`, and answered 2.  For most languages that would be a cosmetic bug about
//	comments and string literals.  For SliP the operators are the language, so
//	the line is read here instead, byte for byte.
//
//	Returns false only at EOF, which a UART does not reach.
static bool
ReadLine( string& $ ) {
	$.clear();
	while( true ) {
		auto c = fgetc( stdin );
		if( c == EOF ) return false;
		if( c == '\n' || c == '\r' ) {
			fputc( '\n', stdout );
			fflush( stdout );
			return true;
		}
		if( c == 0x08 || c == 0x7F ) {		//	BS, DEL
			if( $.empty() ) continue;
			//	Erase the character, not one byte of it.  UTF-8 continuation
			//	bytes are 10xxxxxx; dropping a single one would leave a broken
			//	character in the buffer and a whole glyph on the screen.
			do {
				$.pop_back();
			} while( $.size() && ( (unsigned char)$.back() & 0xC0 ) == 0x80 );
			fputs( "\b \b", stdout );
			fflush( stdout );
			continue;
		}
		if( c < 0x20 ) continue;			//	every other control byte
		$ += (char)c;
		fputc( c, stdout );					//	echo
		fflush( stdout );
	}
}

static void
REPL( void* ) {

	StartConsole();
	Build();

	Print( "" );
	Print( string( "SliP " ) + SLIP_VERSION + "  —  :help" );
	Print( "" );

	string	block;
	string	line;
	auto	collecting = false;

	while( true ) {
		fputs( collecting ? ".. " : calculatorMode ? "> " : ">> ", stdout );
		fflush( stdout );
		if( !ReadLine( line ) ) {
			vTaskDelay( pdMS_TO_TICKS( 20 ) );
			continue;
		}

		if( collecting ) {
			if( line == ":}" ) {
				collecting = false;
				Run( block );
				block.clear();
			} else {
				block += line + '\n';
			}
			continue;
		}
		if( line == ":{" ) { collecting = true; continue; }
		if( Command( line ) ) continue;
		Run( line );
	}
}

extern "C" void
app_main() {
	xTaskCreatePinnedToCore( REPL, "slip", STACK_BYTES, nullptr, 5, nullptr, CORE );
}
