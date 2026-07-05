# SliP Language Reference

**Version:** draft (2026-07-04)  
**Source of truth:** `C++/Read.cpp`, `C++/Eval.cpp`, `C++/SliP.cpp`  
**Site:** https://slip.828.tokyo

This document supersedes informal notes in the Tutorial where they conflict.

---

## 1. Two surface syntaxes

SliP has one evaluator and two ways to feed it source text.

| Mode | UI checkbox | Input handling | Typical use |
|------|-------------|----------------|-------------|
| **Calculator (sugared)** | Programming mode **off** | One expression per line; see §2 | `2πr`, `sin(0)`, `'r = 2` |
| **Programming** | Programming mode **on** | Toplevel SliP forms; `//` comments stripped | `( 'fact = '… )`, `{ … }`, `« … »` |

Both modes share the same **context within one CALCULATE**: bindings from earlier lines in the source are visible to later lines. By default each **CALCULATE** starts from a fresh context; enable **Keep session between runs** in the settings panel to retain bindings across runs (§5.3).

---

## 2. Calculator (sugared) mode

### 2.1 Line wrapping

On https://slip.828.tokyo, each non-empty source line `expr` (after trim, before `//` comment) is evaluated as:

```
( expr )
```

That is, the line is wrapped in parentheses to form a **Sentence** (§4.1).

The native sugared helper (`SugaredSyntaxLoop` in `SwiftUI-CPP/BH.mm`) parses each line as sentence *contents* with a synthetic closing `)` appended. For typical calculator input the effect matches `( expr )`.

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

Because the line becomes `( 'r = 2 )`, the binding is stored for the rest of that CALCULATE run (§5).

### 2.5 Pre-CALCULATE transforms (web UI)

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
! @ # $ % ' * , . / : ; ? ~ ¡ ¤ ¦ § ¬ ± ¶ · ¿ ∈ ∋ ⊂ ⊃ ∩ ∪ + - × ÷
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

A Sentence `( a b c … )` is evaluated in two passes:

1. **Prefix / quote pass** (`ApplyPrefix`) — right-to-left: quotes and prefix operators consume the following form.
2. **Infix pass** (`ApplyInfix`) — pick the **lowest-priority** infix operator; split; recurse. Equal priority → rightmost wins (left-associative for `+`, `-`, …).

If no infix remains and multiple numerics are adjacent, **implicit multiply** applies.

### 4.2 Prefix and quote

| Form | Behaviour |
|------|-----------|
| `' x` | Quote: evaluate to unevaluated `x` |
| `sin x` | Prefix `sin` applied to evaluated `x` |
| `! x` | Eval `x` again |

### 4.3 Apply `:`

`arg : f` pushes `arg` on the argument stack, evaluates `f`, then pops the stack. Used with `'( @ … )` functions where `@` reads the stack top.

### 4.4 List types at evaluation

| Form | Context | Result |
|------|---------|--------|
| `[ … ]` | — | Literal list |
| `{ s₁ s₂ … }` | **New** child context per block | List of each sentence's value |
| `« s₁ s₂ … »` | **Same** context (sequential) | List of each sentence's value |

`«»` does **not** use threads; it only differs from `{ }` in context handling.

### 4.5 Truth values

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

### 5.3 Web session

By default the WASM embed resets context at the start of each **CALCULATE** (bindings and argument stack cleared; the source editor is the program). With **Keep session between runs** checked in the web UI, context persists across CALCULATE until reload or until the option is turned off.

### 5.4 Mode comparison

| Feature | Calculator mode | Programming mode |
|---------|-----------------|------------------|
| Line comments `//` | Ignored per line | Ignored per line |
| Line wrapping | `( line )` | None — full parser |
| Multiline sentence | One line only (unless user types `(` … `)`) | `( …` spanning lines `… )` |
| Toplevel | One result per line | REPL: many forms, JSON array of results |
| Session | Fresh each CALCULATE by default; optional persist (web UI) | Fresh each CALCULATE by default; optional persist (web UI) |

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

| Pri | Sym | Meaning |
|-----|-----|---------|
| 0 | `=` | Assign to name (left must be name) |
| 10 | `?` | `cond ? [ then else ]` |
| 10 | `¿` | Truthy → eval rhs, else Nil |
| 30 | `∈` | `x ∈ list` — membership |
| 30 | `∋` | `list ∋ x` — contains |
| 30 | `==` `<>` `<` `>` `<=` `>=` | Compare → `T` or Nil |
| 40 | `&&` `\|\|` `^^` | Logical and / or / xor |
| 50 | `§` | Eval rhs in child context = Dict(lhs) |
| 50 | `,` | Prepend left to list right |
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

Graphics canvases are created on `document.body` and removed on each CALCULATE in the main UI.
The graphics surface is sample-driven for now; see [Known Issues](KNOWN_ISSUES.md).

---

## 8. Implementations

| Component | Role |
|-----------|------|
| `C++/` | **Canonical** interpreter |
| `WASM/` | Web build (`SliP.js`) |
| `Web/` | Calculator UI |
| `Swift/SliP.swift` | Legacy Swift interpreter (not spec-compliant) |
| `JP/` | Utility header (`JP.h`) for C++ core; large library also used elsewhere |

---

## 9. Related documents

- [Tutorial](../Web/Tutorial.html) — guided introduction
- [Phase 0 audit](phase0-audit.md) — discrepancy log and fix history
