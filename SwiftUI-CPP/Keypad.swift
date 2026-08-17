import SwiftUI
#if os(macOS)
import AppKit
#else
import UIKit
#endif

//	The keypad, which is most of what makes this a calculator rather than a text
//	editor: almost none of the language's operators is on anybody's keyboard.
//
//	There are two arrangements here, and they are the two the panels arrived at.
//	The Tab5 has a touch panel and nothing else, so its keys are tabbed — four
//	alphabets over one unchanging block of digits, one alphabet showing — and iOS
//	follows it.  The web calculator sits beside a real keyboard, so it drops the
//	tabs and the Latin letters and shows the rest at once in a column that
//	scrolls, and macOS follows that.
//
//	What is on the keys is the same on both, because it is the same language.

//	────────────────────────────────  where a key lands

//	A key has to land where the caret is, and the caret belongs to the text view
//	rather than to the binding: appending to the string would put every symbol at
//	the end of the document however far up the reader had clicked.  So the editor
//	lends its view out through this, and the keypad reaches the text the way the
//	keyboard does.
//
//	On iOS it deliberately does not take first responder.  Becoming one raises the
//	system keyboard, which covers the keypad that was just pressed.
@MainActor final class
EditorProxy: ObservableObject {

	#if os(macOS)
	weak var
	view	: NSTextView?
	#else
	weak var
	view	: UITextView?
	#endif

	func
	insert( _ text: String ) {
		#if os(macOS)
		guard let view else { return }
		view.window?.makeFirstResponder( view )
		view.insertText( text, replacementRange: view.selectedRange() )
		#else
		guard let view, let current = view.text else { return }
		let	range = view.selectedRange
		view.text = ( current as NSString ).replacingCharacters( in: range, with: text )
		view.selectedRange = NSRange(
			location:	range.location + ( text as NSString ).length
		,	length:		0
		)
		view.delegate?.textViewDidChange?( view )
		#endif
	}

	func
	backspace() {
		#if os(macOS)
		guard let view else { return }
		view.window?.makeFirstResponder( view )
		view.deleteBackward( nil )
		#else
		guard let view, let current = view.text else { return }
		let	range = view.selectedRange
		let	text  = current as NSString
		//	Nothing selected means take the character before the caret — and one
		//	character, not one UTF-16 unit, or 𝑒 comes apart into half a surrogate
		//	pair and the text is no longer text.
		let	cut = range.length > 0
		?	range
		:	range.location > 0
			?	text.rangeOfComposedCharacterSequence( at: range.location - 1 )
			:	NSRange( location: 0, length: 0 )
		guard cut.length > 0 else { return }
		view.text = text.replacingCharacters( in: cut, with: "" )
		view.selectedRange = NSRange( location: cut.location, length: 0 )
		view.delegate?.textViewDidChange?( view )
		#endif
	}
}

//	────────────────────────────────  what is on the keys

//	The two keys that do not put their own label in: a space is invisible, so it
//	is drawn as ␣, and ⏎ finishes the line — which in the calculator means run it
//	and in programming means start another one.
let	KEY_SPACE	= "␣"
let	KEY_RETURN	= "⏎"

enum SliPKeys {

	//	The block that does not move.  Six across and four down, the same in both
	//	modes and on all four hosts, because the digits are the digits.  Its last
	//	column is the line: 𝑒 at the top because it is a number and belongs with
	//	them, then open it, close it, finish it.
	static let
	fixed = [
		[ "7", "8", "9", "+", "'", "𝑒" ]
	,	[ "4", "5", "6", "-", "=", "(" ]
	,	[ "1", "2", "3", "×", "@", ")" ]
	,	[ "0", ".", KEY_SPACE, "÷", ":", KEY_RETURN ]
	]

