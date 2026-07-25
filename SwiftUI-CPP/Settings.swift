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

	var
	body: some View {
		Form {
			Section {
				Stepper(
					"Rounding precision: \( roundPrecision )"
				,	value	: $roundPrecision
				,	in		: 1 ... 17
				)
				.onChange( of: roundPrecision ) { SliPEngine.setRoundPrecision( roundPrecision ) }

				Text( "Significant digits used when printing a float. 15 matches the web calculator; 17 is the most a double can round-trip." )
					.font( .caption )
					.foregroundStyle( .secondary )
			}
		}
		.formStyle( .grouped )
		.frame( width: 380 )
		.padding()
	}
}
