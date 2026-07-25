#include "../C++/SliP.hpp"

extern SP< SliP >
Read( iReader&, char32_t );

static auto
READ( const string& _ ) {
//	cerr << _ << endl;
	StringReader R( _ );
	return Read( R, -1 );
};

static auto
TestRead( string const& _ ) {
	auto
	$ = READ( _ )->REPR();
//	{	auto _ = READ( $ )->REPR();
//		if( $ != _ ) {
//			cerr << "_: " << _ << endl;
//			cerr << "$: " << $ << endl;
//		}
//	}
	A( $ == READ( $ )->REPR() );
}

static auto
TestReadException( string const& _, string const& expected ) {
	try {
		READ( _ );
		cerr << "--------:" << _ << endl;
		A( false );
	} catch( exception const& e ) {
//		if( e.what() != expected ) {
//			cerr << "--------:" << _ << endl;
//			cerr << "e.what():" << e.what() << endl;
//			cerr << "expected:" << expected << endl;
//		}
		A( e.what() == expected );
	}
}

//	SPEC §5.4 promises `//` line comments; the reader, not the web UI, is what
//	has to deliver them, or a commented .slip file cannot be run from the CLI.
static void
CommentTest() {
	extern SP< SliP >	Eval( SP< Context >, SP< SliP > );
	extern SP< SliP >	Nil;
	extern bool			IsNil( SP< SliP > );

	auto	C = MS< Context >();

	auto	EVAL = [ & ]( string const& _ ) {
		StringReader	R( _ );
		SP< SliP >		$ = Nil;
		while( auto form = Read( R, -1 ) ) $ = Eval( C, form );
		return $;
	};

	A( Cast< Bits >( EVAL( "// nothing but a comment\n( 1 + 1 )" ) )->$ == 2 );
	A( Cast< Bits >( EVAL( "( 1 + 1 ) // trailing" ) )->$ == 2 );
	A( Cast< Bits >( EVAL( "( 1 +\n// inside a form\n1 )" ) )->$ == 2 );
	A( Cast< Bits >( EVAL( "( 7 / 2 )" ) )->$ == 3 );			//	One / still divides
	A( Cast< Bits >( EVAL( "( 7 / 2 ) // c" ) )->$ == 3 );
	//	A // inside a string is text, not a comment
	A( Cast< Literal >( EVAL( "( `http://x` )" ) )->$ == "http://x" );
	A( IsNil( EVAL( "// only a comment" ) ) );
}

void
ReadTest() {

	CommentTest();

	
	A( READ( "(1-)" )->REPR() == "( 1 - )" );

	TestReadException( "⟨@⟩", "All elements of the matrix must be numeric." );
	{	auto $ = Cast< Matrix >( READ( "⟨1 2 3 4 5 6⟩" ) );
		{	auto [ nRows, nCols ] = $->Shape();
			A( nRows == 1 );
			A( nCols == 6 );
		}
		{	$->nCols = 2;
			auto [ nRows, nCols ] = $->Shape();
			A( nCols == 2 );
			A( nRows == 3 );
		}
		{	$->nCols = -3;
			auto [ nRows, nCols ] = $->Shape();
			A( nCols == 3 );
			A( nRows == 2 );
			A( (*$)( 1, 2 ) == 6 );
		}
		A( $->REPR() == "(⟨ 1.000000 2.000000 3.000000 4.000000 5.000000 6.000000 ⟩±-3)" );
	}
	A( READ( "⟨1 2 3 4 5 6⟩" )->REPR() == "⟨ 1.000000 2.000000 3.000000 4.000000 5.000000 6.000000 ⟩" );

	A( READ( " [ ] " )->REPR() == "[]" );
	A( READ( "" ) == nullptr );

	A( READ( "\"\\\\\"" )->REPR() == "\"\\\"" );
	TestRead( R"("\a")" );
	TestRead( R"("\0")" );
	TestRead( R"("\f")" );
	TestRead( R"("\n")" );
	TestRead( R"("\r")" );
	TestRead( R"("\t")" );
	TestRead( R"("\v")" );
//	TestRead( R"("\\\\")" );
	TestRead( "1" );
	A( Cast< Literal >( READ( "\"A\\0\"" ) )->$[ 1 ] == 0 );
	A( Cast< Literal >( READ( "\"A\\f\"" ) )->$[ 1 ] == U'\f' );
	A( Cast< Literal >( READ( "\"A\\n\"" ) )->$[ 1 ] == U'\n' );
	A( Cast< Literal >( READ( "\"A\\r\"" ) )->$[ 1 ] == U'\r' );
	A( Cast< Literal >( READ( "\"A\\t\"" ) )->$[ 1 ] == U'\t' );
	A( Cast< Literal >( READ( "\"A\\v\"" ) )->$[ 1 ] == U'\v' );
	A( Cast< Literal >( READ( "\"A\\v\"" ) )->$[ 1 ] == U'\v' );
	A( Cast< Literal >( READ( "\"A\\X\"" ) )->$[ 1 ] == U'X' );

	TestReadException( "\\", "Invalid escape" );

	TestRead( "[=]" );
	TestRead( "[A=]" );
	TestRead( "[AΩ]" );

	TestRead( "[ - -1 ]" );
	TestRead( "[ + -1 ]" );
	TestRead( "[ - +1 ]" );
	TestRead( "[ + +1 ]" );

	A( READ( "\"A\\\\B\"" )->REPR() == "\"A\\B\"" );

	TestRead( "[ -1 ]" );

	TestReadException( "]", "Detect unopened close parenthesis: ]" );
	TestReadException( "}", "Detect unopened close parenthesis: }" );
	TestReadException( ")", "Detect unopened close parenthesis: )" );
	TestReadException( "»", "Detect unopened close parenthesis: »" );
	TestReadException( "⟩", "Detect unopened close parenthesis: ⟩" );
	TestReadException( "`", "Unterminated string: " );

	TestRead( "[A]" );

	TestRead( "[1.23.45]" );

	TestRead( "!@¡" );

	TestRead( "[@1||2@]" );
	TestRead( "{@1||2@}" );
	TestRead( "(@1||2@)" );
	TestRead( "«@1||2@»" );

	TestRead( "[ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩαβγδεζηθικλμνξοπρστυφχψως𝑒∞∅]" );
	
	extern UM< string, SP< SliP > > BUILTINS;
	for( auto const& [ k, v ]: BUILTINS ) TestRead( k );
}
