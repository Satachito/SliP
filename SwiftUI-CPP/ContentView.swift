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

	@AppStorage( Preference.keepSessionKey ) private var
	keepSession	= false

	//	The keypad's way into the editor's caret — see EditorProxy.
	@StateObject private var
	editor		= EditorProxy()

	#if !os(macOS)
	//	What the source would like to be tall, which the phone grants up to a point
	//	— see Shared.
	@State private var
	editorHeight	= CGFloat( 0 )
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
	}

	private var
	Keys: some View {
		Keypad( proxy: editor, program: mode == .programming, run: Run )
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
			CodeEditor( text: $document.text, proxy: editor, height: $editorHeight )
				.frame( height: min( max( editorHeight, 52 ), 260 ) )
		}
	}
	#endif

	private var
	Toolbar: some View {
		HStack( spacing: 12 ) {
			Picker( "", selection: $mode ) {
				ForEach( SliPMode.allCases ) { Text( $0.title ).tag( $0 ) }
			}
			.pickerStyle( .segmented )
			#if os(macOS)
			.frame( width: 220 )
			#else
			.frame( maxWidth: 260 )
			#endif
			.help( mode.help )

			#if os(macOS)
			Toggle( "Keep session", isOn: $keepSession )
				.help( "Carry bindings over from the previous run" )
			#else
			Toggle( "Keep", isOn: $keepSession )
				.labelsHidden()
				.accessibilityLabel( "Keep session" )
			#endif

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

	private func
	Run() {
		if !keepSession { session.reset() }
		results = session.run( document.text, mode: mode )
	}
}

#Preview {
	ContentView( document: .constant( SwiftUI_CPPDocument() ) )
}
