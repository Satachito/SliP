# SliP on an M5Stack Tab5

A calculator on a 5-inch panel, and a serial REPL over USB, on an M5Stack Tab5 —
an ESP32-P4. The interpreter in [`C++/`](../C++) is compiled in unchanged, and
nothing in it names this chip.

The chip is RISC-V, where the [ESP32 port](../ESP32/README.md) is Xtensa and the
[RP2350 port](../RP2350/README.md) is ARM. That is the whole of what a third
instruction set cost: ESP-IDF and the interpreter each cross it on their own.

Both inputs feed one session, so a binding made by tapping is visible to a line
typed over the wire and the other way round.

## Build and flash

Needs ESP-IDF and the RISC-V toolchain. `install.sh` does not fetch the latter
unless a RISC-V target is asked for, and the Xtensa one it already has is no use
here:

```sh
~/esp/esp-idf/install.sh esp32p4
```

```sh
. ~/esp/esp-idf/export.sh
cd Tab5 && idf.py build flash monitor
```

The board appears as `/dev/cu.usbmodem*`. There is no serial adapter on it — no
CH340, nothing to install a driver for — because the P4 speaks USB itself.

## The chip revision matters

This board's P4 is **revision v1.3**, and ESP-IDF 5.5 builds for v3.1 and above
by default. The two are not one family with a bugfix between them; the SDK's own
Kconfig says support for revisions below 3.0 and from 3.0 up is *mutually
exclusive*, and a binary built for one is refused at flash time by the other:

```
A fatal error occurred: bootloader/bootloader.bin requires chip revision
in range [v3.1 - v3.99] (this chip is revision v1.3).
```

So `sdkconfig.defaults` asks for the older side explicitly rather than leaving it
to a default that assumes a newer chip. Read the revision off the chip rather
than off the box — `esptool.py chip_id` prints it.

## Using the panel

```
┌────────┬────────┬────────┬────────┐
│ Reset  │ ☐ prog │  RUN   │ Delete │
└────────┴────────┴────────┴────────┘

           transcript, dragged to scroll
           the line being typed

┌───────────┬───────────┬───────────┐
│   SliP    │   func    │    αβγ    │
└───────────┴───────────┴───────────┘
      whichever of the three is selected

        7  8  9  +  '  𝑒
        4  5  6  −  =  (
        1  2  3  ×  @  )
        0  .  ␣  ÷  :  ⏎
```

The block at the bottom never moves. Six across and four down, the full width,
the same size whatever else is showing — the digits are the digits, and a key
that moves when something else changes is a key you have to look at before
pressing. Its last column is the line: `𝑒` because it is a number, then open it,
close it, run it.

Above it, one of three panels:

| | |
|---|---|
| `SliP` | the operators — `∥ ⟨ ⟩ ⊂ ⊃ ∩ ∪ ⊤ ⊥ ∅ « » ¶ ¤ ¦ § ¬ ± · ¿ ∈ ∋ £` and the rest |
| `func` | the transcendental functions the interpreter already has |
| `αβγ` | the Greek alphabet, both cases, 48 letters |

They are tabbed rather than stacked because they are three alphabets and a line
is being written in one of them. Each keeps its own size: a function is a
five-letter name and wants room, the operators are forty-odd keys and only fit
small.

**This is the first host where a name can be invented on the panel.** The reader
has always taken Greek letters as names — `Read.cpp` has listed them since long
before any of this — and until now there was never room to offer them. The
RP2350's pad had no letters at all, so a session there had `M1` and `M2` and no
other names.

```
( ' Δ = ' 7 )
( Δ × Δ )       → 49
```

The bar along the top is the three things that do not put a character in: throw
the session away, change what a line means, take a character back. RUN is there
only in programming mode.

### The two modes

The `prog` box is the same switch the wire has as `:calc` and `:prog`, and the
same one the web calculator has. Ticking it from either side ticks it on both.

- **Unticked** — a line is one sentence, so `2πr` is `( 2πr )`. `⏎` runs it.
- **Ticked** — a line is toplevel forms. `⏎` starts another line and **RUN**
  runs the lot, so a program is written before it is run rather than a line at a
  time. While it is being written it lives in the space the transcript uses,
  because a program wants to be seen whole; RUN turns it into the transcript's
  next few entries and hands the space back.

Changing the mode clears the screen and whatever was half-typed. What was there
was read under the other mode, and is not the same thing under this one.

## The session is still there after a power cut

Switch the board off and on and last session comes back, transcript and all:

```
SliP 2.1.1  —  :help
5 lines restored
```

