#undef	DEBUG
#define	DEBUG

#include "SliP.hpp"

#include <cassert>

void
ReadTest();

void
EvalTest( SP< Context > );

extern void Build();

void
Test() {
	A( MS< SliP >()->REPR() == "SliP" );
}

int
main( int argc, char* argv[] ) {
//	freopen( "TEST.slip", "r", stdin );
	try {
		Build();
		
		auto
		C = MS< Context >();
		EvalTest( C );

		Test();

		ReadTest();

		_Z( "TESTING ENDS" );
	} catch ( const exception& e ) {
		cerr << e.what() << endl;
	}
}
