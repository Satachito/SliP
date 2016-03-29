//	Written by Satoru Ogura, Tokyo.
//
import Cocoa

@NSApplicationMain class
AppDelegate: NSObject, NSApplicationDelegate {
}

class
WindowController: NSWindowController, NSWindowDelegate {
}

class
ViewController: NSViewController {
	@IBAction func
	Do( _: AnyObject ) {
		if let w = view.window?.windowController?.document as? Document { w.Do() }
	}
	
	@IBAction func
	DoInsert( p: NSButton ) {
		if let w = sourceItemView as? NSTextView { w.insertText( p.title , replacementRange: w.selectedRange() ) }
	}
}

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

	dynamic	var	u = "//  SliP ver 1.0 2016 written by Satoru Ogura, Tokyo."
	dynamic	var	result = ""
	dynamic	var	output = ""

	override
	init() {
		super.init()
	}
	
	init( type: String ) throws {
		super.init()
		if	let wURL = ResourceURL( "Sample", "slip" )
		,	let	wData = Data( wURL )
		,	let	wString = UTF8String( wData ) {
			u = wString
		}
	}
	
	override class func
	autosavesInPlace() -> Bool {
		return true
	}

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
		NSApplication.sharedApplication().keyWindow?.makeFirstResponder( nil )	//	Sync NSTextView and u
		if let v = UTF8Data( u ) { return v } else {
			throw NSError( domain: "ApplicationErrorDomain", code: 1, userInfo: nil )
		}
	}

	override func
	readFromData( data: NSData, ofType typeName: String ) throws {
		if let w = UTF8String( data ) { u = w } else {
			throw NSError( domain: "ApplicationErrorDomain", code: 2, userInfo: nil )
		}
	}

	class
	GUIContext		:	Context {
		let		u	:	Document
		init( _ p	:	Document ) { u = p }
		override func
		Print( p: Object ) {
			Main{ self.u.output = self.u.output + "\(p)" }
		}
	}
	func
	Do() {
		NSApplication.sharedApplication().keyWindow?.makeFirstResponder( nil )	//	Sync NSTextView and u
		let	wContext = GUIContext( self )
		let	wReader	= PreProcessor( u )
		result = ""
		output = ""
		Sub {
			while true {
				do {
					let wObjects = try ReadObjects( wReader, ";" as UnicodeScalar )
					if wObjects.count == 0 { break }
					let w = try List( wObjects, .Sentence ).Eval( wContext )
					Main{ self.result = self.result + "\(w)\n" }
				} catch let e {
					Main{ self.result = self.result + "\(e)\n" }
				}
			}
		}
	}
}
