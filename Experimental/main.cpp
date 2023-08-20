#include <iostream>
#include <sstream>
#include "SliP.hpp"
#include "JP.h"

using namespace std;
int
main( int argc, const char* argv[] ) {
	chdir( "/Users/s/Desktop/SliP/Samples" );
	try {
		auto _ = GetFileContent( "_.slip" );
		Interpret(
			new SliP()
		,	string( _.begin(), _.end() )
		,	[]( string const& _ ) { cout << _ << endl; }
		);
	} catch ( char const* _ ) {
		cerr << _ << endl;
	} catch ( exception const& _ ) {
        cerr << "Caught an exception: " << _.what() << endl;
	}
	return 0;
}


