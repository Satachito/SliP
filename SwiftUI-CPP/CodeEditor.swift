import SwiftUI
import AppKit

//	SwiftUI's TextEditor cannot turn off macOS text substitution, and for this
//	language that is fatal: ' is the quote operator, so smart quotes rewrite
//	( 'r = 2 ) into ( ’r = 2 ) and the reader answers "Undefined name: ’r".
//	Dashes matter too — an em dash where the source said - is not subtraction.
//
//	So the editor is an NSTextView with every substitution off.

struct
CodeEditor: NSViewRepresentable {

	@Binding var
	text: String

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

		return scroll
	}

	func
	updateNSView( _ scroll: NSScrollView, context: Context ) {
		guard let view = scroll.documentView as? NSTextView else { return }
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
