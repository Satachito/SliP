# SliP on ESP32

A serial REPL. The board has no screen, no DOM and no file system, so this host
is the language and nothing else: a line in on the console UART, a value or an
error out.

Built and tested against a Freenove ESP32-WROOM dev board (ESP32-WROOM-32E,
dual-core 240 MHz, 520 KB SRAM, 4 MB flash, no PSRAM) with ESP-IDF v5.x.

## Build and flash

Install ESP-IDF v5.x once (about 2 GB), then:

```sh
. $HOME/esp/esp-idf/export.sh && cd ESP32 && idf.py set-target esp32 && idf.py build
```

Flash and open the console — replace the port with whatever the board enumerates
as (`ls /dev/cu.usbserial-*` on macOS). The Freenove board bridges USB with a
CH340, which macOS has driven itself since 11.3 through
`com.apple.DriverKit-AppleUSBCHCOM.dext`. Do not install WCH's own driver: it
conflicts with the built-in one.

```sh
idf.py -p /dev/cu.usbserial-0001 flash monitor
```

`Ctrl-]` leaves the monitor.

## Using it

```
SliP 2.1.1  —  :help

> 2πr
! Undefined name: r
> ( 'r = 3 )
= 3
> 2πr
= 18.8495559215388
```

| | |
|---|---|
| `<expr>` | evaluate it |
| `:calc` | a line is one sentence — `2πr` is `( 2πr )` — **default** |
| `:prog` | a line is toplevel forms, stopping at the first error |
| `:{` … `:}` | collect lines, then run them as one |
| `:reset` | forget every binding |
| `:free` | free heap |
| `:version` | the language version this build implements |

Bindings persist until `:reset` or a reboot.

The two modes are the same two the other hosts have: `:calc` is what the web
calculator does to a line, `:prog` is what `slip -p` does to a file.

There is no line editing and no history — backspace, and that is all. The line
is read by `ReadLine` in `main.cpp` rather than by ESP-IDF's linenoise, which
this used at first and which cannot carry SliP's input:

> Every line linenoise returns has been through its `sanitize()`, which keeps
> only the bytes satisfying `isprint()` — and in the C locale no byte of a
> multi-byte character does. The stripping happens *after* the echo, so the
> character appears on the screen and is gone from the buffer. On the board,
> `2π` reached the reader as `2` and answered `2`; `` `aπb` `` came back as
> `` `ab` ``. For most languages that would be a bug about comments and string
> literals. For SliP the operators are the language.

Backspace erases a character rather than a byte: it pops UTF-8 continuation
bytes (`10xxxxxx`) until it reaches the lead byte, and emits one erase, because
the terminal drew one glyph.

## What is different from the desktop build

The interpreter in [`C++/`](../C++) is compiled in unchanged. Two places in it
needed to know about this chip, and say so where they are:

- **`SLIP_NO_THREADS`** (`C++/SliP.hpp`) — `∥` runs its branches sequentially.
  `std::async` threads get the FreeRTOS pthread default stack, far too small for
  a recursive evaluator. The value is identical either way: branches are
  isolated and collected in source order, so the sequential path reports the
  earliest error, not whichever thread lost a race. The browser build has taken
  this same path since it shipped.
- **`RandomSeed()`** (`C++/SliP.cpp`) — `¤` and `random` seed from the hardware
  RNG. `std::random_device` reads `/dev/urandom`, which ESP-IDF does not
  provide, and throws when it cannot open it.

And one thing this host must do that the CLI need not: catch everything. The CLI
can let an exception reach `main` and exit with a diagnostic. Here the frame
above the interpreter is a FreeRTOS task function, and an exception escaping it
reaches `std::terminate`, which panics and reboots the chip. Every entry into
the interpreter goes through `Guarded` in `main.cpp`, including a catch-all for
the bare `string` that the UTF-8 decoder in `JP.h` still throws.

`Embed.cpp` is deliberately not built. Its JSON contract exists so the browser
and the Mac app cannot drift apart; a serial line wants text, so `main.cpp`
calls `Read` / `Eval` the way `CLI.cpp` does.

## The settings that are not optional

In `sdkconfig.defaults`:

- `CONFIG_COMPILER_CXX_EXCEPTIONS=y` — every error in the language is a throw.
- `CONFIG_COMPILER_CXX_RTTI=y` — dispatch is `dynamic_pointer_cast` throughout.

