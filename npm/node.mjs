//	Node entry point for the SliP WASM engine.
//
//	The engine is built with -sENVIRONMENT=web, so it expects to fetch its .wasm
//	and it expects a DOM: WASM/pre.js installs window.createCanvas at load time.
//	Neither holds in Node, so this module reads the binary from disk and puts a
//	minimal window/document in place before loading the engine.
//
//	The graphics operators ( canvas, fill, stroke, path2D, gl:* ) reach a real
//	DOM and therefore do not work here.  Everything else — the language itself —
//	does.  Use the browser build for graphics.

import { readFileSync }	from 'node:fs'
import { fileURLToPath }	from 'node:url'
import { dirname, join }	from 'node:path'

const dir = dirname( fileURLToPath( import.meta.url ) )

const g = globalThis
if ( !g.window ) g.window = g
if ( !g.document ) g.document = {
	body: { appendChild( _ ) { return _ } },
	createElement() {
		throw new Error( 'SliP: graphics operators need a browser; use the browser build' )
	},
}

const createSliP	= ( await import( join( dir, 'SliP.js' ) ) ).default
const engine		= await createSliP( { wasmBinary: readFileSync( join( dir, 'SliP.wasm' ) ) } )

engine.INIT()

//	Version of the language, matching `slip -v`.
export const
version = () => engine.VERSION()

//	Evaluate one expression and return its printed value.
//	Throws the SliP error, rather than returning it, so mistakes are not silent.
export const
evaluate = _ => {
	const $ = JSON.parse( engine.REP( _ ) )
	if ( $.error ) throw new Error( $.error )
	return $.response
}

//	Run a whole program. Returns one entry per toplevel form, in order:
//	{ source, response } or { source, error }.  Evaluation stops at the first
//	error, so the last entry carries it.
export const
run = _ => JSON.parse( engine.REPL( _ ) )

//	Discard all bindings and the argument stack.
export const
reset = () => engine.ResetContext()

//	Significant digits used when printing a Float. Default 15.
export const
setRoundPrecision = _ => engine.SetRoundPrecision( _ )

export default { version, evaluate, run, reset, setRoundPrecision }
