import Foundation



struct
StandardErrorTOS: TextOutputStream {
	func
	write( _ p: String ) {
		FileHandle.standardError.write( p.data( using: .utf8 )! )
	}
}
var sTOS = StandardErrorTOS()

let	wContext = Context( printer: { p in FileHandle.standardOutput.write( p.data( using: .utf8 )! ) } )
do {

	for i in 1 ..< CommandLine.arguments.count {
		let	wReader	= PreProcessor( try String( contentsOfFile: CommandLine.arguments[ i ], encoding: .utf8 ) )
		while true {
			do {
				let _ = try Sentence( try ReadObjects( wReader, ";" as UnicodeScalar ) ).Eval( wContext )
			//	print( v.description, to: &sTOS )
			} catch let e {
				if e is ReaderError { break }
				throw e
			}
		}
	}
/*
	let	w = "'A ∈ [ A B C ] =\n"

	let	wReader	= PreProcessor( w )
	while true {
		do {
			let _ = try Sentence( try ReadObjects( wReader, ";" as UnicodeScalar ) ).Eval( wContext )
		//	print( v.description, to: &sTOS )
		} catch let e {
			if e is ReaderError { break }
			throw e
		}
	}
*/
} catch let e {
	debugPrint( e, to: &sTOS )
}
