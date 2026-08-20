import Testing
@testable import SliP

//	The two mode conversions.  They are text transforms with a scanner in them,
//	which is exactly the sort of thing that looks right in one example and is
//	wrong in the next.

struct SliPTextTests {

	@Test func calculatorLinesBecomeForms() {
		#expect(
			SliPText.parenthesised( "1 + 2 × 3\ncosπ" )
			== "( 1 + 2 × 3 )\n( cosπ )"
		)
	}

	@Test func blanksAndCommentsAreLeftAlone() {
		#expect(
			SliPText.parenthesised( "// a note\n\n2πr" )
			== "// a note\n\n( 2πr )"
		)
	}

	@Test func aFormIsNotWrappedTwice() {
		#expect( SliPText.parenthesised( "( 'r = 2 )" ) == "( 'r = 2 )" )
		//	Two forms on one line is not one form, so that line does get wrapped.
		#expect( SliPText.parenthesised( "( a ) ( b )" ) == "( ( a ) ( b ) )" )
	}

	@Test func formsBecomeLines() {
		#expect(
			SliPText.flattened( "( 1 + 2 × 3 )\n( cosπ )" )
			== "1 + 2 × 3\ncosπ"
		)
	}

	@Test func aFormSpreadOverLinesBecomesOne() {
		#expect(
			SliPText.flattened( "(\n\t'fact = '(\n\t\t@ 1\n\t)\n)" )
			== "'fact = '( @ 1 )"
		)
	}

	@Test func twoFormsOnOneLineBecomeTwoLines() {
		#expect( SliPText.flattened( "( a ) ( b )" ) == "a\nb" )
	}

	@Test func whatIsNotAFormIsKept() {
		//	A list, a bare expression, a comment: all toplevel, none of them a
		//	parenthesised form, and none of them ours to unwrap.
		#expect(
			SliPText.flattened( "[ 1 2 3 ] + 10\n// a note\n( x )" )
			== "[ 1 2 3 ] + 10\n// a note\nx"
		)
	}

	@Test func bracketsAndStringsAreNotDepth() {
		//	A parenthesis inside a string is a character, and a comment inside a
		//	form would swallow the rest of that form once it is on one line.
		#expect( SliPText.flattened( "( \"a )\" )" ) == "\"a )\"" )
		#expect( SliPText.flattened( "(\n\ta\t// why\n\tb\n)" ) == "a b" )
		#expect( SliPText.flattened( "( [ 1 2 ] )" ) == "[ 1 2 ]" )
	}

	@Test func aRoundTripKeepsWhatALineCanHold() {
		let	lines = "1 + 2 × 3\ncosπ\n[ 1 2 3 ] + 10"
		#expect( SliPText.flattened( SliPText.parenthesised( lines ) ) == lines )
	}
}

//	The marker on the first line, which is how a file says which mode it is in.

struct SliPMarkerTests {

	@Test func aMarkedFileIsAProgram() {
		#expect( SliPText.isProgram( "//SLIP\n( a )" ) )
		#expect( SliPText.isProgram( "  //SLIP  \n( a )" ) )
		#expect( !SliPText.isProgram( "1 + 1\n//SLIP" ) )
		#expect( !SliPText.isProgram( "// SLIP\n( a )" ) )
		#expect( !SliPText.isProgram( "" ) )
	}

	@Test func markingIsIdempotent() {
		#expect( SliPText.marked( "( a )" ) == "//SLIP\n( a )" )
		#expect( SliPText.marked( "//SLIP\n( a )" ) == "//SLIP\n( a )" )
	}

	@Test func unmarkingTakesOnlyTheMarker() {
		#expect( SliPText.unmarked( "//SLIP\n( a )\n( b )" ) == "( a )\n( b )" )
		#expect( SliPText.unmarked( "( a )" ) == "( a )" )
		#expect( SliPText.unmarked( SliPText.marked( "x" ) ) == "x" )
	}
}
