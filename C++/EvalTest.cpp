#include "SliP.hpp"

extern bool				IsNil( SP< SliP > );
extern bool				IsT( SP< SliP > );

static auto
READ( const string& _ ) {
	StringReader R( _ );
	return Read( R, -1 );
};

template< typename T, typename F > void
TestEval( SP< Context > C, string const& _, F f ) {
//cerr << _ << endl;
	auto
	$ = Cast< T >( Eval( C, READ( _ ) ) );
//cerr << $->REPR() << endl;
	A( f( $ ) );
}

static auto
TestEvalException( SP< Context > C, string const& _, string const& expected ) {
//cerr << _ << endl;
	try {
		Eval( C, READ( _ ) );
		A( false );
	} catch( exception const& e ) {
//cerr << e.what() << ':' << expected << endl;
		A( e.what() == expected );
	}
}

void
EvalTest( SP< Context > C ) {

	TestEval< SliP >( C, "( 1 == 2 )", []( auto const& _ ){ return IsNil( _ ); } );
	TestEval< SliP >( C, "( 1 <> 2 )", []( auto const& _ ){ return IsT( _ ); } );
	TestEval< SliP >( C, "( 1 < 2 )", []( auto const& _ ){ return IsT( _ ); } );
	TestEval< SliP >( C, "( 1 > 2 )", []( auto const& _ ){ return IsNil( _ ); } );
	TestEval< SliP >( C, "( 1 <= 2 )", []( auto const& _ ){ return IsT( _ ); } );
	TestEval< SliP >( C, "( 1 >= 2 )", []( auto const& _ ){ return IsNil( _ ); } );
	TestEval< Bits >( C, "( 1 | 2 )", []( auto const& _ ){ return _->$ == 3; } );
	TestEval< Bits >( C, "( 1 & 2 )", []( auto const& _ ){ return _->$ == 0; } );
	TestEval< Bits >( C, "( 1 ^ 3 )", []( auto const& _ ){ return _->$ == 2; } );

	TestEvalException( C, "( π × `a` )", "Illegal operand type: `a`" );

	TestEval< Float >(
		C
	,	"(9223372036854775807×9223372036854775807)"
	,	[]( auto const& _ ){
			cerr << _->Double() << ':' << endl;
			return _->Double() == (double)9223372036854775807 * (double)9223372036854775807;
		}
	);
	
	TestEval< Float >( C, "(π×π)", []( auto const& _ ){ return _->Double() == numbers::pi * numbers::pi; } );

	TestEvalException( C, "@"				, "Stack underflow" );

	TestEval< Bits >( C, "(3:@)"			, []( auto const& _ ){ return _->$ == 3; } );
	TestEval< List >( C, "(3:@@)"			, []( auto const& _ ){ return _->REPR() == "[ 3 ]"; } );
	{	auto _ = MS< Bits >( numeric_limits< int64_t >::min() );
		auto $ = Cast< Float >( _->Negate() );
		A( $->$ == 9223372036854775808.0 );
	}
	{	auto _ = MS< NumericConstant >( "" );
		try {
			_->Double();
		} catch ( exception const& e ) {
			A( strcmp( e.what(), "eh?" ) == 0 );
		}
	}
	TestEval< NumericConstant >( C, "∞"	, []( auto const& _ ){ return _->Double() == numeric_limits< double >::infinity(); } );
	TestEval< NumericConstant >( C, "𝑒"	, []( auto const& _ ){ return _->Double() == numbers::e; } );
	TestEval< NumericConstant >( C, "π"	, []( auto const& _ ){ return _->Double() == numbers::pi; } );
	TestEval< NumericConstant >( C, "log2e"	, []( auto const& _ ){ return _->Double() == numbers::log2e; } );
	TestEval< NumericConstant >( C, "log10e"	, []( auto const& _ ){ return _->Double() == numbers::log10e; } );
	TestEval< NumericConstant >( C, "ln2"	, []( auto const& _ ){ return _->Double() == numbers::ln2; } );
	TestEval< NumericConstant >( C, "ln10"	, []( auto const& _ ){ return _->Double() == numbers::ln10; } );
	TestEval< NumericConstant >( C, "γ"	, []( auto const& _ ){ return _->Double() == numbers::egamma; } );
	TestEval< NumericConstant >( C, "φ"	, []( auto const& _ ){ return _->Double() == numbers::phi; } );
	TestEval< NumericConstant >( C, "(-π)"	, []( auto const& _ ){ return _->REPR() == "(-π)"; } );
	TestEval< Bits >( C, "(-9223372036854775807)"	, []( auto const& _ ){ return _->$ == -9223372036854775807L;	} );
	TestEval< Bits >( C, "9223372036854775807"		, []( auto const& _ ){ return _->$ == 9223372036854775807L;		} );
	TestEval< Float >( C, "9223372036854775808"		, []( auto const& _ ){ return _->$ == 9223372036854775808.0;	} );
	TestEval< Float >(C,  "(-9223372036854775808)"	, []( auto const& _ ){ return _->$ == -9223372036854775808.0;	} );

	TestEval< Float >( C, "(+3.0)"			, []( auto const& _ ){ return _->$ == 3; } );
	TestEval< Float >( C, "(-3.0)"			, []( auto const& _ ){ return _->$ == -3; } );

	TestEval< Bits >( C, "( 3 × 4 + 2 )"	, []( auto const& _ ){ return _->$ == 14; } );

	TestEvalException( C, "a"				, "Undefined name: a" );
	Eval( C, READ( "('a=3)" ) );
	Eval( C, READ( "('b=4)" ) );
	TestEval< Bits >( C, "a"				, []( auto const& _ ){ return _->$ == 3; } );
	TestEval< Bits >( C, "(a)"				, []( auto const& _ ){ return _->$ == 3; } );
	TestEval< Bits >( C, "(-a)"				, []( auto const& _ ){ return _->$ == -3; } );
	TestEval< Dict >( C, "¤"				, []( auto const& _ ){ return _->REPR() == "{\ta: 3\n,\tb: 4\n}"; } );

	TestEval< List >( C, "∅"					, []( auto const& _ ){ return IsNil( _ ); } );
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