	//	Every operator the reader takes as a single character and that is not on
	//	the block above — the whole of SoloChars, OperatorChars and BreakingChars
	//	less the dozen the block already has, plus £, which is a name rather than
	//	a solo character but is one glyph and belongs with these.
	//
	//	Forty-seven of them.  The count is the point: a key that puts in a
	//	character the reader does not know is a key that answers
	//	"Undefined name", and both panels had one until this list replaced the
	//	two hand-written ones.
	static let
	operators = [
		"!", "#", "$", "%", "*", "/", ";", "?", "`", "~", "<", ">"
	,	"¦", "§", "¬", "¶", "·", "¿", "∈", "∋", "∥", "£", "¤", "¡"
	,	"⊂", "⊃", "∩", "∪", "⊤", "⊥", "∅", "«", "»", "⟨", "⟩", "±"
	,	"&", "|", "^", "[", "]", "{", "}", ",", "\"", "\\", "∞"
	]

	//	The transcendental functions the interpreter already has.
	static let
	functions = [
		"sin", "cos", "tan", "asin", "acos", "atan"
	,	"sinh", "cosh", "tanh", "asinh", "acosh", "atanh"
	,	"exp", "log", "log2", "log10", "sqrt", "cbrt"
	,	"pow", "hypot", "atan2", "abs", "round", "trunc"
	]

	//	Both alphabets, both cases.  The reader has always taken these letters as
	//	names; π is one of them and lives here rather than among the digits, even
	//	though the interpreter also knows it as a constant.  ς is left out — it is
	//	a σ that has reached the end of a word, and a name does not care.
	static let
	greek = [
		"α", "β", "γ", "δ", "ε", "ζ", "η", "θ", "ι", "κ", "λ", "μ"
	,	"ν", "ξ", "ο", "π", "ρ", "σ", "τ", "υ", "φ", "χ", "ψ", "ω"
	,	"Α", "Β", "Γ", "Δ", "Ε", "Ζ", "Η", "Θ", "Ι", "Κ", "Λ", "Μ"
	,	"Ν", "Ξ", "Ο", "Π", "Ρ", "Σ", "Τ", "Υ", "Φ", "Χ", "Ψ", "Ω"
	]

	static let
	latin = [
		"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m"
	,	"n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z"
	,	"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M"
	,	"N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"
	]

	//	The lists are flat and the width is the host's business: twelve Greek
	//	letters across fits the Tab5's panel and eight fits a sidebar, and it is
	//	the same alphabet either way.
	static func
	rows( _ keys: [ String ], _ columns: Int ) -> [ [ String ] ] {
		stride( from: 0, to: keys.count, by: columns ).map {
			Array( keys[ $0 ..< min( $0 + columns, keys.count ) ] )
		}
	}
}

enum
KeypadSection: String, CaseIterable, Identifiable {
	case	operators	= "SliP"
	case	functions	= "func"
	case	latin		= "abc"
	case	greek		= "αβγ"

	var	id: String { rawValue }
}

//	────────────────────────────────  the panel

