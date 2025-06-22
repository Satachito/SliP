#include "SliP.hpp"

void
ReadEvalPrint( const string& _ ) {
	StringReader	R( _ );
	auto			C = MS< Context >();
	while( auto _ = Read( R, -1 ) ) cout << Eval( C, _ )->REPR() << endl;
}

void
Main() {
	ReadEvalPrint(
		string(
			istreambuf_iterator< char >( cin )
		,	istreambuf_iterator< char >()
		)
	);
}

int
main( int argc, char* argv[] ) {
	Main();
}
