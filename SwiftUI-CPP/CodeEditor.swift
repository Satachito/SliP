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

	//	The same four the iOS editor has, and for the same reasons: there are three
	//	regions on screen now and two of them are editors, on this host as well.
	var
	claims: Bool = false

	var
	singleLine: Bool = false

	var
	onReturn: ( () -> Void )?

	var
	height: Binding< CGFloat >?

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
		view.isVerticallyResizable	= true

		if singleLine {
			scroll.hasVerticalScroller	= false
			view.textContainer?.widthTracksTextView	= true
			view.textContainer?.maximumNumberOfLines	= 1
		}
		view.font				= .monospacedSystemFont( ofSize: 13, weight: .regular )
		view.textContainerInset	= NSSize( width: 6, height: 8 )
		view.delegate			= context.coordinator
		view.string				= text

		//	At the end, so that the first key pressed lands after the text rather
		//	than in front of it.
		view.setSelectedRange( NSRange( location: ( text as NSString ).length, length: 0 ) )

		if claims {
			proxy?.view = view
			//	And the caret starts here.  Two editors are on screen and the first
			//	one built takes first responder otherwise, which is the history —
			//	so the app opened with the cursor in what has already been said
			//	rather than in the line being written.  Asynchronously, because
			//	there is no window to be first responder in yet.
			DispatchQueue.main.async { view.window?.makeFirstResponder( view ) }
		}

		return scroll
	}

	func
	updateNSView( _ scroll: NSScrollView, context: Context ) {
		guard let view = scroll.documentView as? NSTextView else { return }
		context.coordinator.parent = self
		//	Only when the model moved out from under the view — assigning while
		//	the user types would reset the insertion point on every keystroke.
		if view.string != text { view.string = text }
		Report( view )
	}

	//	What the text would like to be tall.  Asked of the layout manager rather
	//	than counted, because a line that wraps is two.  Asynchronously, for the
	//	same two reasons as on iOS — this runs inside a layout pass, and on the
	//	pass after a mode change the view has not been laid out yet.
	func
	Report( _ view: NSTextView ) {
		guard let height else { return }
		DispatchQueue.main.async {
			guard
				let manager = view.layoutManager
			,	let container = view.textContainer
			,	view.bounds.width > 0
			else { return }
			manager.ensureLayout( for: container )
			let	fitted = manager.usedRect( for: container ).height + view.textContainerInset.height * 2
			if abs( height.wrappedValue - fitted ) > 0.5 { height.wrappedValue = fitted }
		}
	}

	func
	makeCoordinator() -> Coordinator { Coordinator( self ) }

	final class
	Coordinator: NSObject, NSTextViewDelegate {

		var
		parent: CodeEditor

		init( _ parent: CodeEditor ) { self.parent = parent }

		func
		textDidChange( _ notification: Notification ) {
			guard let view = notification.object as? NSTextView else { return }
			parent.text = view.string
			parent.Report( view )
		}

		//	The keys follow the caret.
		func
		textDidBeginEditing( _ notification: Notification ) {
			guard let view = notification.object as? NSTextView else { return }
			parent.proxy?.view = view
		}

		//	Return is not a character in a one-line field; it is the end of the
		//	line, which is somebody else's business.
		func
		textView( _ view: NSTextView, doCommandBy selector: Selector ) -> Bool {
			guard parent.singleLine, selector == #selector( NSResponder.insertNewline(_:) )
			else { return false }
			parent.onReturn?()
			return true
		}
	}
}
#else
struct CodeEditor: UIViewRepresentable {

	@Binding var text: String

	//	The keypad's way in — see EditorProxy.
	var proxy: EditorProxy?

	//	Which of the editors on screen the keypad types into to begin with.  The
	//	calculator has two — the line being written and the history above it — and
	//	the keys go to the line until the reader taps into the history, at which
	//	point they follow, because that is where the caret went.
	var claims = false

	//	One line, which is what the calculator's input is: Return finishes it
	//	rather than lengthening it, and there is nothing to scroll.
	var singleLine = false

	//	What Return does when there is only one line — the keypad's ⏎ calls the
	//	same thing, and a hardware keyboard should not be a different calculator.
	var onReturn: ( () -> Void )?

	//	When bound, the editor reports the height its text would like and the
	//	caller decides how much of it to grant.  The phone gives the line being
	//	written what it needs and the transcript everything else, which is the
	//	Tab5's arrangement; scrolling stays on, so a program longer than the cap
	//	still scrolls inside what it was given.
	var height: Binding< CGFloat >?

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
		view.isScrollEnabled = !singleLine
		if singleLine { view.textContainer.maximumNumberOfLines = 1 }

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

		if claims { proxy?.view = view }
		return view
	}

	func updateUIView(_ view: UITextView, context: Context) {
		context.coordinator.parent = self
		if view.text != text { view.text = text }
		Report( view )
	}

	//	Asynchronously, and the measuring happens there too.  Two reasons, and the
	//	second one is why the width is read late: this is called during a layout
	//	pass, so writing somebody's @State here would be changing the view tree
	//	while it is being read — and on the pass that follows a mode change the
	//	view is brand new and has no width yet, so measuring it now measures
	//	nothing and the editor keeps whatever height it had.
	func Report( _ view: UITextView ) {
		guard let height else { return }
		DispatchQueue.main.async {
			guard view.bounds.width > 0 else { return }
			let	fitted = view.sizeThatFits(
				CGSize( width: view.bounds.width, height: .greatestFiniteMagnitude )
			).height
			if abs( height.wrappedValue - fitted ) > 0.5 { height.wrappedValue = fitted }
		}
	}

	func makeCoordinator() -> Coordinator { Coordinator(self) }

	final class Coordinator: NSObject, UITextViewDelegate {
		var parent: CodeEditor

		init(_ parent: CodeEditor) { self.parent = parent }

		//	The keys follow the caret.
		func textViewDidBeginEditing(_ textView: UITextView) {
			parent.proxy?.view = textView
		}

		func textViewDidChange(_ textView: UITextView) {
			parent.text = textView.text
			parent.Report( textView )
		}

		//	Return is not a character in a one-line field; it is the end of the
		//	line, which is somebody else's business.
		func textView(
			_ view: UITextView
		,	shouldChangeTextIn range: NSRange
		,	replacementText text: String
		) -> Bool {
			guard parent.singleLine, text == "\n" else { return true }
			parent.onReturn?()
			return false
		}
	}
}
#endif
