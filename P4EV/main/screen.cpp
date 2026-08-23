//	The Tab5's panel.
//
//	Nothing like the RP2350's.  There the frame lives in RAM and is pushed to the
//	panel over SPI, whole, on every change — 150 KB a keystroke, which works
//	because the panel is small.  Here the panel is driven by MIPI-DSI and the
//	hardware scans the frame buffer out by itself, continuously, from PSRAM.
//	There is nothing to push: drawing is writing to memory, and the picture
//	follows on its own.  So the thing the RP2350's README called the next port's
//	problem — DMA and partial updates for a bigger panel — turns out not to exist
//	on this one.
//
//	What it has instead is that the panel takes a page of vendor register writes
//	to wake up.  Here it is an EK79007, 1024x600, on the board's MIPI-DSI
//	connector, with a GT911 for touch on the shared I2C bus at 0x5D — and both
//	of them arrive over the same ribbon, which is why they failed together.  The
//	cable goes in reverse; until it did, the panel drew nothing and the GT911 was
//	not on the bus at all.
//
//	Which is why this file calls Espressif's board support package rather than
//	driving the panel itself.  Every number involved — the timings, the lane
//	rate, the initialisation table, which of the two panels is fitted — belongs
//	to the board rather than to SliP, and the people who make the board publish
//	them and keep them right.  Doing it here by hand did light the DSI up and
//	left the screen black, which is the other argument.
//
//	The BSP brings up the panel and stops.  Turning the display on and turning
//	the backlight up are the caller's, deliberately — its own header says so —
//	and forgetting either of them looks exactly like a panel that does not work.
//
//	And the frame buffer is in PSRAM, which the processor writes through a cache
//	and the display reads without one.  Writing to it is not the whole of drawing
//	after all: what has been written has to be pushed out of the cache before the
//	panel can see it.  Something large is pushed out on its own, by the writes
//	that come after it — the first thing drawn here was four bands of 450 KB and
//	they appeared — and something small sits in the cache and never arrives. The
//	white square in the corner of the test pattern did not, which is what said
//	so.

#include <cstdint>

#include "esp_cache.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_ops.h"

#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "bsp/touch.h"
#include "esp_lcd_touch.h"

static char const*	TAG = "screen";

int constexpr	SCREEN_W	= BSP_LCD_H_RES;	//	1024
int constexpr	SCREEN_H	= BSP_LCD_V_RES;	//	600

static esp_lcd_panel_handle_t	thePanel	= nullptr;
static uint16_t*				theFrame	= nullptr;

uint16_t*	ScreenFrame()	{ return theFrame; }

esp_err_t
ScreenInit() {
	if( thePanel ) return ESP_OK;

	//	Not null. The BSP reads config->dsi_bus without checking it, so passing
	//	nothing is a load fault two lines inside it.
	bsp_display_config_t	config = {
		.dsi_bus = {
			//	Zero, meaning let the driver choose — which is what this board's
			//	BSP passes when it is the one calling.  Not
			//	MIPI_DSI_PHY_CLK_SRC_DEFAULT, which the Tab5 uses: that is a
			//	legacy alias, and on a v3.2 P4 the register write behind it
			//	asserts inside esp_lcd_new_dsi_bus and aborts with no message.
			.phy_clk_src		= (mipi_dsi_phy_clock_source_t)0
		,	.lane_bit_rate_mbps	= BSP_LCD_MIPI_DSI_LANE_BITRATE_MBPS
		}
	};
	bsp_lcd_handles_t	handles = {};
	auto	e = bsp_display_new_with_handles( &config, &handles );
	if( e != ESP_OK ) return e;
	thePanel = handles.panel;

	//	The backlight is not done for us. Without it the panel is dark with the
	//	panel on, which looks exactly like a panel that does not work.
	//
	//	disp_on_off is not checked. The Tab5's ST7123 has that command and this
	//	EK79007 does not — a DPI panel is displaying as soon as the peripheral is
	//	scanning it out, and the backlight is the only switch there is. Checking
	//	it here is how the first attempt at this board sat in a reboot loop.
	esp_lcd_panel_disp_on_off( thePanel, true );
	ESP_ERROR_CHECK( bsp_display_brightness_init() );
	ESP_ERROR_CHECK( bsp_display_brightness_set( 100 ) );

	//	The frame buffer is the panel's own, in PSRAM, and the display hardware
	//	reads it out continuously. Writing to it is the whole of drawing.
	ESP_ERROR_CHECK( esp_lcd_dpi_panel_get_frame_buffer( thePanel, 1, (void**)&theFrame ) );
	ESP_LOGI( TAG, "frame buffer %p, %d x %d", theFrame, SCREEN_W, SCREEN_H );
	return ESP_OK;
}

//	Hand the written pixels to the panel: write the cache back to the PSRAM the
//	display is reading. Whole frame, because the cost is a writeback rather than
//	a copy — 1.8 MB of it takes well under a millisecond — and because a dirty
//	rectangle is a thing to get wrong quietly.
void
ScreenFlush() {
	if( !theFrame ) return;
	esp_cache_msync( theFrame, SCREEN_W * SCREEN_H * 2, ESP_CACHE_MSYNC_FLAG_DIR_C2M );
}

//	The touch controller: a GT911 at 0x5D, on the same I2C bus as the audio codec
//	and reached through the same ribbon as the display.
static esp_lcd_touch_handle_t	theTouch = nullptr;

esp_err_t
TouchInit() {
	if( theTouch ) return ESP_OK;
	return bsp_touch_new( nullptr, &theTouch );
}

//	Where a finger is, or false if there is none. One point: this panel reports
//	five, and a calculator has no use for the other four.
bool
TouchPoint( int& x, int& y ) {
	if( !theTouch ) return false;
	esp_lcd_touch_read_data( theTouch );

	uint16_t	xs[ 1 ], ys[ 1 ];
	uint8_t		n = 0;
	if( !esp_lcd_touch_get_coordinates( theTouch, xs, ys, nullptr, &n, 1 ) || !n ) return false;
	x = xs[ 0 ];
	y = ys[ 0 ];
	return true;
}

void
ScreenFill( int x, int y, int w, int h, uint16_t colour ) {
	if( !theFrame ) return;
	if( x < 0 ) { w += x; x = 0; }
	if( y < 0 ) { h += y; y = 0; }
	if( x + w > SCREEN_W ) w = SCREEN_W - x;
	if( y + h > SCREEN_H ) h = SCREEN_H - y;
	if( w <= 0 || h <= 0 ) return;
	for( auto r = y; r < y + h; r++ ) {
		auto	p = theFrame + r * SCREEN_W + x;
		for( auto c = 0; c < w; c++ ) *p++ = colour;
	}
}

//	Something with a known top, a known left and known colours, because the only
//	way to check a screen from here is to have someone say what is on it.
void
ScreenTestPattern() {
	if( !theFrame ) return;
	ScreenFill( 0, 0,                SCREEN_W, SCREEN_H / 4, 0xF800 );	//	red
	ScreenFill( 0, SCREEN_H / 4,     SCREEN_W, SCREEN_H / 4, 0x07E0 );	//	green
	ScreenFill( 0, SCREEN_H / 2,     SCREEN_W, SCREEN_H / 4, 0x001F );	//	blue
	ScreenFill( 0, SCREEN_H * 3 / 4, SCREEN_W, SCREEN_H / 4, 0x0000 );
	//	A white square in one corner: it names which corner is which.
	ScreenFill( 0, 0, 80, 80, 0xFFFF );
	ScreenFlush();
}
