#include	"SliP.hpp"

#include <cassert>

auto
READ( const string& _ ) {
	StringReader R( _ );
	return Read( R, -1 )->REPR();
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
			auto $ = READ( _ );
			cerr << $ << endl;
			assert( $ == READ( $ ) );
		}
	);
}

int
main( int argc, char* argv[] ) {
	TestReadPrint();
}

