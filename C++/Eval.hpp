#ifdef EVAL_ALREADY_INCLUDED
#error "This header should be included once."
#else
#define EVAL_ALREADY_INCLUDED
#endif

inline SP< SliP >
Eval( const SP< Context >& C, const SP< SliP >& _ );

inline SP< SliP >
EvalSentence( const SP< Context >& C, const vector< SP< SliP > >& _ ) {
	if( _.size() == 0 ) throw runtime_error( "Syntax Error: No infix operand or null sentence" );
	if( _.size() == 1 ) return Eval( C, _[ 0 ] );

	auto
	infixEntries = ranges::to< vector >(
		filter(
			zipIndex(
				project(
					_
				,	[]( const SP< SliP >& _ ) { return Cast< Infix >( _ ); }
				)
			)
		,	[]( const auto& _ ) { return _.first != 0; }	//	show bool inplicitly
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
		,	EvalSentence( C, ranges::to< vector >( take( _, index ) ) )
		,	EvalSentence( C, ranges::to< vector >( drop( _, index + 1 ) ) )
		);
	} else {
		auto index = _.size() - 1;
		if(	Cast< Prefix >( _[ index ] ) ) throw runtime_error( "Syntax Error: No prefix operand" );
		SP< SliP > $ = Eval( C, _[ index ] );
		while( index-- ) {
			auto slip = _[ index ];
			if( auto prefix = Cast< Prefix >( slip ) ) {
				$ = prefix->$( C, $ );
			} else {
				$ = Eval( C, slip );
			}
		}
		return $;
	}
}

inline SP< SliP >
Eval( const SP< Context >& C, const SP< SliP >& _ ) {
	if(	const auto name = Cast< Name >( _ ) ) {
		SP< Context >	c = C;
		while( c ) {
			if ( c->dict.contains( name->$ ) ) return c->dict[ name->$ ];
			c = c->next;
		}
		throw runtime_error( "Undefined" );
	}
	if( const auto primitive = Cast< Primitive >( _ ) ) return primitive->$( C );
	if( const auto parallel = Cast< Parallel >( _ ) ) return MS< List >(
		ranges::to< vector >(
			project(
				parallel->$
			,	[ & ]( const auto& _ ){ return Eval( C, _ ); }
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
				,	[ & ]( const auto& _ ){ return Eval( C, _ ); }
				)
			)
		);
	}
	if( const auto sentence = Cast< Sentence >( _ ) ) return EvalSentence( C, sentence->$ );
	return _;
}
