#include	"SliP.hpp"

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
	cerr << $ << endl;
	assert( $ == READ( $ )->REPR() );
}

auto
Test() {

	TestRead( "[1.23.45]" );

	try {
		READ( "!@¡" );
	} catch( exception const& e ) {
		cerr << e.what() << endl;
	}
	TestRead( "[@1@@2]" );
	TestRead( "[ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩαβγδεζηθικλμνξοπρστυφχψως𝑒∞]" );

	auto $ = vector< string >{
		"3.14"
	,	"123456789"
	,	"[1 2 3 4 5]"
	,	"ABCDEFG"
	,	"\"ABCDEFG\""
	,	"∞"
	,	"𝑒"
	,	"π"
	,	"γ"
	,	"φ"
	,	"log2e"
	,	"log10e"
	,	"ln2"
	,	"ln10"
	,	"@"		//	Stack top
	,	"@@"	//	Stack list
	,	"¤"		//	make Dict
	,	"'"		//	Quote
	,	"¡"		//	Throw
	,	"!"		//	Eval
	,	"~"		//	Bit not
	,	"¬"		//	Logical not
	,	"¶"		//	Convert to literal
	,	"#"		//	Number of elements
	,	"*"		//	CDR
	,	"$"		//	Last element
	,	"."		//	stdout
	,	"¦"		//	stderr
	,	"§"		//	Open new context with dict(l) then eval r
	,	"="		//	assign
	,	"?"		//	if else
	,	"¿"		//	if
	,	"&&"	//	Logical and
	,	"||"	//	Logical or
	,	"^^"	//	Logical exclusive or
	,	"∈"		//	Member of
	,	"∋"		//	Includes
	,	"=="	//	Equal
	,	"<>"	//	Not Equal
	,	"<"		//	Less than
	,	">"		//	Greater than
	,	"<="	//	Less equal
	,	">="	//	Greater equal
	,	","		//	[ l, ...r ]
	,	"&"		//	And
	,	"|"		//	Or
	,	"^"		//	Exclusive or
	,	"+"		//	Plus
	,	"-"		//	Minus
	,	"·"		//	Dot product
	,	"×"		//	Multiple
	,	"÷"		//	Div
	,	"%"		//	Remainder
	,	":"		//	Apply
	};

	for( auto const& _: $ ) TestRead( _ );
}

int
main( int argc, char* argv[] ) {
	BuildUp();
	Test();
}

