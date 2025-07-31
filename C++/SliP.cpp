#include	"SliP.hpp"

extern SP< SliP > Eval( SP< Context >, SP< SliP > );

V< SP< SliP > >
theStack;

mutex
stackMutex;

void
Push( SP< SliP > _ ) {
	lock_guard< mutex >	lock( stackMutex );
	theStack.push_back( _ );
}
SP< SliP >
Pop() {
	if ( theStack.empty() ) _Z( "Stack underflow" );
	lock_guard< mutex >	lock( stackMutex );
	auto $ = theStack.back();
	theStack.pop_back();
	return $;
}

V< SP< SliP > >
StackCopy() {
	lock_guard< mutex >	lock( stackMutex );
	return theStack;
}

bool
IsNil( SP< SliP > _ ) {
	auto list = Cast< List >( _ );
	return list
	?	list->$.size() == 0
	:	false
	;
}
bool
IsT( SP< SliP > _ ) {
	return !IsNil( _ );
}

SP< Numeric >
Mul( SP< SliP > l, SP< SliP > r ) {
	{	auto L = Cast< Bits >( l ), R = Cast< Bits >( r );
		if( L && R ) {
			int64_t	$;
			if( !ckd_mul( &$, L->$, R->$ ) ) return MS< Bits >( $ );
		}
	}
	auto L = Z( "Illegal operand type: " + l->REPR(), Cast< Numeric >( l ) );
	auto R = Z( "Illegal operand type: " + r->REPR(), Cast< Numeric >( r ) );
	return MS< Float >( L->Double() * R->Double() );
}

auto
T = MS< SliP >();

SP< SliP >
Nil = MS< List >( V< SP< SliP > >{} );

auto
prefixPlus = MS< Prefix >(
	[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
		return Z( "Not numeric", Cast< Numeric >( Eval( C, _ ) ) );
	}
,	"+"
);

auto
prefixMinus = MS< Prefix >(
	[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
		return Z( "Not numeric", Cast< Numeric >( Eval( C, _ ) ) )->Negate();
	}
,	"-"
);

auto
infixPlus = MS< Infix >(
	[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
		{	auto L = Cast< Bits >( l ), R = Cast< Bits >( r );
			if( L && R ) {
				int64_t	$;
				if( !ckd_add( &$, L->$, R->$ ) ) return MS< Bits >( $ );
			}
		}
		{	auto L = Cast< Numeric >( l ), R = Cast< Numeric >( r );
			if( L && R ) return MS< Float >( L->Double() + R->Double() );
		}
		{	auto L = Cast< Literal >( l ), R = Cast< Literal >( r );
			if( L && R ) return MS< Literal	>( L->$ + R->$, L->mark );
		}
		{	auto L = Cast< Sentence		>( l ), R = Cast< Sentence	>( r );
			if( L && R ) return MS< Sentence	>( L->$ + R->$ );
		}
		{	auto L = Cast< Procedure	>( l ), R = Cast< Procedure	>( r );
			if( L && R ) return MS< Procedure	>( L->$ + R->$ );
		}
		{	auto L = Cast< Parallel		>( l ), R = Cast< Parallel	>( r );
			if( L && R ) return MS< Parallel	>( L->$ + R->$ );
		}
		{	auto L = Cast< List			>( l ), R = Cast< List		>( r );
			if( L && R ) return MS< List		>( L->$ + R->$ );
		}
		return MS< List >( V< SP< SliP > >{ l, r } );
	}
,	"+"		//	Plus
,	30
);
auto
infixMinus = MS< Infix >(
	[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
		{	auto L = Cast< Bits	>( l ), R = Cast< Bits	>( r );
			if( L && R ) {
				int64_t	$;
				if( !ckd_sub( &$, L->$, R->$ ) ) return MS< Bits >( $ );
			}
		}
		auto L = Z( "Illegal operand type: " + l->REPR(), Cast< Numeric >( l ) );
		auto R = Z( "Illegal operand type: " + r->REPR(), Cast< Numeric >( r ) );
		return MS< Float >( L->Double() - R->Double() );
	}
,	"-"		//	Minus
,	30
);
int
_Compare( SP< SliP > l, SP< SliP > r ) {
	{	auto L = Cast< Bits >( l ), R = Cast< Bits >( r );
		if( L && R ) return L->$ == R->$ ? 0 : L->$ < R->$ ? -1 : 1;
	}
	{	auto L = Cast< Numeric >( l ), R = Cast< Numeric >( r );
		if( L && R ) return L->Double() == R->Double() ? 0 : L->Double() < R->Double() ? -1 : 1;
	}
	{	auto L = Cast< Literal >( l ), R = Cast< Literal >( r );
		if( L && R ) return L->$ == R->$ ? 0 : L->$ < R->$ ? -1 : 1;
	}
	{	auto L = Cast< List >( l ), R = Cast< List >( r );
		if( L && R ) {
			if( L->$.size() == R->$.size() ) {
				for( size_t _ = 0; _ < L->$.size(); _++ ) {
					auto $ = _Compare( L->$[ _ ], R->$[ _ ] );
					if( $ != 0 ) return $;
				}
				return 0;
			} else {
				return L->$.size() < R->$.size() ? -1 : 1;
			}
		}
	}
	{	auto L = Cast< Matrix >( l ), R = Cast< Matrix >( r );
		if( L && R ) {
			if( L->size == R->size ) {
				if( L->nCols == 0 && R->nCols == 0 ) {
					for( size_t _ = 0; _ < L->size; _++ ) {
						if( L->$[ _ ] == R->$[ _ ] ) continue;
						return L->$[ _ ] < R->$[ _ ] ? -1 : 1;
					}
					return 0;
				}
				auto [ nRows, nCols ] = L->Shape();
				auto [ rNRows, rNCols ] = R->Shape();
				if( nRows == rNRows ) {					//	No need to check nCols == rNCols because size is already checked
					for( size_t _R = 0; _R < nRows; _R++ ) {
						for( size_t _C = 0; _C < nCols; _C++ ) {
							if( (*L)( _R, _C ) == (*R)( _R, _C ) ) continue;
							return (*L)( _R, _C ) < (*R)( _R, _C ) ? -1 : 1;
						}
					}
					return 0;
				}
				return nRows < rNRows ? -1 : 1;
			} else {
				return L->size < R->size ? -1 : 1;
			}
		}
	}
	return l == r ? 0 : l < r ? -1 : 1;
}

