//	Smoke test for the WASM build (run from repo root after WASM/build.sh).
import { fileURLToPath } from 'node:url'
import { dirname, join } from 'node:path'

const dir = dirname( fileURLToPath( import.meta.url ) )

const createSliP = ( await import( join( dir, '../Web/SliP.js' ) ) ).default
const SliP = await createSliP()
SliP.INIT()

const rep = json => JSON.parse( SliP.REP( json ) )

let r = rep( '( 2 + 2 )' )
if ( r.response !== '4' ) throw new Error( `2+2: expected 4, got ${ r.response }` )

r = rep( '( 3 == 3 )' )
if ( r.response !== 'T' ) throw new Error( `3==3: expected T, got ${ r.response }` )

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
