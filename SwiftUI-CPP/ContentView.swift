import SwiftUI

struct
ContentView: View {

	@Binding	var
	document	: SwiftUI_CPPDocument

	@State		private var
	mode		= SliPMode.calculator

	@State		private var
	results		: [ SliPResult ] = []

	@State		private var
	canvases	: [ SliPCanvas ] = []

	@StateObject	private var
	session		= SliPSession()


	//	The keypad's way into the editor's caret — see EditorProxy.
	@StateObject private var
	editor		= EditorProxy()

	//	What the source would like to be tall, which the display grants up to a
	//	point — see Shared.
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

	//	The keypad is where the panels are followed: macOS keeps it beside the
	//	source the way the web page does, iOS keeps it under the source the way the
	//	Tab5 does.  See Keypad.swift.
	var
	body: some View {
		ZStack( alignment: .topLeading ) {
		//	Beside the display, the keypad runs the whole height — the toolbar goes
		//	inside the display's column rather than across the top of both.  The bar
		//	only ever held a mode picker and a version number, so spanning the window
		//	with it left a band of nothing directly above the keys.
		#if os(macOS)
		HStack( spacing: 0 ) {
			VStack( spacing: 0 ) {
				Toolbar
				Divider()
				Shared
			}
			Divider()
			//	Fixed.  The keypad is not a pane to be traded against the work;
			//	it is as wide as seven columns of keys need and no wider, and
			//	dragging it would only ever make the keys wrong.
			Keys()
				.frame( width: KEYS_W )
		}
		.frame( minWidth: 720, minHeight: 420 )
		.focusedSceneValue( \.slipRunAction, Run )
		.onAppear { Open() }
		#else
		//	One rule: the keypad goes beside the display where there is width for
		//	it, and under the display where there is not.  Turning the phone over is
		//	the same question as picking up the iPad.
		GeometryReader { geometry in
			if geometry.size.width > geometry.size.height {
				HStack( spacing: 0 ) {
					VStack( spacing: 0 ) {
						Toolbar
						Divider()
						Shared
					}
					Divider()
					Keys( shape: Shape( geometry ) )
						.frame( width: KeysWidth( geometry.size.width ) )
				}
			} else {
				VStack( spacing: 0 ) {
					Toolbar
					Divider()
					Shared
					Divider()
					Keys( shape: Shape( geometry ) )
				}
			}
		}
		.focusedSceneValue( \.slipRunAction, Run )
		.onAppear { Open() }
		#endif

		#if os(macOS)
		ForEach( Array( canvases.enumerated() ), id: \.element.id ) { index, canvas in
			FloatingSliPCanvas( canvas: canvas ) {
				canvases.removeAll { $0.id == canvas.id }
			}
			.offset( x: CGFloat( 32 + index * 20 ), y: CGFloat( 56 + index * 20 ) )
		}
		#endif
		}
	}

	//	Changing mode converts the text; adopting the mode a file was saved in does
	//	not, because the text is already in that shape.  Writing that through the
	//	binding rather than through onChange is what keeps the two apart — nothing
	//	has to guess which kind of assignment it is watching.

	private var
	Mode: Binding< SliPMode > {
		Binding(
			get: { mode }
		,	set: {
				guard $0 != mode else { return }
				Convert( to: $0 )
				mode = $0
			}
		)
	}

	//	Seven columns of keys, and DEL and RUN are three letters each while a digit
	//	is one.  Everything narrower than this clips something.
	private let	KEYS_W = CGFloat( 276 )

	//	Beside the display, the keypad takes a little under half of it — enough for
	//	thirteen letters across without leaving the transcript a gutter.
	private func
	KeysWidth( _ width: CGFloat ) -> CGFloat {
		min( max( width * 0.46, KEYS_W ), 560 )
	}

	//	Which shape of space the keypad is being given.  A column beside the display
	//	is a column whatever is holding it, so it gets the sidebar; only a strip
	//	along the bottom has to ask how wide it is, and an iPad has six hundred
	//	points of short side whichever way up it is while a phone never does.
	private func
	Shape( _ geometry: GeometryProxy ) -> KeypadShape {
		if geometry.size.width > geometry.size.height { return .sidebar }
		return min( geometry.size.width, geometry.size.height ) >= 600 ? .beside : .tabbed
	}

	private func
	Keys( shape: KeypadShape = .sidebar ) -> some View {
		Keypad(
			proxy:		editor
		,	program:	mode == .programming
		,	run:		Run
		,	enter:		Accept
		,	shape:		shape
		)
	}

	//	The display, on every host.  It does not have the width for two panes and did not have the height
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
			//	An answer is worth going back to the bottom for; UIPrint on the board
			//	says the same thing by resetting its scroll.  The modifier arrived
			//	after this app's oldest macOS, which is what the check is for.
			if #available( macOS 14.0, iOS 17.0, * ) {
				Results.defaultScrollAnchor( .bottom )
			} else {
				Results
			}
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

			#if os(macOS)
			Menu {
				ForEach( SliPSample.allCases ) { sample in
					Button( sample.title ) { Load( sample ) }
				}
			} label: {
				Label( "Samples", systemImage: "books.vertical" )
			}
			.menuStyle( .borderlessButton )
			.fixedSize()
			.help( "Load a sample program" )
			#endif

			Spacer()

			#if os(macOS)
			Text( "SliP \( SliPEngine.version )" )
				.foregroundStyle( .secondary )
				.font( .caption )
			#endif
			//	No Run and no Delete up here on any host.  Both are keys on the
			//	block, which is always on screen; a second one on the far side of
			//	the window is one more thing to look at and the same thing to press.
			//	⌘↩ still runs it — see slipRunAction, which is what the menu bar and
			//	any hardware keyboard reach.
		}
		.padding( 8 )
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
		let run = session.run( Source(), mode: mode )
		results = run.results
		canvases = run.canvases
	}

	//	The marker is a comment, so it costs the interpreter nothing to read it.
	private func
	Source() -> String { document.text }

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
		let run = session.run( line, mode: .calculator )
		results += run.results
		if !run.canvases.isEmpty { canvases = run.canvases }
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
		let run = session.run( Source(), mode: mode )
		results = run.results
		canvases = run.canvases
	}

	#if os(macOS)
	private func
	Load( _ sample: SliPSample ) {
		guard let url = Bundle.main.url( forResource: sample.rawValue, withExtension: "slip" ),
			  let source = try? String( contentsOf: url, encoding: .utf8 )
		else { return }
		document.text = source
		mode = .programming
		entry = ""
		session.reset()
		let run = session.run( source, mode: .programming )
		results = run.results
		canvases = run.canvases
	}
	#endif

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
}

