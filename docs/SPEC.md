# SliP Language Reference

**Version:** 2.3.0 (2026-08-21) — see [CHANGELOG](../CHANGELOG.md)
**Source of truth:** `C++/Read.cpp`, `C++/Eval.cpp`, `C++/SliP.cpp`, and `WASM/BuildJS.cpp` for Web graphics
**Site:** https://slip.828.tokyo

This document supersedes informal notes in the Tutorial where they conflict.

---

## 1. Two surface syntaxes

SliP has one evaluator and two ways to feed it source text.

| Mode | UI checkbox | Input handling | Typical use |
|------|-------------|----------------|-------------|
| **Calculator (sugared)** | `prog` **off** | One expression per line; see §2 | `2πr`, `sin(0)`, `'r = 2` |
| **Programming** | `prog` **on** | Toplevel SliP forms; `//` comments stripped | `( 'fact = '… )`, `{ … }`, `« … »` |

Both modes share the same **context within one RUN**: bindings from earlier forms or lines in the source are visible to later ones. Every **RUN** starts from a fresh context. The source is the complete, reproducible program; bindings are not carried from one RUN to the next (§5.3).

---

## 2. Calculator (sugared) mode

### 2.1 Line wrapping

On https://slip.828.tokyo, each non-empty source line `expr` (after trim, before `//` comment) is evaluated as:

```
( expr )
```

That is, the line is wrapped in parentheses to form a **Sentence** (§4.1).

The native helper (`Sugared` in `C++/Embed.cpp`, shared by the WASM build and the Mac app) parses each line as sentence *contents* with a synthetic closing `)` appended. For typical calculator input the effect matches `( expr )`. Unlike programming mode it does not stop at the first failing line.

### 2.2 Implicit multiplication

Inside a Sentence, when no infix operator sits between consecutive **numeric** values, they are multiplied:

| Input | Meaning |
|-------|---------|
| `2 3 4` | `2 × 3 × 4` → `24` |
| `2πr` | `2 × π × r` (after `r` is bound) |
| `cosπ` | `cos` applied to `π` via prefix call syntax (§2.3) |

### 2.3 Function call sugar

Math functions are **prefix** operators registered by name. Calculator syntax uses parentheses for the argument:

| Surface | Parsed as |
|---------|-----------|
| `sin(0)` | `sin` applied to `0` |
| `sin 0` | same (whitespace-separated) |
| `atan2[ 1 1 ]` | `atan2` applied to list `[ 1 1 ]` |
| `max[ 3 2 1 ]` | `max` applied to list `[ 3 2 1 ]` |

Functions without arguments (e.g. `¤`) omit parentheses.

### 2.4 Assignment on a line

A line may use programming operators, including quote-assign:

```
'r = 2
```

Because the line becomes `( 'r = 2 )`, the binding is stored for the rest of that RUN (§5).

### 2.5 Pre-RUN transforms (Web UI)

Optional normalizations applied before evaluation:

| Option | Effect |
|--------|--------|
| `* → ×`, `/ → ÷` | Keyboard aliases |
| `x,X → ×` | Variable-looking multiply |
| Convert numeric | Fullwidth digits → ASCII (`ReplaceNumeral`) |
| Convert `([{}])` | Unicode bracket variants → ASCII |

---

## 3. Lexical structure

### 3.1 Whitespace

Whitespace separates tokens. Inside `"…"` and `` `…` ``, whitespace is literal.

### 3.2 Numbers

- Integer run → `Bits` (signed 64-bit when in range).
- One `.` in digit run → `Float`.
- Integer overflow on parse → `Float`.

### 3.3 Strings

Delimited by `"` or `` ` ``. Backslash escapes: `\n`, `\t`, `\"`, `\\`, etc.

### 3.4 Names

Read from the first character until a **break** condition:

**Solo characters** — always a one-character name by themselves:

```
Greek letters (Α…Ω, α…ω, ς), 𝑒, ∞, ∅, ⊤, ⊥
! @ # $ % ' * , . / : ; ? ~ ¡ ¤ ¦ § ¬ ± ¶ · ¿ ∈ ∋ ∥ ⊂ ⊃ ∩ ∪ + - × ÷
```

**Operator names** — if the name starts with `& | ^ = < >`, subsequent operator chars continue the name (`==`, `<=`, `&&`, …).

**Alphabetic names** — start with a letter (or `_` etc. that is not solo/breaking), continue until whitespace, a solo char, an operator char, or a breaking delimiter.

