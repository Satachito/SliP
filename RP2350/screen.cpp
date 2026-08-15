//	The 2.8-inch panel and the touch panel on it.
//
//	ST7789T3 over spi1 and CST328 over i2c1.  The pins, the SPI mode, the
//	register-init sequence and the touch read are all specific to this board;
//	they were taken from Waveshare's own demo for it, which is the only place
//	they are written down — the wiki gives the part numbers and puts the pinout
//	in a picture.
//
//	The frame is held in RAM and pushed whole.  240x320 at 16 bits is 150 KB,
//	which the chip has and to spare, and it means the UI above never has to think
//	about partial updates or tearing.

#include "screen.hpp"

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"

#include "font.h"

namespace {

//	ST7789T3
#define	LCD_SPI		spi1
constexpr uint	LCD_SCLK	= 10;
constexpr uint	LCD_MOSI	= 11;
constexpr uint	LCD_CS		= 13;
constexpr uint	LCD_DC		= 14;
constexpr uint	LCD_RST		= 15;
constexpr uint	LCD_BL		= 16;

//	CST328
#define	TP_I2C		i2c1
constexpr uint	TP_SDA		= 6;
constexpr uint	TP_SCL		= 7;
constexpr uint	TP_RST		= 17;
constexpr uint	TP_INT		= 18;
constexpr uint8_t TP_ADDR	= 0x1A;
constexpr uint16_t TP_FLAG	= 0xD005;		//	low nibble is the number of points
constexpr uint16_t TP_FIRST	= 0xD000;

uint16_t	frame[ SCREEN_W * SCREEN_H ];

void
Cmd( uint8_t _ ) {
	gpio_put( LCD_DC, 0 );
	gpio_put( LCD_CS, 0 );
	spi_write_blocking( LCD_SPI, &_, 1 );
	gpio_put( LCD_CS, 1 );
}

void
Data( uint8_t _ ) {
	gpio_put( LCD_DC, 1 );
	gpio_put( LCD_CS, 0 );
	spi_write_blocking( LCD_SPI, &_, 1 );
	gpio_put( LCD_CS, 1 );
}

void
Data( uint8_t const* _, size_t n ) {
	gpio_put( LCD_DC, 1 );
	gpio_put( LCD_CS, 0 );
	spi_write_blocking( LCD_SPI, _, n );
	gpio_put( LCD_CS, 1 );
}

//	Panel-specific.  These are not general ST7789 values: the gamma curves and
//	the voltages belong to the panel Waveshare fitted, and are theirs.
void
RegisterInit() {
	Cmd( 0x11 );					//	sleep out
	sleep_ms( 120 );
	Cmd( 0x36 ); Data( 0x00 );		//	MADCTL — portrait, RGB
	Cmd( 0x3A ); Data( 0x05 );		//	16 bits per pixel

	Cmd( 0xB0 ); Data( 0x00 ); Data( 0xE8 );
	Cmd( 0xB2 ); Data( 0x0C ); Data( 0x0C ); Data( 0x00 ); Data( 0x33 ); Data( 0x33 );
	Cmd( 0xB7 ); Data( 0x75 );
	Cmd( 0xBB ); Data( 0x1A );
	Cmd( 0xC0 ); Data( 0x2C );
	Cmd( 0xC2 ); Data( 0x01 );
	Cmd( 0xC3 ); Data( 0x13 );
	Cmd( 0xC4 ); Data( 0x20 );
	Cmd( 0xC6 ); Data( 0x0F );
	Cmd( 0xD0 ); Data( 0xA4 ); Data( 0xA1 );
	Cmd( 0xD6 ); Data( 0xA1 );

	static const uint8_t gammaP[] = {
		0xD0, 0x0D, 0x14, 0x0D, 0x0D, 0x09, 0x38, 0x44, 0x4E, 0x3A, 0x17, 0x18, 0x2F, 0x30 };
	static const uint8_t gammaN[] = {
		0xD0, 0x09, 0x0F, 0x08, 0x07, 0x14, 0x37, 0x44, 0x4D, 0x38, 0x15, 0x16, 0x2C, 0x2E };
	Cmd( 0xE0 ); for( auto _: gammaP ) Data( _ );
	Cmd( 0xE1 ); for( auto _: gammaN ) Data( _ );

	Cmd( 0x21 );					//	inversion on — this panel wants it
	Cmd( 0x29 );					//	display on
}

int
GlyphOf( uint32_t codepoint ) {
	int lo = 0, hi = FONT_N - 1;
	while( lo <= hi ) {
		auto mid = ( lo + hi ) / 2;
		if( FONT_CODEPOINT[ mid ] == codepoint ) return mid;
		if( FONT_CODEPOINT[ mid ] <  codepoint ) lo = mid + 1; else hi = mid - 1;
	}
	return -1;
}

void
WriteReg16( uint16_t reg, uint8_t const* data, size_t n ) {
	uint8_t head[ 2 ] = { uint8_t( reg >> 8 ), uint8_t( reg ) };
	i2c_write_blocking( TP_I2C, TP_ADDR, head, 2, true );
	i2c_write_blocking( TP_I2C, TP_ADDR, data, n, false );
}

bool
ReadReg16( uint16_t reg, uint8_t* data, size_t n ) {
	uint8_t head[ 2 ] = { uint8_t( reg >> 8 ), uint8_t( reg ) };
	if( i2c_write_blocking( TP_I2C, TP_ADDR, head, 2, true ) < 0 ) return false;
	return i2c_read_blocking( TP_I2C, TP_ADDR, data, n, false ) == (int)n;
}

}	//	namespace

