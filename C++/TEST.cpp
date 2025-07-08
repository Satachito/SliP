#undef	DEBUG
#define	DEBUG

#include "SliP.hpp"

#include <cassert>

#include "ReadTest.hpp"

template< typename T, typename F > void
TestEval( SP< Context > C, string const& _, F f ) {
	auto
	$ = Cast< T >( Eval( C, READ( _ ) ) );
//	cerr << $->REPR() << endl;
	A( $ != 0 && f( $ ) );
}

void
_( SP< Context > C ) {
	auto
	slip = READ( "( 3 × 5 )" );
	slip = Eval( C, slip );
	cerr << slip->REPR() << endl;
}

void
EvalTest( SP< Context > C ) {
	TestEval< Bits >( C, "( 1+ +1 )", []( auto const& _ ){ return _->$ == 2; } );
	TestEval< Bits >( C, "( 1+ -1 )", []( auto const& _ ){ return _->$ == 0; } );
	TestEval< Bits >( C, "( 1- +1 )", []( auto const& _ ){ return _->$ == 0; } );
	TestEval< Bits >( C, "( 1- -1 )", []( auto const& _ ){ return _->$ == 2; } );
	TestEval< Bits >( C, "( 1 )", []( auto const& _ ){ return _->$ == 1; } );
	TestEval< Bits >( C, "( +1 )", []( auto const& _ ){ return _->$ == 1; } );
	TestEval< Bits >( C, "( -1 )", []( auto const& _ ){ return _->$ == -1; } );
	TestEval< Bits >( C, "( 1+1 )", []( auto const& _ ){ return _->$ == 2; } );
	TestEval< Bits >( C, "( 1-1 )", []( auto const& _ ){ return _->$ == 0; } );

	TestEval< Float >( C, "( 3 ÷ 5 )", []( auto const& _ ){ return _->$ == 0.6; } );

	TestEval< Bits >( C, "( 3 + 5 )", []( auto const& _ ){ return _->$ == 8; } );
	TestEval< Bits >( C, "( 3 × 5 )", []( auto const& _ ){ return _->$ == 15; } );
	TestEval< Bits >( C, "( 3 - 5 )", []( auto const& _ ){ return _->$ == -2; } );
	TestEval< Bits >( C, "( 7 / 3 )", []( auto const& _ ){ return _->$ == 2; } );
	TestEval< Bits >( C, "( 7 % 3 )", []( auto const& _ ){ return _->$ == 1; } );
	TestEval< Bits >( C, "( 12 ^ 10 )", []( auto const& _ ){ return _->$ == 6; } );
	TestEval< Bits >( C, "( 12 | 10 )", []( auto const& _ ){ return _->$ == 14; } );
	TestEval< Bits >( C, "( 12 & 10 )", []( auto const& _ ){ return _->$ == 8; } );
	TestEval< SliP >( C, "( 0 < 1 )", []( auto const& _ ){ return IsT( _ ); } );
	TestEval< SliP >( C, "( 0 < 0 )", []( auto const& _ ){ return IsNil( _ ); } );
	TestEval< SliP >( C, "( 0 <= 0 )", []( auto const& _ ){ return IsT( _ ); } );
	TestEval< SliP >( C, "( 1 <= 0 )", []( auto const& _ ){ return IsNil( _ ); } );
	TestEval< SliP >( C, "( 1 > 0 )", []( auto const& _ ){ return IsT( _ ); } );
	TestEval< SliP >( C, "( 0 > 0 )", []( auto const& _ ){ return IsNil( _ ); } );
	TestEval< SliP >( C, "( 0 >= 0 )", []( auto const& _ ){ return IsT( _ ); } );
	TestEval< SliP >( C, "( 0 >= 1 )", []( auto const& _ ){ return IsNil( _ ); } );
}

int
main( int argc, char* argv[] ) {
//	freopen( "TEST.slip", "r", stdin );
	try {
		BuildUp();
		ReadTest();
		auto
		C = MS< Context >();
		EvalTest( C );
	} catch ( const exception& e ) {
		cerr << e.what() << endl;
	}
}


/*
OperatorChars = {
	U'!'
,	U'#'
,	U'$'
,	U'%'
,	U'&'
,	U'*'
,	U'+'
,	U','
,	U'-'
,	U'.'
,	U'/'
,	U':'
,	U';'
,	U'<'
,	U'='
,	U'>'
,	U'?'
,	U'@'
,	U'^'
,	U'`'
,	U'|'
,	U'~'
,	U'¡'
,	U'¤'
,	U'¦'
,	U'§'
,	U'¬'
,	U'±'
,	U'¶'
,	U'·'
,	U'¿'
,	U'×'
,	U'÷'
,	U'∈'
,	U'∋'
,	U'⊂'
,	U'⊃'
,	U'∩'
,	U'∪'
};
*/
