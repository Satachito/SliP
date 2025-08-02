#include "../C++/SliP.hpp"

extern SP< SliP >
Eval( SP< Context >, SP< SliP > );

extern SP< SliP >
Read( iReader&, char32_t );

static auto
READ( const string& _ ) {
	StringReader R( _ );
	return Read( R, -1 );
};

template< typename T, typename F > void
TestMath( SP< Context > C, string const& _, F f ) {
//cerr << _ << endl;
	f( Cast< T >( Eval( C, READ( _ ) ) ) );
}
void
TestMathApprox( SP< Context > C, string const& _, double $ ) {
//	{	auto l = Cast< Float >( Eval( C, READ( _ ) ) )->$;
//		if( !( abs( l - $ ) < numeric_limits<double>::epsilon() ) ) {
//			cerr << setprecision( 16 ) << l << ':' << $ << ':' << numeric_limits<double>::epsilon() << endl;
//		}
//	}
	A( abs( Cast< Float >( Eval( C, READ( _ ) ) )->$ - $ ) < numeric_limits<double>::epsilon() );
}

void
MathTest( SP< Context > C ) {
	TestMath< Float >( C, "( acos 1 )", []( auto const& _ ){ A( _->$ == acos( 1 ) ); } );
	TestMath< Float >( C, "( acosh 1 )", []( auto const& _ ){ A( _->$ == acosh( 1 ) ); } );
	TestMath< Float >( C, "( asin 1 )", []( auto const& _ ){ A( _->$ == asin( 1 ) ); } );
	TestMath< Float >( C, "( asinh 1 )", []( auto const& _ ){ A( _->$ == asinh( 1 ) ); } );
	TestMath< Float >( C, "( atan 1 )", []( auto const& _ ){ A( _->$ == atan( 1 ) ); } );
	TestMath< Float >( C, "( atanh 1 )", []( auto const& _ ){ A( _->$ == atanh( 1 ) ); } );
	TestMath< Float >( C, "( cbrt 1 )", []( auto const& _ ){ A( _->$ == cbrt( 1 ) ); } );
	TestMath< Float >( C, "( ceil 1 )", []( auto const& _ ){ A( _->$ == ceil( 1 ) ); } );
	TestMath< Float >( C, "( cos 1 )", []( auto const& _ ){ A( _->$ == cos( 1 ) ); } );
	TestMath< Float >( C, "( cosh 1 )", []( auto const& _ ){ A( _->$ == cosh( 1 ) ); } );
	TestMath< Float >( C, "( exp 1 )", []( auto const& _ ){ A( _->$ == exp( 1 ) ); } );
	TestMath< Float >( C, "( floor 1 )", []( auto const& _ ){ A( _->$ == floor( 1 ) ); } );
	TestMath< Float >( C, "( log 1 )", []( auto const& _ ){ A( _->$ == log( 1 ) ); } );
	TestMath< Float >( C, "( log10 1 )", []( auto const& _ ){ A( _->$ == log10( 1 ) ); } );
	TestMath< Float >( C, "( log2 1 )", []( auto const& _ ){ A( _->$ == log2( 1 ) ); } );
	TestMath< Float >( C, "( round 1 )", []( auto const& _ ){ A( _->$ == round( 1 ) ); } );
	TestMath< Float >( C, "( sin 1 )", []( auto const& _ ){ A( _->$ == sin( 1 ) ); } );
	TestMath< Float >( C, "( sinh 1 )", []( auto const& _ ){ A( _->$ == sinh( 1 ) ); } );
	TestMath< Float >( C, "( sqrt 1 )", []( auto const& _ ){ A( _->$ == sqrt( 1 ) ); } );
	TestMath< Float >( C, "( tan 1 )", []( auto const& _ ){ A( _->$ == tan( 1 ) ); } );
	TestMath< Float >( C, "( tanh 1 )", []( auto const& _ ){ A( _->$ == tanh( 1 ) ); } );
	TestMath< Float >( C, "( trunc 1 )", []( auto const& _ ){ A( _->$ == trunc( 1 ) ); } );
	
	TestMathApprox( C, "( atan2 [ 1 1.7320508 ] )", 0.5235987833701112);
	TestMathApprox( C, "( hypot [ 3 4 ] )", 5 );
	TestMathApprox( C, "( max [ 2 3 4 ] )", 4 );
	TestMathApprox( C, "( min [ 2 3 4 ] )", 2 );
	TestMathApprox( C, "( pow [ 2 3 ] )", 8 );
	TestMath< Float >( C, "( random [ 0 1 ] )", []( auto const& _ ){ A( 0 <= _->$ && _->$ <= 1 ); } );
	TestMath< Float >( C, "¤", []( auto const& _ ){ A( 0 <= _->$ && _->$ <= 1 ); } );
	TestMath< Bits >( C, "( sign -2 )", []( auto const& _ ){ A( _->$ == -1 ); } );
	TestMath< Bits >( C, "( sign 0 )", []( auto const& _ ){ A( _->$ == 0 ); } );
	TestMath< Bits >( C, "( sign 2 )", []( auto const& _ ){ A( _->$ == 1 ); } );

	// log 系
	TestMath< Float >( C, "( log 1 )",    []( auto const& _ ){ A( _->$ == 0 ); } );
	TestMath< Float >( C, "( log 𝑒 )",    []( auto const& _ ){ A( _->$ == 1 ); } );
	TestMath< Float >( C, "( log10 1 )",  []( auto const& _ ){ A( _->$ == 0 ); } );
	TestMath< Float >( C, "( log10 10 )", []( auto const& _ ){ A( _->$ == 1 ); } );
	TestMath< Float >( C, "( log10 1000 )", []( auto const& _ ){ A( _->$ == 3 ); } );
	TestMath< Float >( C, "( log2 1 )",   []( auto const& _ ){ A( _->$ == 0 ); } );
	TestMath< Float >( C, "( log2 2 )",   []( auto const& _ ){ A( _->$ == 1 ); } );
	TestMath< Float >( C, "( log2 1024 )",[]( auto const& _ ){ A( _->$ == 10 ); } );

	// exp
	TestMath< Float >( C, "( exp 0 )",    []( auto const& _ ){ A( _->$ == 1 ); } );
	TestMath< Float >( C, "( exp 1 )",    []( auto const& _ ){ A( _->$ == numbers::e ); } );

	// sin/cos/tan
	TestMath< Float >( C, "( sin 0 )",      []( auto const& _ ){ A( _->$ == 0 ); } );
	TestMath< Float >( C, "( sin (π ÷ 2) )", []( auto const& _ ){ A( _->$ == 1 ); } );
	TestMath< Float >( C, "( cos 0 )",      []( auto const& _ ){ A( _->$ == 1 ); } );
	TestMath< Float >( C, "( cos π )",     []( auto const& _ ){ A( _->$ == -1 ); } );
	TestMath< Float >( C, "( tan 0 )",      []( auto const& _ ){ A( _->$ == 0 ); } );
	TestMathApprox( C, "( tan (π ÷ 4) )", 1 );

	// asin/acos/atan
	TestMathApprox( C, "( asin 0 )",     0 );
	TestMathApprox( C, "( asin 1 )",     numbers::pi/2 );
	TestMathApprox( C, "( acos 1 )",     0 );
	TestMathApprox( C, "( acos 0 )",     numbers::pi/2 );
	TestMathApprox( C, "( atan 0 )",     0 );
	TestMathApprox( C, "( atan 1 )",     numbers::pi/4 );

	// ceil/floor/round/trunc
	TestMathApprox( C, "( ceil 1.2 )",   2 );
	TestMathApprox( C, "( floor 1.8 )",	1 );
	TestMathApprox( C, "( round 1.5 )",   2 );
	TestMathApprox( C, "( trunc -1.8 )", -1 );

	// sqrt/cbrt
	TestMathApprox( C, "( sqrt 4 )",     2 );
	TestMathApprox( C, "( cbrt 8 )",     2 );

	// sinh/cosh/tanh
	TestMathApprox( C, "( sinh 0 )",     0 );
	TestMathApprox( C, "( cosh 0 )",    1 );
	TestMathApprox( C, "( tanh 0 )",     0 );

	// asinh/acosh/atanh
	TestMathApprox( C, "( asinh 0 )",    0 );
	TestMathApprox( C, "( acosh 1 )",    0 );
	TestMathApprox( C, "( atanh 0 )",    0 );

}


