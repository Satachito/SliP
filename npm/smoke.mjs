//	Smoke test for the staged npm package. Run from the repository root after
//	npm/build.sh has copied the engine in:  node npm/smoke.mjs
import { readFileSync }	from 'node:fs'
import { fileURLToPath }	from 'node:url'
import { dirname, join }	from 'node:path'

const dir	= dirname( fileURLToPath( import.meta.url ) )
const slip	= await import( join( dir, 'node.mjs' ) )
const pkg	= JSON.parse( readFileSync( join( dir, 'package.json' ) ) )

const eq = ( got, want, what ) => {
	if ( got !== want ) throw new Error( `${ what }: expected ${ JSON.stringify( want ) }, got ${ JSON.stringify( got ) }` )
}

eq( slip.evaluate( '( 2 + 2 )' ), '4', 'evaluate' )
eq( slip.evaluate( '( 1 + 1 ) // comment' ), '2', 'comment' )
eq( slip.evaluate( '( 3 > 1 && 5 > 4 )' ), 'T', 'comparison binds tighter than &&' )
eq( slip.evaluate( "( 1 , 2 , [ 3 ] )" ), '[ 1 2 3 ]', 'cons is right-associative' )
eq( slip.evaluate( "( ∥ '[ ( 1 + 1 ) ( 2 + 2 ) ] )" ), '[ 2 4 ]', '∥ sequential fallback' )

//	The published version must be the engine's version, or `npm i` ships a
//	package whose number means nothing.
eq( slip.version(), pkg.version, 'engine version matches package.json' )

//	Errors are thrown, not returned, so a mistake cannot pass for a value.
let threw = false
try { slip.evaluate( '( 3 × )' ) } catch { threw = true }
if ( !threw ) throw new Error( 'evaluate should throw on error' )

//	run() reports per form and stops at the first error.
const $ = slip.run( "( 'a = 2 )\n( a + 1 )" )
eq( $.length, 2, 'run returns one entry per form' )
eq( $[ 1 ].response, '3', 'run evaluates in one context' )

slip.reset()
eq( slip.run( 'a' )[ 0 ].error?.includes( 'Undefined name' ), true, 'reset clears bindings' )

console.log( 'npm package smoke: OK' )
