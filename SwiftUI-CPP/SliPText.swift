import Foundation

//	The two shapes the same work can be written in, and how to get from either to
//	the other.
//
//	Calculator mode is a line at a time, and the parentheses around that line are
//	the reader's business rather than the writer's: `2πr` is `( 2πr )`.
//	Programming mode is toplevel forms, where the parentheses are written down and
//	one form may take as many lines as it wants.
//
//	Changing mode should not throw the work away, so each direction rewrites what
//	is there into what the other mode would have written.  Neither direction is
//	lossless — programming mode can say things a line cannot, and flattening a
//	form loses how it was laid out — but both are what the reader meant.

enum SliPText {

	//	────────────────────────────────  which mode a file is in

	//	A file says how it is meant to be read, on its first line, in the language's
	//	own comment syntax — so the marker costs a reader nothing and costs the
	//	interpreter nothing either.  Without it a file is a calculator's history:
	//	one expression a line, which is what the calculator writes.
	static let	MARKER = "//SLIP"

	static func
	isProgram( _ text: String ) -> Bool {
		FirstLine( text ).trimmingCharacters( in: .whitespaces ) == MARKER
	}

	static func
	marked( _ text: String ) -> String {
		isProgram( text ) ? text : MARKER + "\n" + text
	}

	static func
	unmarked( _ text: String ) -> String {
		guard isProgram( text ) else { return text }
		let	lines = text.split( separator: "\n", omittingEmptySubsequences: false )
		return lines.dropFirst().joined( separator: "\n" )
	}

	private static func
	FirstLine( _ text: String ) -> String {
		String( text.split( separator: "\n", omittingEmptySubsequences: false ).first ?? "" )
	}

	private static let	OPENERS : Set< Character > = [ "(", "[", "{", "⟨", "«" ]
	private static let	CLOSERS : Set< Character > = [ ")", "]", "}", "⟩", "»" ]

	//	────────────────────────────────  calculator → programming

	//	Each line becomes the form the calculator was reading it as.  A blank line
	//	stays blank and a comment stays a comment; a line that is already one
	//	toplevel form is left alone rather than wrapped again.
	static func
	parenthesised( _ text: String ) -> String {
		text
		.split( separator: "\n", omittingEmptySubsequences: false )
		.map {
			let	line = $0.trimmingCharacters( in: .whitespaces )
			if line.isEmpty || line.hasPrefix( "//" ) { return String( $0 ) }
			if IsOneForm( line ) { return line }
			return "( " + line + " )"
		}
		.joined( separator: "\n" )
	}

	//	True when the whole of this line is a single parenthesised form, so that
	//	`( 'r = 2 )` does not come back as `( ( 'r = 2 ) )`.
	private static func
	IsOneForm( _ line: String ) -> Bool {
		let	c = Array( line )
		guard c.first == "(" else { return false }
		return Match( c, 0 ) == c.count
	}

	//	────────────────────────────────  programming → calculator

	//	Each toplevel form on a line of its own, flattened, with the parentheses
	//	the calculator would have supplied taken back off.  Anything at toplevel
	//	that is not a parenthesised form — a comment, a list, a bare expression —
	//	is kept as it stands, on one line.
	static func
	flattened( _ text: String ) -> String {
		let	c = Array( text )
		var	i = 0
		var	out: [ String ] = []

		while i < c.count {
			while i < c.count, c[ i ].isWhitespace { i += 1 }
			guard i < c.count else { break }

			//	A comment keeps its line.  Folding it into the form above would
			//	comment out whatever followed it.
			if c[ i ] == "/", i + 1 < c.count, c[ i + 1 ] == "/" {
				var	j = i
				while j < c.count, c[ j ] != "\n" { j += 1 }
				out.append( String( c[ i ..< j ] ).trimmingCharacters( in: .whitespaces ) )
				i = j
				continue
			}

			if c[ i ] == "(" {
				let	end = Match( c, i )
				let	inner = end > i + 1 ? String( c[ ( i + 1 ) ..< ( end - 1 ) ] ) : ""
				out.append( OneLine( inner ) )
				i = end
				continue
			}

			//	Everything up to the next newline that is not inside anything.
			let	end = LineEnd( c, i )
			let	chunk = OneLine( String( c[ i ..< end ] ) )
			if !chunk.isEmpty { out.append( chunk ) }
			i = end
		}

		return out.joined( separator: "\n" )
	}

	//	────────────────────────────────  the scanner

	//	From an index at an opener, the index just past its match — counting depth
	//	rather than kinds, and stepping over strings and comments, where a bracket
	//	is a character like any other.
	private static func
	Match( _ c: [ Character ], _ from: Int ) -> Int {
		var	i = from
		var	depth = 0
		while i < c.count {
			let	ch = c[ i ]
			if ch == "\"" { i = StringEnd( c, i ); continue }
			if ch == "/", i + 1 < c.count, c[ i + 1 ] == "/" {
				while i < c.count, c[ i ] != "\n" { i += 1 }
				continue
			}
			if OPENERS.contains( ch ) { depth += 1 }
			else if CLOSERS.contains( ch ) {
				depth -= 1
				if depth == 0 { return i + 1 }
			}
			i += 1
		}
		return c.count
	}

	//	The next newline that is not inside brackets or a string.
	private static func
	LineEnd( _ c: [ Character ], _ from: Int ) -> Int {
		var	i = from
		var	depth = 0
		while i < c.count {
			let	ch = c[ i ]
			if ch == "\"" { i = StringEnd( c, i ); continue }
			if ch == "/", i + 1 < c.count, c[ i + 1 ] == "/" {
				while i < c.count, c[ i ] != "\n" { i += 1 }
				continue
			}
			if ch == "\n", depth == 0 { return i }
			if OPENERS.contains( ch ) { depth += 1 }
			else if CLOSERS.contains( ch ) { depth -= 1 }
			i += 1
		}
		return c.count
	}

	//	From an index at a quote, the index just past the closing one.
	private static func
	StringEnd( _ c: [ Character ], _ from: Int ) -> Int {
		var	i = from + 1
		while i < c.count {
			if c[ i ] == "\\" { i += 2; continue }
			if c[ i ] == "\"" { return i + 1 }
			i += 1
		}
		return c.count
	}

	//	One line out of however many it was written on: runs of space become one
	//	space, and comments go — a comment folded into a line takes the rest of
	//	that line with it.
	private static func
	OneLine( _ text: String ) -> String {
		let	c = Array( text )
		var	i = 0
		var	out = ""
		var	space = false
		while i < c.count {
			let	ch = c[ i ]
			if ch == "\"" {
				let	end = StringEnd( c, i )
				if space, !out.isEmpty { out += " " }
				space = false
				out += String( c[ i ..< end ] )
				i = end
				continue
			}
			if ch == "/", i + 1 < c.count, c[ i + 1 ] == "/" {
				while i < c.count, c[ i ] != "\n" { i += 1 }
				space = true
				continue
			}
			if ch.isWhitespace { space = true; i += 1; continue }
			if space, !out.isEmpty { out += " " }
			space = false
			out.append( ch )
			i += 1
		}
		return out.trimmingCharacters( in: .whitespaces )
	}
}
