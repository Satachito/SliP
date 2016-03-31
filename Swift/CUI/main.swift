///	Written by Satoru Ogura, Tokyo.
//
import Foundation

print( "SliP ver 0.2(Swift) 2016 written by Satoru Ogura, Tokyo.×÷¡¿·¬«»¦¯" )

class
PreProcessor: Reader< UnicodeScalar > {
	var
	m	= String.UnicodeScalarView()
	override func
	_Read() -> UnicodeScalar? {
		while m.count == 0 {
			if let w = readLine( stripNewline: false ) {
				m = w.componentsSeparatedByString( "//" )[ 0 ].unicodeScalars
			} else {
				return nil
			}
		}
		let v = m.first
		m = m.dropFirst()
		return v
	}
}

var	sReader		=	PreProcessor()
var	sContext	=	Context()
var	sFileHandle	=	NSFileHandle.fileHandleWithStandardOutput()
while true {
	do {
		print(
			try List( try ReadObjects( sReader, ";" as UnicodeScalar ), .Sentence ).Eval(
				sContext
			,	{ a in sFileHandle.writeData( UTF8Data( a )! ) }
			)
		)
	} catch let e {
		print( e )
	}
}
