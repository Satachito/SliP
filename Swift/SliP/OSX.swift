//	Written by Satoru Ogura, Tokyo.
//
import Cocoa

@NSApplicationMain class
OSXAD	:	NSObject, NSApplicationDelegate {
}

class
OSXVC	:	NSViewController {
	@IBAction func
	Do( _: AnyObject ) {
		if let w = view.window?.windowController?.document as? Document { w.Do() }
	}

	@IBAction func
	DoLoadSample( _: AnyObject ) {
		if	let wURL = ResourceURL( "Sample", "slip" )
		,	let	wData = Data( wURL )
		,	let	wString = UTF8String( wData )
		,	let w = view.window?.windowController?.document as? Document {
			w.m = wString
		}
	}
	
	@IBAction func
	DoInsert( a: NSButton ) {
		if let w = sourceItemView as? NSTextView { w.insertText( a.title , replacementRange: w.selectedRange() ) }
	}
}

class
Document	:	NSDocument {

	dynamic	var	m = ""
	dynamic	var	result = ""
	dynamic	var	output = ""

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
		if let v = UTF8Data( m ) { return v } else {
			throw NSError( domain: "ApplicationErrorDomain", code: 1, userInfo: nil )
		}
	}

	override func
	readFromData( data: NSData, ofType typeName: String ) throws {
		if let w = UTF8String( data ) { m = w } else {
			throw NSError( domain: "ApplicationErrorDomain", code: 2, userInfo: nil )
		}
	}

	func
	Do() {
		NSApplication.sharedApplication().keyWindow?.makeFirstResponder( nil )	//	Sync NSTextView and u
		let	wContext = Context()
		let	wReader	= GUIPreProcessor( m )
		result = ""
		output = ""
		Sub {
			while true {
				do {
					let wObjects = try ReadObjects( wReader, ";" as UnicodeScalar )
					if wObjects.count == 0 { continue }
					let w = try List( wObjects, .Sentence ).Eval(
						wContext
					,	{ a in Main{ self.output = self.output + a } }
					)
					Main{ self.result = self.result + "\(w)\n" }
				} catch let e {
					Main{ self.result = self.result + "\(e)\n" }
				}
			}
		}
	}
}
