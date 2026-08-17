import SwiftUI
#if os(macOS)
import AppKit
#else
import UIKit
#endif

//	SwiftUI's TextEditor cannot turn off macOS text substitution, and for this
//	language that is fatal: ' is the quote operator, so smart quotes rewrite
//	( 'r = 2 ) into ( ’r = 2 ) and the reader answers "Undefined name: ’r".
//	Dashes matter too — an em dash where the source said - is not subtraction.
//
//	So the editor is an NSTextView with every substitution off.

#if os(macOS)
struct CodeEditor: NSViewRepresentable {

	@Binding var
	text: String

	//	The keypad's way in — see EditorProxy.
	var
	proxy: EditorProxy?

	func
	makeNSView( context: Context ) -> NSScrollView {

		let	scroll = NSTextView.scrollableTextView()
		guard let view = scroll.documentView as? NSTextView else { return scroll }

		//	The substitutions this editor exists to prevent
		view.isAutomaticQuoteSubstitutionEnabled		= false
		view.isAutomaticDashSubstitutionEnabled		= false
		view.isAutomaticTextReplacementEnabled		= false
		view.isAutomaticSpellingCorrectionEnabled	= false
		view.isAutomaticDataDetectionEnabled		= false
		view.isAutomaticLinkDetectionEnabled		= false
		view.isContinuousSpellCheckingEnabled		= false
		view.isGrammarCheckingEnabled				= false
		view.smartInsertDeleteEnabled				= false

		view.isRichText			= false
		view.allowsUndo			= true
		view.font				= .monospacedSystemFont( ofSize: 13, weight: .regular )
		view.textContainerInset	= NSSize( width: 6, height: 8 )
		view.delegate			= context.coordinator
		view.string				= text

		//	At the end, so that the first key pressed lands after the text rather
		//	than in front of it.
		view.setSelectedRange( NSRange( location: ( text as NSString ).length, length: 0 ) )

		proxy?.view = view

		return scroll
	}

	func
	updateNSView( _ scroll: NSScrollView, context: Context ) {
		guard let view = scroll.documentView as? NSTextView else { return }
		proxy?.view = view
		//	Only when the model moved out from under the view — assigning while
		//	the user types would reset the insertion point on every keystroke.
		if view.string != text { view.string = text }
	}

	func
	makeCoordinator() -> Coordinator { Coordinator( self ) }

	final class
	Coordinator: NSObject, NSTextViewDelegate {

		private let
		parent: CodeEditor

		init( _ parent: CodeEditor ) { self.parent = parent }

		func
		textDidChange( _ notification: Notification ) {
			guard let view = notification.object as? NSTextView else { return }
			parent.text = view.string
		}
	}
}
#else
struct CodeEditor: UIViewRepresentable {

	@Binding var text: String

	//	The keypad's way in — see EditorProxy.
	var proxy: EditorProxy?

	func makeUIView(context: Context) -> UITextView {
		let view = UITextView()
		view.autocorrectionType = .no
		view.autocapitalizationType = .none
		view.smartQuotesType = .no
		view.smartDashesType = .no
		view.smartInsertDeleteType = .no
		view.spellCheckingType = .no
		view.keyboardType = .asciiCapable
		view.font = .monospacedSystemFont(ofSize: 15, weight: .regular)
		view.textContainerInset = UIEdgeInsets(top: 10, left: 8, bottom: 10, right: 8)
		view.delegate = context.coordinator
		view.text = text

		//	No system keyboard on the phone.  Everything the language is written in
		//	is on the keypad below — digits, operators, both alphabets, the function
		//	names — and the keyboard's only contribution was to cover it.
		//
		//	An empty inputView is a keyboard of no height rather than no keyboard:
		//	the caret still blinks, the selection still works, long-press still
		//	offers Paste, and a hardware keyboard still types.  The iPad keeps its
		//	own, where there is room for both.
		if UIDevice.current.userInterfaceIdiom == .phone {
			view.inputView = UIView( frame: .zero )
		}

		//	At the end, so that the first key pressed lands after the text rather
		//	than in front of it.
		view.selectedRange = NSRange( location: ( text as NSString ).length, length: 0 )

		proxy?.view = view
		return view
	}

	func updateUIView(_ view: UITextView, context: Context) {
		proxy?.view = view
		if view.text != text { view.text = text }
	}

	func makeCoordinator() -> Coordinator { Coordinator(self) }

	final class Coordinator: NSObject, UITextViewDelegate {
		private let parent: CodeEditor

		init(_ parent: CodeEditor) { self.parent = parent }

		func textViewDidChange(_ textView: UITextView) {
			parent.text = textView.text
		}
	}
}
#endif
