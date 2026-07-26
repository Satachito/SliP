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

	//	The operators are most of the language and none of them are on a keyboard.
	private static let
	symbols = [
		"'", "@", "£", "¶", "¤", "∅",
		"×", "÷", "±", "·", "∈", "∋",
		"¿", "¬", "¡", "¦", "§", "∥",
		"⟨", "⟩", "«", "»", "𝑒", "π",
	]

	var
	body: some View {
		VStack( spacing: 0 ) {
			Toolbar
			Divider()
			#if os(macOS)
			HSplitView {
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
					VStack( spacing: 0 ) {
						Editor
							.frame( height: geometry.size.height * 0.58 )
						Divider()
						Results
					}
				}
			}
			#endif
		}
		#if os(macOS)
		.frame( minWidth: 720, minHeight: 420 )
		#endif
		.focusedSceneValue( \.slipRunAction, Run )
	}

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
			#endif

			Button( "Run" ) { Run() }
				.keyboardShortcut( .return, modifiers: .command )
		}
		.padding( 8 )
	}

	private var
	Editor: some View {
		VStack( spacing: 0 ) {
			CodeEditor( text: $document.text )
			Divider()
			Keypad
		}
		#if os(macOS)
		.frame( minWidth: 280 )
		#endif
	}

	private var
	Keypad: some View {
		#if os(macOS)
		let columnCount = 12
		#else
		let columnCount = 6
		#endif
		return LazyVGrid( columns: Array( repeating: GridItem( .flexible(), spacing: 4 ), count: columnCount ), spacing: 4 ) {
			ForEach( Self.symbols, id: \.self ) { symbol in
				Button( symbol ) { document.text.append( symbol ) }
					.buttonStyle( .bordered )
					.font( .system( .body, design: .monospaced ) )
					#if !os(macOS)
					.frame( minHeight: 32 )
					#endif
			}
		}
		.padding( 6 )
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
