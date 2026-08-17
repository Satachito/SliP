//	The calculator on the panel.
//
//	Three keypads over one session, the way the web calculator has three modes:
//	a calculator, a scientific calculator, and the language.  They are the same
//	interpreter and the same transcript throughout — what changes is how much of
//	SliP is within reach of a finger, and therefore how many keys have to fit
//	across 720 pixels, and therefore how big the type is.  Hence three fonts
//	rather than one scaled three ways.
//
//	  CALC  the RP2350's pad, which was all that fitted on 240 pixels, less its
//	        two memory keys — and the Greek lowercase instead.  This is the first
//	        host where a name can be invented on the panel: the reader has always
//	        taken Greek letters as names, and until now there was no room to
//	        offer them.
//	  SCI   and the transcendental functions the interpreter already has.
//	  PROG  and the operators, which on every earlier board were reachable only
//	        over the wire.
//
//	A row carries its own column count.  A row of digits wants eight keys across
//	and a row with `acosh` on it wants six, and forcing one grid on both makes
//	either the digits cramped or the names clipped.  The RP2350 learned this at
//	six columns; here it matters more.
//
//	The USB REPL runs alongside, into the same session, so a binding made by
//	tapping is visible to a line typed over the wire and the other way round.

#include <string>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"

#include "font.h"
#include "ui.hpp"

extern esp_err_t	ScreenInit();
extern esp_err_t	TouchInit();
extern bool			TouchPoint( int&, int& );
extern void			ScreenFill( int, int, int, int, uint16_t );
extern void			ScreenFlush();
extern uint16_t*	ScreenFrame();

int constexpr	SCREEN_W	= 720;
int constexpr	SCREEN_H	= 1280;

//	The same palette the other hosts use.
static constexpr uint16_t	BG		= 0x0861;
static constexpr uint16_t	FG		= 0xDEFB;
static constexpr uint16_t	DIM		= 0x8410;
static constexpr uint16_t	ACCENT	= 0x64FF;
static constexpr uint16_t	GOOD	= 0x7FED;
static constexpr uint16_t	BAD		= 0xFBEE;
static constexpr uint16_t	KEYBG	= 0x2124;
static constexpr uint16_t	KEYDN	= 0x4A69;
static constexpr uint16_t	EDITBG	= 0x1082;
static constexpr uint16_t	TABBG	= 0x18E3;

//	────────────────────────────────  the font

struct Face {
	int					w, h;
	uint32_t const*		bits;		//	FONT_N * h
};

static Face const	FACE[ 3 ] = {
	{ FONT_L_W, FONT_L_H, &FONT_L_BITS[ 0 ][ 0 ] }
,	{ FONT_M_W, FONT_M_H, &FONT_M_BITS[ 0 ][ 0 ] }
,	{ FONT_S_W, FONT_S_H, &FONT_S_BITS[ 0 ][ 0 ] }
};

//	The codepoints are sorted, so this is a binary search.
static int
GlyphIndex( uint32_t cp ) {
	auto lo = 0, hi = FONT_N - 1;
	while( lo <= hi ) {
		auto mid = ( lo + hi ) / 2;
		if( FONT_CODEPOINT[ mid ] == cp ) return mid;
		if( FONT_CODEPOINT[ mid ] <  cp ) lo = mid + 1; else hi = mid - 1;
	}
	return -1;
}

