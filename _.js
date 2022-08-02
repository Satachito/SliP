import { NewContext, StringReader, Read, Eval } from './SliP.js'

import * as fs from 'fs'
const r = new StringReader(
	fs.readFileSync( '/dev/stdin', 'utf8' ).split( '\n' ).map(
		_ => {
			const index = _.indexOf( '//' )
			return index < 0 ? _ : _.substring( 0, index )
		}
	).join( '\n' )
)

const
c = NewContext()

while ( true ) {
	try {
		const _ = Read( r )
		if ( !_ ) break
		console.log( _.string() )
		const $ = Eval( c, _ )
	//	console.log( 'CONTEXT:', c )
	//	console.log( 'VALUE:', $.string() + '\t:', $ )
		console.log( $.string() )
	} catch ( e ) {
		console.log( 'EXCEPTION:', e )
	}
}

