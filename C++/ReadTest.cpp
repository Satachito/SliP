#include "SliP.hpp"

static auto
READ( const string& _ ) {
	StringReader R( _ );
	return Read( R, -1 );
};

static auto
TestRead( string const& _ ) {
	auto
	$ = READ( _ )->REPR();
//	cerr << $ << endl;
	A( $ == READ( $ )->REPR() );
}

template < ranges::range R > void
TestReads( R const& _ ) {
    ::apply( _, []( auto const& _ ) { TestRead( _ ); } );
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

	TestRead( R"("	")" );
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

	A( READ( "" ) == nullptr );

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
	TestRead( "⟨@1||2@⟩" );

	TestRead( "[ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩαβγδεζηθικλμνξοπρστυφχψως𝑒∞∅]" );

	extern V< SP< Function > >			Functions;
	TestReads(
		project(
			Functions
		,	[]( SP< Function > const& _ ){ return _->label; }
		)
	);

	extern V< SP< NumericConstant > >	NumericConstants;
	TestReads(
		project(
			NumericConstants
		,	[]( SP< NumericConstant > const& _ ){ return _->$; }
		)
	);
}