#if os(macOS)
private enum SliPSample: String, CaseIterable, Identifiable {
	case Koch, MovingComplex, BarnsleyFern, LorenzAttractor
	var id: Self { self }
	var title: String {
		switch self {
		case .Koch: return String( localized: "Koch Curve & Snowflake" )
		case .MovingComplex: return String( localized: "Moving Complex Orbit" )
		case .BarnsleyFern: return String( localized: "Barnsley Fern" )
		case .LorenzAttractor: return String( localized: "Lorenz Attractor" )
		}
	}
}
#endif

#if os(macOS)
private struct FloatingSliPCanvas: View {
	let canvas: SliPCanvas
	let close: () -> Void
	@State private var drag = CGSize.zero
	@State private var resting = CGSize.zero

	var body: some View {
		VStack( spacing: 0 ) {
			HStack {
				Text( "SliP Canvas" ).font( .caption ).foregroundStyle( .secondary )
				Spacer()
				Button( action: close ) { Image( systemName: "xmark" ) }
					.buttonStyle( .plain )
			}
			.padding( .horizontal, 8 )
			.frame( height: 28 )
			.background( .bar )
			.contentShape( Rectangle() )
			.gesture(
				DragGesture()
					.onChanged { drag = $0.translation }
					.onEnded { resting.width += $0.translation.width; resting.height += $0.translation.height; drag = .zero }
			)

			Canvas { context, _ in
				for command in canvas.commands {
					for points in command.paths {
						guard let first = points.first, first.count >= 2 else { continue }
						var path = Path()
						path.move( to: CGPoint( x: first[ 0 ], y: first[ 1 ] ) )
						for point in points.dropFirst() where point.count >= 2 {
							path.addLine( to: CGPoint( x: point[ 0 ], y: point[ 1 ] ) )
						}
						let color = SliPColor( command.kind == "clear" ? "white" : command.color )
						if command.kind == "stroke" {
							context.stroke( path, with: .color( color ), lineWidth: command.width )
						} else {
							context.fill( path, with: .color( color ) )
						}
					}
				}
			}
			.frame( width: canvas.width, height: canvas.height )
			.background( Color.white )
		}
		.fixedSize()
		.background( .regularMaterial )
		.clipShape( RoundedRectangle( cornerRadius: 8 ) )
		.overlay( RoundedRectangle( cornerRadius: 8 ).stroke( .secondary.opacity( 0.45 ) ) )
		.shadow( radius: 12 )
		.offset( x: resting.width + drag.width, y: resting.height + drag.height )
	}

	private func SliPColor( _ value: String ) -> Color {
		let name = value.lowercased()
		switch name {
		case "black": return .black
		case "white": return .white
		case "red": return .red
		case "green": return .green
		case "blue": return .blue
		case "orange": return .orange
		case "yellow": return .yellow
		case "purple": return .purple
		case "gray", "grey": return .gray
		default: break
		}
		guard name.hasPrefix( "#" ) else { return .black }
		let hex = String( name.dropFirst() )
		guard ( hex.count == 6 || hex.count == 8 ), let bits = UInt64( hex, radix: 16 ) else { return .black }
		let hasAlpha = hex.count == 8
		return Color(
			red: Double( ( bits >> ( hasAlpha ? 24 : 16 ) ) & 255 ) / 255,
			green: Double( ( bits >> ( hasAlpha ? 16 : 8 ) ) & 255 ) / 255,
			blue: Double( ( bits >> ( hasAlpha ? 8 : 0 ) ) & 255 ) / 255,
			opacity: hasAlpha ? Double( bits & 255 ) / 255 : 1
		)
	}
}
#endif

#Preview {
	ContentView( document: .constant( SwiftUI_CPPDocument() ) )
}
