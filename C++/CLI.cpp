#include "SliP.hpp"

int
main( int argc, char* argv[] ) {

	extern void Build();
	Build();

	auto			_ = string(
		istreambuf_iterator< char >( cin )
	,	istreambuf_iterator< char >()
	);
	StringReader	R( _ );
	auto			C = MS< Context >();

	SP< SliP > Read( iReader&, char32_t );
	SP< SliP > Eval( SP< Context >, SP< SliP > );
	while( auto _ = Read( R, -1 ) ) cout << Eval( C, _ )->REPR() << endl;
}
