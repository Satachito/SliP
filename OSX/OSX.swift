//	Written by Satoru Ogura, Tokyo.
//
import Cocoa

@NSApplicationMain class
OSXAD	: NSObject, NSApplicationDelegate {
}

class
OSXVC	: NSViewController {
	@IBAction func
	Do( _: AnyObject ) {
		if let w = view.window?.windowController?.document as? Document { w.Do() }
	}
	
	/*
	override var representedObject: Any? {
		didSet {
		// Update the view, if already loaded.
		}
	}
	*/
	
	@IBAction func
	DoLoadSample( _: AnyObject ) {
		if	let wURL = ResourceURL( "Sample", "slip" )
		,	let	wData = try? Data( contentsOf: wURL )
		,	let	wString = UTF8String( wData )
		,	let w = representedObject as? Document {
			w.m = wString
		}
	}
	
	@IBAction func
	DoInsert( _ a: NSButton ) {
		if let w = sourceItemView as? NSTextView { w.insertText( a.title , replacementRange: w.selectedRange() ) }
	}
}

class
Document	: NSDocument {

	@objc dynamic	var	m = ""
	@objc dynamic	var	result = ""
	@objc dynamic	var	output = ""

	override class var
	autosavesInPlace: Bool {
		return false
	}

	override func
	makeWindowControllers() {
		if let wWC = NSStoryboard( name: "Main", bundle: nil ).instantiateController(
			withIdentifier: NSStoryboard.SceneIdentifier( "Document Window Controller" )
		) as? NSWindowController {
			if let wVC = wWC.contentViewController { wVC.representedObject = self }
			addWindowController( wWC )
		}
	}

	override func
	data( ofType typeName: String ) throws -> Data {
		NSApplication.shared.keyWindow?.makeFirstResponder( nil )	//	Sync NSTextView and u
		if let v = UTF8Data( m ) { return v } else {
			throw NSError( domain: "ApplicationErrorDomain", code: 1, userInfo: nil )
		}
	}

	override func
	read( from data: Data, ofType typeName: String ) throws {
		if let w = UTF8String( data ) { m = w } else {
			throw NSError( domain: "ApplicationErrorDomain", code: 2, userInfo: nil )
		}
	}

	func
	Do() {
		NSApplication.shared.keyWindow?.makeFirstResponder( nil )	//	Sync NSTextView and u
		let	wContext = Context( printer: { a in Main{ self.output = self.output + a } } )
		let	wReader	= PreProcessor( m )
		result = ""
		output = ""
		Sub {
			while true {
				do {
					let wObjects = try ReadObjects( wReader, ";" as UnicodeScalar )
					let v = try Sentence( wObjects ).Eval( wContext )
					Main{ self.result = self.result + v.debug + "\n" }
				} catch let e {
					if e is ReaderError { break }
					Main{ self.result = self.result + "\(e)\n" }
				}
			}
		}
	}
}
