//	Smoke test for the WASM build (run from repo root after WASM/build.sh).
import { readFileSync } from 'node:fs'
import { fileURLToPath } from 'node:url'
import { dirname, join } from 'node:path'

const dir = dirname( fileURLToPath( import.meta.url ) )
const wasmPath = join( dir, '../Web/SliP.wasm' )

// Emscripten web build expects browser globals (see WASM/pre.js).
const g = globalThis
g.window = g
g.document = {
	body: { appendChild( node ) { return node } },
	createElement() {
		const canvas = {
			width: 0, height: 0, style: {},
			setAttribute() {},
			getBoundingClientRect() { return { left: 0, top: 0 } },
			parentNode: { removeChild() {} },
			getContext() {
				return {
					fillRect() {}, strokeRect() {}, clearRect() {},
					beginPath() {}, moveTo() {}, lineTo() {}, stroke() {},
					fillText() {}, measureText() { return { width: 0 } },
					putImageData() {}, createImageData() { return { data: [] } },
					getImageData() { return { data: [] } },
				}
			},
		}
		return canvas
	},
}

const createSliP = ( await import( join( dir, '../Web/SliP.js' ) ) ).default
const SliP = await createSliP( { wasmBinary: readFileSync( wasmPath ) } )
SliP.INIT()

const rep = json => JSON.parse( SliP.REP( json ) )

let r = rep( '( 2 + 2 )' )
if ( r.response !== '4' ) throw new Error( `2+2: expected 4, got ${ r.response }` )

r = rep( '( 3 == 3 )' )
if ( r.response !== 'T' ) throw new Error( `3==3: expected T, got ${ r.response }` )

r = rep( '( pi == π )' )
if ( r.response !== 'T' ) throw new Error( `pi alias: expected T, got ${ r.response }` )

r = rep( '( euler == 𝑒 )' )
if ( r.response !== 'T' ) throw new Error( `euler alias: expected T, got ${ r.response }` )

r = rep( '( inf == ∞ )' )
if ( r.response !== 'T' ) throw new Error( `inf alias: expected T, got ${ r.response }` )

{
	SliP.REPL( "( 'x = 9 )" )
	r = rep( 'x' )
	if ( r.response !== '9' ) throw new Error( `x after assign: expected 9, got ${ r.response }` )
	SliP.ResetContext()
	r = rep( 'x' )
	if ( !r.error?.includes( 'Undefined name' ) ) {
		throw new Error( `x after reset: expected undefined, got ${ JSON.stringify( r ) }` )
	}
}

console.log( 'WASM smoke: OK' )
