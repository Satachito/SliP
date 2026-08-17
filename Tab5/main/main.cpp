//	SliP on an M5Stack Tab5 — an ESP32-P4 — as a serial REPL over USB.
//
//	The same shape as ESP32/main/main.cpp, and for the same reasons, which is why
//	it reads like it: C++/ is compiled in unchanged, every entry into the
//	interpreter goes through Guarded because an exception that escapes a FreeRTOS
//	task function reaches std::terminate and panics the chip, and the line is read
//	here byte for byte because a line editor that counts bytes as characters eats
//	the operators.
//
//	Two things are different, and neither is the language.
//
//	The chip is RISC-V rather than Xtensa. Nothing here says so — ESP-IDF and the
//	interpreter both cross that on their own, which is the whole point of having
//	the port be this thin.
//
//	The console is not a UART. The P4 speaks USB to the host itself, so there is
//	no CH340 in the way and no baud rate to agree on; the driver is different and
//	that is the extent of it.
//
//	The panel is not here yet. This board has 720x1280 of it, and the way the
//	RP2350 drives its screen — one frame in RAM, pushed whole on every change —
//	is 1.8 MB a keystroke here. That wants DMA and partial updates, and it wants
//	them as its own piece of work rather than as a detail of getting the language
//	up.

#include "SliP.hpp"

#include <cstdio>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_heap_caps.h"
#include "esp_system.h"
#include "esp_err.h"

#include "ui.hpp"

#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag_vfs.h"

extern void				Build();
extern SP< SliP >		Read( iReader&, char32_t );
extern V< SP< SliP > >	ReadList( iReader&, char32_t );
extern SP< SliP >		Eval( SP< Context >, SP< SliP > );
extern void				ClearStack();

//	store.cpp.  Declared rather than included, as the interpreter's own entry
//	points above are.
extern void				StoreInit();
extern string const&	StoreLog();
extern void				StoreAppend( string const& );
extern void				StoreForget();

//	board.cpp — the parts of the Tab5 that are not the chip.
extern esp_err_t		BoardReleaseResets();
extern string			BoardScan();



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

//	Set while the saved session is being replayed at boot.  A terminal opened
//	afterwards would otherwise scroll the whole of last time past before saying
//	anything about now.
static auto				replaying		= false;

//	The wire alone. For the things a terminal wants and the panel does not: its
//	banner, which the panel draws its own version of, and the answers to commands
//	whose effect the panel is already showing.
static void
Say( string const& _ ) {
	fputs( _.c_str(), stdout );
	fputc( '\n', stdout );
	fflush( stdout );
}

