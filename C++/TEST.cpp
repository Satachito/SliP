#include	"SliP.hpp"

#include <cassert>

auto
READ( const string& _ ) {
	StringReader R( _ );
	return Read( R, -1 );
};


auto
TestReadPrint() {
	auto $ = string()
//	Nagator
	+	"3.14\n"
	+	"123456789\n"
	+	"[1 2 3 4 5]\n"
	+	"\"ABCDEFG\"\n"
	;

	Vector< string >( Split( $ ) ).apply(
		[]( const string& _ ) {
			auto $ = READ( _ )->REPR();
			cerr << $ << endl;
			assert( $ == READ( $ )->REPR() );
		}
	);
}

auto
TestNegator() {
	auto slip = READ( string( "(-3)" ) );
	cerr << slip->REPR() << endl;
	assert( slip->REPR() == "( - 3 )" );
	auto nagator = Cast< Negator >( slip[ 0 ] );
	assert( nagator );
	auto numeric = Cast< Numeric >( nagator->$ ); 
	assert( numeric );
}
int
main( int argc, char* argv[] ) {

	TestNegator();

	TestReadPrint();
}

