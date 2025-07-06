#undef	DEBUG
#define	DEBUG

#include "SliP.hpp"

#include <cassert>

auto
READ( const string& _ ) {
	StringReader R( _ );
	return Read( R, -1 );
};

auto
TestRead( string const& _ ) {
	auto
	$ = READ( _ )->REPR();
	A( $ == READ( $ )->REPR() );
}

auto
TestException( string const& _, string const& expected ) {
	try {
		READ( _ );
	} catch( exception const& e ) {
		A( e.what() == expected );
	}
}

auto
TestReads() {

	TestException( "]", "Detect close parenthesis" );
	TestException( "⟩", "Detect close parenthesis" );
	TestException( "}", "Detect close parenthesis" );
	TestException( ")", "Detect close parenthesis" );
	TestException( "»", "Detect close parenthesis" );

	TestRead( "[A]" );

	TestRead( "[1.23.45]" );

	TestException( "!@¡", "No such operator: !@¡" );

	TestRead( "[@1||2@]" );
	TestRead( "{@1||2@}" );
	TestRead( "(@1||2@)" );
	TestRead( "«@1||2@»" );

	TestRead( "[ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩαβγδεζηθικλμνξοπρστυφχψως𝑒∞∅]" );

	for(
		auto const& _: vector< string >{
			"3.14"
		,	"123456789"
		,	"[1 2 3 4 5]"
		,	"ABCDEFG"
		,	"\"ABCDEFG\""
		}
	) TestRead( _ );

	for(
		auto const& _: project(
			Functions
		,	[]( SP< Function > const& _ ){ return _->label; }
		)
	) TestRead( _ );

	for(
		auto const& _: project(
			NumericConstants
		,	[]( SP< NumericConstant > const& _ ){ return _->$; }
		)
	) TestRead( _ );

	for(
		auto const& _: vector< string >{
			"∞"
		,	"𝑒"
		,	"π"
		,	"γ"
		,	"φ"
		,	"log2e"
		,	"log10e"
		,	"ln2"
		,	"ln10"
		}
	) TestRead( _ );
}

int
main( int argc, char* argv[] ) {
	BuildUp();
	TestReads();
}

