#include "SliP.hpp"

extern SP< SliP >
Read( SP< Context >, iReader&, char32_t );

static auto
READ( SP< Context > C, const string& _ ) {
//	cerr << _ << endl;
	StringReader R( _ );
	return Read( C, R, -1 );
};

static auto
TestRead( SP< Context > C, string const& _ ) {
	auto
	$ = READ( C, _ )->REPR();
	A( $ == READ( C, $ )->REPR() );
}

static auto
TestReadException( SP< Context > C, string const& _, string const& expected ) {
	try {
		READ( C, _ );
	} catch( exception const& e ) {
		A( e.what() == expected );
	}
}

void
ReadTest( SP< Context > C ) {
	
	TestReadException( C, "⟨@⟩", "All elements of the matrix must be numeric." );
	{	auto $ = Cast< Matrix >( READ( C, "⟨1 2 3 4 5 6⟩" ) );
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
	}
	A( READ( C, "⟨1 2 3 4 5 6⟩" )->REPR() == "⟨ 1.000000 2.000000 3.000000 4.000000 5.000000 6.000000 ⟩" );

	A( READ( C, " [ ] " )->REPR() == "[]" );
	A( READ( C, "" ) == nullptr );

	A( READ( C, "\"\\\\\"" )->REPR() == "\"\\\"" );
	TestRead( C, R"("\a")" );
	TestRead( C, R"("\0")" );
	TestRead( C, R"("\f")" );
	TestRead( C, R"("\n")" );
	TestRead( C, R"("\r")" );
	TestRead( C, R"("\t")" );
	TestRead( C, R"("\v")" );
//	TestRead( C, R"("\\\\")" );
	TestRead( C, "1" );
	A( Cast< Literal >( READ( C, "\"A\\0\"" ) )->$[ 1 ] == 0 );
	A( Cast< Literal >( READ( C, "\"A\\f\"" ) )->$[ 1 ] == U'\f' );
	A( Cast< Literal >( READ( C, "\"A\\n\"" ) )->$[ 1 ] == U'\n' );
	A( Cast< Literal >( READ( C, "\"A\\r\"" ) )->$[ 1 ] == U'\r' );
	A( Cast< Literal >( READ( C, "\"A\\t\"" ) )->$[ 1 ] == U'\t' );
	A( Cast< Literal >( READ( C, "\"A\\v\"" ) )->$[ 1 ] == U'\v' );
	A( Cast< Literal >( READ( C, "\"A\\v\"" ) )->$[ 1 ] == U'\v' );
	A( Cast< Literal >( READ( C, "\"A\\X\"" ) )->$[ 1 ] == U'X' );

	TestReadException( C, "\\", "Invalid escape" );

	TestRead( C, "[=]" );
	TestRead( C, "[A=]" );
	TestRead( C, "[AΩ]" );

	TestRead( C, "[ - -1 ]" );
	TestRead( C, "[ + -1 ]" );
	TestRead( C, "[ - +1 ]" );
	TestRead( C, "[ + +1 ]" );

	A( READ( C, "\"A\\\\B\"" )->REPR() == "\"A\\B\"" );

	TestReadException( C, "[ 3 + = 5 ]", "Syntax error: + =" );


	TestRead( C, "[ -1 ]" );

	TestReadException( C, "]", "Detect close parenthesis" );
	TestReadException( C, "⟩", "Detect close parenthesis" );
	TestReadException( C, "}", "Detect close parenthesis" );
	TestReadException( C, ")", "Detect close parenthesis" );
	TestReadException( C, "»", "Detect close parenthesis" );
	TestReadException( C, "`", "Unterminated string: " );

	TestRead( C, "[A]" );

	TestRead( C, "[1.23.45]" );

	TestReadException( C, "!@¡", "No such operator: !@¡" );

	TestRead( C, "[@1||2@]" );
	TestRead( C, "{@1||2@}" );
	TestRead( C, "(@1||2@)" );
	TestRead( C, "«@1||2@»" );

	TestRead( C, "[ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩαβγδεζηθικλμνξοπρστυφχψως𝑒∞∅]" );

	for( auto const& [ k, v ]: C->$ ) TestRead( C, k );
}
