# Phase 0 Audit — SliP

Date: 2026-07-04  
Purpose: Fix baseline before Phase 1. **Implementation (`C++/SliP.cpp`, `C++/Read.cpp`, `C++/Eval.cpp`) is source of truth.**

---

## 1. Test baseline

### Command run

```sh
cd C++Test
clang++ -DDEBUG -std=c++23 -o _ \
  ../C++/Read.cpp ../C++/Eval.cpp ../C++/SliP.cpp ../C++/json.cpp \
  TEST.cpp EvalTest.cpp ReadTest.cpp MathTest.cpp
./_
```

### Result

| Item | Status |
|------|--------|
| Compile | OK |
| Exit code | 0 |
| `TESTING ENDS` | printed |
| CI runs this | **Yes** (`.github/workflows/test.yml`) |

`test.sh` also builds coverage HTML and runs `open index.html` — not required for the baseline gate.
The Pages deploy workflow separately builds WASM and runs `WASM/smoke.mjs`.

---

## 2. Symbol reference (implementation)

### Delimiters / list types

| Syntax | Type | Eval behaviour |
|--------|------|----------------|
| `( … )` | Sentence | Prefix pass → infix pass (operators) |
| `[ … ]` | List | Evaluates to itself |
| `{ … }` | Procedure | Evaluates each item in **new** child context; returns result list |
| `« … »` | Parallel | Evaluates each item in **same** context (sequential in code); returns result list |
| `⟨ … ⟩` | Matrix | Numeric matrix; optional `±nCols` via infix `±` |
| `"…"` / `` `…` `` | Literal | String; eval returns itself |

### Primitives (no operand)

| Symbol | Meaning |
|--------|---------|
| `@` | Stack top |
| `£` | Copy of entire stack as list |
| `¶` | Snapshot of current context bindings as Dict |
| `∅` | Empty / Nil list `[]` |
| `¤` | Random float in **[0, 1)** (no arguments) |

### Quote / prefix / unary

| Symbol | Kind | Meaning |
|--------|------|---------|
| `'` | Quote | Return next form unevaluated |
| `¡` | Prefix | Throw (runtime error with operand REPR) |
| `~` | Prefix | Bitwise NOT (Bits only) |
| `¬` | Prefix | Logical NOT (Nil ↔ truthy) |
| `+` | Prefix* | Unary plus (numeric) — via read-time PMI rewrite |
| `-` | Prefix* | Unary minus (numeric) — via read-time PMI rewrite |
| `!` | Unary | Eval operand |
| `#` | Unary | Length of list or string |
| `*` | Unary | CDR (drop first element; preserves list flavour) |
| `$` | Unary | Last element of list |
| `;` | Unary | Print REPR to **stdout**; return value |
| `¦` | Unary | Print REPR to **stderr**; return value |

\* `+` / `-` as prefix come from `ReadList` PMI handling, not a separate `BUILTINS` entry.

### Infix operators (by priority, low = tight binding last)

| Pri | Symbol | Meaning |
|-----|--------|---------|
| 0 | `=` | Assign name in current context |
| 10 | `?` | If-else: `cond ? [ then else ]` |
| 10 | `¿` | If: truthy → eval rhs, else Nil |
| 30 | `∈` `∋` `==` `<>` `<` `>` `<=` `>=` | Membership / comparison → truthy or Nil |
| 40 | `&&` `\|\|` `^^` | Logical and / or / **xor** |
| 50 | `§` | Eval rhs in child context whose bindings = Dict(lhs) |
| 50 | `,` | Prepend lhs to list rhs |
| 60 | `+` `-` | Add / subtract (also strings, list concat cases) |
| 70 | `·` `×` `÷` `/` `%` | Dot / matrix mul, multiply, float div, int div, mod |
| 80 | `&` `\|` `^` | Bitwise and / or / xor |
| 90 | `:` | Apply lhs as function to rhs (uses stack for `@`) |
| 100 | `±` | Set matrix column count |
| 100 | `.` | Dict key / list index / unary-on-lhs sugar |

**Implicit multiply:** In a Sentence, adjacent numerics with no infix → multiplied (`2 3` → 6).

### Numeric constants

| Symbol | Value |
|--------|-------|
| `∞` `𝑒` `π` `γ` `φ` `log2e` `log10e` `ln2` `ln10` | IEEE / `std::numbers` |

ASCII aliases: `inf` → `∞`, `euler` → `𝑒`, `pi` → `π`.

**Note:** Napier's number is **`𝑒` (U+1D452)** or `euler`, not ASCII `e`.

### Named functions (prefix)

| Name | Arity | Notes |
|------|-------|-------|
| `abs` `sin` `cos` … | 1 | Float math (`RegisterFloatPrefix`) |
| `atan2` `pow` `random` | 2 | `random lo hi` → uniform in range |
| `hypot` `max` `min` | list | `name[ a b c ]` |
| `int` `str` `string` | 1 / list | Parse / format integers |
| `toJSON` `byJSON` | unary suffix | `operand:toJSON`, `` `…`:byJSON `` |

