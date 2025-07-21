#include "SliP.hpp"

extern bool				IsNil( SP< SliP > );

static auto
READ( const string& _ ) {
	StringReader R( _ );
	return Read( R, -1 );
};

template< typename T, typename F > void
TestEval( SP< Context > C, string const& _, F f ) {
	auto
	$ = Cast< T >( Eval( C, READ( _ ) ) );
//	cerr << $->REPR() << endl;
	A( f( $ ) );
}

static auto
TestEvalException( SP< Context > C, string const& _, string const& expected ) {
	try {
		Eval( C, READ( _ ) );
		A( false );
	} catch( exception const& e ) {
//		cerr << e.what() << ':' << expected << endl;
		A( e.what() == expected );
	}
}

void
EvalTest( SP< Context > C ) {
	TestEval< Bits >( C, "( 3 × 4 + 2 )"	, []( auto const& _ ){ return _->$ == 14; } );

	TestEvalException( C, "a"				, "Undefined name: a" );
	Eval( C, READ( "('a=3)" ) );
	TestEval< Bits >( C, "a"				, []( auto const& _ ){ return _->$ == 3; } );
	TestEval< Bits >( C, "(a)"				, []( auto const& _ ){ return _->$ == 3; } );
	TestEval< Bits >( C, "(-a)"				, []( auto const& _ ){ return _->$ == -3; } );
	TestEval< Dict >( C, "¤"				, []( auto const& _ ){ return _->REPR() == "{\ta: 3\n}"; } );

	Eval( C, READ( "(3:;)" ) );
//	TestEval< Bits >( C, "(3:@)"			, []( auto const& _ ){ return _->$ == 3; } );
//	TestEval< List >( C, "(3:@@)"			, []( auto const& _ ){ return _->REPR() == "[ 3 3 ]"; } );
	TestEval< List >( C, "∅"				, []( auto const& _ ){ return IsNil( _ ); } );
	TestEval< List >( C, "«0»"				, []( auto const& _ ){ return Cast< Bits >( _->$[ 0 ] )->$ == 0; } );
	TestEval< List >( C, "{0}"				, []( auto const& _ ){ return Cast< Bits >( _->$[ 0 ] )->$ == 0; } );

	TestEval< Bits >( C, "( 3 2 )"			, []( auto const& _ ){ return _->$ == 6; } );
	TestEval< Bits >( C, "( 2 + 3 × 4 )"	, []( auto const& _ ){ return _->$ == 14; } );
	
	TestEvalException( C, "(`a` `b`)"		, "Syntax Error: No numeric value: `a`" );
	TestEvalException( C, "(')"				, "Syntax Error: No operand for prefix: '" );
	TestEvalException( C, "(×3)"			, "Syntax Error: No left operand for infix operator: ×" );
	TestEvalException( C, "(3×)"			, "Syntax Error: No right operand for infix operator: ×" );

	TestEval< Bits >( C, "(+3)"				, []( auto const& _ ){ return _->$ == 3; } );
	TestEval< Bits >( C, "(-3)"				, []( auto const& _ ){ return _->$ == -3; } );

	TestEval< Float >( C, "( 1.2 - 1.3 )"	, []( auto const& _ ){ return _->REPR() == "-0.100000"; } );

	TestEval< Bits >( C, "( 1+ +1 )"		, []( auto const& _ ){ return _->$ == 2; } );
	TestEval< Bits >( C, "( 1+ -1 )"		, []( auto const& _ ){ return _->$ == 0; } );
	TestEval< Bits >( C, "( 1- +1 )"		, []( auto const& _ ){ return _->$ == 0; } );
	TestEval< Bits >( C, "( 1- -1 )"		, []( auto const& _ ){ return _->$ == 2; } );
	TestEval< Bits >( C, "( 1 )"			, []( auto const& _ ){ return _->$ == 1; } );
	TestEval< Bits >( C, "( +1 )"			, []( auto const& _ ){ return _->$ == 1; } );
	TestEval< Bits >( C, "( -1 )"			, []( auto const& _ ){ return _->$ == -1; } );
	TestEval< Bits >( C, "( 1+1 )"			, []( auto const& _ ){ return _->$ == 2; } );
	TestEval< Bits >( C, "( 1-1 )"			, []( auto const& _ ){ return _->$ == 0; } );

	TestEval< Float >( C, "( 3 ÷ 5 )"		, []( auto const& _ ){ return _->$ == 0.6; } );

	TestEval< Bits >( C, "( 3 + 5 )"		, []( auto const& _ ){ return _->$ == 8; } );
	TestEval< Bits >( C, "( 3 × 5 )"		, []( auto const& _ ){ return _->$ == 15; } );
	TestEval< Bits >( C, "( 3 - 5 )"		, []( auto const& _ ){ return _->$ == -2; } );
	TestEval< Bits >( C, "( 7 / 3 )"		, []( auto const& _ ){ return _->$ == 2; } );
	TestEval< Bits >( C, "( 7 % 3 )"		, []( auto const& _ ){ return _->$ == 1; } );
	TestEval< Bits >( C, "( 12 ^ 10 )"		, []( auto const& _ ){ return _->$ == 6; } );
	TestEval< Bits >( C, "( 12 | 10 )"		, []( auto const& _ ){ return _->$ == 14; } );
	TestEval< Bits >( C, "( 12 & 10 )"		, []( auto const& _ ){ return _->$ == 8; } );
	TestEval< SliP >( C, "( 0 < 1 )"		, []( auto const& _ ){ return !IsNil( _ ); } );
	TestEval< SliP >( C, "( 0 < 0 )"		, []( auto const& _ ){ return IsNil( _ ); } );
	TestEval< SliP >( C, "( 0 <= 0 )"		, []( auto const& _ ){ return !IsNil( _ ); } );
	TestEval< SliP >( C, "( 1 <= 0 )"		, []( auto const& _ ){ return IsNil( _ ); } );
	TestEval< SliP >( C, "( 1 > 0 )"		, []( auto const& _ ){ return !IsNil( _ ); } );
	TestEval< SliP >( C, "( 0 > 0 )"		, []( auto const& _ ){ return IsNil( _ ); } );
	TestEval< SliP >( C, "( 0 >= 0 )"		, []( auto const& _ ){ return !IsNil( _ ); } );
	TestEval< SliP >( C, "( 0 >= 1 )"		, []( auto const& _ ){ return IsNil( _ ); } );

	TestEval< Bits >( C, "( 0 )"			, []( auto const& _ ){ return _->$ == 0; } );
	TestEval< List >( C, "()"				, []( auto const& _ ){ return IsNil( _ ); } );
}


//	Eval name 'a = 3, a

