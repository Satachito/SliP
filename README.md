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
| [docs/KNOWN_ISSUES.md](docs/KNOWN_ISSUES.md) | Current limits and reserved surface |
| [docs/phase0-audit.md](docs/phase0-audit.md) | Maintenance audit log |

## Implementation map

| Path | Status |
|------|--------|
| [C++/](C++) | Canonical interpreter used by tests and WASM |
| [WASM/](WASM) | Web build that powers `Web/` |
| [Web/](Web) | Calculator, tutorial, samples, and graphics demos |
| [Swift/SliP.swift](Swift/SliP.swift) | Legacy interpreter, kept for history |
| [JP/](JP) | Shared utility code used by the C++ core and older experiments |

## Build & test

```sh
sh C++Test/ci.sh              # C++ tests
sh WASM/build.sh              # Web/SliP.js (requires emscripten)
node WASM/smoke.mjs           # after WASM build
```

GitHub Actions runs the C++ tests and the WASM smoke test in
[.github/workflows/test.yml](.github/workflows/test.yml).

## License

SliP is released under the [MIT License](LICENSE).