`random` (2-arg) and `¤` (0-arg, 0..1) are **two different** random APIs.

### Truthy / Nil

| Value | Role |
|-------|------|
| `[]` (empty list) | Nil / false |
| Any other non-empty value | Truthy |
| `T` (internal `MS<SliP>()`) | Canonical truthy; **REPR = `"SliP"`** |

### Web-only (`WASM/BuildJS.cpp`)

Canvas / WebGL / `path2D`, `get` / `set` on JS arrays, etc. Not in CLI build.

---

## 3. Documentation discrepancies

### 3.1 Critical (wrong symbol meaning)

| Symbol | Implementation | Tutorial (`Web/Tutorial.html`) | Keypad (`Web/index.html`) |
|--------|----------------|--------------------------------|---------------------------|
| `¶` | Context → Dict | *(not covered)* | **Stringify** ❌ |
| `¤` | Random 0..1 | **Context → Dict** ❌ (lines 285–307) | **Context dictionary** ❌ (typo: dictionay) |
| `.` | Dict/list index | *(not covered)* | **Console.log** ❌ |
| `;` | stdout print | *(not covered)* | *(not on keypad)* |
| `¦` | stderr print | *(not covered)* | Console.error ✓ (label ok) |
| `^^` | Logical XOR | *(not covered)* | **Logical or** ❌ |
| `𝑒` | Constant | Documents ASCII **`e`** ❌ (line 85) | Button `𝑒` ✓ |

**Tutorial fix for Context section:** Should use **`¶`**, not `¤`.

### 3.2 Misleading wording (not a wrong symbol)

| Topic | Tutorial / Keypad | Implementation |
|-------|-------------------|----------------|
| `«»` | "Parallel evaluation" | Sequential `Eval` in same context; no threads |
| `§` | Keypad: "Eval R in context L" | Requires **Dict** as left operand |
| `random` | Tutorial: bare `random` | 0-arg form is **`¤`**; `random` needs 2 args via `random[lo hi]` |
| Session | Implies line-by-line calculator | WASM keeps **one global `Context`** for page lifetime |

### 3.3 HTML / markup bugs (Tutorial)

| Location | Issue |
|----------|-------|
| Line 89 | `<h3>Variables</h2>` — mismatched heading tag |
| Lines 114–115 | Stray `</pre>` after Greek-letter example |
| Lines 217 | Stray `</pre>` after `?` operator section |

### 3.4 Keypad gaps (not wrong, but missing)

| Symbol | In impl | On programming keypad |
|--------|---------|------------------------|
| `;` | stdout | No |
| `£` | stack list | No |
| `∅` | empty | No |
| `±` | matrix cols | No |
| `toJSON` / `byJSON` | yes | No |

### 3.5 Samples vs docs

| File | Note |
|------|------|
| `Web/Samples.sugared.slip` | Uses `𝑒`, `'a = …`, programming-ish lines — OK for impl, blurs Normal/Programming boundary |
| `Samples/*.slip` | Programming syntax; matches impl |

### 3.6 Repo / build gaps (Phase 0 inventory)

| Item | Status |
|------|--------|
| Root `LICENSE` | Missing (only subproject LICENSEs) |
| `Web/SliP.js` / `.wasm` | Gitignored; requires `WASM/build.sh` |
| Submodules | `git@github.com:...` SSH URLs in `.gitmodules` |
| `Swift/SliP.swift` | Separate 2015 interpreter; not WASM/CLI truth |
| `C++Test/TEST.cpp` | DEBUG probes call `forX`, `reduce`, `member`, `fib2` — **not registered** in `Build()` |

---

## 4. Reproduction steps

### R1 — Session variables persist across CALCULATE

**Where:** https://slip.828.tokyo (or local `Web/` after WASM build)

1. Normal mode, uncheck Programming mode.
2. Source line 1: `'r = 2`
3. CALCULATE → should succeed.
4. Clear source (or new line only): `2πr`
5. CALCULATE → **12.566…** (uses persisted `r`).
6. Reload page → step 5 fails until step 2 repeated.

**Cause:** `WASM/WASM.cpp` — `auto C = MS<Context>();` lives for WASM module lifetime; never reset from UI.

---

### R2 — ASCII `e` is not Napier's number

1. Normal mode.
2. Source: `e` (ASCII letter)
3. CALCULATE → `Undefined name: e` (or similar).

**Compare:** Keypad `𝑒` or `exp(1)` → works.

---

### R3 — Keypad `¤` is not “context dictionary”

1. Programming mode ON.
2. Source: `( ¤ )`
3. CALCULATE → random float 0..1, **not** `{}`.

**Compare:** `( ¶ )` after some assignments → Dict of bindings.

---

### R4 — Keypad `.` is not console output