Without either, the build does not compile, let alone run.

Also there, and worth knowing about:

- **`partitions.csv`** gives the app 3 MB. That is headroom, not need: the image
  is 690 KB and would fit the 1 MB of the default layout with a third to spare.
  The board has 4 MB and this firmware will never do OTA.
- **`CONFIG_ESP_TASK_WDT_CHECK_IDLE_TASK_CPU1=n`**, with the REPL task pinned to
  core 1. A long evaluation then starves only core 1's idle task. Core 0 keeps
  its idle task and its watchdog, so a hang on the system side is still caught.
- **`CONFIG_COMPILER_OPTIMIZATION_SIZE=y`** — flash is the binding constraint,
  not speed. A REPL waiting on a 115200 baud line is not short of cycles.

Two headers in [`compat/`](../compat), shared with the RP2350, Android and
Windows builds, exist to get this codebase past a C library that gets in its
way. Each explains itself at the top:

- **`stdckdint.h`** shadows the toolchain's C23 header, whose `ckd_*` macros are
  guarded on `__STDC_VERSION__` and so declare nothing when included from C++.
  `JP.h` reaches for it on every host that is not Apple.
- **`undef-libc-ctype.h`** is force-included ahead of every translation unit.
  picolibc's `<ctype.h>` defines eight single-letter macros — `_U _L _N _S _P _C
  _X _B` — and only in C++, "to build libstdc++". They collide with `JP.h`'s
  error helper `_X` and with the column index `_C` in `SliP.cpp`'s matrix
  comparison. They cannot simply be undefined: `bits/ctype_base.h` is a
  libstdc++ *header* that bakes them into the ctype masks, so `<locale>` has to
  be parsed first. That ordering is the whole content of the file, and newlib on
  the RP2350 needs exactly the same treatment.

## What it costs on the chip

Measured on the board, with `idf.py size` and `:free`:

| | |
|---|---|
| Image | 690 KB (`.text` 394 KB, `.rodata` 203 KB) |
| DRAM at link | 23.9 KB — 13% |
| IRAM | 49 KB — 38% |
| Heap free after `Build()` | 248 KB, largest block 116 KB |

`Build()` registers the whole operator table at boot, and that is most of what
the 248 KB has already paid for.

## Limits

- **Stack.** The REPL task gets 32 KB (`STACK_BYTES` in `main.cpp`). `Eval`
  recurses once per level of nesting; a runaway recursion overflows and the
  stack canary reports it as a stack overflow rather than corrupting the heap.
- **Heap.** Every value is a `shared_ptr`, so a large list is a large number of
  small allocations. There is no PSRAM on this board.
- **No file system, no Wi-Fi, no GPIO operators.** The language cannot reach the
  pins from here; this build is the interpreter and a console.

## Verified on hardware

[`conformance/board.py`](../conformance/board.py) drives the board over the
serial line and runs the whole [`conformance/`](../conformance) suite on it —
each case pasted in programming mode as one `:{ … :}` block, the printed values
compared against the `.out` the desktop interpreter is held to:

```sh
python3 conformance/board.py /dev/cu.usbserial-3110
```

```
  ok    apply.slip  (18 values)
  ok    context.slip  (15 values)
  ok    juxtaposition.slip  (8 values)
  ok    logic.slip  (17 values)
  ok    operators.slip  (15 values)
  ok    values.slip  (21 values)
  ok    no-right-operand.slip  ('Syntax Error: No right operand for infix operator: ×')
  ok    parallel-earliest.slip  ('`first`')
  ok    throw.slip  ('`deliberate`')
  ok    undefined-name.slip  ('Undefined name: nosuchname')
  ok    unopened-paren.slip  ('Detect unopened close parenthesis: ]')

/dev/cu.usbserial-3110: 11 passed, 0 failed
```

Error messages are compared without the `file:line:` prefix, which a one-line
console does not have. `parallel-earliest` is the case that pins `∥`'s
earliest-error rule, and it passes on the sequential path.

It took two firmware bugs to get there, and the suite found both: the linenoise
stripping described above, and `stol` in `Read.cpp`, which silently turned every
integer over 2³¹-1 into a float on this 32-bit target. That second one was never
an ESP32 bug — see the changelog.

The same script answers for the [RP2350 port](../RP2350/README.md); nothing in
it knows which chip is on the other end.
