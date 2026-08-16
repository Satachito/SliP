# Changelog

SliP follows [semantic versioning](https://semver.org/): the major version
changes when existing programs can change meaning.

Versions 1.x were the JavaScript engine in [`JS/`](JS), still published to npm
as `@satachito/slip`. Version 2 is the C++ engine in [`C++/`](C++), which powers
the CLI, the Xcode targets and the WASM build behind
[slip.828.tokyo](https://slip.828.tokyo). The two are not the same language;
where they disagree, `C++/` is correct.

## Unreleased

### Fixed

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