1. Programming mode.
2. Source: `( "hello" . 0 )` — expects string index / wrong type error, not print.
3. For print: `( "hello" : ; )` or unary `;` on sentence — prints to browser console only via WASM stdio hook (if connected).

**UI label "Console.log" on `.` is incorrect** — use `;` / `¦`.

---

### R5 — `^^` is XOR, not OR

1. Programming mode.
2. `( [] ^^ [ 1 ] )` → truthy (one side truthy).
3. `( [ 1 ] ^^ [ 1 ] )` → Nil (both truthy, XOR false).

---

### R6 — Tutorial Context section contradicts impl

Follow Tutorial lines 285–307 using `¤` expecting Dict before/after `{ }` / `« »`.

- After `{ … }` block: `¤` still returns **random**, not `{}`.
- After `« … »`: `¤` returns **random**, not `{ a: 2, b: 3 }`.

**Expected per impl:** Use `¶` for context snapshot.

---

### R7 — `«»` is not parallel threads

1. Programming mode.
2. Run Tutorial parallel example; observe deterministic order `[ 2 3 5 ]`.
3. No race; same as sequential eval with shared context.

---

### R8 — Truthy displays as `SliP`

1. Programming mode.
2. `( 3 == 3 )` → result REPR: **`SliP`**, not `T` or `true`.

---

### R9 — Type errors lack operand detail

1. Programming mode.
2. `( 3 . 3 )` → `Illegal operand combination` with **no** left/right types in message.

(Code: `SliP.cpp` line 590 — REPR detail commented out.)

---

### R10 — Graphics vanish on recalculate

1. Run a sample that creates canvas (e.g. `Web/Mandelbrot.html` or graphics `.slip` via site).
2. Press CALCULATE again on main UI → `RemoveCanvas()` strips all canvases.

---

## 5. Phase 1 fix queue (from this audit)

Priority order for edits:

1. `Web/index.html` — programming keypad labels (`¶` `¤` `.` `^^` `;`?)
2. `Web/Tutorial.html` — `𝑒`, `¶`/`¤` context section, `«»` wording, HTML fixes
3. `C++/SliP.cpp` — uncomment operand detail in `Illegal operand combination`
4. *(Phase 2)* WASM `ResetContext` + UI button; `T` REPR

---

## 6. Files touched in Phase 0

| File | Action |
|------|--------|
| `docs/phase0-audit.md` | Created (this file) |
| Source code | **Not modified** (audit only) |

---

## 7. Phase 1 status (2026-07-04)

| # | Item | Status |
|---|------|--------|
| 1 | `Web/index.html` keypad labels | Done |
| 2 | `Web/Tutorial.html` symbols, context, HTML | Done |
| 3 | `C++/SliP.cpp` operand detail in `.` errors | Done |
| 4 | `C++Test/EvalTest.cpp` expectations updated | Done |

**Next:** Phase 3 — CI tests, regression tests for Phase 1–2.

---

## 8. Phase 2 status (2026-07-04)

| # | Item | Status |
|---|------|--------|
| 5 | Session notice + `Reset context` button | Done (`Web/index.html`, `WASM/WASM.cpp`) |
| 6 | `«»` wording | Done in Phase 1 |
| 7 | `random` / `¤` docs | Done in Phase 1 |
| 8 | Truthy REPR → `T` | Done (`Verum` in `C++/SliP.hpp`, `ClearStack` for reset) |

---

## 9. Phase 3 status (2026-07-04)

| # | Item | Status |
|---|------|--------|
| 9 | CI `C++Test` | Done (`.github/workflows/test.yml`, `C++Test/ci.sh`) |
| 10 | Regression tests | Done (`C++Test/RegressionTest.cpp`) |
| 11 | WASM smoke test | Done (`WASM/smoke.mjs`, also in `pages.yml`) |
| 12 | Submodule HTTPS URLs | Done (`.gitmodules`) |

**Next:** Phase 5 — editor UX (Cmd+Enter, line errors, syntax highlight). **Done** — see §11.

---

## 10. Phase 4 status (2026-07-04)

| # | Item | Status |
|---|------|--------|
| 13 | Sugared mode spec (§2) | Done (`docs/SPEC.md`) |
| 14 | Token / name rules (§3) | Done |
| 15 | Context / mode table (§5–6) | Done |
| 16 | Operator reference (§6) | Done |
| — | Web deploy copy | `Web/SPEC.html` |
| — | Tutorial / README links | Done |

---

## 11. Phase 5 status (2026-07-04)

| # | Item | Status |
|---|------|--------|
| 17 | ⌘↩ / Ctrl↩ calculate | Done |
| 18 | Line / expression errors | Done (`Line N:` / `#N:`) |
| 19 | Editor (monospace, line nums, Tab) | Done |
| 20 | Mode switch confirm | Done |
| 21 | Mobile layout (768px) | Done (basic stack) |
