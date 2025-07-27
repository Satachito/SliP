#include "SliP.hpp"

SP< SliP >
Eval( SP< Context > C, SP< SliP > S );

extern SP< SliP > Nil;

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
		,	ApplyInfix( C, ranges::to< V >( ::take( Ss, infixI + 0 ) ) )
		,	ApplyInfix( C, ranges::to< V >( ::drop( Ss, infixI + 1 ) ) )
		);
	} else {
		if( Ss.size() == 0 ) return Nil;
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
//SP< SliP >
//ApplyInfix( SP< Context > C, V< SP< SliP > > const& Ss );
//
//SP< SliP >
//ApplyInfix( SP< Context > C, V< SP< SliP > > const& Ss ) {
//	cerr << '<' << MS< List >( Ss )->REPR() << endl;
//	auto
//	$ = _ApplyInfix( C, Ss );
//	cerr << '>' << $->REPR() << endl;
//	cerr << endl;
//	return $;
//}

V< SP< SliP > >
ReplacePrefix( V< SP< SliP > > const& Ss ) {
	size_t	I = Ss.size();
	if( I < 2 ) return Ss;
	auto _ = Cast< Prefix >( Ss[ --I ] );

	V< SP< SliP > >	$;
	while( I ) {
		auto prefix = Cast< Prefix >( Ss[ --I ] );
		if( prefix ) {
			$.insert( $.begin(), Ss[ I + 1 ] );
		} else {
			if ( _ ) {
				if(	_->label == "+" ) {
					extern SP< Infix > infixPlus;
					$.insert( $.begin(), infixPlus );
				} else if( _->label == "-" ) {
					extern SP< Infix > infixMinus;
					$.insert( $.begin(), infixMinus );
				} else {
					$.insert( $.begin(), Ss[ I + 1 ] );
				}
			} else {
				$.insert( $.begin(), Ss[ I + 1 ] );
			}
		}
		_ = prefix;
	}
	$.insert( $.begin(), Ss[ 0 ] );
	return $;
}

V< SP< SliP > >
ApplyPrefix( SP< Context > C, V< SP< SliP > > const& Ss ) {
	size_t	I = Ss.size();
	if( I < 2 ) return Ss;

	auto _ = Ss[ --I ];
	
	V< SP< SliP > >	$;
	while( I ) {
		auto prefix = Cast< Prefix >( Ss[ --I ] );
		if( prefix ) {
			_ = prefix->$( C, _ );
		} else {
			$.insert( $.begin(), _ );
			_ = Ss[ I ];
		}
	}
	$.insert( $.begin(), _ );
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
				,	[ & ]( auto const& _ ){ return Eval( C, _ ); }
				)
			)
		);
	}
	if( const auto sentence = Cast< Sentence >( S ) ) {
		return ApplyInfix( C, ApplyPrefix( C, ReplacePrefix( sentence->$ ) ) );
	}
	return S;
}
//	while ( auto draft = Read( C, _, close ) ) {
//		auto draftPrefix = Cast< Prefix >( Eval( C, draft ) );
//		if( prefix && draftPrefix ) {
//			if(	draftPrefix->label == "+" ) {
//				$.push_back( infixPlus );
//			} else if( draftPrefix->label == "-" ) {
//				$.push_back( infixMinus );
//			} else {
//				_Z( "Syntax error: " + draftPrefix->label + ' ' + draftPrefix->label );
//			}
//		} else {
//			$.push_back( draft );
//		}
//		slip = draft;
//		prefix = draftPrefix;
//	}
//	DEBUG
//	for ( auto const& _: $ ) {
//		auto prefix = Cast< Prefix >( _ );
//		cout << ( prefix ? ( ':' + prefix->label + ':' ) : _->REPR() ) << ' ';
//	}
//	cout << endl;
//
//	return $;

