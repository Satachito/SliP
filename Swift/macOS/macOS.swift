//	Written by Satoru Ogura, Tokyo.
//
//	The AppKit front end. It runs the canonical C++ engine through Bridge/,
//	the same one the SwiftUI app uses — the Swift interpreter this file used to
//	call implemented an older dialect, where a sentence ended at `;`.
//
import Cocoa

@NSApplicationMain class
AppDelegate	: NSObject, NSApplicationDelegate {
	func
	applicationShouldTerminateAfterLastWindowClosed( _: NSApplication ) -> Bool { true }
}

class
ViewController	: NSViewController {

	//	Not named sourceItemView: NSViewController already owns a property by
	//	that name for popover anchoring, so an outlet bound to it is AppKit's to
	//	manage and does not reliably survive as the editor reference.
	@IBOutlet weak var
	sourceTextView: NSTextView!

	private var
	editor: NSTextView? { sourceTextView }

	override func
	viewDidLoad() {
		super.viewDidLoad()
		//	The storyboard sets these as runtime attributes, and that is not
		//	enough: macOS restores substitution per application — the Format ▸
		//	Substitutions menu writes it back — so a quote typed here becomes ’
		//	and ( 'r = 2 ) reports "Undefined name: ’r".  ' is the quote
		//	operator; this has to be off and stay off.
		guard let editor else { return }
		editor.isAutomaticQuoteSubstitutionEnabled		= false
		editor.isAutomaticDashSubstitutionEnabled		= false
		editor.isAutomaticTextReplacementEnabled		= false
		editor.isAutomaticSpellingCorrectionEnabled		= false
		editor.isContinuousSpellCheckingEnabled			= false
		editor.smartInsertDeleteEnabled					= false
		editor.font = .monospacedSystemFont( ofSize: 13, weight: .regular )
	}

	@IBAction func
	Do( _: Any? ) {
		( representedObject as? Document )?.Do()
	}

	@IBAction func
	DoLoadSample( _: Any? ) {
		guard
			let url = ResourceURL( "Sample", "slip" ),
			let data = try? Data( contentsOf: url ),
			let text = UTF8String( data )
		else { return }		//	Missing resource is not worth crashing over
		Insert( text )
	}

	@IBAction func
	DoInsert( _ sender: NSButton ) {
		Insert( sender.title )
	}

	//	Edit the storage rather than calling insertText: the button takes first
	//	responder when clicked, an NSTextView that is not first responder ignores
	//	insertText, and taking focus back makes the value binding push its old
	//	string over what was just typed.  shouldChangeText / didChangeText is the
	//	supported way to change the text and have the binding notice.
	private func
	Insert( _ text: String ) {
		guard
			let editor,
			let storage = editor.textStorage
		else { return }
		let	caret = editor.selectedRange()
		guard editor.shouldChangeText( in: caret, replacementString: text ) else { return }
		storage.replaceCharacters( in: caret, with: text )
		editor.didChangeText()
		editor.setSelectedRange( NSRange( location: caret.location + ( text as NSString ).length, length: 0 ) )
	}
}

class
Document	: NSDocument {

	@objc dynamic	var	m	= ""	//	Source
	@objc dynamic	var	out	= ""	//	Values, one per form
	@objc dynamic	var	err	= ""	//	Whatever failed

	override class var
	autosavesInPlace: Bool { false }

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
		NSApplication.shared.keyWindow?.makeFirstResponder( nil )	//	Sync NSTextView and m
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
		NSApplication.shared.keyWindow?.makeFirstResponder( nil )	//	Sync NSTextView and m

		//	Each run starts from a clean context, as the web calculator does.
		SliPEngine.reset()
		let	results = SliPEngine.run( m, mode: .programming )

		out = results.compactMap { $0.value }.joined( separator: "\n" )
		err = results.compactMap {
			guard let e = $0.error else { return nil }
			return $0.source.map { "\( $0 )\n\( e )" } ?? e
		}.joined( separator: "\n" )
	}
}
