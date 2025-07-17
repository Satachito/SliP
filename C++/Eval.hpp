#pragma once

inline SP< SliP >
Eval( SP< Context > const& C, SP< SliP > const& _ );

inline SP< SliP >
EvalSentence( SP< Context > const& C, vector< SP< SliP > > const& _ ) {
	if( _.size() == 0 ) return Nil;

	auto
	infixEntries = ranges::to< vector >(
		filter(
			zipIndex(
				project(
					_
				,	[]( SP< SliP > const& _ ) { return Cast< Infix >( _ ); }
				)
			)
		,	[]( auto const& _ ) { return std::get<0>(_) != 0; }
		)
	);

	if( infixEntries.size() ) {
		auto $ = infixEntries[ 0 ];
		for( size_t _ = 1; _ < infixEntries.size(); _++ ) {
			auto entry = infixEntries[ _ ];
			if ( get< 0 >( entry )->priority >= get< 0 >( $ )->priority ) $ = entry;
		}
		const auto [ infix, index ] = $;
		return infix->$(
			C
		,	EvalSentence( C, ranges::to< vector >( ::take( _, index ) ) )
		,	EvalSentence( C, ranges::to< vector >( ::drop( _, index + 1 ) ) )
		);
	} else {
		SP< Numeric > $ = MS< Bits >( 1 );
		auto I = size_t( 0 );
		while( I < _.size() ) {
			if( auto numeric = Cast< Numeric >( _[ I ] ) ) {
				I++;
				$ = Mul( $, numeric );
				continue;
			}
			if( auto prefix = Cast< Prefix >( _[ I ] ) ) {
				I++;
				if( I == _.size() ) throw runtime_error( "Syntax Error: No operand for prefix: " + prefix->label );
				if( auto numeric = Cast< Numeric >( prefix->$( C, _[ I++ ] ) ) ) {
					I++;
					$ = Mul( $, numeric );
					continue;
				}
			}
			throw runtime_error( "Syntax Error: no numeric value: " + _[ I++ ]->REPR() );
		}
		return $;
	}
}

inline SP< SliP >
Eval( SP< Context > const& C, SP< SliP > const& _ ) {
	if(	const auto name = Cast< Name >( _ ) ) {
		SP< Context >	c = C;
		while( c ) {
			if ( c->dict.contains( name->$ ) ) return c->dict[ name->$ ];
			c = c->next;
		}
		_Z( "Undefined name: " + name->$ );
	}
	if( const auto primitive = Cast< Primitive >( _ ) ) return primitive->$( C );
	if( const auto parallel = Cast< Parallel >( _ ) ) return MS< List >(
		ranges::to< vector >(
			project(
				parallel->$
			,	[ & ]( auto const& _ ){ return Eval( C, _ ); }
			)
		)
	);
	if( const auto procedure = Cast< Procedure >( _ ) ) {
		auto
		newC = MS< Context >( C );
		return MS< List >(
			ranges::to< vector >(
				project(
					procedure->$
				,	[ & ]( auto const& _ ){ return Eval( C, _ ); }
				)
			)
		);
	}
	if( const auto sentence = Cast< Sentence >( _ ) ) return EvalSentence( C, sentence->$ );
	return _;
}
