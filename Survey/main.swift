import Foundation

//let	s = CharacterSet.whitespaces.bitmapRepresentation
//print( "NumBytes:", s.count )
for ( i, w ) in CharacterSet.whitespaces.bitmapRepresentation.enumerated() {
	if w != 0 {
		for j in 0 ..< 8 {
			if w & ( 1 << j ) != 0 {
				print( String( format:"%02X", i * 8 + j ) )
			}
		}
	}
}

func
PreProcess( _ p: String ) -> String {
	var wLines = p.components( separatedBy: .newlines )
	wLines = wLines.map {
		let v = $0.components( separatedBy: "//" )[ 0 ].trimmingCharacters( in: .whitespaces )
		return v.unicodeScalars.last == "="
		?	"(" + String( v.unicodeScalars.dropLast() ) + "):¦;"
		:	v
	}
	return wLines.joined( separator: "\n" )
}
