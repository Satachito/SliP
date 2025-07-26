#include "SliP.hpp"

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
	A( $ == READ( $ )->REPR() );
}

static auto
TestReadException( string const& _, string const& expected ) {
	try {
		READ( _ );
	} catch( exception const& e ) {
		A( e.what() == expected );
	}
}

void
ReadTest() {
	
	TestReadException( "⟨@⟩", "All elements of the matrix must be numeric." );
	{	auto $ = Cast< Matrix >( READ( "⟨1 2 3 4 5 6⟩" ) );
		A( $->NCols() == 0 );
		A( $->NRows() == 0 );
		$->nCols = 2;
		A( $->NCols() == 2 );
		A( $->NRows() == 3 );
		$->nCols = -3;
		A( $->NCols() == 3 );
		A( $->NRows() == 2 );
		A( Cast< Numeric >( $->operator()( 1, 2 ) )->Double() == 6 );
	}
	try {
		MS< Matrix >( V< SP< SliP > >{}, numeric_limits< int64_t >::min() );
	} catch( exception const& e ) {
		A( e.what() == string( "nCols must not be numeric_limits< int64_t >::min()" ) );
	}
	A( READ( "⟨1 2 3 4 5 6⟩" )->REPR() == "TODO:MATRIX REPR()" );

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

	TestReadException( "[ 3 + = 5 ]", "Syntax error: + =" );


	TestRead( "[ -1 ]" );

	TestReadException( "]", "Detect close parenthesis" );
	TestReadException( "⟩", "Detect close parenthesis" );
	TestReadException( "}", "Detect close parenthesis" );
	TestReadException( ")", "Detect close parenthesis" );
	TestReadException( "»", "Detect close parenthesis" );
	TestReadException( "`", "Unterminated string: " );

	TestRead( "[A]" );

	TestRead( "[1.23.45]" );

	TestReadException( "!@¡", "No such operator: !@¡" );

	TestRead( "[@1||2@]" );
	TestRead( "{@1||2@}" );
	TestRead( "(@1||2@)" );
	TestRead( "«@1||2@»" );

	TestRead( "[ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩαβγδεζηθικλμνξοπρστυφχψως𝑒∞∅]" );

	extern V< SP< Primitive	> >			Primitives;
	::apply(
		Primitives
	,	[]( SP< Primitive	> const& _ ) { TestRead( _->label ); }
	);

	extern V< SP< Prefix	> >			Prefixes;
	::apply(
		Prefixes
	,	[]( SP< Prefix		> const& _ ) { TestRead( _->label ); }
	);

	extern V< SP< Unary		> >			Unaries;
	::apply(
		Unaries
	,	[]( SP< Unary		>	 const& _ ) { TestRead( _->label ); }
	);

	extern V< SP< Infix		> >			Infixes;
	::apply(
		Infixes
	,	[]( SP< Infix		> const& _ ) { TestRead( _->label ); }
	);

	extern V< SP< NumericConstant > >	NumericConstants;
	::apply(
		NumericConstants
	,	[]( SP< NumericConstant > const& _ ) { TestRead( _->$ ); }
	);
}
