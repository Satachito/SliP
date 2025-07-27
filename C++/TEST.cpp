#undef	DEBUG
#define	DEBUG

#include "SliP.hpp"

#include <cassert>

int
main( int argc, char* argv[] ) {

//	freopen( "TEST.slip", "r", stdin );

	try {
		extern SP< Context > BuiltinContext();
		auto
		C = BuiltinContext();

		void ReadTest( SP< Context > );
		ReadTest( C );

		void EvalTest( SP< Context > );
		EvalTest( C );

		_Z( "TESTING ENDS" );
	} catch ( const exception& e ) {
		cerr << e.what() << endl;
	}
}