//	One character out of a UTF-8 string, and how many bytes it was.
static uint32_t
Decode( std::string const& _, size_t& i ) {
	auto	c = (unsigned char)_[ i ];
	if( c < 0x80 )			{ i += 1; return c; }
	if( ( c & 0xE0 ) == 0xC0 && i + 1 < _.size() ) {
		auto v = ( ( c & 0x1F ) << 6 ) | ( _[ i + 1 ] & 0x3F );
		i += 2; return v;
	}
	if( ( c & 0xF0 ) == 0xE0 && i + 2 < _.size() ) {
		auto v = ( ( c & 0x0F ) << 12 ) | ( ( _[ i + 1 ] & 0x3F ) << 6 ) | ( _[ i + 2 ] & 0x3F );
		i += 3; return v;
	}
	if( ( c & 0xF8 ) == 0xF0 && i + 3 < _.size() ) {
		auto v = ( ( c & 0x07 ) << 18 ) | ( ( _[ i + 1 ] & 0x3F ) << 12 )
		       | ( ( _[ i + 2 ] & 0x3F ) << 6 ) | ( _[ i + 3 ] & 0x3F );
		i += 4; return v;
	}
	i += 1; return 0xFFFD;
}

static void
GlyphAt( Face const& f, int x, int y, uint32_t cp, uint16_t colour, uint16_t back ) {
	auto	g = GlyphIndex( cp );
	if( g < 0 ) g = GlyphIndex( '?' );
	if( g < 0 ) return;
	auto	rows  = f.bits + (size_t)g * f.h;
	auto	frame = ScreenFrame();
	if( !frame ) return;
	for( auto r = 0; r < f.h; r++ ) {
		auto	py = y + r;
		if( py < 0 || py >= SCREEN_H ) continue;
		auto	bits = rows[ r ];
		auto	p    = frame + (size_t)py * SCREEN_W;
		for( auto c = 0; c < f.w; c++ ) {
			auto	px = x + c;
			if( px < 0 || px >= SCREEN_W ) continue;
			p[ px ] = ( bits >> ( f.w - 1 - c ) ) & 1 ? colour : back;
		}
	}
}

static int
TextWidth( Face const& f, std::string const& _ ) {
	auto	n = 0;
	for( size_t i = 0; i < _.size(); ) { Decode( _, i ); n++; }
	return n * f.w;
}

static void
TextAt( Face const& f, int x, int y, std::string const& _, uint16_t colour, uint16_t back ) {
	for( size_t i = 0; i < _.size(); ) {
		auto cp = Decode( _, i );
		GlyphAt( f, x, y, cp, colour, back );
		x += f.w;
	}
}

//	────────────────────────────────  the keypads

//	Spelled out. ⌫ is one glyph on a bar drawn at the middle size, and one
//	glyph of a backspace arrow at that size is a smudge.
char const* const	KEY_DEL	= "Delete";
char const* const	KEY_RUN	= "⏎";
char const* const	KEY_SP	= "␣";
char const* const	BLANK	= "";

constexpr auto	MAX_COLS = 13;

struct Row {
	int			n;
	char const*	key[ MAX_COLS ];
};

//	The block that does not move.  Six across and four down, the full width of
//	the panel, the same size in every mode — because the digits are the digits
//	whichever mode you are in, and a key that moves or resizes when the mode
//	changes is a key you have to look at before pressing.  Everything a mode adds
//	goes above this, never through it.
//
//	The last column is the line: 𝑒 at the top because it is a number and belongs
//	with them, then open it, close it, run it.  Correcting it is up on the bar
//	with Reset, away from the digits.
static constexpr int	FIXED_COLS	= 6;
static constexpr int	FIXED_ROWS	= 4;
static constexpr int	FIXED_KEY_H	= 96;

static char const* const	FIXED[ FIXED_ROWS ][ FIXED_COLS ] = {
	{ "7", "8", "9", "+", "'", "𝑒"     }
,	{ "4", "5", "6", "-", "=", "("     }
,	{ "1", "2", "3", "×", "@", ")"     }
,	{ "0", ".", KEY_SP, "÷", ":", KEY_RUN }
};

