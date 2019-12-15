//	Written by Satoru Ogura, Tokyo.
//
import Cocoa

@NSApplicationMain class
OSXAD	: NSObject, NSApplicationDelegate {
}

class
OSXVC	: NSViewController {
	@IBAction func
	Do( _: Any? ) {
		if let w = representedObject as? Document { w.Do() }
	}
	
	@IBAction func
	DoLoadSample( _: Any? ) {
		if	let wURL = ResourceURL( "Sample", "slip" )
		,	let	wData = try? Data( contentsOf: wURL )
		,	let	wString = UTF8String( wData )
		,	let w = representedObject as? Document {
			w.m = wString
		}
	}
	
	@IBAction func
	DoInsert( _ p: NSButton ) {
		if let w = sourceItemView as? NSTextView { w.insertText( p.title , replacementRange: w.selectedRange() ) }
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
		if let v = DataByUTF8( m ) { return v } else {
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
		let	wContext = Context()
		wContext.Load( SliPBuiltins() )
		let	wReader	= PreProcessor( m )
		result = ""
		output = ""
		Sub {
			while true {
				do {
					let wObjects = try ReadObjects( wReader, ";" as UnicodeScalar )
					let v = try Sentence( ArraySlice( wObjects ) ).Eval( Chain< Context >( wContext ) )
					Main{ self.result += v.str + "\n" }
				} catch let e {
					Main{ self.result += "\(e)\n" }
				}
			}
		}
	}
}
