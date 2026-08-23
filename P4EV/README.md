# SliP on an ESP32-P4-Function-EV-Board

A calculator on the 7-inch 1024x600 panel this board ships with, and a serial
REPL over USB. The interpreter in [`C++/`](../C++) is compiled in unchanged, and
nothing in it names this chip.

The same chip as the [Tab5](../Tab5/README.md), and almost the same port. What
was not the same is written down below, because all three of those cost an
afternoon and none of them said what was wrong.

## Build and flash

Needs ESP-IDF and the RISC-V toolchain:

```sh
~/esp/esp-idf/install.sh esp32p4
```

```sh
. ~/esp/esp-idf/export.sh
cd P4EV && idf.py build flash monitor
```

The board appears as `/dev/cu.usbmodem*` on the port marked **USB-UART**, which
is the P4's own USB-Serial-JTAG and is what flashes it. The **USB-OTG** port next
to it enumerates as nothing.

## Wiring the panel

Two ribbons come in the box, one for the display and one for the camera, and
four Dupont wires between the LCD adapter board's **J6** header and the board:

| Panel header | Board  |
|--------------|--------|
| `5V`         | 5V     |
| `GND`        | GND    |
| `RST_LCD`    | GPIO27 |
| `PWM`        | GPIO26 |

`UP/DN`, `SHLR`, `INT_TP` and `RST_TP` are not connected. The BSP has
`BSP_LCD_TOUCH_RST` and `BSP_LCD_TOUCH_INT` as `GPIO_NUM_NC` and means it.

**The DSI ribbon goes in reversed.** Espressif's user guide says so in three
words and it is the whole of what a first afternoon with this board is. Both
faults it causes look like something else:

- the panel is black, because nothing arrives to draw
- the GT911 is absent from the I2C bus, because the touch controller's I2C comes
  down the same ribbon

They look like two problems, one of them a wiring mistake and the other a broken
touch panel. They are one problem, and `:i2c` finds it: an empty bus where the
codec answers at 0x18 and nothing answers at 0x5D is a ribbon in backwards.

The shipped `esp_brookesia_demo` makes this worse by calling `ESP_ERROR_CHECK`
on `bsp_touch_new`, so a board with no touch panics before it has drawn anything
and sits in a boot loop showing nothing at all.

## What differs from the Tab5

Three things, and each of them fails in a way that does not name itself.

**The PHY clock source.** The Tab5 passes `MIPI_DSI_PHY_CLK_SRC_DEFAULT`. That
is a legacy alias, and on this board's v3.2 P4 the register write behind it
asserts inside `esp_lcd_new_dsi_bus` and calls `abort()` with no message. Zero —
meaning let the driver choose — is what the BSP itself passes.

**`esp_lcd_panel_disp_on_off`.** The Tab5's ST7123 has that command; this
EK79007 does not. A DPI panel is displaying as soon as the peripheral is
scanning it out, and the backlight is the only switch there is. Checking the
return is a boot loop.

**`bsp_display_brightness_init`.** The Tab5 does not need it. Here the backlight
does not come on without it, which looks exactly like a panel that does not
work.

And one that is not a difference so much as an opposite: the Tab5's
`sdkconfig.defaults` asks for the pre-v3 half of a mutually exclusive Kconfig
choice, because its P4 is revision v1.3. This one is v3.2, which is what ESP-IDF
5.5 defaults to, so those two lines are absent — carrying them over refuses to
flash the chip, which is the same wall from the other side.

## The screen

Landscape, where the Tab5 is portrait, so the keypad is a column on the right
and the transcript takes the rest. That is the arrangement every host has now
arrived at independently: the keys beside the work where there is width for
them, and under it where there is not. Here there is width and not much height,
which decides it.

The panel column is 546 pixels, which is thirteen columns of forty-two for the
Latin alphabet; everything else divides more easily than that does.

The two columns are measured apart. On the Tab5 they are one stack, so the line
being written sits on top of the panel and moves with it when a section changes
height. Here the line belongs to the bottom of its own column, and leaving the
two coupled produced a transcript of negative height and a panic on the first
draw.

## What works

Verified on the board:

- `:version` — 2.3.0
- `:i2c` — `0x18 ES8311`, `0x5D GT911`
- `-π × 2` — `-6.283185307179586`, the constant-sign fix of 2.2.0
- `( 'r = 2 )` then `( 2 π r )` — `12.5663706143592`, so the session carries
- `:free` — 32 MB, the PSRAM entirely available to the language
- the panel, the backlight, and touch coordinates across the whole 1024x600

## What is not done

The session is not saved across a power cut here. `store.cpp` came over from the
Tab5 and compiles, but nothing has been tested; the ESP32 and the RP2350 both do
this and this board has the same NVS underneath, so it is a matter of running it
rather than of writing it.
