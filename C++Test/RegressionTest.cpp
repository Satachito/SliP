//	Regression tests for Phase 1–2 fixes (symbols, truthy REPR, sugar, errors).

#include "../C++/SliP.hpp"

extern SP< SliP >
Eval( SP< Context >, SP< SliP > );

extern SP< SliP >
Read( iReader&, char32_t );

extern V< SP< SliP > >
ReadList( iReader&, char32_t );

extern bool	IsNil( SP< SliP > );

static auto
READ( const string& _ ) {
	StringReader R( _ );
	return Read( R, -1 );
};

template< typename T, typename F > void
TestEval( SP< Context > C, string const& _, F f ) {
	f( Cast< T >( Eval( C, READ( _ ) ) ) );
}

static void
TestEvalException( SP< Context > C, string const& _, string const& expected ) {
	try {
		Eval( C, READ( _ ) );
		A( false );
	} catch( exception const& e ) {
		A( e.what() == expected );
	}
}

static vector< string >
SugaredLines( string const& source, SP< Context > C ) {
	return ranges::to< vector >(
		project(
			Split( source )
		,	[ & ]( string const& line ) -> string {
				if( line.empty() ) return string();
				StringReader R( line + ')' );
				return Eval( C, MS< Sentence >( ReadList( R, U')' ) ) )->REPR();
			}
		)
	);
}

void
RegressionTest( SP< Context > C ) {

	//	Phase 2: truthy prints as T
	TestEval< Verum >( C, "( 3 == 3 )", []( auto const& _ ){ A( _->REPR() == "T" ); } );
	TestEval< List >( C, "( 3 == 4 )", []( auto const& _ ){ A( IsNil( _ ) ); } );

	//	Phase 1: ¶ = context Dict, ¤ = random 0..1 (not each other)
	TestEval< Dict >( MS< Context >(), "¶", []( auto const& _ ){ A( _->REPR() == "{}" ); } );
	TestEval< Float >( C, "( ¤ )", []( auto const& _ ){
		A( 0 <= _->$ && _->$ < 1 );
	} );

	//	Phase 1: 𝑒 constant; ASCII e is a name
	TestEval< NumericConstant >( C, "𝑒", []( auto const& _ ){
		A( _->Double() == numbers::e );
	} );
	TestEval< NumericConstant >( C, "euler", []( auto const& _ ){
		A( _->Double() == numbers::e );
	} );
	TestEval< NumericConstant >( C, "pi", []( auto const& _ ){
		A( _->Double() == numbers::pi );
	} );
	TestEval< NumericConstant >( C, "inf", []( auto const& _ ){
		A( isinf( _->Double() ) );
	} );
	TestEvalException( C, "e", "Undefined name: e" );

	//	Phase 1: . operand detail in errors
	TestEvalException( C, "( 3 . 3 )", "Illegal operand combination: 3 . 3" );

	//	Sugared calculator lines (implicit open paren per line)
	{	auto fresh = MS< Context >();
		auto $ = SugaredLines( "2+3\n2π", fresh );
		A( $.size() == 2 );
		A( $[ 0 ] == "5" );
		Eval( fresh, READ( "( 'r = 2 )" ) );
		$ = SugaredLines( "2πr", fresh );
		A( $.size() == 1 );
		A( $[ 0 ] != "2πr" );
	}

	//	random[ lo hi ] vs ¤
	TestEval< Float >( C, "( random[ 0 1 ] )", []( auto const& _ ){
		A( 0 <= _->$ && _->$ < 1 );
	} );

	//	Phase 3: comparisons bind tighter than && || ^^
	TestEval< Verum >( C, "( 3 > 1 && 5 > 4 )", []( auto const& _ ){ A( _ != nullptr ); } );
	TestEval< List >( C, "( 1 < 2 && 3 < 2 )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< Verum >( C, "( 1 == 1 || 2 == 3 )", []( auto const& _ ){ A( _ != nullptr ); } );
	TestEval< Verum >( C, "( 1 == 1 ^^ 2 == 3 )", []( auto const& _ ){ A( _ != nullptr ); } );

	//	Phase 3: && || ¿ short-circuit — falsy side never evaluates rhs
	TestEval< List >( C, "( [] && ( ¡ `boom` ) )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< Verum >( C, "( 1 || ( ¡ `boom` ) )", []( auto const& _ ){ A( _ != nullptr ); } );
	TestEval< List >( C, "( [] ¿ ( ¡ `boom` ) )", []( auto const& _ ){ A( IsNil( _ ) ); } );
	TestEval< Bits >( C, "( 1 ¿ ( 3 + 5 ) )", []( auto const& _ ){ A( _->$ == 8 ); } );
	TestEval< Bits >( C, "( 1 ¿ '( 3 + 5 ) )", []( auto const& _ ){ A( _->$ == 8 ); } );

	//	Phase 3: `,` and `=` are right-associative
	TestEval< List >( C, "( 1 , 2 , [ 3 ] )", []( auto const& _ ){ A( _->REPR() == "[ 1 2 3 ]" ); } );
	{	auto fresh = MS< Context >();
		TestEval< Bits >( fresh, "( 'p = 'q = 7 )", []( auto const& _ ){ A( _->$ == 7 ); } );
		TestEval< Bits >( fresh, "p", []( auto const& _ ){ A( _->$ == 7 ); } );
		TestEval< Bits >( fresh, "q", []( auto const& _ ){ A( _->$ == 7 ); } );
	}

	//	Phase 3: a prefix absorbs a following run of bare numerics as one product,
	//	but a parenthesized argument stays a plain call
	TestEval< Float >( C, "( sin 2 π )", []( auto const& _ ){ A( _->$ == sin( 2 * numbers::pi ) ); } );
	TestEval< Float >( C, "( cos 2 π )", []( auto const& _ ){ A( _->$ == cos( 2 * numbers::pi ) ); } );
	TestEval< Float >( C, "( sin(1) π )", []( auto const& _ ){ A( _->$ == sin( 1 ) * numbers::pi ); } );
	TestEval< Float >( C, "( 2 π sin 3 )", []( auto const& _ ){ A( _->$ == 2 * numbers::pi * sin( 3 ) ); } );
	TestEval< Float >( C, "( 6 ÷ 2 π )", []( auto const& _ ){ A( _->$ == 6 / ( 2 * numbers::pi ) ); } );
	TestEval< Float >( C, "( sin 2 π + 1 )", []( auto const& _ ){ A( _->$ == sin( 2 * numbers::pi ) + 1 ); } );
}
