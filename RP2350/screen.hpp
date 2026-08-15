//	The panel, as much of it as anything above needs to know.
//
//	Coordinates are pixels; text is placed on a character grid, because that is
//	what this UI is: a transcript and a keypad, both of them cells.

#pragma once

#include <cstdint>
#include <cstddef>

constexpr int SCREEN_W = 240;
constexpr int SCREEN_H = 320;

//	RGB565.
constexpr uint16_t
RGB( uint8_t r, uint8_t g, uint8_t b ) {
	return uint16_t( ( ( r & 0xF8 ) << 8 ) | ( ( g & 0xFC ) << 3 ) | ( b >> 3 ) );
}

void	ScreenInit();
void	ScreenClear( uint16_t colour );
void	ScreenFill( int x, int y, int w, int h, uint16_t colour );
//	Pixels, for anything whose box is not a multiple of the cell — the keypad,
//	whose keys are 30 tall against a 20-tall cell.  Placing those on the cell
//	grid rounded each row differently and tilted the whole pad.
void	ScreenGlyphAt( int x, int y, uint32_t codepoint, uint16_t fg, uint16_t bg );
int		ScreenTextAt( int x, int y, char const* utf8, uint16_t fg, uint16_t bg );

//	Cells, for the transcript, which is a grid.
void	ScreenGlyph( int col, int row, uint32_t codepoint, uint16_t fg, uint16_t bg );
int		ScreenText( int col, int row, char const* utf8, uint16_t fg, uint16_t bg );

//	How wide a string is, in pixels, for centring it.
int		ScreenWidth( char const* utf8 );
void	ScreenFlush();

void	TouchInit();
bool	TouchRead( int& x, int& y );
