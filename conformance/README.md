# Conformance suite

Cases written in SliP, not in any implementation's host language, so they
describe the **language** rather than one interpreter. The C++ unit tests in
[`C++Test/`](../C++Test) check that `C++/` works; these check that an
implementation *is SliP*.

```sh
sh conformance/run.sh              # against C++/SliP
sh conformance/run.sh path/to/slip # against any other implementation
```

CI runs this on every push.

## What an implementation has to provide

Two things, and nothing else:

- `slip -p FILE` runs the file and prints the value of every toplevel form, one
  per line.
- On error, a message on stderr containing `:LINE: MESSAGE`, and a non-zero
  exit status.

## Layout

| Path | Meaning |
|------|---------|
| `cases/NAME.slip` | Its `-p` output must equal `cases/NAME.out` exactly |
| `errors/NAME.slip` | Must exit non-zero, stderr containing `errors/NAME.err` |

Error expectations are substrings, so they can carry the line number without
depending on the path the file was run from.

## Writing a case

Write the expected output **by hand first**, then run it. Generating
expectations from the implementation only pins current behaviour, bugs
included — the point is to record what the language should do and find out
where the implementation disagrees. That is not hypothetical: writing
`apply.slip` recorded `''θ` as `' θ` out of Lisp habit, and the answer is `θ`,
because SliP's `'` yields its operand unevaluated and has no reified quote.

Prefer checking meaning over checking formatting. `values.slip` compares
matrices with `==` rather than printing them, because the printed form is
fixed-point decimal — a property of the printer, not of the language. For the
same reason it checks that integer overflow promotes to float without pinning
the digits, which depend on the configurable rounding precision.

Avoid exact-matching results that come out of `libm`. `sin` and friends may
differ by one unit in the last place between platforms, and CI is Linux while
most development here is macOS. `cos 2π` is safe; `sin(1) × π` is asking for
an intermittent failure.
