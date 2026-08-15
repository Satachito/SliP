//	SliP on an RP2350, as a serial REPL over USB.
//
//	The same shape as ESP32/main/main.cpp, and for the same reasons: C++/ is
//	compiled in unchanged, errors are values rather than exceptions that escape,
//	and the line is read here byte for byte because the operators are the
//	language and a line editor that counts bytes as characters eats them.
//
//	What is different is what is underneath.  There is no scheduler at all here,
//	so there is no task to spawn and no watchdog to feed: main runs the loop.
//
//	And this host has a screen.  The panel shows the transcript and a keypad of
//	the operators, so the board is usable with nothing attached to it; the USB
//	REPL runs alongside, into the same session, so a binding made by tapping is
//	visible to a line typed over the wire and the other way round.

#include "SliP.hpp"

#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#include "pico/stdlib.h"
#include "pico/bootrom.h"		//	reset_usb_boot, for :bootsel

#include "ui.hpp"

extern void				Build();
extern SP< SliP >		Read( iReader&, char32_t );
extern V< SP< SliP > >	ReadList( iReader&, char32_t );
extern SP< SliP >		Eval( SP< Context >, SP< SliP > );
extern void				ClearStack();

static SP< Context >	theContext		= MS< Context >();
static auto				calculatorMode	= true;

//	Everything the REPL says goes to both places.  The panel is not a log of the
//	wire: they are one session with two windows onto it.
static void
Print( string const& _ ) {
	fputs( _.c_str(), stdout );
	fputc( '\n', stdout );
	fflush( stdout );
	UIPrint( _ );
}

//	Errors are reported, never propagated.  Returns whether the body finished, so
//	a caller that must stop at the first failure can tell.  Unlike the CLI, there
//	is nowhere for an exception to go here: it would reach std::terminate, and on
//	bare metal that is a halt with no message.
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

//	Calculator mode, the way the web calculator reads a line: the line is the
//	contents of a sentence, so `2πr` behaves as `( 2πr )`.  The comment is
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
//	failure ends the run rather than reporting its own consequences.
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

//	The heap grows up from the end of .bss and the stack grows down from the top
//	of RAM; the linker script names where the stack stops.  What is left between
//	them is what a program still has.
static size_t
FreeHeap() {
	extern char __StackLimit;
	return (size_t)( &__StackLimit - (char*)sbrk( 0 ) );
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
	Print( "  :bootsel   reboot into BOOTSEL, to flash something else" );
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
		Print( "heap " + to_string( FreeHeap() ) + " free" );
		return true;
	}
	//	Worth having: with the board in a case, the BOOTSEL button is a nuisance,
	//	and picotool can only force a reboot if the firmware offers a way.
	if( _ == ":bootsel" ) {
		Print( "rebooting into BOOTSEL" );
		sleep_ms( 100 );
		reset_usb_boot( 0, 0 );
		return true;
	}
	Print( "! unknown command: " + _ + "   ( :help )" );
	return true;
}

//	Not a line editor.  A line editor that counts bytes as characters eats the
//	operators — which is what happened on the ESP32, where ESP-IDF's linenoise
//	dropped every byte of every multi-byte character after echoing it.  This
//	reads bytes, echoes bytes, and only backspace has to think.
static bool
ReadLine( string& $ ) {
	$.clear();
	while( true ) {
		//	Two inputs, one line.  A line tapped out on the panel arrives here
		//	whole; one typed over USB arrives a byte at a time.
		string	tapped;
		if( UIPoll( tapped ) ) {
			$ = tapped;
			Print( "> " + $ );
			return true;
		}
		auto c = getchar_timeout_us( 0 );
		if( c == PICO_ERROR_TIMEOUT ) {
			tight_loop_contents();
			continue;
		}
		if( c == '\n' || c == '\r' ) {
			fputc( '\n', stdout );
			fflush( stdout );
			return true;
		}
		if( c == 0x08 || c == 0x7F ) {		//	BS, DEL
			UIBackspace();
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

int
main() {

	stdio_init_all();

	Build();
	UIInit();			//	after Build: the panel's first frame says the version

	//	Wait for a terminal, but not forever — the panel works on its own, and
	//	the board should come up when it is powered from a charger.
	for( auto i = 0; i < 300 && !stdio_usb_connected(); i++ ) sleep_ms( 10 );

	Print( "" );
	Print( string( "SliP " ) + SLIP_VERSION + "  —  :help" );
	Print( "" );

	string	block;
	string	line;
	auto	collecting = false;

	while( true ) {
		fputs( collecting ? ".. " : calculatorMode ? "> " : ">> ", stdout );
		fflush( stdout );
		if( !ReadLine( line ) ) continue;

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
		if( Command( line ) ) { UIRedraw(); continue; }
		Run( line );
		UIRedraw();
	}
}
