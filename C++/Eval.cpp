#include	"SliP.hpp"

inline shared_ptr< SliP >
EvalSentence( shared_ptr< Context > C, const vector< shared_ptr< SliP > >& _ ) {

	if( _.size() == 0 ) return Nil;
	if( _.size() == 1 ) return Eval( C, _[ 0 ] );

	auto
	infixEntries = ::filter(
		mapWithIndex(
			_
		, 	[]( auto $, auto index ){ return tuple( $, index ); }
		)
	,	[]( auto _ ){
			return Cast< Infix >( get< 0 >( _ ) );
		}
	);
	if( infixEntries.size() ) {
		auto $ = infixEntries[ 0 ];
		for( auto _ = 1; _ < infixEntries.size(); _++ ) {
			auto entry = infixEntries[ _ ];
			if ( get< 0 >entry->priority >= get< 0 >$->priority ) $ = entry;
		}
		auto [ infix, index ] = $
		return infix->$(
			C
		,	EvalSentence( C, SliceTo( _, index ) )
		,	EvalSentence( C, SliceFrom( _, index + 1 ) )
		)
		)
	} else {
/*
		let index = _.length - 1
		_ = _.slice()
		while ( index-- ) {
			let $ = _[ index ]
			$ instanceof Prefix || ( $ = Eval( c, $ ) )
			$ instanceof Prefix && _.splice( index, 2, $._( c, _[ index + 1 ] ) )
		}
		if ( _.length === 1 ) return _[ 0 ]
		const
		$ = _.map( _ => Eval( c, _ ) )
		if ( $.every( _ => _ instanceof Numeric ) ) return new Numeric( $.reduce( ( $, _ ) => $ * _._, 1 ) )
*/
	}
	throw runtime_error( "Syntax error" );
}

inline shared_ptr< SliP >
Eval( shared_ptr< Context > C, shared_ptr< SliP > _ ) {
	if(	auto name = Cast< Name >( _ ) ) {
		while( C ) {
			if ( C->dict.contains( name->$ ) ) return C->dict[ name->$ ];
			C = C->next;
		}
		throw runtime_error( "Undefined" );
	}
	if( auto primitive = Cast< Primitive >( _ ) ) return primitive->$( C );
	if( auto parallel = Cast< Parallel >( _ ) ) return make_shared< List >(
		map(
			parallel->$
		,	[ & ]( auto _ ){ return Eval( C, _ ); }
		)
	);
	if( auto procedure = Cast< Procedure >( _ ) ) {
		auto
		newC = make_shared< Context >( C );
		return make_shared< List >(
			map(
				procedure->$
			,	[ & ]( auto _ ){ return Eval( C, _ ); }
			)
		);
	}
	if( auto sentence = Cast< Sentence >( _ ) ) return EvalSentence( C, sentence->$ );
	return _;
}