//	What each mode adds, above the block.  A row carries its own column count: a
//	row of Greek wants eight across and a row with `acosh` on it wants six, and
//	forcing one grid on both makes either the letters cramped or the names
//	clipped.
//
//	The Greek alphabet, both cases, in its own order.  This is the first host
//	where a name can be invented on the panel — the reader has always taken Greek
//	letters as names, and until now there was no room to offer them.  π is one of
//	them and lives here rather than among the digits, even though the interpreter
//	also knows it as a constant.
//
//	Twelve across: forty-eight letters at eight would be six rows, and the letters
//	are one glyph each and legible well below the size a five-letter function name
//	needs.
#define	SLIP_GREEK \
	{ 12, { "α", "β", "γ", "δ", "ε", "ζ", "η", "θ", "ι", "κ", "λ", "μ" } }, \
	{ 12, { "ν", "ξ", "ο", "π", "ρ", "σ", "τ", "υ", "φ", "χ", "ψ", "ω" } }, \
	{ 12, { "Α", "Β", "Γ", "Δ", "Ε", "Ζ", "Η", "Θ", "Ι", "Κ", "Λ", "Μ" } }, \
	{ 12, { "Ν", "Ξ", "Ο", "Π", "Ρ", "Σ", "Τ", "Υ", "Φ", "Χ", "Ψ", "Ω" } },

static Row const	GREEK[] = {
	SLIP_GREEK
};

