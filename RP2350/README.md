# SliP on RP2350

A calculator on the panel, and a serial REPL over USB, on a Waveshare
RP2350-Touch-LCD-2.8. The interpreter in [`C++/`](../C++) is compiled in
unchanged.

Both inputs feed one session, so a binding made by tapping is visible to a line
typed over the wire and the other way round. This is the first host where SliP
runs with nothing attached to it at all.

## Build and flash

Needs the Pico SDK and a bare-metal ARM toolchain. Homebrew's
`arm-none-eabi-gcc` is the compiler alone, with no newlib and so no
`nosys.specs`; Arm's own toolchain is the whole thing, and the tarball needs no
installer and no password:

```sh
git clone -b 2.1.1 --depth 1 --recursive https://github.com/raspberrypi/pico-sdk ~/pico/pico-sdk
curl -L -o /tmp/tc.tar.xz 'https://gitlab.arm.com/api/v4/projects/tooling%2Fgnu-toolchains-for-arm/packages/generic/gnu-toolchain/15.3.rel1/arm-gnu-toolchain-15.3.rel1-darwin-arm64-arm-none-eabi.tar.xz'
mkdir -p ~/pico/toolchain && tar xf /tmp/tc.tar.xz -C ~/pico/toolchain
```

```sh
export PICO_SDK_PATH=~/pico/pico-sdk
export PICO_TOOLCHAIN_PATH=~/pico/toolchain/arm-gnu-toolchain-15.3.rel1-darwin-arm64-arm-none-eabi
cd RP2350 && cmake -B build -S . && cmake --build build
picotool load -f -x build/slip.uf2
```

`-f` reboots the board into BOOTSEL over USB, so the button need not be reached;
`:bootsel` at the REPL does the same from the other side.

## Using it

The panel is the calculator: tap keys to build a line, `⏎` to run it, `⌫` to
correct it. The transcript above shows the source in grey, values in
green and errors in red, as the other hosts do.

```
7  8  9  π  +  ⌫
4  5  6  𝑒  −  AC
1  2  3  ∞  ×  M1
0  .  (  )  ÷  M2
'  =  @  :  ␣  ⏎
```

`AC` is all clear, and on this board all reaches further than the line being
typed: the session is saved, so it clears the line, the transcript, every
binding, and the copy in flash. It is `:forget` — the panel sends the command
rather than deciding for itself what forgetting means. `⌫` is still there for a
wrong digit.

Dragging the transcript scrolls it, by display line rather than by entry — a
long value wraps, and scrolling by entry would jump a paragraph at a time.
Content follows the finger, and an answer puts it back at the bottom.

A calculator on the left, the language along the bottom and down the right.
`M1` and `M2` are not a memory register — they are two names, and the bottom row
is what binds and uses one:

```
( ' M1 = ' ( @ × 2 ) )
21 : M1                     → 42
```

Which is the whole of the language, on a keypad, with no keyboard anywhere near
it. New names cannot be typed — there are no letters beyond `M1` and `M2` — and
the operators that are not on the pad are still reachable over USB.

Over USB it is the same REPL the [ESP32 port](../ESP32/README.md) has — same
modes, same commands.

```
SliP 2.1.1  —  :help

> 2π
= 6.28318530717959
> ( 𝑒 π )
= 8.53973422267357
> :free
heap 286720 free
```

## The session is still there after a power cut

Pull the power out and put it back, and the panel comes up showing last session,
with every binding in it:

```
SliP 2.1.1  —  :help
4 lines restored

> 21 : M1
= 42
```

What is saved is the source, not the state — the lines that were run, in the
order they were run — and booting replays them. Nothing shorter would do: a
binding's value is an expression that may close over the context it was made in,
so writing out the values alone would rebuild a different session that answers
the same for a while. Replaying the source rebuilds *that* session, by
definition, and the interpreter does not have to know any of this is happening.

A line joins the log once it has run, whether it was typed or tapped. A line
that failed built nothing and is not kept; `:calc` and `:prog` are, because they
decide what the lines after them mean. The replay is silent on the wire — a
terminal attached afterwards would otherwise scroll the whole of last time past
before saying anything about now — but not on the panel, where the restored
transcript *is* what "the session is still here" looks like.

[`store.cpp`](store.cpp) puts it in the last 64 KB of flash, as sixteen slots of
one sector. A slot holds the whole log, and every write goes to the next slot:
sixteen times the erase budget, and — because the slot being written is not the
slot being read — a power cut in the middle of a write costs the last line rather
than the session. The newest slot is the one with the highest sequence number
whose checksum still agrees; erased flash is `0xFF` everywhere, which is a
valid-looking header for nothing at all, so a slot is only believed if the magic,
the length and the checksum all do. A slot is 4080 bytes of source, about 130
lines; when it is full the oldest go, whole lines at a time.

