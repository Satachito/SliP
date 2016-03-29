///	Written by Satoru Ogura, Tokyo.
//
import Foundation

print( "SliP ver 0.2(Swift) 2016 written by Satoru Ogura, Tokyo.×÷¡¿·¬«»¦¯" )

class
PreProcessor: Reader< UnicodeScalar > {
	var
	u	= String.UnicodeScalarView()
	override func
	_Read() -> UnicodeScalar? {
		while u.count == 0 {
			if let w = readLine( stripNewline: false ) {
				u = w.componentsSeparatedByString( "//" )[ 0 ].unicodeScalars
			} else {
				return nil
			}
		}
		let v = u.first
		u = u.dropFirst()
		return v
	}
}

var	sReader		=	PreProcessor()
var	sContext	=	Context()
while true {
	do {
		print( try List( try ReadObjects( sReader, ";" as UnicodeScalar ), .Sentence ).Eval( sContext ) )
	} catch let e {
		print( e )
	}
}
