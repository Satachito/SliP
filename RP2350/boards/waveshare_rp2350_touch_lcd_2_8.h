//	Waveshare RP2350-Touch-LCD-2.8.  pico-sdk 2.1.1 has no header for this board,
//	and the numbers below were read off the chip with `picotool info -a -f`
//	rather than taken from a datasheet: RP2350A ( QFN60 ), revision A2, 16 MB of
//	flash.  Waveshare's own factory firmware is built against plain `pico2`, so
//	that is what this starts from; only the flash size differs.
//
//	The pico_cmake_ lines below are not decoration.  The SDK scans this file — and
//	only this file, not what it includes — for them before CMake configures, and
//	without the first one the board builds for RP2040: the UF2 comes out with the
//	rp2040 family ID and picotool refuses to load it onto this chip.

// -----------------------------------------------------
// NOTE: THIS HEADER IS ALSO INCLUDED BY ASSEMBLER SO
//       SHOULD ONLY CONSIST OF PREPROCESSOR DIRECTIVES
// -----------------------------------------------------

// pico_cmake_set PICO_PLATFORM=rp2350
// pico_cmake_set_default PICO_FLASH_SIZE_BYTES = (16 * 1024 * 1024)

#ifndef _BOARDS_WAVESHARE_RP2350_TOUCH_LCD_2_8_H
#define _BOARDS_WAVESHARE_RP2350_TOUCH_LCD_2_8_H

#define WAVESHARE_RP2350_TOUCH_LCD_2_8

#ifndef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES	( 16 * 1024 * 1024 )
#endif

//	The display and touch pins are not here yet: this build talks over USB and
//	does not light the screen.  They belong in this file when it does.

#include "boards/pico2.h"

#endif
