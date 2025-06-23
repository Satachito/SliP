import SwiftUI

struct
ContentView: View {

	@Binding	var
	document	: SwiftUI_CPPDocument

	@State		var
	text		: String

	var
	body: some View {
		HStack {
			TextEditor( text: $document.text ).autocorrectionDisabled( true )
			Button( "→" ) {
				var	count: UInt64 = 0;
				let
				reprs = BH_CoreSyntaxLoop( document.text, &count )!
				text = ""
				for i in 0 ..< count {
					if let repr = reprs[ Int( i ) ] {
						print( String( cString: repr ) )
						text += String( cString: repr ) + "\n"
					}
				}
				BH_FreeREPRs( reprs, count );
			}
			TextEditor( text: $text ).autocorrectionDisabled( true )
		}
	}
}

#Preview {
	ContentView( document: .constant( SwiftUI_CPPDocument()), text: "" )
}
