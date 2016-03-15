import Cocoa

class
PreProcessor: Reader< UnicodeScalar > {
	var
	u	= String.UnicodeScalarView()
	init(	_ p	:	String ) {
		var wLines = p.componentsSeparatedByCharactersInSet( NSCharacterSet.newlineCharacterSet() )
		for ( i, w ) in wLines.enumerate() {
			wLines[ i ] = w.componentsSeparatedByString( "//" )[ 0 ]
		}
		u = ( wLines as NSArray ).componentsJoinedByString( "\n" ).unicodeScalars
	}
	override func
	_Read() -> UnicodeScalar? {
		if u.count == 0 { return nil }
		let v = u.first
		u = u.dropFirst()
		return v
	}
}

class
Document	:	NSDocument {

	dynamic	var
	u = ""
	
	override func
	makeWindowControllers() {
		addWindowController(
			NSStoryboard( name: "Main", bundle: nil ).instantiateControllerWithIdentifier(
				"Document Window Controller"
			) as! NSWindowController
		)
	}

	override func
	dataOfType( typeName: String ) throws -> NSData {
		if let v = UTF8Data( u ) { return v } else {
			throw NSError( domain: "ApplicationErrorDomain", code: 1, userInfo: nil )
		}
	}

	override func
	readFromData( data: NSData, ofType typeName: String ) throws {
		if let w = UTF8String( data ) { u = w } else {
			throw NSError(domain: NSOSStatusErrorDomain, code: unimpErr, userInfo: nil)
		}
	}

	func
	Do() {
		SetupBuiltin()
		let	wPP	=	PreProcessor( u )
		while true {
			do {
				let w = try ReadObjects( wPP, ";" as UnicodeScalar )
				if w.count == 0 { break }
				print( try List( w, .Sentence ).Eval() )
			} catch let e {
				print( e )
			}
		}
//		let wSUR = StringUnicodeReader( u )
//		while true {
//			do {
//				if let w = try Read( wSUR ) { print( try w.Eval() ) } else { break }
//			} catch let e {
//				print( e )
//			}
//		}
	}
}
