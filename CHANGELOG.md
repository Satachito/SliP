# Changelog

SliP follows [semantic versioning](https://semver.org/): the major version
changes when existing programs can change meaning.

Versions 1.x were the JavaScript engine in [`JS/`](JS), still published to npm
as `@satachito/slip`. Version 2 is the C++ engine in [`C++/`](C++), which powers
the CLI, the Xcode targets and the WASM build behind
[slip.828.tokyo](https://slip.828.tokyo). The two are not the same language;
where they disagree, `C++/` is correct.

## Unreleased

### Added

- **The calculator on iOS is a REPL, in three parts.** The answers at the top,
  the history under them, the line being written at the bottom. ⏎ finishes the
  line: it joins the history — which is the document, and so the file — is
  answered under itself in the transcript, and the line clears. The session
  carries on between lines, because `'r = 2` and then `2πr` is the whole point of
  a calculator that has names in it.

  The history is editable, and RUN reads the whole of it again from a fresh
  session. A tape you can correct and re-run is worth more than one you can only
  type at. Programming mode is the same two keys against the document: ⏎ starts
  another line, RUN runs the lot.

- **Opening a document replays it.** In either mode, what is on the screen when a
  file opens is what that file says — the boards do the same with the session
  they saved. A new document is empty; it was three examples of somebody else's
  work before.

- **A file says which mode it is in.** A first line of `//SLIP` opens it in
  programming mode, and programming mode writes that line. Without it a file is a
  calculator's history — one expression a line, which is what the calculator
  writes. The marker is a comment in the language's own syntax, so it costs the
  interpreter nothing.

- **Changing mode rewrites the text into what the other mode would have written.**
  Calculator to programming parenthesises each line; programming to calculator
  puts each toplevel form on one line and takes those parentheses back off.
  Comments keep their own line either way — folded into a form, a comment takes
  the rest of it with them — and a parenthesis inside a string is a character
  rather than a depth. Leaving the text alone was the alternative, and it would
  have made every line a syntax error in the mode it arrived in.

- **The ESP-IDF host runs on the ESP32-C3 as well.** `set-target esp32c3` is the
  whole of it — no second project, no `#ifdef` in the interpreter — plus a
  `sdkconfig.defaults.esp32c3` for what the part itself needs. The one thing that
  was not a preference: the REPL is pinned to core 1 so that a long evaluation
  starves only the idle task the watchdog has been told to ignore, and a
  single-core part has no core 1 to pin it to. `CONFIG_FREERTOS_UNICORE` decides
  that now, and there the watchdog gives up on core 0's idle task instead.
  Verified on the chip: 11 of 11 conformance cases over the wire, and a session
  that survives a power cut.

- **The iPad splits the keypad in two.** The digits sit on the left at the width
  they have on a phone, and the alphabets take the rest of the room to the right
  of them. The block does not grow with the screen: it is the part that does not
  move, and a digit that changes size when the iPad is turned is a digit you have
  to look at before pressing. A large phone in landscape gets the same, where
  height is the scarce thing.

### Changed

- **The block is seven across on every host, and carries DEL and RUN.** `%` and
  `/` join the operators beside the digits, and the seventh column is the three
  keys that are not characters — delete, run, and a newline. Two of those were on
  a bar before: on the phone that is the far side of the screen from where the
  thumb already is, and on the web page it was the same panel twice. ⏎ is only
  ever a newline now, since RUN is a key beside it. ⌘↩ still runs it, and the
  Mac keeps Run in its toolbar, which is outside the keypad it can collapse.

  `𝑒` moves off the block to sit beside `∞`, which is the other constant among
  the operators.

  The section below the block drops whatever the block took, so nothing is
  offered twice. That subtraction is written down once rather than kept in two
  lists by hand — which is how `√` and `‹` came to be on the panels, and how `'`
  briefly came to be on neither.

- **The iPhone stopped dividing its screen in two.** It had given the source most
  of the height whether there was anything in it or not, and the answers the rest
  whether they fitted or not. Now it does what the Tab5 does: bottom up, the
  keypad, the line being written at whatever height its text needs, and the
  transcript in everything that is left. The transcript already carried both
  halves — every answer is printed under the form it came from — which is what
  makes one region enough. It stays anchored to the bottom, so an answer appears
  next to what was written rather than somewhere above it.

- **The iPhone no longer raises the system keyboard.** Everything the language is
  written in is on the keypad — digits, operators, both alphabets, the function
  names — and the keyboard's only contribution was to cover it. The caret,
  selection, long-press Paste and any hardware keyboard all still work; it is a
  keyboard of no height rather than no keyboard. The iPad keeps its own, where
  there is room for both.

- **The macOS and iOS keypads follow the panels.** Both had shown the same flat
  two dozen symbols since the Xcode targets were written. iOS now has the Tab5's
  arrangement — four alphabets tabbed over one unchanging block of digits, one
  showing — and macOS has the web calculator's, which is the block and then
  everything else under a heading, in a column beside the source. The keys land
  where the caret is rather than at the end of the document, and ⏎ runs the line
  in the calculator and starts another in programming, the way it does on the
  other two hosts.

### Fixed

- **Two keys that put in characters the reader has never heard of.** The Tab5's
  operator panel offered `√` and the web page's offered `‹`, both of them there
  to round a grid out to forty-eight. There are forty-seven single-character
  operators; the forty-eighth slot is now empty, and pressing it no longer
  answers `Undefined name`.

## 2.2.0 — 2026-08-17

The language is unchanged.  This release is four more hosts, a session that
survives a power cut, and one arithmetic bug that had been wrong on every host
since the constants were added.

### Fixed

- **A negated numeric constant was not negative.** `NumericConstant` carries a
  `negative` flag; `Negate()` flipped it and `REPR()` printed it, so `-π` read
  back as `(-π)` and looked right. Every operator takes its operands' values
  through `Double()`, and `Double()` ignored the flag — so **`-π × 2` computed
  `+π × 2`**, and `-π + 1` computed `π + 1`. Silently, with no error, on every
  host.

  `-3` was never affected: the reader builds a negative integer there, not a
  negated constant. It is the named constants — `π`, `𝑒`, `∞`, `γ`, `φ`, `log2e`,
  `log10e`, `ln2`, `ln10` — that lost their sign.

  Found by putting `Web/Koch.html`'s commented-out SliP into `Web/Koch.slip` and
  finding that it drew a four-pixel smudge. The instruction trace was right —
  1024 `lineTo`, 1023 `rotate`, the correct lengths — and every rotation was
  positive: the `-π × 2 ÷ 3` that should turn back was turning further forward,
  so the four sub-curves of every generation landed on top of each other.
  `conformance/cases/values.slip` now checks the sign of a constant in
  arithmetic.

- **Integers above 2³¹-1 became floats on 32-bit targets.** The reader parsed
  integer literals with `stol`, which returns a `long` — 64 bits on macOS and
  Linux, but 32 bits wherever the target is ILP32. `stol` threw `out_of_range`
  there, and the fallback that exists for genuine overflow turned the literal
  into a `Float`: `9223372036854775807` read back as `9223372036854779904`.
  `Bits` has always held an `int64_t`, so the reader now uses `stoll`. On LP64
  the two are the same function and nothing changes.

  **This affected the WASM build, and so [slip.828.tokyo](https://slip.828.tokyo)
  — wasm32 is ILP32.** It was found by running the conformance suite on an
  ESP32, which is the first host to run the suite on a 32-bit target; `run.sh`
  only ever exercised the native CLI. The web build needs a redeploy to pick
  this up.

### Added

- **[`ESP32/`](ESP32) — the interpreter as serial-REPL firmware for an ESP32
  dev board.** `C++/` is compiled in unchanged; two places in it now name the
  chip, at `SLIP_NO_THREADS` and `RandomSeed`. See
  [ESP32/README.md](ESP32/README.md).

- **[`Android/`](Android) — the app on Android.** `C++/` is compiled through the
  NDK and reached over JNI; the screen follows `SwiftUI-CPP/ContentView.swift`.
  Strings cross the JNI boundary as UTF-8 bytes rather than as `jstring`,
  because JNI's modified UTF-8 would corrupt `𝑒` (U+1D452). See
  [Android/README.md](Android/README.md).

- **[`RP2350/`](RP2350) — firmware for a Waveshare RP2350-Touch-LCD-2.8:** a
  calculator on the 2.8-inch panel with a touch keypad, and the same serial REPL
  the ESP32 has, both feeding one session. The first host where SliP runs with
  nothing attached to it.

  `SLIP_NO_THREADS` and `RandomSeed` name the chip alongside the ESP32; beyond
  that it needed one thing the ESP32 did not, which is that `theStack` cannot be
  `thread_local` where there is no threading library — on ARM that is a call to
  `__aeabi_read_tp`, and nothing defines it. The glyphs are generated into
  `RP2350/font.h`, because no stock font has this alphabet and no single face on
  a Mac has all of it. See [RP2350/README.md](RP2350/README.md).

- **The session survives a power cut, on both boards.** Switch an
  [ESP32](ESP32/README.md) or an [RP2350](RP2350/README.md) off and on and the
  bindings are still there; the RP2350's panel comes up showing the transcript
  as it was.

  What is saved is the source — the lines that were run, in the order they were
  run — and booting replays them. Nothing shorter would do: a binding's value is
  an expression that may close over the context it was made in, so writing the
  values out would rebuild a different session that answers the same for a
  while. It also means nothing in `C++/` had to change. A line is kept once it
  has run; a line that failed built nothing. `:calc` and `:prog` are kept too,
  because they decide what the lines after them mean.

  `:reset` erases the saved copy as well as the live one — it is the log that
  built the bindings, so leaving it would put them all back at the next
  power-up. `:forget` is the same command under the name that says it, and `AC`
  on the RP2350's keypad sends it: all clear now means all of it. On the RP2350
  the log lives above the program in flash, so **it survives `picotool load`**
  too. `RP2350/store.cpp` is sixteen sector-sized slots written round-robin,
  each checksummed; `ESP32/main/store.cpp` is one NVS blob, since NVS already
  does the levelling and the power-cut safety.

- **[`Tab5/`](Tab5) — firmware for an M5Stack Tab5:** a calculator on the
  5-inch 720x1280 panel and the same serial REPL, both feeding one session. An
  ESP32-P4, so RISC-V — a third instruction set after Xtensa and ARM, and
  nothing in `C++/` names it.

  The panel is Espressif's board support package rather than a driver written
  here, because M5 changed the display partway through production and the two
  are not interchangeable; `:i2c` prints the bus and settles which is fitted.
  Nothing is pushed to this panel — the display scans the frame buffer out of
  PSRAM itself — so the DMA and partial updates `RP2350/README.md` left as the
  next board's homework are not needed. A cache is: the processor writes that
  memory through one and the display reads it without one.

  The panel carries one fixed block of digits and one of four tabbed
  alphabets — the operators, the transcendental functions, the Latin letters and
  the Greek. **The first host where a name can be invented on the panel**: the
  reader has always taken those letters as names, and 240 pixels had nowhere to
  put them. See [Tab5/README.md](Tab5/README.md).

- **The web keypad follows the Tab5's panel.** The block of digits is six across
  and four down at the top of the sidebar, the same wherever else you are, and
  the operators, the transcendental functions and the Greek are laid out below
  it rather than hidden behind tabs.

  The keypad and the evaluation mode are now separate things. Choosing the
  "Code" keypad used to change how every line would be read — hence the
  confirmation it had to ask — and the two are one control no longer: the
  keypads are keypads, and `prog` alone decides what a line means. `RUN` appears
  only in programming mode, where `⏎` starts another line; in the calculator `⏎`
  is what runs, as it is on the panel.

  `Web/Koch.slip` is a sample rather than a page of JavaScript, which is what
  turned up the constant-sign bug above.

- **[`conformance/board.py`](conformance/board.py)** — the serial harness, which
  used to live under `ESP32/`. It takes the port as an argument and knows
  nothing about which chip answers.

- **[`Windows/`](Windows) — the app on Windows**, cross-compiled from macOS with
  mingw-w64 into one statically linked `SliP.exe`. See
  [Windows/README.md](Windows/README.md).

- **`SessionRun` in [`C++/Embed.hpp`](C++/Embed.hpp)** — the list of results
  before it is written out as JSON. The JSON exists for hosts on the far side of
  a language boundary; a host written in C++ was encoding a list only to parse it
  straight back, or else writing the modes' semantics out again for itself.
  `REPL` and `Sugared` are now this with `json_escape` applied, and their output
  is unchanged byte for byte.

- **[`compat/stdckdint.h`](compat/stdckdint.h)** — shared by the ESP32, Android
  and Windows builds, whose toolchains all compile `JP.h` with a `<stdckdint.h>`
  that is either missing or hides its macros from C++. It used to live under
  `ESP32/`.

## 2.1.1 — 2026-07-25

### Fixed

- **The Mac app shipped as `SwiftUI-CPP.app`.** Only its display name had been
  set to SliP, and the name a user sees in Finder is `PRODUCT_NAME`. It builds
  as `SliP.app` now, identifies as `tokyo.828.SliP`, and its executable is
  `SliP`.

### Removed

- The AppKit app. It ran the canonical engine as of 2.1.0, but it was a second
  app doing the same job as the SwiftUI one, and two apps both called SliP is
  worse than one. Its bridge layer stays — `Bridge/` and `C++/Embed.cpp` are
  what the SwiftUI app and the WASM build now share.

## 2.1.0 — 2026-07-25

The language is unchanged. This release is the macOS app, and the plumbing the
app needed.

### Added

- **The macOS app.** Calculator and programming modes, results per form with
  errors shown rather than fatal, a keypad for the operators no keyboard has,
  rounding precision in Settings, and `.slip` documents. Attached to this
  release as `SliP-macos.zip`; see [docs/MACOS_APP.md](docs/MACOS_APP.md).
- `Sugared()` in the WASM build: calculator-mode evaluation, one entry per
  line, matching what the web UI used to do in JavaScript.
- The Xcode targets are built in CI. The `SwiftUI-CPP` target had stopped
  linking — its build phase never included the interpreter sources — and
  nothing noticed, because nothing outside Xcode compiled it.

### Fixed

- **The Mac app no longer dies on a SliP error.** The bridge threw C++
  exceptions across the Objective-C++ boundary, where nothing catches them, so
  any mistake in the editor terminated the process. Errors are values now, and
  the evaluation session, the JSON contract and the error handling are shared
  with the WASM build rather than duplicated.
- **The Mac editor no longer corrupts `'`.** macOS text substitution rewrote
  the quote operator into a curly quote, so `( 'r = 2 )` reported
  `Undefined name: ’r`. SwiftUI's `TextEditor` cannot turn substitution off,
  so the editor is an `NSTextView`.
- **Calculator mode does not stop at the first bad line**, matching the web
  calculator: a typo on line 1 must not hide the answer on line 6. Programming
  mode still stops, because there a later form usually depends on an earlier
  one.
- `REPL` returned invalid JSON — `[,{ … }]` — when the *first* form failed, so
  the web UI showed a JSON parse error instead of the actual SliP error.

## 2.0.0 — 2026-07-25

### Breaking

- **`&&`, `||` and `^^` bind looser than comparisons.** They moved from
  priority 40 to 20, so `x > 0 && y > 0` now reads as `( x > 0 ) && ( y > 0 )`.
  Previously it parsed as `( x > ( 1 && 5 ) ) > 4`-style nonsense and quietly
  returned the wrong answer — `( 3 > 1 && 5 > 4 )` was `[]`, and is now `T`.
  Programs that worked around the old grouping with extra parentheses are
  unaffected; programs that did not were already wrong.
- **`=` and `,` are right-associative.** `( 'p = 'q = 7 )` now binds both
  names, and `( 1 , 2 , [ 3 ] )` builds `[ 1 2 3 ]` instead of failing with
  "Right operand must be List".
- **`±` returns a new matrix instead of reshaping its operand.** `( m ± 2 )`
  used to write the column count through to `m` itself, so `m` was permanently
  reshaped and every later reference saw it. It is now a value operation like
  every other operator.
- **A prefix operator absorbs the run of bare numerics after it.** `sin 2π` is
  `sin( 2 × π )`, not `sin( 2 ) × π`. A parenthesised argument is unaffected:
  `sin(2) π` is still `sin( 2 ) × π`.
- **The CLI no longer echoes by default.** Running a program prints only what
  the program prints. `slip -i` restores the old `> form` / `< value`
  transcript.

### Added

- **`∥`, parallel evaluation.** `∥ '[ s₁ s₂ … ]` evaluates the elements
  concurrently, each in its own child context, collecting results in source
  order. Branches cannot observe each other, so the value equals sequential
  evaluation. Native builds use threads; the WASM build falls back to
  sequential — see [Known Issues](docs/KNOWN_ISSUES.md). SPEC §4.6.
- **The CLI runs programs.** `slip program.slip`, plus `-e EXPR`, `-p`, `-i`,
  `-v` and `-h`. Errors are reported as `file:line: message` with exit status
  1, instead of escaping as an uncaught exception with an abort message.
- `slip -v`, and `VERSION()` in the WASM build, report this version.

### Fixed

- **`//` comments work outside the web UI.** SPEC §5.4 has always promised
  them in both modes, but only the browser delivered them, by stripping
  comments before the reader saw them. The reader handles them now, so a
  commented `.slip` file can be run from the CLI. A single `/` still divides,
  and `//` inside a string is still text.
- **`&&`, `||` and `¿` short-circuit.** The right-hand side used to be
  evaluated before the operator ever ran, so `( [] ¿ ( ¡ "boom" ) )` threw even
  though the condition was false. Quoting the right side, as in
  `cond ¿ '( … )`, is no longer required for correctness — though it remains
  valid.
- Side effects within a sentence now run left to right. The prefix pass used to
  evaluate right to left.
- `Matrix` holds its elements in a `vector` rather than a raw pointer with no
  copy constructor.

### Internal

- The test suite reported success unconditionally: an exception in any phase
  skipped every later test and still exited 0. It now exits 1 and names the
  failure, and the stack-underflow checks fail when the expected exception does
  not arrive.
- The argument stack is thread-local rather than a mutex-guarded global, which
  is what allows `∥` to work at all: `:` is Push / Eval / Pop, an invariant a
  lock cannot protect.
