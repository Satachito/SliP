# Known Issues and Reserved Surface

This page describes the current intentional gaps and rough edges in SliP.
For past maintenance history, see [phase0-audit.md](phase0-audit.md).

## Language Core

### Reserved solo symbols

The reader tokenizes these symbols as one-character names, but the canonical
interpreter does not currently assign builtin behaviour to them:

```text
⊤ ⊥ ⊂ ⊃ ∩ ∪
```

Treat them as reserved for future logical and set-related forms. Until they are
registered as builtins or assigned in the current context, evaluating them raises
an undefined-name error.

### JSON parser scope

`byJSON` is useful for simple JSON values used by samples and demos, but it is
not intended to be a fully strict JSON implementation yet. Escape handling and
edge cases are intentionally narrower than a production JSON parser.

### Apply syntax

The apply operator is `arg : f`. The left side is pushed as the stack-top
argument, the right side is evaluated, and then the argument stack is popped.
Inside quoted functions, `@` reads that stack-top argument.

## Web and Graphics

### Web-only graphics

Canvas, WebGL, and `path2D` helpers live in the WASM build
(`WASM/BuildJS.cpp`). They are available on the web calculator, not in the C++
CLI build. The graphics surface is sample-driven for now rather than specified
operator by operator.

### Canvas lifetime

The main web UI removes existing canvases at the start of each CALCULATE and resets the evaluation context unless **Keep session between runs** is checked.
Reloading the page clears bindings and the visual state.

## Repository Shape

### Legacy implementations

`C++/` is the canonical interpreter. `Swift/SliP.swift` and older `JP/`
experiments remain in the repository for history and reuse, but they are not the
language specification.

