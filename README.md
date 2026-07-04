# SliP, the computer language.

Visit the calculator and tutorial:

- https://slip.828.tokyo
- https://slip.828.tokyo/Tutorial.html
- https://slip.828.tokyo/SPEC.html — language reference

## Documentation

| Doc | Audience |
|-----|----------|
| [Web/Tutorial.html](Web/Tutorial.html) | First-time users |
| [Web/SPEC.html](Web/SPEC.html) / [docs/SPEC.md](docs/SPEC.md) | Language specification |
| [docs/phase0-audit.md](docs/phase0-audit.md) | Maintenance audit log |

## Build & test

```sh
sh C++Test/ci.sh              # C++ tests
sh WASM/build.sh              # Web/SliP.js (requires emscripten)
node WASM/smoke.mjs           # after WASM build
```
