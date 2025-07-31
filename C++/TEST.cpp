#undef	DEBUG
#define	DEBUG

#include "SliP.hpp"

#include <cassert>

void
ExtraTest() {

//	auto numeric = MS< Bits >( 3 );
//	auto nega = numeric->Negate();
//	A( nega->Double() == -3 );
	

	extern SP< SliP >	T;
	A( T->REPR() == "SliP" );
	extern bool _Compare( SP< SliP >, SP< SliP > );
	A( _Compare( T, T ) == 0 );

	extern SP< SliP >	Nil;
	A( Nil->REPR() == "[]" );

//	auto reader = new StringReader( "abc" );
//	iReader* i = reader;
//	assert( i->Avail() );
//	assert( i->Peek() == 'a' );
//	assert( i->Read() == 'a' );
//	delete reader;
	
//	auto um = new UM< string, string >{};
//	delete um;
//	auto us = new US< string >{};
//	delete us;
}

int
main( int argc, char* argv[] ) {

//	freopen( "TEST.slip", "r", stdin );

	try {
		extern void Build();
		Build();

		ExtraTest();
		
		void ReadTest();
		ReadTest();

		auto
		C = MS< Context >();

		void EvalTest( SP< Context > );
		EvalTest( C );

		_Z( "TESTING ENDS" );
	} catch ( const exception& e ) {
		cerr << e.what() << endl;
	}
}
