import SwiftUI

//	Preferences that outlive a document, stored where macOS expects them.

enum
Preference {
	static let	roundPrecisionKey	= "roundPrecision"
	static let	keepSessionKey		= "keepSession"

	static let	defaultRoundPrecision = 15		//	Matches RoundPrecision in SliP.hpp

	//	Called at launch: UserDefaults is the source of truth, and the engine
	//	starts at its own default until something tells it otherwise.
	static func
	apply() {
		UserDefaults.standard.register(
			defaults: [
				roundPrecisionKey	: defaultRoundPrecision
			,	keepSessionKey		: false
			]
		)
		SliPEngine.setRoundPrecision( UserDefaults.standard.integer( forKey: roundPrecisionKey ) )
	}
}

struct
SettingsView: View {

	@AppStorage( Preference.roundPrecisionKey ) private var
	roundPrecision = Preference.defaultRoundPrecision
	#if os(macOS)
	@Environment( \.openWindow ) private var openWindow
	#endif

	var
	body: some View {
		Form {
			Section {
				Stepper(
					"Rounding precision: \( roundPrecision )"
				,	value	: $roundPrecision
				,	in		: 1 ... 17
				)
				.onChange( of: roundPrecision ) { _ in SliPEngine.setRoundPrecision( roundPrecision ) }

				Text( "Significant digits used when printing a float. 15 matches the web calculator; 17 is the most a double can round-trip." )
					.font( .caption )
					.foregroundStyle( .secondary )

				Divider()

				#if os(macOS)
				HStack {
					Button( "SliP Help" ) { openWindow( id: "help" ) }
					Button( "Privacy Policy" ) { openWindow( id: "privacy" ) }
				}
				#endif
			}
		}
		#if os(macOS)
		.formStyle( .grouped )
		.frame( width: 380 )
		#endif
		.padding()
	}
}

struct HelpView: View {
	var body: some View {
		ScrollView {
			VStack( alignment: .leading, spacing: 18 ) {
				Text( "Welcome to SliP" )
					.font( .largeTitle.bold() )

				Text( "SliP is a programmable symbolic calculator. Write expressions on the left and press ⌘↩; results appear on the right." )

				Group {
					Text( "Calculator mode" ).font( .title2.bold() )
					Text( "Evaluates one expression per line and continues after an error. Try:")
					CodeSample( "1 + 2 × 3\ncosπ\n[1 2 3] + 10" )

					Text( "Programming mode" ).font( .title2.bold() )
					Text( "Reads top-level language forms and stops at the first error, so definitions can depend on earlier forms.")
					CodeSample( "( 'r = 2 )\n( 2 π r )" )

					Text( "Sessions and documents" ).font( .title2.bold() )
					Text( "Keep session preserves bindings between runs in the current document window. Each window has its own independent session.")

					Text( "Keyboard and symbols" ).font( .title2.bold() )
					Text( "Run with ⌘↩. The keypad inserts operators that are difficult to type on a standard keyboard.")
				}

				Link( "Open the complete language reference", destination: URL( string: "https://slip.828.tokyo/SPEC.html" )! )
			}
			.frame( maxWidth: .infinity, alignment: .leading )
			.padding( 28 )
		}
	}
}

private struct CodeSample: View {
	let text: String

	init( _ text: String ) { self.text = text }

	var body: some View {
		Text( text )
			.font( .system( .body, design: .monospaced ) )
			.textSelection( .enabled )
			.padding( 12 )
			.frame( maxWidth: .infinity, alignment: .leading )
			.background( .quaternary, in: RoundedRectangle( cornerRadius: 8 ) )
	}
}

struct PrivacyPolicyView: View {
	var body: some View {
		ScrollView {
			VStack( alignment: .leading, spacing: 16 ) {
				Text( "Privacy Policy" )
					.font( .largeTitle.bold() )
				Text( "Effective July 26, 2026" )
					.foregroundStyle( .secondary )

				Text( "SliP does not collect, transmit, sell, or share personal information or usage data.")
				Text( "Programs and results are evaluated locally on your device. Documents are read or written only when you create, open, or save them using standard system document controls.")
				Text( "SliP contains no advertising, analytics, account system, or third-party tracking SDK. Opening the online language reference leaves the app and is subject to your browser and the website’s policies.")

				Text( "Contact" ).font( .title2.bold() )
				Link( "SliP project and support", destination: URL( string: "https://github.com/Satachito/SliP" )! )
			}
			.frame( maxWidth: .infinity, alignment: .leading )
			.padding( 28 )
		}
	}
}
