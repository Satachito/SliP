#include "SliP.hpp"

SP< SliP >
ApplyInfix( SP< Context > C, V< SP< SliP > > const& Ss ) {

	size_t		infixI	= 0;
	SP< Infix >	infix	= nullptr;
	{	for( size_t _ = 0; _ < Ss.size(); _++ ) {
			if( auto $ = Cast< Infix >( Ss[ _ ] ) ) {
				if( infixI ) {
					if( infix->priority < $->priority ) {
						infixI = _;
						infix = $;
					}
				} else {
					infixI = _;
					infix = $;
				}
			}
		}
	}

	if( infix ) {
		if( infixI == 0 )				_Z( "Syntax Error: No left operand for infix operator: " + infix->label );
		if( infixI == Ss.size() - 1 )	_Z( "Syntax Error: No right operand for infix operator: " + infix->label );

//cerr << "Infix: " << infix->label << endl;
		return infix->$(
			C
		,	ApplyInfix( C, ranges::to< V >( ::take( Ss, infixI ) ) )
		,	ApplyInfix( C, ranges::to< V >( ::drop( Ss, infixI + 1 ) ) )
		);
	} else {
		if( Ss.size() == 1 ) return Ss[ 0 ];
		SP< Numeric > $ = MS< Bits >( 1 );
		for( size_t _ = 0; _ < Ss.size(); _++ ) {
			if( auto numeric = Cast< Numeric >( Ss[ _ ] ) ) {
				extern	SP< Numeric >	Mul( SP< SliP >, SP< SliP > );
				$ = Mul( $, numeric );
			} else {
				_Z( "Syntax Error: No numeric value: " + Ss[ _ ]->REPR() );
			}
		}
		return $;
	}
}

SP< SliP >
Eval( SP< Context > C, SP< SliP > S ) {
	if(	const auto _ = Cast< Name >( S ) ) {
		SP< Context >	c = C;
		while( c ) {
			if ( c->dict.contains( _->$ ) ) return c->dict[ _->$ ];
			c = c->next;
		}
		_Z( "Undefined name: " + _->$ );
	}
	if( const auto _ = Cast< Primitive >( S ) ) {
		return _->$( C );
	}
	if( const auto _ = Cast< Parallel >( S ) ) return MS< List >(
		ranges::to< V >(
			project(
				_->$
			,	[ & ]( auto const& _ ){ return Eval( C, _ ); }
			)
		)
	);
	if( const auto _ = Cast< Procedure >( S ) ) {
		auto
		newC = MS< Context >( C );
		return MS< List >(
			ranges::to< V >(
				project(
					_->$
				,	[ & ]( auto const& _ ){ return Eval( C, _ ); }
				)
			)
		);
	}
	if( const auto _ = Cast< Sentence >( S ) ) {
		auto const& Ss = _->$;
		extern	SP< SliP >		Nil;
		if( Ss.size() == 0 ) return Nil;
		
		if( auto _ = Cast< Prefix >( Ss.back() ) ) _Z( "Syntax Error: No operand for prefix: " + _->label );
		if( Ss.size() == 1 ) return Eval( C, Ss[ 0 ] );

		V< SP< SliP > >	$;
		{	size_t	I = Ss.size() - 1;
			auto	next = Ss[ I ];
			while( I-- ) {
				if( auto _ = Cast< Prefix >( Ss[ I ] ) ) {
					next = _->$( C, next );
				} else {
					if( next ) $.insert( $.begin(), next );
					next = Ss[ I ];
				}
			}
			if( next ) $.insert( $.begin(), next );
		}
		if( $.size() == 1 ) return $[ 0 ];

		return ApplyInfix( C, $ );
	}
	return S;
}
