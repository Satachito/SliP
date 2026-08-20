import SwiftUI

struct
ContentView: View {

	@Binding	var
	document	: SwiftUI_CPPDocument

	@State		private var
	mode		= SliPMode.calculator

	@State		private var
	results		: [ SliPResult ] = []

	@StateObject	private var
	session		= SliPSession()


	//	The keypad's way into the editor's caret — see EditorProxy.
	@StateObject private var
	editor		= EditorProxy()

	#if !os(macOS)
	//	What the source would like to be tall, which the phone grants up to a point
	//	— see Shared.
	@State private var
	editorHeight	= CGFloat( 0 )

	//	The calculator's input, which is one line and is not the document.  The
	//	document is the history: a line joins it when it is accepted, which is also
	//	when it reaches the file.
	@State private var
	entry			= ""

	//	The file has been read, its mode taken from it and its history replayed;
	//	doing that twice would undo whatever the reader has since chosen.
	@State private var
	opened			= false

	//	The calculator's history wants a height of its own — it is a third region,
	//	not the same editor under another name.
	@State private var
	historyHeight	= CGFloat( 0 )
	#endif

	//	The keypad is where the panels are followed: macOS keeps it beside the
	//	source the way the web page does, iOS keeps it under the source the way the
	//	Tab5 does.  See Keypad.swift.
	var
	body: some View {
		VStack( spacing: 0 ) {
			Toolbar
			Divider()
			#if os(macOS)
			HSplitView {
				Keys
					//	Seven columns now, so a little wider than the web page's nav
					//	was; DEL and RUN are three letters each and the digits are one.
					.frame( minWidth: 244, idealWidth: 276, maxWidth: 400 )
				Editor
				Results
			}
			#else
			GeometryReader { geometry in
				if geometry.size.width >= 700 {
					HStack( spacing: 0 ) {
						Editor
							.frame( width: geometry.size.width * 0.52 )
						Divider()
						Results
					}
				} else {
					Shared
				}
			}
			Divider()
			Keys
			#endif
		}
		#if os(macOS)
		.frame( minWidth: 720, minHeight: 420 )
		#endif
		.focusedSceneValue( \.slipRunAction, Run )
		#if !os(macOS)
		.onAppear { Open() }
		#endif
	}

	//	Changing mode converts the text; adopting the mode a file was saved in does
	//	not, because the text is already in that shape.  Writing that through the
	//	binding rather than through onChange is what keeps the two apart — nothing
	//	has to guess which kind of assignment it is watching.
	//	⏎ on a host with a history to put the line into accepts it; everywhere else
	//	finishing a line in the calculator is still what runs the lot.
	private func
	Enter() {
		#if os(macOS)
		Run()
		#else
		Accept()
		#endif
	}

	private var
	Mode: Binding< SliPMode > {
		#if os(macOS)
		return $mode
		#else
		return Binding(
			get: { mode }
		,	set: {
				guard $0 != mode else { return }
				Convert( to: $0 )
				mode = $0
			}
		)
		#endif
	}

	private var
	Keys: some View {
		Keypad(
			proxy:		editor
		,	program:	mode == .programming
		,	run:		Run
		,	enter:		Enter
		)
	}

	#if !os(macOS)
	//	The phone does not have the width for two panes and did not have the height
	//	for two either: it gave the source most of the screen whether there was
	//	anything in it or not, and the answers the rest whether they fitted or not.
	//
	//	So it does what the Tab5 does.  Bottom up: the keypad, the line being
	//	written, and the transcript in whatever is left.  The transcript already
	//	carries both halves — every answer is printed under the form it came from —
	//	which is what makes one region enough.
	private var
	Shared: some View {
		VStack( spacing: 0 ) {
			Results
				//	An answer is worth going back to the bottom for; UIPrint on the
				//	board says the same thing by resetting its scroll.
				.defaultScrollAnchor( .bottom )
			Divider()
			if mode == .calculator {
				//	The history, which is the document and therefore the file.  It is
				//	editable: a calculator whose tape can be corrected and run again
				//	is worth more than one that can only be typed at.
				CodeEditor( text: $document.text, proxy: editor, height: $historyHeight )
					.frame( height: min( max( historyHeight, 44 ), 140 ) )
				Divider()
				//	And the line being written, which is not in the file until ⏎ puts
				//	it there.
				CodeEditor(
					text:		$entry
				,	proxy:		editor
				,	claims:		true
				,	singleLine:	true
				,	onReturn:	Accept
				)
				.frame( height: 48 )
			} else {
				CodeEditor( text: $document.text, proxy: editor, claims: true, height: $editorHeight )
					.frame( height: min( max( editorHeight, 52 ), 260 ) )
			}
		}
	}
	#endif

	private var
	Toolbar: some View {
		HStack( spacing: 12 ) {
			Picker( "", selection: Mode ) {
				ForEach( SliPMode.allCases ) { Text( $0.title ).tag( $0 ) }
			}
			.pickerStyle( .segmented )
			#if os(macOS)
			.frame( width: 220 )
			#else
			.frame( maxWidth: 260 )
			#endif
			.help( mode.help )

			Spacer()

			#if os(macOS)
			Text( "SliP \( SliPEngine.version )" )
				.foregroundStyle( .secondary )
				.font( .caption )

			Button( "Run" ) { Run() }
				.keyboardShortcut( .return, modifiers: .command )
			#else
			//	No Run and no Delete up here.  Both are keys on the block now, where
			//	the thumb already is; a second one on the far side of the screen is
			//	one more thing to look at and the same thing to press.  ⌘↩ still runs
			//	it — see slipRunAction, which is what the menu bar and any hardware
			//	keyboard reach.
			#endif
		}
		.padding( 8 )
	}

	private var
	Editor: some View {
		CodeEditor( text: $document.text, proxy: editor )
		#if os(macOS)
			.frame( minWidth: 280 )
		#endif
	}

	private var
	Results: some View {
		ScrollView {
			LazyVStack( alignment: .leading, spacing: 6 ) {
				ForEach( results ) { Row( $0 ) }
			}
			.frame( maxWidth: .infinity, alignment: .leading )
			.padding( 8 )
		}
		#if os(macOS)
		.frame( minWidth: 280 )
		#endif
	}

	@ViewBuilder private func
	Row( _ result: SliPResult ) -> some View {
		VStack( alignment: .leading, spacing: 2 ) {
			if let source = result.source {
				Text( source )
					.foregroundStyle( .secondary )
					.font( .system( .caption, design: .monospaced ) )
					.textSelection( .enabled )
			}
			Text( result.error ?? result.value ?? "" )
				.foregroundStyle( result.failed ? Color.red : Color.primary )
				.font( .system( .body, design: .monospaced ) )
				.textSelection( .enabled )
		}
	}

	//	RUN runs the whole of what is written — the program in one mode, the history
	//	in the other — and replaces the answers, because it is one reading of one
	//	thing from the start.
	//	The session goes back to nothing first, always.  RUN is one reading of the
	//	whole of what is written, from the start; a switch that suspended that was
	//	a second rule to hold in mind, and with the history editable there is
	//	nothing left for it to preserve that is not already written down.
	private func
	Run() {
		session.reset()
		results = session.run( Source(), mode: mode )
	}

	//	The marker is a comment, so it costs the interpreter nothing to read it.
	private func
	Source() -> String { document.text }

	#if !os(macOS)
	//	A line at a time.  It joins the history — which is the document, and so the
	//	file — and is answered under itself in the transcript.  The session is not
	//	reset between lines: `'r = 2` and then `2πr` is the whole point of a
	//	calculator that has names in it.  RUN is the other half of that rule: it
	//	reads the history again from nothing.
	private func
	Accept() {
		let	line = entry.trimmingCharacters( in: .whitespaces )
		guard !line.isEmpty else { return }
		if !document.text.isEmpty, !document.text.hasSuffix( "\n" ) { document.text += "\n" }
		document.text += line + "\n"
		results += session.run( line, mode: .calculator )
		entry = ""
	}

	//	Opening a file replays it.  What is on the screen when a document opens
	//	should be what that document says, and in the calculator that is a
	//	transcript — the boards do the same thing with the session they saved.
	private func
	Open() {
		guard !opened else { return }
		opened = true
		mode = SliPText.isProgram( document.text ) ? .programming : .calculator
		guard !document.text.trimmingCharacters( in: .whitespacesAndNewlines ).isEmpty
		else { return }
		results = session.run( Source(), mode: mode )
	}

	//	Changing mode rewrites what is there into what the other mode would have
	//	written — see SliPText.  Neither direction is lossless, and leaving the
	//	text alone is worse: every line would be a syntax error in the mode it
	//	arrived in.
	private func
	Convert( to: SliPMode ) {
		let	bare = SliPText.unmarked( document.text )
		document.text = to == .programming
		?	SliPText.marked( SliPText.parenthesised( bare ) )
		:	SliPText.flattened( bare )
		entry = ""
	}
	#endif
}

#Preview {
	ContentView( document: .constant( SwiftUI_CPPDocument() ) )
}