Writing means erasing, and the code is in the flash being erased, so the
interrupts go off for the tens of milliseconds it takes. One core is running and
nothing else touches it, so that is the whole of the exclusion. USB goes quiet
for that long and the host waits.

`picotool` writes the program, and the program does not reach this far up the
flash — so **a session survives being reflashed**. That is a nice property and a
surprising one, which is the other reason `:reset` erases it. It has to anyway:
the log is what built the bindings, so leaving it would put every one of them
back at the next power-up, and `:reset` would be a lie with a delay on it.

## What the chip needed

Two lines in `C++/`, and both were already there in shape from the ESP32 port:

- **`SLIP_NO_THREADS`** (`C++/SliP.hpp`) now names `PICO_ON_DEVICE` too. `∥`
  runs its branches sequentially, which is the same value.
- **`RandomSeed()`** (`C++/SliP.cpp`) takes `get_rand_64()` here.
  `std::random_device` reads `/dev/urandom`, which bare metal does not have.

And one that the ESP32 did not need:

- **`thread_local` is dropped where there are no threads.** `theStack` was
  `thread_local`, which on ARM compiles to a call to `__aeabi_read_tp` — and
  nothing provides that symbol on a target with no threading library at all. The
  ESP32 has FreeRTOS underneath and so never noticed. Under `SLIP_NO_THREADS`
  the storage is now plain, which is what one thread means anyway.

## The board header

`boards/waveshare_rp2350_touch_lcd_2_8.h`. The SDK has no header for this board,
and the numbers in it were read off the chip with `picotool info -a -f` rather
than taken from a datasheet: RP2350A, revision A2, 16 MB of flash.

The `// pico_cmake_set PICO_PLATFORM=rp2350` line in it is load-bearing. The SDK
scans the board header — and only that file, not what it includes — before CMake
configures. Without it the board builds for RP2040: everything compiles, the UF2
comes out with the rp2040 family ID, and `picotool` refuses to load it.

## The screen

`screen.cpp` drives the ST7789T3 over spi1 and the CST328 over i2c1. The pins,
the SPI mode, the register-init sequence and the touch read are specific to this
board, and came from Waveshare's own demo for it: the wiki names the parts and
puts the pinout in a picture.

The frame lives in RAM — 240×320 at 16 bits is 150 KB, which this chip has and
to spare — and is pushed whole. Nothing above it has to think about partial
updates or tearing. It is also unswapped: RAMCTRL is given `0xE8`, whose bit 3
puts the panel in little-endian mode, and the chip is little-endian too, so the
frame is already in the order the panel reads. Swapping it "into big-endian"
swapped it out of agreement instead, and the first build came up with a magenta
background and cyan text.

[`font.py`](font.py) bakes the glyphs into `font.h`, which is committed so the
firmware builds without it. No stock font has SliP's alphabet, and no single
face on a Mac has all of it either: Menlo carries ASCII and most of the
operators, Apple Symbols nearly all the rest, and `𝑒` — U+1D452, in the
Mathematical Alphanumeric Symbols block — only STIX Two Math. Every glyph is
placed on one baseline taken from the primary face; centring each glyph's ink
box in its cell instead, which is what it did at first, puts every character on a
line of its own.

## Verified on hardware

```sh
python3 conformance/board.py /dev/cu.usbmodem31201
```

11 passed, 0 failed — the whole [`conformance/`](../conformance) suite, run on
the board over its serial line. The same script answers for the ESP32; nothing in
it knows which chip is on the other end.

| | |
|---|---|
| Flash | 706 KB of 16 MB |
| RAM at link | 165 KB, of which 150 KB is the frame |
| Heap free after `Build()` | 280 KB |
| Session log | 64 KB of flash, 4080 bytes used at most |
| conformance | 11 / 11 |

## Not done here

- **The frame goes out whole on every change**, over blocking SPI — about 20 ms
  a keystroke at 240×320. It is quick enough here and would not be on a bigger
  panel; DMA and partial updates are the answer when there is one.
- **Most of the operators are not on the pad**, nor are letters: 240 pixels has
  to choose between more keys and keys big enough to hit, and 40 across is about
  the floor. Everything is still reachable over USB.
- No file system, no Wi-Fi, no GPIO operators.
- Flashing this replaces whatever was on the board. Waveshare's factory demo was
  what it shipped with, and they publish it as a UF2 if you want it back.
