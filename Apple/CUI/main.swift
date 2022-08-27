import Foundation

struct
StandardErrorTOS: TextOutputStream {
	func
	write( _ p: String ) {
		FileHandle.standardError.write( DataByUTF8( p )! )
	}
}
var sSE = StandardErrorTOS()

let	sContext = Context()
sContext.Load( SliPBuiltins() )
let	sCC = Chain< Context >( sContext )

let	sArguments = CommandLine.arguments.dropFirst()
//let	sArguments = []

if sArguments.count == 0 {	//	REPL
	let	r = StdinUnicodeReader()
	while true {
		do {
			let v = try Sentence( ReadList( r, ";" ) ).Eval( sCC )
			print( v.str, to: &sSE )
		} catch let e {
			debugPrint( e, to: &sSE )
		}
	}
} else {
	do {
		for w in sArguments {
			let	r = StringUnicodeReader( PreProcess( try String( contentsOfFile: w ) ) )
			while true {
				let _ = try Sentence( ReadList( r, ";" ) ).Eval( sCC )
//				let w = try Sentence( ReadList( r, ";" ) )
//				let v = try w.Eval( sCC )
//				print( v.str, to: &sSE )
			}
		}
	} catch let e {
		debugPrint( e, to: &sSE )
	}
}


/*
	let	w = """
		'factorial = '(
			@ == 1 ? [ 1 ( @ × ( @ - 1 ):factorial ) ]
		);
		4 : factorial =
	"""

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
