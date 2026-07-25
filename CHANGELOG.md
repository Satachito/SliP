# Changelog

SliP follows [semantic versioning](https://semver.org/): the major version
changes when existing programs can change meaning.

Versions 1.x were the JavaScript engine in [`JS/`](JS), still published to npm
as `@satachito/slip`. Version 2 is the C++ engine in [`C++/`](C++), which powers
the CLI, the Xcode targets and the WASM build behind
[slip.828.tokyo](https://slip.828.tokyo). The two are not the same language;
where they disagree, `C++/` is correct.

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
