#pragma once

inline SP< SliP >
Eval( SP< Context > const&, SP< SliP > const& );

inline auto
EvalPrefix( SP< Context > const& C, vector< SP< SliP > > const& S ) {

	if( auto _ = Cast< Prefix >( S.back() ) ) throw runtime_error( "Syntax Error: No operand for prefix: " + _->label );

	vector< SP< SliP > >	$;

	size_t		I = S.size() - 1;
	SP< SliP >	prev = S[ I ];
	while( I-- ) {
		if( auto _ = Cast< Prefix >( S[ I ] ) ) {
			$.insert( $.begin(), _->$( C, prev ) );
			prev = nullptr;
		} else {
			if( prev ) $.insert( $.begin(), Eval( C, prev ) );
			prev = S[ I ];
		}
	}
	if( prev ) $.insert( $.begin(), Eval( C, prev ) );
	return $;
}

inline	auto
InfixI( vector< SP< SliP > > const& S ) {

	pair< size_t, SP< Infix > >	$;

	for( size_t I = 0; I < S.size(); I++ ) {
		if( auto _ = Cast< Infix >( S[ I ] ) ) {
			if( get< 0 >( $ ) ) {
				if( _->priority > get< 1 >( $ )->priority ) {
					$ = { I, _ };
				}
			} else {
				$ = { I, _ };
			}
		}
	}
	return $;
}

inline SP< SliP >
EvalInfix( SP< Context > const& C, vector< SP< SliP > > const& S ) {
//cerr << ':' << MS< Sentence >( S )->REPR() << ':' << endl;

	auto [ infixI, infix ] = InfixI( S );

	if( infix ) {
		if( infixI == 0 )				throw runtime_error( "Syntax Error: No left operand for infix operator: " + infix->label );
		if( infixI == S.size() - 1 )	throw runtime_error( "Syntax Error: No right operand for infix operator: " + infix->label );

//cerr << "Infix: " << infix->label << endl;
		return infix->$(
			C
		,	EvalInfix( C, ranges::to< vector >( ::take( S, infixI ) ) )
		,	EvalInfix( C, ranges::to< vector >( ::drop( S, infixI + 1 ) ) )
		);
	} else {
		if( S.size() == 1 ) return S[ 0 ];
		SP< Numeric > $ = MS< Bits >( 1 );
		for( size_t I = 0; I < S.size(); I++ ) {
			if( auto _ = Cast< Numeric >( S[ I ] ) ) {
				$ = Mul( $, _ );
			} else {
				throw runtime_error( "Syntax Error: No numeric value: " + S[ I ]->REPR() );
			}
		}
		return $;
	}
}

inline SP< SliP >
Eval( SP< Context > const& C, SP< SliP > const& S ) {
	if(	const auto _ = Cast< Name >( S ) ) {
		SP< Context >	c = C;
		while( c ) {
			if ( c->dict.contains( _->$ ) ) return c->dict[ _->$ ];
			c = c->next;
		}
		_Z( "Undefined name: " + _->$ );
	}
	if( const auto _ = Cast< Primitive >( S ) ) return _->$( C );
	if( const auto _ = Cast< Parallel >( S ) ) return MS< List >(
		ranges::to< vector >(
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
			ranges::to< vector >(
				project(
					_->$
				,	[ & ]( auto const& _ ){ return Eval( C, _ ); }
				)
			)
		);
	}
//	if( const auto _ = Cast< Sentence >( S ) ) return _->$.size() == 0 ? Nil : EvalInfix( C, EvalPrefix( C, _->$ ) );
	if( const auto _ = Cast< Sentence >( S ) ) {
//cerr << '>' << _->REPR() << endl;
		auto $ = _->$.size() == 0 ? Nil : EvalInfix( C, EvalPrefix( C, _->$ ) );
//cerr << $->REPR() << endl;
		return $;
	}
	return S;
}