UM< string, SP< SliP > > BUILTINS;

void
RegisterFunction( SP< Function > _ ) { BUILTINS[ _->label ] = _; };
void
RegisterNumericConstant( SP< NumericConstant > _ ) { BUILTINS[ _->$ ] = _; };

auto
Build() {
	
	RegisterFunction(
		MS< Primitive >(
			[]( SP< Context > ) -> SP< SliP > {
				return Pop();
			}
		,	"@"		//	Stack top
		)
	);
	RegisterFunction(
		MS< Primitive >(
			[]( SP< Context > ) -> SP< SliP > {
				return MS< List >( StackCopy() );
			}
		,	"@@"	//	Stack list
		)
	);
	RegisterFunction(
		MS< Primitive >(
			[]( SP< Context > C ) -> SP< SliP > {
				return MS< Dict >( C->$ );
			}
		,	"¤"		//	make Dict
		)
	);
	RegisterFunction(
		MS< Primitive >(
			[]( SP< Context > ) -> SP< SliP > {
				return Nil;
			}
		,	"∅"
		)
	);
	RegisterFunction(
		MS< Prefix >(
			[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
				return _;
			}
		,	"'"		//	Quote
		)
	);
	RegisterFunction(
		MS< Prefix >(
			[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
				_Z( _->REPR() );
			}
		,	"¡"		//	Throw
		)
	);
	RegisterFunction(
		MS< Prefix >(
			[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
				return Eval( C, _ );
			}
		,	"!"		//	Eval
		)
	);
	RegisterFunction(
		MS< Prefix >(
			[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
				return MS< Bits >(
					~Z( "Illegal operand type: " + _->REPR(), Cast< Bits >( _ ) )->$
				);
			}
		,	"~"		//	Bit not
		)
	);
	RegisterFunction(
		MS< Prefix >(
			[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
				return IsNil( _ ) ? T : Nil;
			}
		,	"¬"		//	Logical not
		)
	);
	RegisterFunction(
		MS< Prefix >(
			[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
				auto literal = Cast< Literal >( _ );
				return literal
				?	literal
				:	MS< Literal >( _->REPR() , U'`' )
				;
			}
		,	"¶"		//	Convert to literal
		)
	);
	RegisterFunction(
		MS< Unary >(
			[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
				if( auto list = Cast< List >( _ ) ) return MS< Bits >( list->$.size() );
				return MS< Bits >( Z( "Illegal operand type", Cast< Literal >( _ ) )->$.length() );
			}
		,	"#"		//	Number of elements
		)
	);
	RegisterFunction(
		MS< Unary >(
			[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
				auto list = Z( "Illegal operand type", Cast< List >( _ ) );
				V< SP< SliP > > $;
				$.reserve( list->$.size() - 1 );
				$.insert( $.end(), list->$.begin() + 1, list->$.end() );
				if( Cast< Parallel		>( _ ) ) return MS< Parallel	>( $ );
				if( Cast< Sentence		>( _ ) ) return MS< Sentence	>( $ );
				if( Cast< Procedure		>( _ ) ) return MS< Procedure	>( $ );
				return MS< List >( $ );
			}
		,	"*"		//	CDR
		)
	);
	RegisterFunction(
		MS< Unary >(
			[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
				auto list = Z( "Illegal operand type", Cast< List >( _ ) );
				auto size = list->$.size();
				Z( "Insufficient list", size );
				return list->$[ size - 1 ];
			}
		,	"$"		//	Last element
		)
	);
	RegisterFunction(
		MS< Unary >(
			[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
				cout << _->REPR() << endl;
				return _;
			}
		,	";"		//	stdout
		)
	);
	RegisterFunction(
		MS< Unary >(
			[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
				cerr << _->REPR() << endl;
				return _;
			}
		,	"¦"		//	stderr
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				return Eval(
					MS< Context >(
						C
					,	Z( "Left must be dict.", Cast< Dict >( l ) )->$
					)
				,	r
				);
			}
		,	"§"		//	Open new context with dict(l) then eval r
		,	100
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				return C->$[
					Z( "Only name can be assigned.", Cast< Name >( l ) )->$
				] = r;
			}
		,	"="		//	assign
		,	90
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				return Eval(
					C
				,	Z( "Right operand must be two element List.", Cast< List >( r ) )->$[ IsNil( l ) ? 1 : 0 ]
				);
			}
		,	"?"		//	if else
		,	80
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				return IsT( l ) ? Eval( C, r ) : Nil;
			}
		,	"¿"		//	if
		,	80
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				return ( IsT( l ) && IsT( r ) ) ? T : Nil;
			}
		,	"&&"	//	Logical and
		,	70
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				return ( IsT( l ) || IsT( r ) ) ? T: Nil;
			}
		,	"||"	//	Logical or
		,	70
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				return ( IsT( l ) != IsT( r ) ) ? T: Nil;
			}
		,	"^^"	//	Logical exclusive or
		,	70
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				auto list = Z( "Right operand must be List", Cast< List >( r ) );
				for( auto const& _: list->$ ) {
					if( _Compare( _, l ) == 0 ) return T;
				}
				return Nil;
			}
		,	"∈"		//	Member of
		,	60
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				auto list = Z( "Left operand must be List", Cast< List >( l ) );
				for( auto const& _: list->$ ) {
					if( _Compare( _, r ) == 0 ) return T;
				}
				return Nil;
			}
		,	"∋"		//	Includes
		,	60
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				return _Compare( l, r ) ? Nil : T;
			}
		,	"=="	//	Equal
		,	60
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				return _Compare( l, r ) ? T : Nil;
			}
		,	"<>"	//	Not Equal
		,	60
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				return _Compare( l, r ) == -1 ? T : Nil;
			}
		,	"<"		//	Less than
		,	60
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				return _Compare( l, r ) == 1 ? T : Nil;
			}
		,	">"		//	Greater than
		,	60
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				return _Compare( l, r ) != 1 ? T : Nil;
			}
		,	"<="	//	Less equal
		,	60
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				return _Compare( l, r ) != -1 ? T : Nil;
			}
		,	">="	//	Greater equal
		,	60
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				auto list = Z( "Right operand must be List", Cast< List >( r ) );
				V< SP< SliP > > $;
				$.reserve( list->$.size() + 1 );
				$.push_back( l );
				$.insert( $.end(), list->$.begin(), list->$.end() );
				if( Cast< Parallel		>( r ) ) return MS< Parallel	>( $ );
				if( Cast< Sentence		>( r ) ) return MS< Sentence	>( $ );
				if( Cast< Procedure		>( r ) ) return MS< Procedure	>( $ );
				return MS< List >( $ );
			}
		,	","		//	[ l, ...r ]
		,	50
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				auto L = Z( "Illegal operand type: " + l->REPR(), Cast< Bits >( l ) );
				auto R = Z( "Illegal operand type: " + r->REPR(), Cast< Bits >( r ) );
				return MS< Bits >( L->$ & R->$ );
			}
		,	"&"		//	And
		,	40
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				auto L = Z( "Illegal operand type: " + l->REPR(), Cast< Bits >( l ) );
				auto R = Z( "Illegal operand type: " + r->REPR(), Cast< Bits >( r ) );
				return MS< Bits >( L->$ | R->$ );
			}
		,	"|"		//	Or
		,	40
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				auto L = Z( "Illegal operand type: " + l->REPR(), Cast< Bits >( l ) );
				auto R = Z( "Illegal operand type: " + r->REPR(), Cast< Bits >( r ) );
				return MS< Bits >( L->$ ^ R->$ );
			}
		,	"^"		//	Exclusive or
		,	40
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				auto L = Z( "Illegal operand type: " + l->REPR(), Cast< Matrix >( l ) );
				auto R = Z( "Illegal operand type: " + r->REPR(), Cast< Matrix >( r ) );
				
				if( L->nCols == 0 && R->nCols == 0 ) {
					if( L->size != R->size ) _Z( "The number of elements must mutch in Vector." );
					auto $ = (double)0;
					for( size_t _ = 0; _ < L->size; _++ ) $+= L->$[ _ ] * R->$[ _ ];
					return MS< Float >( $ );
				}
				
				auto [ nRows, nCols ] = L->Shape();
				auto [ rNRows, rNCols ] = R->Shape();
				
				if( nCols != rNRows ) _Z( "The number of columns in the left matrix must match the number of rows in the right matrix." );

				V< SP< SliP > >	$( nRows * rNCols );
				for ( size_t row = 0; row < nRows; row++ ) {
					for ( size_t col = 0; col < rNCols; col++ ) {
						double _ = 0.0;
						for ( size_t k = 0; k < nCols; k++ ) _ += (*L)( row, k ) * (*R)( k, col );
						$[ row * rNCols + col ] = MS< Float >( _ );
					}
				}
				return MS< Matrix >( $, rNCols );
			}
		,	"·"		//	Dot product
		,	20
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				return Mul( l, r );
			}
		,	"×"		//	Multiple
		,	20
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				auto L = Z( "Illegal operand type: " + l->REPR(), Cast< Numeric >( l ) );
				auto R = Z( "Illegal operand type: " + r->REPR(), Cast< Numeric >( r ) );
				return MS< Float >( L->Double() / R->Double() );
			}
		,	"÷"		//	Div
		,	20
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				auto L = Z( "Illegal operand type: " + l->REPR(), Cast< Bits >( l ) );
				auto R = Z( "Illegal operand type: " + r->REPR(), Cast< Bits >( r ) );
				return MS< Bits >( L->$ / R->$ );
			}
		,	"/"		//	iDiv
		,	20
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				auto L = Z( "Illegal operand type: " + l->REPR(), Cast< Bits >( l ) );
				auto R = Z( "Illegal operand type: " + r->REPR(), Cast< Bits >( r ) );
				return MS< Bits >( L->$ % R->$ );
			}
		,	"%"		//	Remainder
		,	20
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				if( auto R = Cast< Bits >( r ) ) return Z( "lhs must be List", Cast< List >( l ) )->$.at( R->$ );
				if( auto R = Cast< Name >( r ) ) return Z( "lhs must be Dict", Cast< Dict >( l ) )->$.at( R->$ );
				if( auto R = Cast< Unary >( r ) ) return R->$( C, l );

				Push( l );
				auto $ = Eval( C, r );
				return $;
			}
		,	":"		//	Apply
		,	10
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				auto L = Z( "Illegal operand type: " + l->REPR(), Cast< Matrix >( l ) );
				auto R = Z( "Illegal operand type: " + r->REPR(), Cast< Bits >( r ) );
				L->nCols = R->$;
				return L;
			}
		,	"±"		//	Set nCols
		,	10
		)
	);
	RegisterFunction(
		MS< Infix >(
			[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
				auto L = Z( "Illegal operand type: " + l->REPR(), Cast< Dict >( l ) );
				auto R = Z( "Illegal operand type: " + r->REPR(), Cast< Name >( r ) );
				return L->$[ R->$ ];
			}
		,	"."		//	Dict element
		,	100
		)
	);
	RegisterNumericConstant( MS< NumericConstant >( "∞"			) );
	RegisterNumericConstant( MS< NumericConstant >( "𝑒"			) );
	RegisterNumericConstant( MS< NumericConstant >( "π"			) );
	RegisterNumericConstant( MS< NumericConstant >( "γ"			) );
	RegisterNumericConstant( MS< NumericConstant >( "φ"			) );
	RegisterNumericConstant( MS< NumericConstant >( "log2e"		) );
	RegisterNumericConstant( MS< NumericConstant >( "log10e"	) );
	RegisterNumericConstant( MS< NumericConstant >( "ln2"		) );
	RegisterNumericConstant( MS< NumericConstant >( "ln10"		) );
}

