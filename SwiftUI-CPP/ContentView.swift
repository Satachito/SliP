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
			HSplitView {
				Editor
				Results
			}
		}
		.frame( minWidth: 720, minHeight: 420 )
		.focusedSceneValue( \.slipRunAction, Run )
	}

	private var
	Toolbar: some View {
		HStack( spacing: 12 ) {
			Picker( "", selection: $mode ) {
				ForEach( SliPMode.allCases ) { Text( $0.title ).tag( $0 ) }
			}
			.pickerStyle( .segmented )
			.frame( width: 220 )
			.help( mode.help )

			Toggle( "Keep session", isOn: $keepSession )
				.help( "Carry bindings over from the previous run" )

			Spacer()

			Text( "SliP \( SliPEngine.version )" )
				.foregroundStyle( .secondary )
				.font( .caption )

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
		.frame( minWidth: 280 )
	}

	private var
	Keypad: some View {
		LazyVGrid( columns: Array( repeating: GridItem( .flexible(), spacing: 2 ), count: 12 ), spacing: 2 ) {
			ForEach( Self.symbols, id: \.self ) { symbol in
				Button( symbol ) { document.text.append( symbol ) }
					.buttonStyle( .bordered )
					.font( .system( .body, design: .monospaced ) )
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
		.frame( minWidth: 280 )
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
