//	The parts of this board that are not the chip.
//
//	Less than the Tab5 needed.  There the panel and the touch controller are both
//	held in reset through an IO expander, so nothing answers until something lets
//	go of them, and a bus scan before that returns an empty room.  Here they are
//	not held at all: the ribbon carries the display and the touch controller's
//	I2C together, and the only question is whether the ribbon is the right way
//	round.  It goes in reversed — the board's own documentation says so — and
//	until it did, the panel drew nothing and the GT911 was absent from the bus.
//	Those looked like two faults and were one.
//
//	The bus is the board support package's, not one opened here.  There is one
//	I2C port involved and two owners would be one too many: whichever asks first
//	gets it and the other is told it is taken, which depending on the order of
//	the morning is either the panel or this file failing for no visible reason.

#include <cstdio>
#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"
#include "esp_err.h"

#include "bsp/esp-bsp.h"

using namespace std;

//	Nothing to release.  Kept so that main.cpp reads the same on both P4 boards,
//	and because starting the bus before scanning it still has to happen.
esp_err_t
BoardReleaseResets() {
	return bsp_i2c_init();
}

//	Names for the addresses this board is documented to use; anything else is
//	reported as a number, because a number where nothing was expected is the
//	interesting part.
static char const*
Known( uint8_t address ) {
	switch( address ) {
	case 0x14: return "touch, GT911 alternate";
	case 0x18: return "ES8311, audio codec";
	case 0x5D: return "touch, GT911";
	default:   return nullptr;
	}
}

string
BoardScan() {
	if( BoardReleaseResets() != ESP_OK ) return "! the board's I2C would not start\n";

	auto	bus = bsp_i2c_get_handle();
	string	out;
	for( uint8_t a = 0x08; a < 0x78; a++ ) {
		if( i2c_master_probe( bus, a, 50 ) != ESP_OK ) continue;
		char	line[ 96 ];
		auto	name = Known( a );
		snprintf( line, sizeof( line ), "  0x%02X  %s", a, name ? name : "?" );
		out += line;
		out += '\n';
	}
	//	An empty room here means the ribbon, nine times in ten.
	return out.empty() ? string( "  nothing answered — is the DSI ribbon in reversed?\n" ) : out;
}