//	Everything the REPL says goes to both places. The panel is not a log of the
//	wire: they are one session with two windows onto it.
static void
Print( string const& _ ) {
	if( !replaying ) {
		fputs( _.c_str(), stdout );
		fputc( '\n', stdout );
		fflush( stdout );
	}
	UIPrint( _ );
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
static bool
RunLine( string const& _ ) {
	auto	expression = _.substr( 0, _.find( "//" ) );
	if( expression.find_first_not_of( " \t\r" ) == string::npos ) return true;
	return Guarded(
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
static bool
RunProgram( string const& _ ) {
	return Guarded(
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

//	Whether every part of it ran.  Only what ran is worth keeping: a line that
//	failed built nothing, and replaying it at the next power-up would report the
//	same failure again for no reason.
static bool
Run( string const& _ ) {
	if( !calculatorMode ) return RunProgram( _ );
	auto	ok = true;
	for( auto const& line: Split( _ ) ) ok = RunLine( line ) && ok;
	return ok;
}

//	A line joins the saved session once it has run.  Nothing is recorded while
//	the saved session is being replayed, or the log would append a copy of itself
//	every time the board was switched on.
static void
Record( string const& _ ) {
	if( !replaying ) StoreAppend( _ );
}

static void
Help() {
	Print( "  <expr>     evaluate it" );
	Print( "  :calc      a line is one sentence — 2πr is ( 2πr )   [default]" );
	Print( "  :prog      a line is toplevel forms, stopping at the first error" );
	Print( "  :{  :}     collect lines, then run them as one" );
	Print( "  :reset     forget every binding, the saved session with it" );
	Print( "  :forget    the same thing, said the other way   ( AC on the panel )" );
	Print( "  :i2c       what is on the board's I2C bus" );
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
	//	The mode is part of the session, not of the terminal: it decides what the
	//	lines after it mean, so it is saved with them.
	//	Said on the wire, where a command that answered nothing would look like one
	//	that did nothing. Not on the panel: what is on the panel was read under the
	//	mode that is being left, so it goes rather than gaining a line about it.
	if( _ == ":calc" || _ == ":prog" ) {
		calculatorMode = _ == ":calc";
		UIClearLog( false );
		UISetProgram( !calculatorMode );
		Say( calculatorMode ? "calculator mode" : "programming mode" );
		Record( _ );
		return true;
	}
	//	The saved session goes too. It has to: it is the log of what built the
	//	bindings, so leaving it would put every one of them back at the next
	//	power-up, and ":reset" would be a lie with a delay on it.
	if( _ == ":reset" || _ == ":forget" ) {
		theContext = MS< Context >();
		ClearStack();
		StoreForget();
		UIClearLog();
		//	Said on the wire, where a command that answered nothing would look
		//	like one that did nothing — and not on the panel, where the emptied
		//	transcript is the answer.
		Say( "forgotten" );
		return true;
	}
	//	Which panel this board has is not a thing to assume — M5 changed it partway
	//	through — so there is a way to ask.
	if( _ == ":i2c" ) {
		BoardReleaseResets();
		fputs( BoardScan().c_str(), stdout );
		fflush( stdout );
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

	usb_serial_jtag_vfs_set_rx_line_endings( ESP_LINE_ENDINGS_CR   );
	usb_serial_jtag_vfs_set_tx_line_endings( ESP_LINE_ENDINGS_CRLF );

	usb_serial_jtag_driver_config_t	config = USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();
	ESP_ERROR_CHECK( usb_serial_jtag_driver_install( &config ) );
	usb_serial_jtag_vfs_use_driver();
}

//	One line of the session, wherever it came from: the wire, or the log at boot.
//	Having one of these rather than the same decisions written out in the main
//	loop is what lets a replay be a replay — the stored lines go through exactly
//	what the typed ones went through, block collection included.
static string	block;
static auto		collecting = false;

static void
Consume( string const& _ ) {
	if( collecting ) {
		if( _ == ":}" ) {
			collecting = false;
			if( Run( block ) ) Record( _ );
			block.clear();
		} else {
			block += _ + '\n';
			Record( _ );
		}
		return;
	}
	if( _ == ":{" ) { collecting = true; Record( _ ); return; }
	if( Command( _ ) ) return;			//	commands that belong in the log record themselves

	//	RUN on the panel hands over everything typed since the last one, newlines
	//	and all. The log is one line per line, so it goes in as what it is — a
	//	block — rather than as a line with newlines inside it, which would come
	//	back from a power cut as several unrelated lines.
	if( Run( _ ) ) {
		if( _.find( '\n' ) == string::npos ) { Record( _ ); return; }
		Record( ":{" );
		for( auto const& l: Split( _ ) ) if( !l.empty() ) Record( l );
		Record( ":}" );
	}
}

//	Put back whatever was here when the power went off, and say how much that
//	was.  The lines are replayed through Consume, so a session that was in
//	programming mode comes back in programming mode, and a block as a block.
static int
Replay() {
	auto const&	log = StoreLog();
	if( log.empty() ) return 0;

	replaying = true;
	auto	n = 0;
	for( auto const& line: Split( log ) ) {
		if( line.empty() ) continue;
		UIPrint( "> " + line );
		Consume( line );
		n++;
	}
	//	A block left open by a power cut ends here rather than swallowing the
	//	first thing typed after it.
	collecting = false;
	block.clear();
	replaying = false;

	//	Nothing has drawn since UIInit: UIPrint fills the transcript and only
	//	UIRedraw paints it. Without this the panel comes up on its first frame
	//	and stays there until a key is pressed, which reads exactly like a
	//	session that was lost.
	UIRedraw();
	return n;
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
//	Never returns false: there is always another finger or another byte to wait
//	for. The signature keeps the shape the other hosts have.
static bool
ReadLine( string& $ ) {
	$.clear();
	while( true ) {
		//	Two inputs, one line. A line tapped out on the panel arrives here
		//	whole; one typed over USB arrives a byte at a time.
		string	tapped;
		if( UIPoll( tapped ) ) {
			$ = tapped;
			Print( "> " + $ );
			return true;
		}
		//	Not fgetc. The console driver is attached to the VFS, which makes a
		//	read wait for a character — and waiting here is waiting on the wire
		//	with a finger already on the panel. A short timeout instead, so the
		//	loop above it runs about a hundred times a second whether or not
		//	anyone is typing.
		uint8_t	ch;
		if( usb_serial_jtag_read_bytes( &ch, 1, pdMS_TO_TICKS( 10 ) ) != 1 ) continue;
		auto	c = (int)ch;
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
	UIInit();			//	after Build: the panel's first frame says the version

	StoreInit();
	auto	restored = Replay();

	//	The terminal's banner is the terminal's: the panel drew its own.
	Say( "" );
	Say( string( "SliP " ) + SLIP_VERSION + "  —  :help" );
	if( restored ) {
		Say( to_string( restored ) + ( restored == 1 ? " line" : " lines" ) + " restored" );
	}
	Say( "" );

	string	line;
	while( true ) {
		fputs( collecting ? ".. " : calculatorMode ? "> " : ">> ", stdout );
		fflush( stdout );
		if( !ReadLine( line ) ) {
			vTaskDelay( pdMS_TO_TICKS( 20 ) );
			continue;
		}
		Consume( line );
		UIRedraw();
	}
}

extern "C" void
app_main() {
	xTaskCreatePinnedToCore( REPL, "slip", STACK_BYTES, nullptr, 5, nullptr, CORE );
}
