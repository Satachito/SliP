#include "SliP.hpp"

void
Main() {
	auto			_ = string(
		istreambuf_iterator< char >( cin )
	,	istreambuf_iterator< char >()
	);
	StringReader	R( _ );
	auto			C = MS< Context >();
	while( auto _ = Read( R, -1 ) ) cout << Eval( C, _ )->REPR() << endl;
}

extern void Build();

int
main( int argc, char* argv[] ) {
	Build();
	Main();
}
