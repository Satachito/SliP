import { NewContext, StringReader, Read } from './SliP.js'

const
c = NewContext()

import * as fs from 'fs'
const r = new StringReader( fs.readFileSync( '/dev/stdin', 'utf8' ) )
while ( true ) {
	try {
		const _ = Read( r )
		if ( !_ ) break
		console.log( 'READ:', _ )
		const $ = _.Eval( c )
		console.log( 'CONTEXT:', c )
		console.log( 'VALUE:', $.string() + '\t:', $ )
	} catch ( e ) {
		console.log( 'EXCEPTION:', e )
	}
}

