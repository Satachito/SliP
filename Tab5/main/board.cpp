//	The parts of the Tab5 that are not the chip.
//
//	Everything on this board hangs off one I2C bus — the touch controller, two IO
//	expanders, the battery monitor, the codec, the clock — and two of the things
//	the panel needs most, its own enable line and the touch controller's, are not
//	GPIOs at all: they are pins on an expander, which is itself on that bus.
//
//	The bus is the board support package's, not one opened here.  There is one
//	I2C port involved and two owners would be one too many: whichever asks first
//	gets it and the other is told it is taken, which depending on the order of
//	the morning is either the panel or this file failing for no visible reason.
//
//	What this file is for is asking who is there.  Which panel M5 fitted decides
//	the timings, the initialisation and the touch driver, and it changed partway
//	through production: an ILI9881C with a separate GT911 up to October 2025, an
//	ST7123 that is display and touch at once after it.  The BSP works it out by
//	probing 0x55, and so can anyone, which is what `:i2c` is.

#include <cstdio>
#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"
#include "esp_err.h"

#include "bsp/m5stack_tab5.h"

using namespace std;

//	Let go of the panel and the touch controller. Both are held down out of
//	power-up through the expander, so neither answers for anything until this
//	runs — which is the trap in scanning the bus before it.
esp_err_t
BoardReleaseResets() {
	auto	e = bsp_i2c_init();
	if( e != ESP_OK ) return e;
	bsp_feature_enable( BSP_FEATURE_LCD,   true );
	bsp_feature_enable( BSP_FEATURE_TOUCH, true );
	vTaskDelay( pdMS_TO_TICKS( 500 ) );		//	the touch controller is slow to answer
	return ESP_OK;
}

//	Names for the addresses this board is documented to use; anything else is
//	reported as a number, because a number where nothing was expected is the
//	interesting part.
static char const*
Known( uint8_t address ) {
	switch( address ) {
	case 0x10: return "audio codec";
	case 0x14: return "touch, GT911 alternate  ( board version 1 )";
	case 0x32: return "clock";
	case 0x41: return "INA226, battery";
	case 0x43: return "PI4IOE5V6408 #1  ( LCD enable, touch enable )";
	case 0x44: return "PI4IOE5V6408 #2";
	case 0x55: return "ST7123, display and touch  ( board version 2 )";
	case 0x5D: return "touch, GT911  ( board version 1 )";
	case 0x68: return "IMU";
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
	return out.empty() ? string( "  nothing answered\n" ) : out;
}