void
ScreenInit() {
	spi_init( LCD_SPI, 62'500'000 );
	spi_set_format( LCD_SPI, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST );
	gpio_set_function( LCD_MOSI, GPIO_FUNC_SPI );
	gpio_set_function( LCD_SCLK, GPIO_FUNC_SPI );

	for( auto pin: { LCD_DC, LCD_CS, LCD_RST, LCD_BL } ) {
		gpio_init( pin );
		gpio_set_dir( pin, GPIO_OUT );
	}
	gpio_put( LCD_CS, 1 );

	gpio_put( LCD_RST, 1 ); sleep_ms( 10 );
	gpio_put( LCD_RST, 0 ); sleep_ms( 20 );
	gpio_put( LCD_RST, 1 ); sleep_ms( 120 );

	RegisterInit();
	ScreenClear( 0 );
	ScreenFlush();
	gpio_put( LCD_BL, 1 );			//	backlight last, so the first thing seen is the frame
}

void
ScreenClear( uint16_t colour ) {
	for( auto& _: frame ) _ = colour;
}

void
ScreenFill( int x, int y, int w, int h, uint16_t colour ) {
	if( x < 0 ) { w += x; x = 0; }
	if( y < 0 ) { h += y; y = 0; }
	if( x + w > SCREEN_W ) w = SCREEN_W - x;
	if( y + h > SCREEN_H ) h = SCREEN_H - y;
	for( auto r = 0; r < h; r++ )
		for( auto c = 0; c < w; c++ )
			frame[ ( y + r ) * SCREEN_W + x + c ] = colour;
}

//	One glyph, from the generated font.  An unknown codepoint draws nothing rather
//	than a box: the alphabet is fixed and a box would only be noise.
void
ScreenGlyphAt( int x, int y, uint32_t codepoint, uint16_t fg, uint16_t bg ) {
	if( x < 0 || y < 0 || x + FONT_W > SCREEN_W || y + FONT_H > SCREEN_H ) return;
	auto g = GlyphOf( codepoint );
	for( auto r = 0; r < FONT_H; r++ ) {
		auto bits = g < 0 ? 0 : FONT_BITS[ g ][ r ];
		for( auto c = 0; c < FONT_W; c++ )
			frame[ ( y + r ) * SCREEN_W + x + c ]
			=	( bits >> ( FONT_W - 1 - c ) ) & 1 ? fg : bg;
	}
}

//	UTF-8 in, glyphs out.  Everything above this speaks UTF-8, as the rest of the
//	interpreter does.
namespace {
uint32_t
Decode( unsigned char const*& p ) {
	uint32_t	cp	= *p;
	auto		n	= 1;
	if     ( ( *p & 0xF8 ) == 0xF0 ) { cp = *p & 0x07; n = 4; }
	else if( ( *p & 0xF0 ) == 0xE0 ) { cp = *p & 0x0F; n = 3; }
	else if( ( *p & 0xE0 ) == 0xC0 ) { cp = *p & 0x1F; n = 2; }
	for( auto i = 1; i < n; i++ ) {
		if( ( p[ i ] & 0xC0 ) != 0x80 ) { n = i; break; }
		cp = ( cp << 6 ) | ( p[ i ] & 0x3F );
	}
	p += n;
	return cp;
}
}

