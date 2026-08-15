//	The calculator on the panel.
//
//	Same two halves the phone apps have, turned to fit 240x320: the transcript
//	above, the keypad below.  The keypad is the reason the screen is worth having
//	— the operators are most of the language and none of them are on a keyboard,
//	so this board is the first host where SliP can be used with nothing else
//	attached to it.
//
//	The USB REPL keeps running alongside.  A line typed there and a line tapped
//	here go to the same session, so bindings made one way are visible the other.

#include "ui.hpp"
#include "screen.hpp"
#include "font.h"

#include "pico/stdlib.h"

#include <string>
#include <vector>

namespace {

constexpr auto	BG		= RGB(  16,  20,  28 );
constexpr auto	FG		= RGB( 220, 226, 234 );
constexpr auto	DIM		= RGB( 128, 140, 155 );
constexpr auto	GOOD	= RGB( 126, 231, 135 );
constexpr auto	BAD		= RGB( 255, 123, 114 );
constexpr auto	KEYBG	= RGB(  34,  40,  52 );
//	The line being typed gets its own ground, darker than a key face.  Sharing
//	KEYBG made it read as one more key in the pad rather than as the field the
//	keys write into.
constexpr auto	EDITBG	= RGB(  22,  27,  42 );
constexpr auto	KEYDN	= RGB(  70,  84, 110 );
constexpr auto	ACCENT	= RGB( 121, 192, 255 );
constexpr auto	RUNBG	= RGB(  38,  72, 116 );

constexpr auto	COLS	= SCREEN_W / FONT_W;	//	20

//	The split is in pixels, not in character rows, so the font can grow without
//	moving the keypad: the keys keep their boxes and only the labels get bigger.
//
//	The keys set the height and everything else follows from it, so shortening a
//	key gives the transcript the difference rather than leaving a gap.
constexpr auto	PAD_ROWS	= 5;
constexpr auto	KEY_H		= 30;
constexpr auto	PAD_TOP		= SCREEN_H - PAD_ROWS * KEY_H;	//	170

//	The line being typed sits in a band of its own, with air above and below it —
//	pressed against the keypad it read as another key.
constexpr auto	EDIT_PAD	= 7;
constexpr auto	EDIT_H		= FONT_H + EDIT_PAD * 2;
constexpr auto	EDIT_Y		= PAD_TOP - EDIT_H;
constexpr auto	LOG_ROWS	= EDIT_Y / FONT_H;

//	A calculator, and only that.  The digits sit where a calculator puts them.
//
//	The operators particular to SliP are not here.  They belong on a panel with
//	room for them, and this one has to choose between more keys and keys big
//	enough to hit.
//
//	Rows carry their own column count, so the bottom row can be three wide keys
//	rather than four narrow ones.  These three are compared by pointer.
char const* const	KEY_RUN	= "⏎";
char const* const	KEY_DEL	= "⌫";
char const* const	KEY_AC	= "AC";
//	The space key shows nothing.  ␣ is U+2423, an open box, and at this size it
//	reads as a fragment of a bracket; an empty key with its frame around it says
//	"space" better than a glyph for it does.
char const* const	KEY_SP	= "";

struct Row {
	int			n;
	char const*	key[ 5 ];
};

//	Five columns: digits, the constants, and the four operations down the right
//	where a calculator keeps them.  The bottom row is four wider keys instead.
Row const PAD[ PAD_ROWS ] = {
	{ 5, { "7", "8", "9", "π", "+" } },
	{ 5, { "4", "5", "6", "𝑒", "-" } },
	{ 5, { "1", "2", "3", "∞", "×" } },
	{ 5, { "0", ".", "(", ")", "÷" } },
	{ 4, { KEY_SP, KEY_DEL, KEY_AC, KEY_RUN } },
};

struct Entry {
	std::string	text;
	uint16_t	colour;
};

std::vector< Entry >	log;
std::string				editing;
int						downKey = -1;

void
Append( std::string const& _, uint16_t colour ) {
	log.push_back( { _, colour } );
	if( log.size() > 64 ) log.erase( log.begin() );
}

void
DrawLog() {
	ScreenFill( 0, 0, SCREEN_W, LOG_ROWS * FONT_H, BG );
	//	Newest at the bottom, so the answer just asked for is where the eye is.
	auto row = LOG_ROWS - 1;
	for( auto i = (int)log.size() - 1; i >= 0 && row >= 0; i-- ) {
		auto const&	e		= log[ i ];
		//	A long value wraps rather than being cut: on a 24-column screen the
		//	interesting end of 9223372036854775807 is the end.
		auto		chars	= 0;
		for( auto p = e.text.c_str(); *p; p++ ) if( ( *p & 0xC0 ) != 0x80 ) chars++;
		auto		lines	= ( chars + COLS - 1 ) / COLS;
		if( lines < 1 ) lines = 1;
		row -= lines - 1;
		if( row < 0 ) break;

		auto	col	= 0;
		auto	r	= row;
		auto	p	= (unsigned char const*)e.text.c_str();
		std::string	one;
		while( *p ) {
			auto n = ( *p & 0xF8 ) == 0xF0 ? 4 : ( *p & 0xF0 ) == 0xE0 ? 3 : ( *p & 0xE0 ) == 0xC0 ? 2 : 1;
			one.assign( (char const*)p, n );
			p += n;
			if( col >= COLS ) { col = 0; r++; }
			if( r >= LOG_ROWS ) break;
			ScreenText( col++, r, one.c_str(), e.colour, BG );
		}
		row--;
	}
}

void
DrawEditing() {
	ScreenFill( 0, EDIT_Y, SCREEN_W, EDIT_H, EDITBG );
	//	Show the tail when the line is longer than the screen, because that is
	//	where the caret is.
	std::vector< std::string >	cells;
	auto p = (unsigned char const*)editing.c_str();
	while( *p ) {
		auto n = ( *p & 0xF8 ) == 0xF0 ? 4 : ( *p & 0xF0 ) == 0xE0 ? 3 : ( *p & 0xE0 ) == 0xC0 ? 2 : 1;
		cells.push_back( std::string( (char const*)p, n ) );
		p += n;
	}
	auto first	= (int)cells.size() > COLS - 1 ? (int)cells.size() - ( COLS - 1 ) : 0;
	auto x		= 0;
	auto y		= EDIT_Y + EDIT_PAD;
	for( auto i = first; i < (int)cells.size(); i++ )
		x = ScreenTextAt( x, y, cells[ i ].c_str(), FG, EDITBG );
	ScreenGlyphAt( x, y, '_', ACCENT, EDITBG );
}

void
DrawKey( int r, int c, bool down ) {
	auto	w		= SCREEN_W / PAD[ r ].n;
	auto	x		= c * w;
	auto	y		= PAD_TOP + r * KEY_H;
	auto	label	= PAD[ r ].key[ c ];
	auto	run		= label == KEY_RUN;
	auto	back	= down ? KEYDN : run ? RUNBG : KEYBG;
	//	Digits and arithmetic in white, the language's own constants and brackets
	//	in blue, the two editing keys dimmer than either.
	//	By position, not by comparing the label: `label == "∞"` compares pointers,
	//	and that it worked was the compiler pooling two identical literals rather
	//	than the test being right.
	auto	edit	= label == KEY_DEL || label == KEY_AC || label == KEY_SP;
	auto	blue	= c == 3 && r < 3;			//	π 𝑒 ∞
	auto	colour	= edit ? DIM : run ? FG : blue ? ACCENT : FG;

	ScreenFill( x, y, w - 1, KEY_H - 1, back );
	//	Centred in pixels.  Rounding this onto the cell grid is what tilted the
	//	pad: 30 does not divide by 20.
	ScreenTextAt(
		x + ( w - ScreenWidth( label ) ) / 2
	,	y + ( KEY_H - FONT_H ) / 2
	,	label, colour, back
	);
}

void
DrawPad() {
	ScreenFill( 0, PAD_TOP, SCREEN_W, SCREEN_H - PAD_TOP, BG );
	for( auto r = 0; r < PAD_ROWS; r++ )
		for( auto c = 0; c < PAD[ r ].n; c++ )
			DrawKey( r, c, false );
}

}	//	namespace

