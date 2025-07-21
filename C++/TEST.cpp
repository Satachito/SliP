#undef	DEBUG
#define	DEBUG

#include "SliP.hpp"

#include <cassert>

void
ReadTest();

void
EvalTest( SP< Context > );

extern void Build();

int
main( int argc, char* argv[] ) {
//	freopen( "TEST.slip", "r", stdin );
	try {
		Build();
		

		ReadTest();
		auto
		C = MS< Context >();
		EvalTest( C );
		_Z( "TESTING ENDS" );
	} catch ( const exception& e ) {
		cerr << e.what() << endl;
	}
}
