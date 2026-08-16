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

//	@ is the argument, so it belongs with ' and = rather than with the digits.
//	The grid is full at six columns, and a seventh would put the keys under 35
//	pixels — so it goes in the bottom row, which is characters and actions
//	already: ␣ inserts one too.
char const* const	KEY_AT	= "@";
char const* const	KEY_AP	= ":";		//	apply: `21 : M1` runs M1 with 21 on the stack

constexpr auto	MAX_COLS = 6;

struct Row {
	int			n;
	char const*	key[ MAX_COLS ];
};

//	Six by five.  A calculator on the left — digits where a calculator puts them,
//	the four operations in a column — and the language along the bottom and down
//	the right.
//
//	M1 and M2 are not a memory register: they are two names, and the bottom row
//	is what binds and uses one.  `( 'M1 = 2π )` stores, `M1` recalls, `21 : M1`
//	applies it.  A calculator's memory that happens to be the language's own.
Row const PAD[ PAD_ROWS ] = {
	{ 6, { "7", "8", "9", "π", "+", KEY_DEL } },
	{ 6, { "4", "5", "6", "𝑒", "-", KEY_AC  } },
	{ 6, { "1", "2", "3", "∞", "×", "M1"    } },
	{ 6, { "0", ".", "(", ")", "÷", "M2"    } },
	{ 6, { "'", "=", KEY_AT, KEY_AP, KEY_SP, KEY_RUN } },
};

struct Entry {
	std::string	text;
	uint16_t	colour;
};

std::vector< Entry >	log;
std::string				editing;
int						downKey = -1;

//	How many display lines back from the newest the transcript is showing.  Zero
//	is the bottom, which is where new output puts it.
int						scroll = 0;

//	Dragging the transcript scrolls it.  These hold where the finger went down.
bool					dragging = false;
int						dragY = 0, dragScroll = 0;

void
Append( std::string const& _, uint16_t colour ) {
	log.push_back( { _, colour } );
	if( log.size() > 64 ) log.erase( log.begin() );
}

//	The log is entries; the screen shows lines.  A long value wraps, so the two
//	are not the same count — and scrolling by entries would jump a paragraph at a
//	time.  Everything below counts lines.
struct Line {
	std::string	text;
	uint16_t	colour;
};

std::vector< Line >
Wrapped() {
	std::vector< Line >	$;
	for( auto const& e: log ) {
		std::string	cur;
		auto		n = 0;
		auto		p = (unsigned char const*)e.text.c_str();
		while( *p ) {
			auto k = ( *p & 0xF8 ) == 0xF0 ? 4 : ( *p & 0xF0 ) == 0xE0 ? 3 : ( *p & 0xE0 ) == 0xC0 ? 2 : 1;
			cur.append( (char const*)p, k );
			p += k;
			if( ++n == COLS ) { $.push_back( { cur, e.colour } ); cur.clear(); n = 0; }
		}
		if( cur.size() || !e.text.size() ) $.push_back( { cur, e.colour } );
	}
	return $;
}

void
DrawLog() {
	ScreenFill( 0, 0, SCREEN_W, LOG_ROWS * FONT_H, BG );

	auto	lines	= Wrapped();
	auto	total	= (int)lines.size();
	auto	most	= total > LOG_ROWS ? total - LOG_ROWS : 0;
	if( scroll > most ) scroll = most;
	if( scroll < 0    ) scroll = 0;

	auto	last	= total - scroll;			//	one past the last line shown
	auto	first	= last > LOG_ROWS ? last - LOG_ROWS : 0;
	//	Short transcripts sit on the bottom, as a terminal's do.
	auto	top		= LOG_ROWS - ( last - first );

	for( auto i = first; i < last; i++ )
		ScreenText( 0, top + i - first, lines[ i ].text.c_str(), lines[ i ].colour, BG );
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
	auto	blue	= !edit && !run && (
						( c == 3 && r <  3 )	//	π 𝑒 ∞
					||	( c == 5 && r >= 2 )	//	M1 M2
					||	r == 4					//	' = @ :
					);
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
	scroll = 0;			//	an answer is worth going back to the bottom for
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
		dragging = false;
		if( downKey >= 0 ) {
			DrawKey( downKey / MAX_COLS, downKey % MAX_COLS, false );
			ScreenFlush();
			downKey = -1;
		}
		return false;
	}
	//	Above the pad is the transcript, and a drag there scrolls it.  Content
	//	follows the finger: dragging down reveals what is older.
	if( y < PAD_TOP ) {
		if( !dragging ) {
			dragging	= true;
			dragY		= y;
			dragScroll	= scroll;
			return false;
		}
		auto want = dragScroll + ( y - dragY ) / FONT_H;
		if( want < 0 ) want = 0;
		if( want != scroll ) {
			scroll = want;
			DrawLog();
			ScreenFlush();
		}
		return false;
	}

	auto r = ( y - PAD_TOP ) / KEY_H;
	if( r < 0 || r >= PAD_ROWS ) return false;
	auto c = x / ( SCREEN_W / PAD[ r ].n );
	if( c < 0 || c >= PAD[ r ].n ) return false;

	auto key = r * MAX_COLS + c;
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
