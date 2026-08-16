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

//	store.cpp.  Declared rather than included, as the interpreter's own entry
//	points above are.
extern void				StoreInit();
extern string const&	StoreLog();
extern void				StoreAppend( string const& );
extern void				StoreForget();

static SP< Context >	theContext		= MS< Context >();
static auto				calculatorMode	= true;

//	Set while the saved session is being replayed at boot.  The panel still gets
//	everything — the transcript is what "the session is still here" looks like —
//	but the wire does not, because a terminal opened afterwards would otherwise
//	scroll the whole of last time past before saying anything about now.
static auto				replaying		= false;

//	The wire alone.  For the things a terminal wants and the panel does not: its
//	banner, which the panel draws its own version of at UIInit, and the answers
//	to commands whose effect the panel is already showing.
static void
Say( string const& _ ) {
	fputs( _.c_str(), stdout );
	fputc( '\n', stdout );
	fflush( stdout );
}

//	Everything the REPL says goes to both places.  The panel is not a log of the
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
//	failure ends the run rather than reporting its own consequences.
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
	Print( "  :reset     forget every binding, the saved session with it" );
	Print( "  :forget    the same thing, said the other way   ( AC on the panel )" );
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
	//	The mode is part of the session, not of the terminal: it decides what the
	//	lines after it mean, so it is saved with them.
	if( _ == ":calc"    ) { calculatorMode = true ; Print( "calculator mode"  ); Record( _ ); return true; }
	if( _ == ":prog"    ) { calculatorMode = false; Print( "programming mode" ); Record( _ ); return true; }
	//	The saved session goes too. It has to: it is the log of what built the
	//	bindings, so leaving it would put every one of them back at the next
	//	power-up, and ":reset" would be a lie with a delay on it.
	if( _ == ":reset" || _ == ":forget" ) {
		theContext = MS< Context >();
		ClearStack();
		StoreForget();
		UIClearLog();
		//	Said on the wire, where a command that answered nothing would look
		//	like one that did nothing — and not on the panel, where the empty
		//	transcript is the answer. A line reading "forgotten" would be the
		//	one thing left on a screen that had just been cleared.
		Say( "forgotten" );
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

//	One line of the session, wherever it came from: the wire, the panel, or the
//	log at boot.  Having one of these rather than the same decisions written out
//	in the main loop is what lets a replay be a replay — the stored lines go
//	through exactly what the typed ones went through, block collection included.
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
	if( Run( _ ) ) Record( _ );
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

	//	Nothing has drawn since UIInit: UIPrint fills the transcript, and the
	//	screen is only painted by UIRedraw. Without this the panel comes up on
	//	its first frame — the banner and nothing else — and stays there until a
	//	key is pressed, which reads exactly like a session that was lost.
	UIRedraw();
	return n;
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

	//	Before the wait for a terminal, so that the panel is already showing last
	//	session by the time anyone looks at it.  What it restored is said after
	//	the banner rather than here: nothing written before a terminal attaches
	//	arrives anywhere.
	StoreInit();
	auto	restored = Replay();

	//	Wait for a terminal, but not forever — the panel works on its own, and
	//	the board should come up when it is powered from a charger.
	for( auto i = 0; i < 300 && !stdio_usb_connected(); i++ ) sleep_ms( 10 );

	//	The terminal's banner is the terminal's. Putting it through Print would
	//	append it to the transcript as well, under the session it has just
	//	restored — a second title, at the bottom, saying the same thing the
	//	panel said at the top.
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
		if( !ReadLine( line ) ) continue;
		Consume( line );
		UIRedraw();
	}
}
