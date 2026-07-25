# @satachito/slip

SliP is a programmable symbolic calculator: a small expression language where
`2πr`, `cosπ`, and quoted programs share bindings within one run.

- Try it: https://slip.828.tokyo
- Language reference: https://slip.828.tokyo/SPEC.html

This package is the WASM build of the canonical C++ engine.

> **2.0 is not 1.x.** Versions 1.x of this package were a separate JavaScript
> engine. The language changed — operator precedence among them — so 1.x
> programs are not guaranteed to mean the same thing. See the
> [changelog](https://github.com/Satachito/SliP/blob/main/CHANGELOG.md).

## Node

```js
import { evaluate, run, version, reset } from '@satachito/slip'

version()                         // '2.0.0'
evaluate( '( 2 + 2 )' )           // '4'
evaluate( '( 2 π )' )             // '6.28318530717959'
evaluate( "( 1 , 2 , [ 3 ] )" )   // '[ 1 2 3 ]'

run( "( 'r = 2 )\n( 2 π r )" )
// [ { source: "( ' r = 2 )", response: '2' },
//   { source: '( 2 π r )',   response: '12.5663706143592' } ]
```

`evaluate` throws on error. `run` returns one entry per toplevel form and stops
at the first failure, whose entry carries `error` instead of `response`.
`reset()` discards all bindings.

## Browser

The default export is the Emscripten ES module; import it and call `INIT()`
before `REP` / `REPL`.

```js
import createSliP from '@satachito/slip'
const slip = await createSliP()
slip.INIT()
JSON.parse( slip.REP( '( 2 + 2 )' ) )   // { source: '( 2 + 2 )', response: '4' }
```

## Limits

- **Graphics need a browser.** The `canvas`, `fill`, `stroke`, `path2D` and
  `gl:*` operators reach a real DOM and do not work under Node.
- **`∥` is sequential here.** Parallel evaluation needs threads, which need
  `SharedArrayBuffer` and COOP/COEP headers. The value is identical either way;
  only elapsed time differs. The native CLI does use threads.

## License

MIT