**Examples**

| Text | Tokens |
|------|--------|
| `abc def` | names `abc`, `def` |
| `abπcd∞ef` | `ab`, `π`, `cd`, `∞`, `ef` |
| `na12me` | one name `na12me` |
| `e` | name `e` (undefined unless assigned) — **not** Napier's number |
| `𝑒` | constant 𝑒 (U+1D452) |

`⊤`, `⊥`, `⊂`, `⊃`, `∩`, and `∪` are tokenized as reserved solo names, but
the current canonical interpreter does not register builtin behaviour for them.
See [Known Issues](KNOWN_ISSUES.md).

### 3.5 Delimiters

| Open | Close | SliP type |
|------|-------|-----------|
| `(` | `)` | Sentence (evaluated) |
| `[` | `]` | List (literal) |
| `{` | `}` | Procedure block |
| `«` | `»` | Shared-context block |
| `⟨` | `⟩` | Matrix literal |

### 3.6 Plus / minus edge cases

`+` and `-` tokens participate in **PMI rewrite** during `ReadList`:

- After value: `3 - 2` → subtract.
- After operator: `3 - - 2` → `3 - (-2)`; `3 - + 2` → `3 - (+2)`.
- Trailing `3 -` keeps unary minus as final token.

---

## 4. Evaluation model

### 4.1 Sentence

A Sentence `( a b c … )` is evaluated by recursive infix splitting:

1. **Infix split** (`ApplyInfix`) — scan the raw form list for the **lowest-priority** infix operator; split there; recurse into both sides. Equal priority → rightmost wins (left-associative), except for operators registered **right-associative** (`=`, `,`), where the leftmost wins.
2. **Prefix / quote pass** (`ApplyPrefix`) — runs only on infix-free segments, right-to-left: quotes and prefix operators consume the following form.
3. If a segment still holds multiple adjacent numerics, **implicit multiply** applies.

Operands are evaluated per segment as the split tree is walked, left side
before right side, so side effects run **left to right**. Because the right
side of an infix operator is not evaluated until its operator asks for it,
`&&`, `||`, and `¿` **short-circuit** (§6.4).

### 4.2 Prefix and quote

| Form | Behaviour |
|------|-----------|
| `' x` | Quote: evaluate to unevaluated `x` |
| `sin x` | Prefix `sin` applied to evaluated `x` |
| `! x` | Eval `x` again |

A prefix operator absorbs the following run of **bare** numerics — numbers,
constants, and names written directly, without parentheses — as one product:

| Input | Meaning |
|-------|---------|
| `sin 2π` | `sin( 2 × π )` |
| `sin 2 π + 1` | `sin( 2 × π ) + 1` |
| `sin(2) π` | `sin( 2 ) × π` — a parenthesized argument is a plain call |
| `2 π sin 3` | `2 × π × sin( 3 )` — a function result ends the run |

### 4.3 Apply `:`

`arg : f` pushes `arg` on the argument stack, evaluates `f`, then pops the stack. Used with `'( @ … )` functions where `@` reads the stack top.

### 4.4 List types at evaluation

| Form | Context | Result |
|------|---------|--------|
| `[ … ]` | — | Literal list |
| `{ s₁ s₂ … }` | **New** child context, shared by the block's sentences | List of each sentence's value |
| `« s₁ s₂ … »` | **Same** context as the caller | List of each sentence's value |

Neither `{ }` nor `« »` uses threads: their sentences share one context and so
may depend on each other's bindings in order. They differ only in *which*
context that is — `« »` writes through to the caller, `{ }` does not.

For concurrency, see `∥` (§4.5), which isolates each branch instead of sharing.

### 4.5 Parallel evaluation `∥`

`∥ '[ s₁ s₂ … ]` evaluates the elements **concurrently**, each in its own child
context, and collects the results **in source order**:

```
( ∥ '[ ( 1 + 1 ) ( 2 + 2 ) ( 3 + 3 ) ] )   →  [ 2 4 6 ]
```

Any list form works (`'[ … ]`, `'{ … }`, `'« … »`); the quote is what defers
evaluation, as with any prefix operator.

**Isolation.** A branch's bindings reach neither its siblings nor the caller:

```
( 'z = 1 )
( ∥ '[ ( 'z = 99 ) ( z ) ] )   →  [ 99 1 ]   ( the second branch still sees 1 )
( z )                          →  1          ( the caller is untouched )
```