The same design as the [ESP32](../ESP32/README.md#the-session-is-still-there-after-a-power-cut)
and the same file: what is saved is the source — the lines that were run, in the
order they were run — and booting replays them. `store.cpp` came from that port
and needed nothing changing.

A program handed over by RUN arrives as one string with newlines in it. It is
recorded as `:{` … `:}` rather than as a line with newlines inside, so it comes
back from a power cut as the block it was and not as several unrelated lines.

`Reset` — `:reset` or `:forget` on the wire — erases the saved copy along with
the live one. It has to: the log is what built the bindings.

## What the chip needed

Nothing in `C++/`. `SLIP_NO_THREADS` does not apply — this host has FreeRTOS, so
`∥` runs its branches on threads as the desktop build does — and `RandomSeed`
takes `esp_random()` through the same branch the ESP32 uses.

The host needed two things. **The console is not a UART**: the ESP32 board
reaches the Mac through a CH340, so its `StartConsole` installs the UART driver;
the P4 is the USB device, so this one installs `usb_serial_jtag`. There is no
baud rate in this file.

And **the console read cannot block.** The ESP32's `ReadLine` calls `fgetc`,
which waits — which is fine on a board with no other input. Here waiting on the
wire is waiting with a finger already on the panel, so the read has a 10 ms
timeout and the loop around it looks at the touch controller a hundred times a
second. The RP2350 never had this problem because `getchar_timeout_us( 0 )` does
not wait. Three ports, three different ways of asking whether a byte has
arrived.

## The panel

`screen.cpp` is thin, and deliberately. Every number a MIPI-DSI panel needs — the
timings, the lane rate, the initialisation table — belongs to the board rather
than to SliP, and M5 changed the panel partway through production:

| | up to October 2025 | after |
|---|---|---|
| Display | ILI9881C | ST7123 |
| Touch | GT911, separate | ST7123, the same part |

They take different timings and different initialisation and are not
interchangeable. **This board answers on 0x55, so it is the later one** — `:i2c`
prints what is on the bus and is how that was settled. The factory firmware that
shipped on it contains the names of both and decides at run time; so does
Espressif's board support package, by the same probe, which is why this port
calls it rather than driving the panel itself. Doing it here by hand lit the DSI
up and left the screen black.

Two things the BSP deliberately leaves to the caller, both of which look exactly
like a panel that does not work: `esp_lcd_panel_disp_on_off` and the backlight.

### Drawing is writing to memory, nearly

Nothing here is pushed to the panel. The frame buffer lives in PSRAM and the
display hardware scans it out continuously, so drawing is writing to memory —
which means the problem the RP2350's README left for the next port, DMA and
partial updates for a bigger screen, does not exist on this one. 720×1280×16 is
1.8 MB and the panel reads it by itself.

What replaces it is that the processor writes that memory through a cache and
the display reads it without one. Something large is pushed out of the cache on
its own by the writes that follow it; something small sits there and never
arrives. The first test pattern was four bands of 450 KB and a white square of
12 KB, and the bands appeared. `ScreenFlush` is the writeback.

## The font

[`font.py`](font.py) bakes the glyphs into `main/font.h`, which is committed so
the firmware builds without it. The same reasoning as the RP2350's: no stock font
has SliP's alphabet, and no single face on a Mac has all of it — Menlo carries
ASCII and most of the operators, Apple Symbols nearly all the rest, and `𝑒`
(U+1D452) only STIX Two Math.

Three sizes rather than one scaled three ways, because scaling a bitmap font by
1.5 is how you get a font that looks like a scaled bitmap font. 186 glyphs.

## What it costs on the chip

| | |
|---|---|
| Image | 1.1 MB of a 4 MB app partition |
| Flash on the board | 16 MB |
| PSRAM | 32 MB, of which 1.8 MB is the frame buffer |
| conformance | 11 / 11 |

## Verified on hardware

```sh
python3 conformance/board.py /dev/cu.usbmodem31201
```

11 passed, 0 failed — the whole [`conformance/`](../conformance) suite, run on
the board over its USB serial line. The same script answers for the ESP32 and the
RP2350; nothing in it knows which chip is on the other end, and it now does not
know which instruction set either.

## Not done here

- The camera, the microphones, the IMU, the real-time clock, Wi-Fi through the
  C6, the SD slot, the battery gauge. `:i2c` can see several of them; the
  language cannot reach any of them.
- No Latin letters on the pad. The Greek is there because the reader takes it as
  names and it fits; the alphabet that would need a keyboard still needs one.
- **Flashing this replaces what the board shipped with.** M5Stack's factory
  firmware here was a face-detection demo in a 10 MB app partition with its model
  in a SPIFFS partition beside it. Read the whole flash out before overwriting it
  if you want it back:
  ```sh
  esptool.py -p PORT read_flash 0 0x1000000 tab5-factory.bin
  ```