void
UIInit() {
	ScreenInit();
	TouchInit();
	ScreenClear( BG );
	Append( "SliP " SLIP_UI_VERSION, ACCENT );
	Append( "tap keys, then ⏎", DIM );
	DrawLog();
	DrawEditing();
	DrawPad();
	ScreenFlush();
}

void
UIPrint( std::string const& _ ) {
	Append(
		_
	,	_.rfind( "= ", 0 ) == 0 ? GOOD
	:	_.rfind( "! ", 0 ) == 0 ? BAD
	:	FG
	);
}

void
UIRedraw() {
	DrawLog();
	DrawEditing();
	ScreenFlush();
}

//	Returns the line when ⏎ was tapped, and nothing otherwise.  The caller owns
//	what running it means; this only builds the string.
bool
UIPoll( std::string& line ) {
	int	x, y;
	if( !TouchRead( x, y ) ) {
		if( downKey >= 0 ) {
			DrawKey( downKey / 5, downKey % 5, false );
			ScreenFlush();
			downKey = -1;
		}
		return false;
	}
	//	Above the pad is the transcript, and a transcript is for reading.
	if( y < PAD_TOP ) return false;

	auto r = ( y - PAD_TOP ) / KEY_H;
	if( r < 0 || r >= PAD_ROWS ) return false;
	auto c = x / ( SCREEN_W / PAD[ r ].n );
	if( c < 0 || c >= PAD[ r ].n ) return false;

	auto key = r * 5 + c;
	if( key == downKey ) return false;			//	still held on the same key
	downKey = key;
	DrawKey( r, c, true );

	auto label = PAD[ r ].key[ c ];
	if( label == KEY_RUN ) {
		if( editing.empty() ) { ScreenFlush(); return false; }
		line = editing;
		editing.clear();
		DrawEditing();
		ScreenFlush();
		return true;
	}
	if( label == KEY_AC ) {
		editing.clear();
	} else if( label == KEY_SP ) {
		editing += ' ';
	} else if( label == KEY_DEL ) {
		//	One character, not one byte: the UTF-8 continuation bytes go with it.
		while( editing.size() ) {
			auto last = (unsigned char)editing.back();
			editing.pop_back();
			if( ( last & 0xC0 ) != 0x80 ) break;
		}
	} else {
		editing += label;
	}
	DrawEditing();
	ScreenFlush();
	return false;
}

void
UIBackspace() {
	if( editing.empty() ) return;
	do {
		editing.pop_back();
	} while( editing.size() && ( (unsigned char)editing.back() & 0xC0 ) == 0x80 );
	DrawEditing();
	ScreenFlush();
}

void
UISetEditing( std::string const& _ ) {
	editing = _;
	DrawEditing();
	ScreenFlush();
}
