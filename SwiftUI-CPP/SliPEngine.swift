import Foundation

//	Swift's view of the engine.  Every bridge call hands back JSON that this file
//	decodes; nothing above it deals in C strings, and a SliP error arrives as a
//	value rather than as a crash.
//
//	The repository's C++ writes `_` for locals and `$` for members.  Swift
//	reserves `_` as a wildcard that cannot be referred to, so this file uses
//	ordinary Swift names.

struct
SliPResult: Identifiable, Equatable {
	let	id		= UUID()
	let	source	: String?	//	The form as the reader understood it
	let	value	: String?	//	Its printed value, when it evaluated
	let	error	: String?	//	Why it did not, when it did not

	var	failed	: Bool { error != nil }
}

enum
SliPMode: String, CaseIterable, Identifiable {
	case	calculator	= "Calculator"
	case	programming	= "Programming"

	var	id: String { rawValue }

	//	One expression per line, versus toplevel forms — see SPEC §1.
	var	help: String {
		switch self {
		case .calculator:	return "One expression per line: 2πr, cosπ, 'r = 2"
		case .programming:	return "Toplevel SliP forms: ( 'fact = '… ), { … }, « … »"
		}
	}
}

enum
SliPEngine {

	static var
	version: String { take( BH_Version() ) }

	static func
	run( _ source: String, mode: SliPMode ) -> [ SliPResult ] {
		decode(
			take(
				mode == .calculator
				?	BH_Sugared( source )
				:	BH_REPL( source )
			)
		)
	}

	static func
	reset() { BH_Reset() }

	static func
	setRoundPrecision( _ precision: Int ) { BH_SetRoundPrecision( Int32( precision ) ) }

	//	The bridge allocates; we own the copy from here.
	private static func
	take( _ pointer: UnsafeMutablePointer< CChar >? ) -> String {
		guard let pointer else { return "" }
		defer { BH_Free( pointer ) }
		return String( cString: pointer )
	}

	private static func
	decode( _ json: String ) -> [ SliPResult ] {
		guard
			let data = json.data( using: .utf8 ),
			let array = try? JSONSerialization.jsonObject( with: data ) as? [ [ String: Any ] ]
		else {
			//	The engine promises parseable JSON.  If that promise is ever
			//	broken, say so plainly instead of showing an empty pane.
			return [ SliPResult( source: nil, value: nil, error: "Engine returned unreadable output: \( json )" ) ]
		}
		return array.map {
			SliPResult(
				source	: $0[ "source"   ] as? String
			,	value	: $0[ "response" ] as? String
			,	error	: $0[ "error"    ] as? String
			)
		}
	}
}