struct
Keypad: View {

	@ObservedObject	var
	proxy		: EditorProxy

	let
	program		: Bool

	let
	run			: () -> Void

	//	iOS only, and it starts on the letters for the same reason the Tab5 does:
	//	a name is the thing you cannot get at any other way.
	@State private var
	section		= KeypadSection.latin

	var
	body: some View {
		#if os(macOS)
		Sidebar
		#else
		Tabbed
		#endif
	}

	//	The web page's arrangement: the block, then everything else under a
	//	heading, in a column that scrolls.  No tabs and no Latin letters — there
	//	is a keyboard right there for those.
	private var
	Sidebar: some View {
		ScrollView {
			VStack( alignment: .leading, spacing: 0 ) {
				Grid( SliPKeys.fixed, height: 26, size: 13 )
				Divider()
					.padding( .vertical, 8 )
				Title( KeypadSection.operators.rawValue )
				Grid( SliPKeys.rows( SliPKeys.operators, 8 ), height: 22, size: 11 )
				Title( KeypadSection.functions.rawValue )
				Grid( SliPKeys.rows( SliPKeys.functions, 3 ), height: 24, size: 11 )
				Title( KeypadSection.greek.rawValue )
				Grid( SliPKeys.rows( SliPKeys.greek, 8 ), height: 22, size: 12 )
			}
			.padding( 8 )
		}
	}

	//	The Tab5's arrangement: one alphabet of four, over the block.  Tabbed
	//	rather than folded because there is no arrangement where two of these are
	//	worth seeing at once — they are four alphabets and a line is being written
	//	in one of them.
	private var
	Tabbed: some View {
		VStack( spacing: 4 ) {
			Picker( "", selection: $section ) {
				ForEach( KeypadSection.allCases ) { Text( $0.rawValue ).tag( $0 ) }
			}
			.pickerStyle( .segmented )
			.labelsHidden()

			switch section {
			case .operators:	Grid( SliPKeys.rows( SliPKeys.operators, 12 ), height: 32, size: 13 )
			case .functions:	Grid( SliPKeys.rows( SliPKeys.functions, 6 ), height: 32, size: 12 )
			case .latin:		Grid( SliPKeys.rows( SliPKeys.latin, 13 ), height: 32, size: 14 )
			case .greek:		Grid( SliPKeys.rows( SliPKeys.greek, 12 ), height: 32, size: 14 )
			}

			Divider()

			Grid( SliPKeys.fixed, height: 42, size: 19 )
		}
		.padding( 6 )
	}

	private func
	Title( _ text: String ) -> some View {
		Text( text )
			.font( .caption2 )
			.foregroundStyle( .secondary )
			.padding( .bottom, 3 )
	}

	//	A row carries its own width, and a short last row keeps the others' — one
	//	grid over all of them would either cramp the letters or clip `atanh`.
	private func
	Grid( _ rows: [ [ String ] ], height: CGFloat, size: CGFloat ) -> some View {
		let	columns = rows.map( \.count ).max() ?? 1
		return VStack( spacing: 3 ) {
			ForEach( rows, id: \.self ) { row in
				HStack( spacing: 3 ) {
					ForEach( row, id: \.self ) { Key( $0, height: height, size: size ) }
					ForEach( row.count ..< columns, id: \.self ) { _ in
						Color.clear.frame( maxWidth: .infinity, minHeight: height )
					}
				}
			}
		}
	}

	//	Drawn rather than left to .bordered, which keeps a fixed inset on both
	//	sides of its label.  Thirteen letters across a phone is a key thirty points
	//	wide, and that inset ate most of it — `m` and `w` came out clipped.
	private func
	Key( _ key: String, height: CGFloat, size: CGFloat ) -> some View {
		Button {
			Press( key )
		} label: {
			Text( Face( key ) )
				.font( .system( size: size, design: .monospaced ) )
				.fontWeight( key == KEY_RETURN ? .bold : .regular )
				.lineLimit( 1 )
				.minimumScaleFactor( 0.5 )
				.frame( maxWidth: .infinity, minHeight: height )
				.background(
					RoundedRectangle( cornerRadius: 5, style: .continuous )
						.fill( Color.gray.opacity( 0.18 ) )
				)
				.contentShape( Rectangle() )
		}
		.buttonStyle( .plain )
		.accessibilityLabel( Face( key ) )
	}

	//	⏎ says what it will do.  In the calculator a line is the whole of what is
	//	being said, so finishing it runs it; in programming it starts another and
	//	Run in the toolbar finishes the lot.
	private func
	Face( _ key: String ) -> String {
		key == KEY_RETURN && !program ? "RUN" : key
	}

	private func
	Press( _ key: String ) {
		switch key {
		case KEY_SPACE:
			proxy.insert( " " )
		case KEY_RETURN:
			program ? proxy.insert( "\n" ) : run()
		default:
			//	A function is a name, and a name wants air around it: `sin π`
			//	reads, `sinπ` is one name that does not exist.
			proxy.insert( key.count > 1 ? key + " " : key )
		}
	}
}
