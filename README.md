# SliP

SliP is a programmable symbolic calculator: a small expression language that
starts as a math-friendly web calculator and grows into a Lisp-influenced
playground for variables, functions, lists, matrices, JSON, and browser graphics.

Use it when you want a calculator that grows into a tiny language
where `2πr`, `cosπ`, and quoted programs can share bindings within one run.

Visit the calculator and tutorial:

- https://slip.828.tokyo
- https://slip.828.tokyo/Tutorial.html
- https://slip.828.tokyo/SPEC.html — language reference

## Documentation

| Doc | Audience |
|-----|----------|
| [Web/Tutorial.html](Web/Tutorial.html) | First-time users |
| [Web/SPEC.html](Web/SPEC.html) / [docs/SPEC.md](docs/SPEC.md) | Language specification |
| [CHANGELOG.md](CHANGELOG.md) | What changed, and what it breaks |
| [docs/KNOWN_ISSUES.md](docs/KNOWN_ISSUES.md) | Current limits and reserved surface |
| [docs/MACOS_APP.md](docs/MACOS_APP.md) | The Mac app: what it does, and how to sign it |
| [conformance/](conformance) | Language-level test suite, in SliP |
| [docs/phase0-audit.md](docs/phase0-audit.md) | Maintenance audit log |

## Running programs

```sh
slip program.slip     # run it
slip -e '( 2 π )'     # evaluate one expression
slip -p program.slip  # print the value of every toplevel form
slip -i               # transcript: echo each form and its value
```

Errors are reported as `file:line: message`, with exit status 1.

## Implementation map

| Path | Status |
|------|--------|
| [C++/](C++) | Canonical interpreter used by tests and WASM |
| [WASM/](WASM) | Web build that powers `Web/` |
| [Web/](Web) | Calculator, tutorial, samples, and graphics demos |
| [SwiftUI-CPP/](SwiftUI-CPP) | The macOS app, shipped as `SliP.app` — see [docs/MACOS_APP.md](docs/MACOS_APP.md) |
| [Android/](Android) | The Android app — see [Android/README.md](Android/README.md) |
| [ESP32/](ESP32) | Serial REPL firmware for an ESP32 dev board — see [ESP32/README.md](ESP32/README.md) |
| [Bridge/](Bridge) | Objective-C++ and Swift sides of the embedding bridge |
| [Swift/](Swift) | The original Swift interpreter. Nothing builds it; kept for history |
| [JS/](JS) | Original JavaScript engine; source of the npm package's 1.x releases. Does not track this spec |
| [JP/](JP) | Shared utility submodule ([Satachito/JP](https://github.com/Satachito/JP)) used by the C++ core and other projects |

`C++/` defines the language, and everything shipped runs it: the CLI, the web
build, the npm package, the Mac and Android apps and the ESP32 firmware. `Swift/` and `JS/`
are earlier interpreters of an older dialect, kept as history.

## Build & test

```sh
sh C++Test/ci.sh              # C++ tests
sh conformance/run.sh         # language conformance suite
sh WASM/build.sh              # Web/SliP.js (requires emscripten)
node WASM/smoke.mjs           # after WASM build
sh npm/build.sh               # stage the npm package, after the WASM build
```

GitHub Actions runs the C++ tests and the WASM smoke test in
[.github/workflows/test.yml](.github/workflows/test.yml).

## License

SliP is released under the [MIT License](LICENSE).