//	The Latin alphabet, both cases.  Thirteen across, which is the width that
//	makes fifty-two letters four exact rows.
//
//	It is here for the same reason the Greek is: the reader takes these as names,
//	and until this panel there was nowhere to put them.  It is also how `2πr`
//	gets written on the panel at all — π is a constant and r is a name, and
//	before this the name had to come over the wire.
static Row const	LATIN[] = {
	{ 13, { "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m" } },
	{ 13, { "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z" } },
	{ 13, { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M" } },
	{ 13, { "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z" } },
};

//	The transcendental functions the interpreter already has.  Six across,
//	because `atanh` is five characters and a key has to hold its own name.
static Row const	FUNCTIONS[] = {
	{ 6, { "sin", "cos", "tan", "asin", "acos", "atan" } },
	{ 6, { "sinh", "cosh", "tanh", "asinh", "acosh", "atanh" } },
	{ 6, { "exp", "log", "log2", "log10", "sqrt", "cbrt" } },
	{ 6, { "pow", "hypot", "atan2", "abs", "round", "trunc" } },
};

//	The operators, which on every earlier board were reachable only over the wire.
static Row const	OPERATORS[] = {
	{ 12, { "!", "#", "$", "%", "*", "/", ";", "?", "`", "~", "<", ">" } },
	{ 12, { "¦", "§", "¬", "¶", "·", "¿", "∈", "∋", "∥", "£", "¤", "¡" } },
	{ 12, { "⊂", "⊃", "∩", "∪", "⊤", "⊥", "∅", "«", "»", "⟨", "⟩", "±" } },
	{ 12, { "&", "|", "^", "[", "]", "{", "}", ",", "\"", "\\", "∞", "√" } },
};

//	Three panels over one block, one of them showing.  There is no arrangement
//	where two of these are worth seeing at once — they are three alphabets, and a
//	line is being written in one of them — so they are tabbed rather than
//	folded, and the panel spends its height on the one in use instead of on the
//	headers of the two that are not.
//
//	Each keeps its own size.  A function is a five-letter name and wants the
//	middle size; the operators are forty-odd keys and only fit small.  That was
//	the difference between the three modes this started as, and it survives them.
struct Section {
	char const*	name;
	Row const*	rows;
	int			n;
	int			face;			//	index into FACE
	int			keyH;
};

static constexpr int	HEAD_H = 44;

//	The transcript is not the keypad.  The keys are as big as their section can
//	afford; what has been read and answered is the same reading whichever drawers
//	are open, and having it change size underfoot was the wrong thing to tie
//	together.  The middle size.
static constexpr int	LOG_FACE = 1;

static Section const	SECTION[] = {
	{ "SliP",	OPERATORS,	4, 2, 48 }
,	{ "func",	FUNCTIONS,	4, 1, 64 }
,	{ "abc",	LATIN,		4, 1, 64 }
,	{ "αβγ",	GREEK,		4, 1, 64 }
};
static constexpr int	SECTIONS = (int)( sizeof( SECTION ) / sizeof( *SECTION ) );

static int	theSection = 2;		//	abc

//	Calculator or programming — the same two the wire has as :calc and :prog, and
//	the same two the web calculator has.  Unticked is the calculator, where a line
//	is one sentence and `2πr` means `( 2πr )`.
static bool	theProgram = false;

//	────────────────────────────────  the layout

static constexpr int	TAB_H		= 64;
static constexpr int	EDIT_PAD	= 12;

//	Bottom up: the block, then whatever the mode adds above it, then the line
//	being typed, then the transcript in what is left.
static int	FixedH()	{ return FIXED_ROWS * FIXED_KEY_H; }
static int	FixedTop()	{ return SCREEN_H - FixedH(); }
//	The tab row, then the keys of whichever is selected.
static int	ExtraH()	{ return HEAD_H + SECTION[ theSection ].n * SECTION[ theSection ].keyH; }
static int	ExtraTop()	{ return FixedTop() - ExtraH(); }
static int	KeysTop()	{ return ExtraTop() + HEAD_H; }
static int	EditH()		{ return FACE[ LOG_FACE ].h + EDIT_PAD * 2; }
static int	EditTop()	{ return ExtraTop() - EditH(); }
static int	LogTop()	{ return TAB_H; }
static int	LogH()		{ return EditTop() - LogTop(); }

//	────────────────────────────────  the transcript

struct Entry {
	std::string	text;
	uint16_t	colour;
};

static std::vector< Entry >	log;
static std::string			editing;
static int					downKey = -1;
static int					scroll  = 0;

static bool					dragging = false;
static int					dragY = 0, dragScroll = 0;

//	The transcript is a list of lines, so anything arriving with newlines in it
//	becomes several.  RUN hands a whole program over at once and main.cpp echoes
//	it back as one string; left whole it would be drawn as one line, and the
//	newlines in it would come out as the glyph for a character the font does not
//	have.
void
UIPrint( std::string const& _ ) {
	scroll = 0;			//	an answer is worth going back to the bottom for

	auto	colour =
		_.rfind( "= ", 0 ) == 0 ? GOOD
	:	_.rfind( "! ", 0 ) == 0 ? BAD
	:	FG;

	size_t	from = 0;
	while( from <= _.size() ) {
		auto	to = _.find( '\n', from );
		log.push_back( { _.substr( from, to == std::string::npos ? to : to - from ), colour } );
		if( to == std::string::npos ) break;
		from = to + 1;
	}
	while( log.size() > 200 ) log.erase( log.begin() );
}

//	The log is entries; the screen shows lines.  A long value wraps, so the two
//	are not the same count — and scrolling by entries would jump a paragraph at
//	a time.  Everything below counts lines.
struct Line {
	std::string	text;
	uint16_t	colour;
};

//	Break one entry into as many display lines as it takes.
static void
WrapInto( std::vector< Line >& $, std::string const& text, uint16_t colour, int cols ) {
	std::string	run;
	auto		n = 0;
	for( size_t i = 0; i < text.size(); ) {
		auto	start = i;
		Decode( text, i );
		run.append( text, start, i - start );
		if( ++n == cols ) { $.push_back( { run, colour } ); run.clear(); n = 0; }
	}
	if( n || text.empty() ) $.push_back( { run, colour } );
}

static void
Wrapped( std::vector< Line >& $ ) {
	auto&	f    = FACE[ LOG_FACE ];
	auto	cols = SCREEN_W / f.w;
	for( auto const& e: log ) {
		std::string	run;
		auto		n = 0;
		for( size_t i = 0; i < e.text.size(); ) {
			auto	start = i;
			Decode( e.text, i );
			run.append( e.text, start, i - start );
			if( ++n == cols ) { $.push_back( { run, e.colour } ); run.clear(); n = 0; }
		}
		if( n || e.text.empty() ) $.push_back( { run, e.colour } );
	}
}

static void
DrawLog() {
	auto&	f = FACE[ LOG_FACE ];
	//	With every drawer open there is next to nothing left, which is the honest
	//	arithmetic of 720x1280 and forty-odd keys. Do not draw into a negative
	//	height; close something instead.
	if( LogH() <= 0 ) return;
	ScreenFill( 0, LogTop(), SCREEN_W, LogH(), BG );

	std::vector< Line >	lines;
	//	Programming mode writes a program, not a line, and a program wants to be
	//	seen whole while it is being written. So the space that holds the
	//	transcript holds the draft instead, until RUN turns the draft into the
	//	transcript's next few entries and hands the space back.
	if( theProgram && !editing.empty() ) {
		auto	cols = SCREEN_W / f.w;
		size_t	from = 0;
		while( from <= editing.size() ) {
			auto	to = editing.find( '\n', from );
			auto	one = editing.substr( from, to == std::string::npos ? to : to - from );
			WrapInto( lines, one, FG, cols );
			if( to == std::string::npos ) break;
			from = to + 1;
		}
	} else {
		Wrapped( lines );
	}

	auto	fits = LogH() / f.h;
	auto	max  = (int)lines.size() - fits;
	if( max < 0 ) max = 0;
	if( scroll > max ) scroll = max;
	if( scroll < 0 ) scroll = 0;

	auto	first = (int)lines.size() - fits - scroll;
	if( first < 0 ) first = 0;

	auto	y = LogTop();
	for( auto i = first; i < (int)lines.size() && y + f.h <= EditTop(); i++ ) {
		TextAt( f, 0, y, lines[ i ].text, lines[ i ].colour, BG );
		y += f.h;
	}
}

static void
DrawEditing() {
	auto&	f = FACE[ LOG_FACE ];
	ScreenFill( 0, EditTop(), SCREEN_W, EditH(), EDITBG );
	//	The last line of it, and the tail of that when it is longer than the
	//	panel: what is being typed now is the part worth seeing.
	auto	cols = SCREEN_W / f.w;
	std::string	shown = editing;
	auto		nl = shown.rfind( '\n' );
	auto		more = nl != std::string::npos;
	if( more ) shown = shown.substr( nl + 1 );
	auto		n = 0;
	for( size_t i = 0; i < shown.size(); ) { Decode( shown, i ); n++; }
	if( n > cols ) {
		auto	drop = n - cols;
		size_t	i = 0;
		while( drop-- ) Decode( shown, i );
		shown = shown.substr( i );
	}
	if( more ) {
		TextAt( f, 0, EditTop() + EDIT_PAD, "…", DIM, EDITBG );
		TextAt( f, f.w, EditTop() + EDIT_PAD, shown, FG, EDITBG );
	} else {
		TextAt( f, 0, EditTop() + EDIT_PAD, shown, FG, EDITBG );
	}
}

//	Reset, the mode, and ⌫, across the top.  None of the three puts a character
//	in: one throws the session away, one changes what a line means, and one takes
//	a character back.  The RP2350 had ⌫ and AC down among the digits because there
//	was nowhere else to put them.
static constexpr int	TABS		= 4;
static constexpr int	TAB_RESET	= 0;
static constexpr int	TAB_PROG	= 1;
static constexpr int	TAB_RUN		= 2;
static constexpr int	TAB_DEL		= 3;

static void
DrawTab( int i, bool down ) {
	auto&	f = FACE[ 1 ];
	auto	w = SCREEN_W / TABS;
	auto	x = i * w;

	if( i == TAB_PROG ) {
		auto	back = down ? KEYDN : TABBG;
		ScreenFill( x, 0, w - 2, TAB_H, back );
		//	A box, drawn rather than lettered: no font here has one, and two
		//	rectangles say it better than a glyph would at this size.
		auto	box = 26;
		auto	bx  = x + 18;
		auto	by  = ( TAB_H - box ) / 2;
		ScreenFill( bx, by, box, box, theProgram ? ACCENT : back );
		ScreenFill( bx + 3, by + 3, box - 6, box - 6, theProgram ? ACCENT : back );
		for( auto t = 0; t < 3; t++ ) {
			ScreenFill( bx, by + t, box, 1, ACCENT );
			ScreenFill( bx, by + box - 1 - t, box, 1, ACCENT );
			ScreenFill( bx + t, by, 1, box, ACCENT );
			ScreenFill( bx + box - 1 - t, by, 1, box, ACCENT );
		}
		TextAt( f, bx + box + 12, ( TAB_H - f.h ) / 2, "prog", theProgram ? ACCENT : DIM, back );
		return;
	}

	//	RUN belongs to programming mode alone. In the calculator ⏎ is what runs a
	//	line, and a second key that did the same thing would be a question to
	//	answer every time rather than a key.
	if( i == TAB_RUN && !theProgram ) {
		ScreenFill( x, 0, w - 2, TAB_H, TABBG );
		return;
	}

	auto	tint = i == TAB_RESET ? BAD : i == TAB_RUN ? GOOD : ACCENT;
	auto	back = down ? tint : TABBG;
	ScreenFill( x, 0, w - 2, TAB_H, back );
	auto	label = std::string(
		i == TAB_RESET	? "Reset"
	:	i == TAB_RUN	? "RUN"
	:	KEY_DEL
	);
	TextAt(
		f
	,	x + ( w - TextWidth( f, label ) ) / 2
	,	( TAB_H - f.h ) / 2
	,	label
	,	down ? BG : tint
	,	back
	);
}

//	What a key press has to repaint. In the calculator that is the line; in
//	programming mode the draft above it as well.
static void
DrawInput() {
	DrawEditing();
	if( theProgram ) DrawLog();
}

static void
DrawTabs() {
	for( auto i = 0; i < TABS; i++ ) DrawTab( i, false );
}

//	One key, wherever it is.  Centred in pixels: rounding a label onto a cell
//	grid is what tilted the RP2350's pad, because a key's height does not divide
//	by a glyph's.
static void
Key( Face const& f, int x, int y, int w, int h, std::string const& label, bool down ) {
	auto	back = down ? KEYDN : KEYBG;
	auto	run  = label == KEY_RUN;
	ScreenFill( x, y, w - 2, h - 2, run ? ( down ? ACCENT : 0x2A7F ) : back );
	if( label.empty() ) return;
	TextAt(
		f
	,	x + ( w - TextWidth( f, label ) ) / 2
	,	y + ( h - f.h ) / 2
	,	label
	,	run ? BG : FG
	,	run ? ( down ? ACCENT : 0x2A7F ) : back
	);
}

static void
DrawExtraKey( int r, int c, bool down ) {
	auto&	S   = SECTION[ theSection ];
	auto&	row = S.rows[ r ];
	auto	w   = SCREEN_W / row.n;
	Key( FACE[ S.face ], c * w, KeysTop() + r * S.keyH, w, S.keyH, row.key[ c ], down );
}

static void
DrawHead( int sec, bool down ) {
	auto&	f = FACE[ 1 ];
	auto	w = SCREEN_W / SECTIONS;
	auto	on = sec == theSection;
	auto	back = on ? ACCENT : ( down ? KEYDN : TABBG );
	ScreenFill( sec * w, ExtraTop(), w - 2, HEAD_H - 2, back );
	auto	label = std::string( SECTION[ sec ].name );
	TextAt(
		f
	,	sec * w + ( w - TextWidth( f, label ) ) / 2
	,	ExtraTop() + ( HEAD_H - f.h ) / 2
	,	label
	,	on ? BG : DIM
	,	back
	);
}

static void
DrawFixedKey( int r, int c, bool down ) {
	auto	w = SCREEN_W / FIXED_COLS;
	Key( FACE[ 0 ], c * w, FixedTop() + r * FIXED_KEY_H, w, FIXED_KEY_H, FIXED[ r ][ c ], down );
}

static void
DrawPad() {
	ScreenFill( 0, ExtraTop(), SCREEN_W, SCREEN_H - ExtraTop(), BG );
	for( auto i = 0; i < SECTIONS; i++ ) DrawHead( i, false );
	auto&	S = SECTION[ theSection ];
	for( auto r = 0; r < S.n; r++ ) {
		for( auto c = 0; c < S.rows[ r ].n; c++ ) DrawExtraKey( r, c, false );
	}
	for( auto r = 0; r < FIXED_ROWS; r++ ) {
		for( auto c = 0; c < FIXED_COLS; c++ ) DrawFixedKey( r, c, false );
	}
}

void
UIRedraw() {
	DrawTabs();
	DrawLog();
	DrawEditing();
	DrawPad();
	ScreenFlush();
}

void
UIInit() {
	ScreenInit();
	TouchInit();
	ScreenFill( 0, 0, SCREEN_W, SCREEN_H, BG );
	UIPrint( "SliP " SLIP_UI_VERSION );
	UIPrint( "tap keys, then ⏎" );
	UIRedraw();
}

void
UIClearLog( bool banner ) {
	log.clear();
	scroll = 0;
	if( !banner ) return;
	UIPrint( "SliP " SLIP_UI_VERSION );
	UIPrint( "tap keys, then ⏎" );
}

//	────────────────────────────────  the finger

//	One character off the end, not one byte: the UTF-8 continuation bytes go
//	with it.
static void
DropCharacter( std::string& $ ) {
	while( $.size() ) {
		auto last = (unsigned char)$.back();
		$.pop_back();
		if( ( last & 0xC0 ) != 0x80 ) break;
	}
}

//	Where a press landed.  Negative is nothing; the three regions are numbered
//	apart so that releasing one knows how to un-draw itself.
static constexpr int	HIT_TAB		= 10000;
static constexpr int	HIT_FIXED	= 20000;

//	A section key is numbered ( section, row, column ); a head is a row of its
//	own, past the last real one.
static constexpr int	HIT_HEAD	= MAX_COLS - 1;

static void
DrawPressed( int key, bool down ) {
	if( key < 0 ) return;
	if( key >= HIT_FIXED ) {
		auto k = key - HIT_FIXED;
		DrawFixedKey( k / FIXED_COLS, k % FIXED_COLS, down );
		return;
	}
	if( key >= HIT_TAB ) { DrawTab( key - HIT_TAB, down ); return; }
	auto	r = key / MAX_COLS;
	auto	c = key % MAX_COLS;
	if( r == HIT_HEAD ) DrawHead( c, down );
	else				DrawExtraKey( r, c, down );
}

bool
UIPoll( std::string& line ) {
	int	x, y;
	if( !TouchPoint( x, y ) ) {
		if( downKey >= 0 ) {
			DrawPressed( downKey, false );
			downKey = -1;
			ScreenFlush();
		}
		dragging = false;
		return false;
	}

	//	Dragging the transcript scrolls it. Content follows the finger.
	if( y >= LogTop() && y < EditTop() ) {
		auto&	f = FACE[ LOG_FACE ];
		if( !dragging ) { dragging = true; dragY = y; dragScroll = scroll; }
		scroll = dragScroll + ( y - dragY ) / f.h;
		DrawLog();
		ScreenFlush();
		return false;
	}
	dragging = false;

	std::string	label;
	auto		key = -1;

	if( y < TAB_H ) {
		auto	i = x / ( SCREEN_W / TABS );
		if( i < 0 || i >= TABS ) return false;
		key = HIT_TAB + i;
		if( key == downKey ) return false;
		downKey = key;
		DrawTab( i, true );
		if( i == TAB_RESET ) {
			//	It goes out as the command, so that one place decides what
			//	forgetting means.
			ScreenFlush();
			editing.clear();
			line = ":forget";
			return true;
		}
		if( i == TAB_PROG ) {
			//	Likewise: the mode belongs to the session, and the session is
			//	main.cpp's. It answers by calling UISetProgram back.
			//
			//	What is half-typed goes with it. The two modes do not read a line
			//	the same way — in the calculator it is one sentence and in the
			//	other it is a series of forms — so a line begun under one of them
			//	is not the same line under the other.
			editing.clear();
			DrawInput();
			ScreenFlush();
			line = theProgram ? ":calc" : ":prog";
			return true;
		}
		if( i == TAB_RUN ) {
			if( !theProgram || editing.empty() ) { ScreenFlush(); return false; }
			line = editing;
			editing.clear();
			DrawInput();
			ScreenFlush();
			return true;
		}
		DropCharacter( editing );
		DrawInput();
		ScreenFlush();
		return false;
	}

	if( y >= FixedTop() ) {
		auto	r = ( y - FixedTop() ) / FIXED_KEY_H;
		auto	c = x / ( SCREEN_W / FIXED_COLS );
		if( r < 0 || r >= FIXED_ROWS || c < 0 || c >= FIXED_COLS ) return false;
		key = HIT_FIXED + r * FIXED_COLS + c;
		if( key == downKey ) return false;
		downKey = key;
		DrawFixedKey( r, c, true );
		label = FIXED[ r ][ c ];
	} else if( y >= KeysTop() ) {
		auto&	S = SECTION[ theSection ];
		auto	r = ( y - KeysTop() ) / S.keyH;
		if( r < 0 || r >= S.n ) return false;
		auto	c = x / ( SCREEN_W / S.rows[ r ].n );
		if( c < 0 || c >= S.rows[ r ].n ) return false;
		key = r * MAX_COLS + c;
		if( key == downKey ) return false;
		downKey = key;
		DrawExtraKey( r, c, true );
		label = S.rows[ r ].key[ c ];
	} else if( y >= ExtraTop() ) {
		auto	sec = x / ( SCREEN_W / SECTIONS );
		if( sec < 0 || sec >= SECTIONS ) return false;
		key = HIT_HEAD * MAX_COLS + sec;
		if( key == downKey ) return false;
		downKey = key;
		if( sec != theSection ) { theSection = sec; UIRedraw(); }
		return false;
	} else {
		return false;
	}

	if( label.empty() ) { ScreenFlush(); return false; }

	//	⏎ finishes the line in the calculator, where a line is the whole of what
	//	is being said. In programming mode it starts another one, and RUN is what
	//	finishes the lot — which is the difference between the modes made visible.
	if( label == KEY_RUN ) {
		if( theProgram ) {
			editing += '\n';
			DrawInput();
			ScreenFlush();
			return false;
		}
		if( editing.empty() ) { ScreenFlush(); return false; }
		line = editing;
		editing.clear();
		DrawInput();
		ScreenFlush();
		return true;
	}
	if( label == KEY_DEL ) {
		DropCharacter( editing );
	} else if( label == KEY_SP ) {
		editing += ' ';
	} else {
		editing += label;
		//	A function is a name, and a name wants air around it: `sin π` reads,
		//	`sinπ` is one name that does not exist.
		if( label.size() > 1 && label[ 0 ] >= 'a' && label[ 0 ] <= 'z' ) editing += ' ';
	}
	DrawInput();
	ScreenFlush();
	return false;
}

void
UIBackspace() {
	if( editing.empty() ) return;
	DropCharacter( editing );
	DrawEditing();
	ScreenFlush();
}

//	main.cpp says which mode the session is in, because it owns that and because
//	the wire can change it too: `:prog` typed over USB has to tick the box.
void
UISetProgram( bool _ ) {
	if( theProgram == _ ) return;
	theProgram = _;
	DrawTab( TAB_PROG, false );
	DrawTab( TAB_RUN, false );		//	it appears and disappears with the mode
	DrawLog();
	ScreenFlush();
}

void
UISetEditing( std::string const& _ ) {
	editing = _;
	DrawEditing();
	ScreenFlush();
}