int
ScreenTextAt( int x, int y, char const* _, uint16_t fg, uint16_t bg ) {
	auto p = (unsigned char const*)_;
	while( *p ) {
		ScreenGlyphAt( x, y, Decode( p ), fg, bg );
		x += FONT_W;
	}
	return x;
}

int
ScreenWidth( char const* _ ) {
	auto p = (unsigned char const*)_;
	auto n = 0;
	while( *p ) { Decode( p ); n++; }
	return n * FONT_W;
}

void
ScreenGlyph( int col, int row, uint32_t codepoint, uint16_t fg, uint16_t bg ) {
	ScreenGlyphAt( col * FONT_W, row * FONT_H, codepoint, fg, bg );
}

int
ScreenText( int col, int row, char const* _, uint16_t fg, uint16_t bg ) {
	return ScreenTextAt( col * FONT_W, row * FONT_H, _, fg, bg ) / FONT_W;
}

void
ScreenFlush() {
	Cmd( 0x2A ); Data( 0 ); Data( 0 ); Data( ( SCREEN_W - 1 ) >> 8 ); Data( ( SCREEN_W - 1 ) & 0xFF );
	Cmd( 0x2B ); Data( 0 ); Data( 0 ); Data( ( SCREEN_H - 1 ) >> 8 ); Data( ( SCREEN_H - 1 ) & 0xFF );
	Cmd( 0x2C );
	//	Straight out, no byte swapping.  RAMCTRL was given 0xE8 above, whose bit 3
	//	puts the panel in little-endian mode, and the chip is little-endian too —
	//	so the frame is already in the order the panel reads.  Swapping it "into
	//	big-endian" swapped it out of agreement instead: the background came up
	//	magenta and the text cyan, which is exactly 0x10A3 and 0xDF1D read the
	//	wrong way round.
	gpio_put( LCD_DC, 1 );
	gpio_put( LCD_CS, 0 );
	spi_write_blocking( LCD_SPI, (uint8_t const*)frame, sizeof( frame ) );
	gpio_put( LCD_CS, 1 );
}

void
TouchInit() {
	i2c_init( TP_I2C, 400'000 );
	gpio_set_function( TP_SDA, GPIO_FUNC_I2C );
	gpio_set_function( TP_SCL, GPIO_FUNC_I2C );
	gpio_pull_up( TP_SDA );
	gpio_pull_up( TP_SCL );

	gpio_init( TP_INT ); gpio_set_dir( TP_INT, GPIO_IN ); gpio_pull_up( TP_INT );
	gpio_init( TP_RST ); gpio_set_dir( TP_RST, GPIO_OUT );

	gpio_put( TP_RST, 0 ); sleep_ms( 10 );
	gpio_put( TP_RST, 1 ); sleep_ms( 50 );
}

//	One finger is all this UI wants.  Returns false when nothing is down.
bool
TouchRead( int& x, int& y ) {
	uint8_t	flag = 0;
	if( !ReadReg16( TP_FLAG, &flag, 1 ) ) return false;
	if( ( flag & 0x0F ) == 0 ) return false;

	uint8_t	buf[ 27 ];
	if( !ReadReg16( TP_FIRST, buf, sizeof( buf ) ) ) return false;
	//	The controller marks a real report in the low nibble of the first byte;
	//	anything else is a frame that arrived mid-update.
	if( ( buf[ 0 ] & 0x0F ) != 0x06 ) return false;

	x = ( buf[ 1 ] << 4 ) | ( buf[ 3 ] >> 4 );
	y = ( buf[ 2 ] << 4 ) | ( buf[ 3 ] & 0x0F );

	//	Tell it the report was taken, or the same one comes back for ever.
	uint8_t zero = 0;
	WriteReg16( TP_FLAG, &zero, 1 );

	return x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H;
}