Because branches cannot observe each other and results are source-ordered, the
value is exactly what sequential evaluation would produce. Use `« »` when the
sentences *must* see each other's bindings.

**Argument stack.** Each branch is seeded with a copy of the spawning thread's
argument stack, so `@` reads the argument of the enclosing application:

```
( 5 : '( ∥ '[ ( @ + 1 ) ( @ × 2 ) ] ) )   →  [ 6 10 ]
```

**Errors.** If several branches throw, the **earliest branch in source order**
reports, so failures are reproducible.

**Where threads actually run.** Native builds (CLI, Xcode) evaluate branches on
real threads. The browser (WASM) build evaluates them sequentially — see
[Known Issues](KNOWN_ISSUES.md). Both produce the same value; only elapsed time
differs.

### 4.6 Truth values

| Value | Meaning | Printed |
|-------|---------|---------|
| `[]` | Nil / false | `[]` |
| Other non-empty values | Truthy in `?`, `¿`, `&&`, … | (value's own REPR) |
| Canonical true from `==`, etc. | Truthy sentinel | `T` |

---

## 5. Context and session

### 5.1 Context chain

A **Context** is a name → value map plus an optional parent. Lookup walks upward. Assignment `'`name = value` writes to the current context.

### 5.2 Snapshot `¶`

`¶` evaluates to a **Dict** copy of the current context's bindings.

### 5.3 RUN and session

Every host treats **RUN** as one reading of the complete source from the beginning. The context and argument stack are reset first. Bindings remain visible between forms or calculator lines within that RUN, but do not persist into the next RUN.

The calculator interfaces may also accept one line at a time with **⏎**. That live calculator session carries bindings between accepted lines and appends them to the editable history. RUN is still deterministic: it replays the complete history from a fresh context.

### 5.4 Mode comparison

| Feature | Calculator mode | Programming mode |
|---------|-----------------|------------------|
| Line comments `//` | Ignored per line | Ignored per line |
| Line wrapping | `( line )` | None — full parser |
| Multiline sentence | One line only (unless user types `(` … `)`) | `( …` spanning lines `… )` |
| Toplevel | One result per line | REPL: many forms, JSON array of results |
| RUN | Fresh context; reads every line from the start | Fresh context; reads every top-level form from the start |
| Error handling | Report the bad line and continue | Stop at the first error |

---

## 6. Operator reference

Priority: **lower number binds looser** (split first). Omitted infix between numerics → multiply.

### 6.1 Primitives (no operand)

| Sym | Name | Result |
|-----|------|--------|
| `@` | stack-top | Top of apply stack |
| `£` | stack-list | Copy of stack as list |
| `¶` | context-dict | Dict of current bindings |
| `∅` | empty | `[]` |
| `¤` | random-unit | Uniform float in **[0, 1)** |

### 6.2 Quote and prefix

| Sym | Result |
|-----|--------|
| `'` | Quote next form |
| `¡` | Throw error with operand REPR |
| `~` | Bitwise NOT (`Bits`) |
| `¬` | Logical NOT (Nil ↔ truthy) |
| `∥` | Evaluate a list's elements concurrently, isolated, in source order (§4.5) |

### 6.3 Unary

| Sym | Result |
|-----|--------|
| `!` | Eval operand |
| `#` | Length of list or string |
| `*` | CDR — drop first element |
| `$` | Last element of list |
| `;` | Print to stdout, return value |
| `¦` | Print to stderr, return value |

### 6.4 Infix

All operators are left-associative except `=` and `,`, which are
**right-associative** (`'a = 'b = 2` binds both names; `1 , 2 , [ 3 ]` builds
`[ 1 2 3 ]`).

`&&`, `||`, and `¿` **short-circuit**: the right side is not evaluated when
the left side already decides the result. `?` defers both branches because
`[ then else ]` is a literal list.

| Pri | Sym | Meaning |
|-----|-----|---------|
| 0 | `=` | Assign to name (left must be name); right-assoc |
| 10 | `?` | `cond ? [ then else ]` |
| 10 | `¿` | Truthy → eval rhs, else Nil (rhs untouched) |
| 20 | `&&` `\|\|` `^^` | Logical and / or / xor; `&&` `\|\|` short-circuit |
| 30 | `∈` | `x ∈ list` — membership |
| 30 | `∋` | `list ∋ x` — contains |
| 30 | `==` `<>` `<` `>` `<=` `>=` | Compare → `T` or Nil |
| 50 | `§` | Eval rhs in child context = Dict(lhs) |
| 50 | `,` | Prepend left to list right; right-assoc |
| 60 | `+` `-` | Add / subtract; strings; list concat |
| 70 | `·` | Matrix / vector inner product |
| 70 | `×` | Multiply |
| 70 | `÷` | Float divide |
| 70 | `/` | Integer divide (`Bits`) |
| 70 | `%` | Integer remainder |
| 80 | `&` `\|` `^` | Bitwise and / or / xor |
| 90 | `:` | Apply |
| 100 | `±` | Set matrix column count |
| 100 | `.` | `dict.name`, `list.index`, or unary-suffix sugar |

Comparisons bind tighter than the logical operators, so
`x > 0 && y > 0` reads as `( x > 0 ) && ( y > 0 )`.

### 6.5 Numeric constants

`∞` `𝑒` `π` `γ` `φ` `log2e` `log10e` `ln2` `ln10`

ASCII aliases are available for the most common hard-to-type constants:

| Alias | Constant |
|-------|----------|
| `pi` | `π` |
| `euler` | `𝑒` |
| `inf` | `∞` |

Use keypad **𝑒** or `euler`, not ASCII `e`. ASCII `e` remains an ordinary name.

### 6.6 Math functions (prefix)

**One argument:** `abs` `sin` `cos` `tan` `asin` `acos` `atan` `sinh` `cosh` `tanh` `asinh` `acosh` `atanh` `sqrt` `cbrt` `exp` `log` `log2` `log10` `ceil` `floor` `round` `trunc` `sign`

**Two arguments** (`name[ a b ]`): `atan2` `pow` `random` (uniform in **[lo, hi)**)

**List argument:** `hypot` `max` `min`

### 6.7 Random

| Form | Semantics |
|------|-----------|
| `¤` | Uniform **[0, 1)** |
| `random[ lo hi ]` | Uniform **[lo, hi)** |

Not cryptographically secure.

### 6.8 String / integer

| Form | Result |
|------|--------|
| `int` / `int[ str base ]` | Parse integer |
| `str` / `str[ n base ]` | Format integer in base |
| `string` | Operand's REPR as `"…"` literal |
| `toJSON` | JSON text as `` `…` `` |
| `byJSON` | Parse JSON from string literal |

### 6.9 JSON note

`byJSON` uses a SliP-oriented parser (escape handling incomplete for all JSON edge cases). Prefer simple JSON in practice.

---

## 7. Web-only graphics

When built for WASM (`WASM/BuildJS.cpp`), additional operators bind to browser Canvas / WebGL (e.g. `canvas`, `fill`, `stroke`, `path2D`, shader helpers). These are **not** available in the CLI binary.

Graphics canvases are created on `document.body` and removed at the start of each RUN in the main Web UI. Drag a canvas to move it; double-click it to close it.

The top bar includes editable SliP graphics programs:

- **Mandelbrot** — WebGL.
- **Koch** — recursive Canvas 2D curves.
- **Complex** — the orbit `zₙ₊₁ = zₙ² + c`.
- **Fern** — a Barnsley fern.
- **Lorenz** — a Lorenz attractor.

The graphics surface is sample-driven for now; see [Known Issues](KNOWN_ISSUES.md).

---

## 8. Implementations

| Component | Role |
|-----------|------|
| `C++/` | **Canonical** interpreter |
| `WASM/` | Web build (`SliP.js`) |
| `Web/` | Calculator UI |
| `SwiftUI-CPP/` | The shared iOS/iPadOS/macOS app |
| `Bridge/` | Objective-C++ and Swift sides of the embedding bridge |
| `Android/` | Android app embedding the canonical engine through JNI |
| `Windows/` | Native Windows app |
| `ESP32/` | Serial REPL firmware for ESP32 targets |
| `RP2350/` | Touch calculator and serial REPL firmware |
| `Tab5/` | Touch calculator host |
| `Swift/` | The original Swift interpreter; not spec-compliant, and no longer built |
| `JS/` | Original JavaScript engine, published as npm `@satachito/slip`; not spec-compliant |
| `JP/` | Utility submodule (`JP.h`) for the C++ core; shared with other projects |

---

## 9. Related documents

- [Tutorial](../Web/Tutorial.html) — guided introduction
- [Phase 0 audit](phase0-audit.md) — discrepancy log and fix history
