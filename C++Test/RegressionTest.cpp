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
}
