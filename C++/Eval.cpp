#include "SliP.hpp"

SP< SliP >
Eval( SP< Context > C, SP< SliP > S );

extern SP< SliP > Nil;

//	A Sentence is evaluated by splitting at the loosest (lowest-priority) infix
//	operator first, on the raw form list, then recursing into both sides.
//	Equal priority: the rightmost occurrence wins (left-associative) unless the
//	operator is registered right-associative (`=`, `,`).
//	Because operands are only evaluated inside infix-free segments, a LazyInfix
//	(`&&`, `||`, `¿`) can skip its right side entirely, and side effects run
//	left to right.

V< SP< SliP > >
ApplyPrefix( SP< Context > C, V< SP< SliP > > const& Ss );

SP< SliP >
ApplyInfix( SP< Context > C, V< SP< SliP > > const& Ss ) {

	size_t		infixI	= 0;
	SP< Infix >	infix	= nullptr;
	{	for( size_t _ = 0; _ < Ss.size(); _++ ) {
			if( auto $ = Cast< Infix >( Ss[ _ ] ) ) {
				if( !infix
				||	$->priority < infix->priority
				||	( $->priority == infix->priority && !$->rightAssoc )
				) {	infixI = _;
					infix = $;
				}
			}
		}
	}

	if( infix ) {
		if( infixI == 0 )				_Z( "Syntax Error: No left operand for infix operator: " + infix->label );
		if( infixI == Ss.size() - 1 )	_Z( "Syntax Error: No right operand for infix operator: " + infix->label );

		auto
		l = ApplyInfix( C, ranges::to< V >( ::take( Ss, infixI + 0 ) ) );
		function< SP< SliP >() >
		r = [ & ]() { return ApplyInfix( C, ranges::to< V >( ::drop( Ss, infixI + 1 ) ) ); };
		if( auto lazy = Cast< LazyInfix >( infix ) ) return lazy->lazy( C, l, r );
		return infix->$( C, l, r() );
	} else {
		auto Vs = ApplyPrefix( C, Ss );
		if( Vs.size() == 0 ) return Nil;
		if( Vs.size() == 1 ) return Vs[ 0 ];
		SP< Numeric > $ = MS< Bits >( 1 );
		for( size_t _ = 0; _ < Vs.size(); _++ ) {
			if( auto numeric = Cast< Numeric >( Vs[ _ ] ) ) {
				extern	SP< Numeric >	Mul( SP< SliP >, SP< SliP > );
				$ = Mul( $, numeric );
			} else {
				_Z( "Syntax Error: No numeric value: " + Vs[ _ ]->REPR() );
			}
		}
		return $;
	}
}

//	A "bare" numeric is one written directly as a number, constant, or name —
//	not the result of a parenthesized sentence or a function application.
//	A prefix operator absorbs the following run of bare numerics as one
//	product: `sin 2π` is sin( 2 × π ), while `sin(2) π` stays sin( 2 ) × π.
static bool
IsBareNumeric( SP< SliP > raw, SP< SliP > value ) {
	return ( Cast< Numeric >( raw ) != nullptr || Cast< Name >( raw ) != nullptr )
	&&	Cast< Numeric >( value ) != nullptr
	;
}

V< SP< SliP > >
ApplyPrefix( SP< Context > C, V< SP< SliP > > const& Ss ) {
	size_t	I = Ss.size();
	if( I == 0 ) return Ss;
	if( I == 1 ) return V< SP< SliP > >{ Eval( C, Ss[ 0 ] ) };

	extern	SP< Numeric >	Mul( SP< SliP >, SP< SliP > );

	auto _ = Ss[ --I ];
	auto applied = false;
	V< SP< SliP > >	$;
	size_t	bareRun = 0;	//	Leading elements of $ that are juxtaposed bare numerics
	auto
	Insert = [ & ]() {
		auto value = applied ? _ : Eval( C, _ );
		bareRun = !applied && IsBareNumeric( _, value ) ? bareRun + 1 : 0;
		$.insert( $.begin(), value );
	};
	while( I-- ) {
		if( auto quote = Cast< Quote >( Ss[ I ] ) ) {
			_ = quote->$( C, _ );
			applied = true;
		} else if( auto prefix = Cast< Prefix >( Ss[ I ] ) ) {
			auto operand = applied ? _ : Eval( C, _ );
			if( !applied && IsBareNumeric( _, operand ) ) {
				while( bareRun ) {
					operand = Mul( operand, $.front() );
					$.erase( $.begin() );
					bareRun--;
				}
			}
			_ = prefix->$( C, operand );
			applied = true;
		} else {
			Insert();
			_ = Ss[ I ];
			applied = false;
		}
	}
	Insert();
	return $;
}

SP< SliP >
Eval( SP< Context > C, SP< SliP > S ) {
	if(	const auto name = Cast< Name >( S ) ) {
		SP< Context >	c = C;
		while( c ) {
			if ( c->$.contains( name->$ ) ) return c->$[ name->$ ];
			c = c->next;
		}
		_Z( "Undefined name: " + name->$ );
	}
	if( const auto primitive = Cast< Primitive >( S ) ) {
		return primitive->$( C );
	}
	if( const auto parallel = Cast< Parallel >( S ) ) return MS< List >(
		ranges::to< V >(
			project(
				parallel->$
			,	[ & ]( auto const& _ ){ return Eval( C, _ ); }
			)
		)
	);
	if( const auto procedure = Cast< Procedure >( S ) ) {
		auto
		newC = MS< Context >( C );
		return MS< List >(
			ranges::to< V >(
				project(
					procedure->$
				,	[ & ]( auto const& _ ){ return Eval( newC, _ ); }
				)
			)
		);
	}
	if( const auto sentence = Cast< Sentence >( S ) ) {
		return ApplyInfix( C, sentence->$ );
	}
	return S;
}
