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

//	And the two that are not characters at all, which only the touch block
//	carries: on a Mac the keyboard in front of you already has both.
let	KEY_DELETE	= "DEL"
let	KEY_RUN		= "RUN"

enum SliPKeys {

	//	The block that does not move.  Seven across and four down: the digits, the
	//	operators anybody would expect beside them, the punctuation a line is built
	//	out of — quote among it, because `'r = 2` is how anything gets a name — and
	//	then a seventh column of the three things that are not characters at all,
	//	delete and run and a newline.
	//
	//	The same on every host, which is the whole point of it.  A key that moves
	//	or resizes between one screen and the next is a key you have to look at
	//	before pressing, and that is as true of moving between a phone and a Mac as
	//	it is of moving between two modes.
	//
	//	⏎ is only ever a newline now, because RUN is a key of its own beside it.
	static let
	fixed = [
		[ "7", "8", "9", "+", "%", "'", KEY_DELETE ]
	,	[ "4", "5", "6", "-", "/", "="             ]
	,	[ "1", "2", "3", "×", "(", "@", KEY_RUN    ]
	,	[ "0", ".", KEY_SPACE, "÷", ")", ":", KEY_RETURN ]
	]

	//	Every operator the reader takes as a single character and that is not on
	//	the block above — the whole of SoloChars, OperatorChars and BreakingChars
	//	less the dozen the block already has, plus £, which is a name rather than
	//	a solo character but is one glyph and belongs with these.
	//
	//	Forty-nine of them, and the block takes three.  The count is the point in
	//	both directions: a key that puts in a character the reader does not know
	//	answers "Undefined name", and a character with no key cannot be written at
	//	all — which is what happened to `'` for one commit, when the block stopped
	//	carrying it and nothing here had picked it up.
	static let
	operators = [
		"!", "#", "$", "'", "%", "*", "/", ";", "?", "`", "~", "<", ">"
	,	"¦", "§", "¬", "¶", "·", "¿", "∈", "∋", "∥", "£", "¤", "¡"
	,	"⊂", "⊃", "∩", "∪", "⊤", "⊥", "∅", "«", "»", "⟨", "⟩", "±"
	,	"&", "|", "^", "[", "]", "{", "}", ",", "\"", "\\", "∞", "𝑒"
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

	//	RUN runs the whole of what is written — the program, or the history.
	let
	run			: () -> Void

	//	⏎ finishes a line, which in the calculator means accepting it and in
	//	programming means starting another.
	let
	enter		: () -> Void

	//	The operators the block did not take.  It is written as a subtraction rather
	//	than as a second list because the two lists together are a promise — every
	//	readable character exactly once — and two hand-written lists are how that
	//	promise was broken before, with √ and ‹ on one side and nothing on the
	//	other.  A host that moves % onto its block drops it from here by saying so.
	private static let
	operators = SliPKeys.operators.filter {
		!Set( SliPKeys.fixed.flatMap { $0 } ).contains( $0 )
	}

	//	The height this has been given, when it has been given one.  Beside the
	//	display the keypad is a column and has to fit what it was handed — an
	//	iPhone 14 turned sideways is 390 points tall, and four rows of block under
	//	four rows of alphabet under a tab strip wants nearer 370 of them once the
	//	toolbar has taken its share, so the last two rows were simply cut off.
	//	Under the display it is a strip and takes the height it needs.
	var
	fit			: CGFloat?

	//	Whether there is width to spare.  The caller knows, because it knows the
	//	shape of the space it is handing over: a strip along the bottom of an iPad
	//	has width and nothing else, a column beside the display has height and
	//	nothing else, and a size class cannot tell those two apart.
	var
	wide		= false

	//	iOS only, and it starts on the letters for the same reason the Tab5 does:
	//	a name is the thing you cannot get at any other way.
	@State private var
	section		= KeypadSection.latin

	var
	body: some View {
		#if os(macOS)
		Sidebar
		#else
		if wide { Beside } else { Tabbed }
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
				Grid( SliPKeys.rows( Self.operators, 8 ), height: 22, size: 11 )
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
			//	Pushed to the bottom of the column, where the hand is.  Nothing to
			//	push against in a strip, so this costs nothing there.
			if fit != nil { Spacer( minLength: 0 ) }
			Tabs
			Chosen
			Divider()
			Block
		}
		.padding( 6 )
	}

	//	How tall a key can be.  Given a height to fit, the eight rows and the tab
	//	strip divide it; given none, these are the sizes the panels were drawn at.
	//	The block's keys stay the taller of the two either way — they are the ones
	//	reached for without looking.
	private static let	TABS_H	= CGFloat( 34 )
	private static let	RATIO	= CGFloat( 42 ) / 32

