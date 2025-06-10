#include	"SliP.hpp"

SP< SliP >
EvalSentence( SP< Context > C, const vector< SP< SliP > >& _ ) {

	if( _.size() == 0 ) throw runtime_error( "Syntax Error: No infix operand or null sentence" );
	if( _.size() == 1 ) return Eval( C, _[ 0 ] );

	auto
	infixEntries = ::filter(
		mapWithIndex(
			_
		, 	[]( auto $, auto index ){ return tuple( Cast< Infix >( $ ), index ); }
		)
	,	[]( auto _ ){
			return get< 0 >( _ );
		}
	);
	if( infixEntries.size() ) {
		auto $ = infixEntries[ 0 ];
		for( auto _ = 1; _ < infixEntries.size(); _++ ) {
			auto entry = infixEntries[ _ ];
			if ( get< 0 >( entry )->priority >= get< 0 >( $ )->priority ) $ = entry;
		}
		auto [ infix, index ] = $;
		return infix->$(
			C
		,	EvalSentence( C, sliceTo( _, index ) )
		,	EvalSentence( C, sliceFrom( _, index + 1 ) )
		);
	} else {
		auto index = _.size() - 1;
		if(	Cast< Prefix >( _[ index ] ) ) throw runtime_error( "Syntax Error: No prefix operand" );
		SP< SliP > $ = Eval( C, _[ index ] );
		while( --index ) {
			auto slip = _[ index ];
			if( auto prefix = Cast< Prefix >( slip ) ) {
				$ = prefix->$( C, $ );
			} else {
				$ = Eval( C, _[ index ] );
			}
		}
		return $;
	}
	throw runtime_error( "Syntax error" );
}

SP< SliP >
Eval( SP< Context > C, SP< SliP > _ ) {
	if(	auto name = Cast< Name >( _ ) ) {
		while( C ) {
			if ( C->dict.contains( name->$ ) ) return C->dict[ name->$ ];
			C = C->next;
		}
		throw runtime_error( "Undefined" );
	}
	if( auto primitive = Cast< Primitive >( _ ) ) return primitive->$( C );
	if( auto parallel = Cast< Parallel >( _ ) ) return MS< List >(
		map(
			parallel->$
		,	[ & ]( auto _ ){ return Eval( C, _ ); }
		)
	);
	if( auto procedure = Cast< Procedure >( _ ) ) {
		auto
		newC = MS< Context >( C );
		return MS< List >(
			map(
				procedure->$
			,	[ & ]( auto _ ){ return Eval( C, _ ); }
			)
		);
	}
	if( auto sentence = Cast< Sentence >( _ ) ) return EvalSentence( C, sentence->$ );
	return _;
}