	private var
	sectionH: CGFloat {
		guard let fit else { return 32 }
		let	rest = fit - 12 - 12 - 1 - Self.TABS_H
		guard rest > 0 else { return 22 }
		return min( max( rest / ( 4 + 4 * Self.RATIO ), 20 ), 44 )
	}

	private var
	blockH: CGFloat {
		fit == nil ? 42 : min( max( sectionH * Self.RATIO, 26 ), 56 )
	}

	//	The same two halves, side by side.  The block is on the right, which is the
	//	side the keypad is on everywhere it is a column — so the digits are under
	//	the same hand whichever way the iPad is held.  It keeps the width it would
	//	have had on a phone rather than growing with the screen: it is the part
	//	that does not move, and a digit that changes size when the iPad is turned
	//	is a digit you have to look at before pressing.  The alphabets take the
	//	rest, which is where the room was wanted.
	private var
	Beside: some View {
		HStack( alignment: .top, spacing: 12 ) {
			VStack( spacing: 4 ) {
				Tabs
				Chosen
			}
			//	Drawn as an overlay rather than as a Divider between the two.  A
			//	Divider in an HStack asks for all the height there is, and the
			//	keypad would take half the iPad to show two hundred points of keys.
			.overlay( alignment: .trailing ) {
				Rectangle()
					.frame( width: 0.5 )
					.offset( x: 6 )
					.foregroundStyle( .separator )
			}
			Block
				.frame( width: 380 )
		}
		.padding( 6 )
	}

	private var
	Tabs: some View {
		Picker( "", selection: $section ) {
			ForEach( KeypadSection.allCases ) { Text( $0.rawValue ).tag( $0 ) }
		}
		.pickerStyle( .segmented )
		.labelsHidden()
	}

	//	Named for what it is rather than Section, which SwiftUI has taken.
	@ViewBuilder private var
	Chosen: some View {
		let	h = sectionH
		switch section {
		case .operators:	Grid( SliPKeys.rows( Self.operators, 12 ), height: h, size: h * 0.41 )
		case .functions:	Grid( SliPKeys.rows( SliPKeys.functions, 6 ), height: h, size: h * 0.38 )
		case .latin:		Grid( SliPKeys.rows( SliPKeys.latin, 13 ), height: h, size: h * 0.44 )
		case .greek:		Grid( SliPKeys.rows( SliPKeys.greek, 12 ), height: h, size: h * 0.44 )
		}
	}

	private var
	Block: some View {
		Grid( SliPKeys.fixed, height: blockH, size: blockH * 0.45 )
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
	//
	//	The height is exact rather than a minimum.  A minimum made every key
	//	willing to grow, so the keypad was a second greedy view under a greedy
	//	transcript and the two split the screen between them: on the iPad the block
	//	took half of it and spread four rows down the whole of that.
	private func
	Grid( _ rows: [ [ String ] ], height: CGFloat, size: CGFloat ) -> some View {
		let	columns = rows.map( \.count ).max() ?? 1
		return VStack( spacing: 3 ) {
			ForEach( rows, id: \.self ) { row in
				HStack( spacing: 3 ) {
					ForEach( row, id: \.self ) { Key( $0, height: height, size: size ) }
					ForEach( row.count ..< columns, id: \.self ) { _ in
						Color.clear.frame( maxWidth: .infinity, minHeight: height, maxHeight: height )
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
			Text( key )
				.font( .system( size: size, design: .monospaced ) )
				.fontWeight( key == KEY_RETURN ? .bold : .regular )
				.lineLimit( 1 )
				.minimumScaleFactor( 0.5 )
				.frame( maxWidth: .infinity, minHeight: height, maxHeight: height )
				.background(
					RoundedRectangle( cornerRadius: 5, style: .continuous )
						.fill( Color.gray.opacity( 0.18 ) )
				)
				.contentShape( Rectangle() )
		}
		.buttonStyle( .plain )
		.accessibilityLabel( key )
	}


	private func
	Press( _ key: String ) {
		switch key {
		case KEY_SPACE:
			proxy.insert( " " )
		case KEY_DELETE:
			proxy.backspace()
		case KEY_RUN:
			run()
		case KEY_RETURN:
			program ? proxy.insert( "\n" ) : enter()
		default:
			//	A function is a name, and a name wants air around it: `sin π`
			//	reads, `sinπ` is one name that does not exist.
			proxy.insert( key.count > 1 ? key + " " : key )
		}
	}
}
