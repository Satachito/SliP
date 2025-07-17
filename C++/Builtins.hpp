#pragma once

inline static unordered_map< string, SP< SliP > >
Builtins;

inline static vector< SP< NumericConstant > >
NumericConstants = {
	MS< NumericConstant >( "∞" )
,	MS< NumericConstant >( "𝑒" )
,	MS< NumericConstant >( "π" )
,	MS< NumericConstant >( "γ" )
,	MS< NumericConstant >( "φ" )
,	MS< NumericConstant >( "log2e" )
,	MS< NumericConstant >( "log10e" )
,	MS< NumericConstant >( "ln2" )
,	MS< NumericConstant >( "ln10" )
};

inline static vector< SP< Function > >
Functions = {
	MS< Primitive >(
		[]( SP< Context > ) -> SP< SliP > {
			return Nil;
		}
	,	"∅"
	)
,	MS< Primitive >(
		[]( SP< Context > ) -> SP< SliP > {
			auto $ = theStack.back();
			Pop();
			return $;
		}
	,	"@"		//	Stack top
	)
,	MS< Primitive >(
		[]( SP< Context > ) -> SP< SliP > {
			return MS< List >( StackCopy() );
		}
	,	"@@"	//	Stack list
	)
,	MS< Primitive >(
		[]( SP< Context > C ) -> SP< SliP > {
			return MS< Dict >( C->dict );
		}
	,	"¤"		//	make Dict
	)
,	MS< Prefix >(
		[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
			return _;
		}
	,	"'"		//	Quote
	)
,	MS< Prefix >(
		[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
			_Z( _->REPR() );
		}
	,	"¡"		//	Throw
	)
,	MS< Prefix >(
		[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			return Eval( C, Eval( C, _ ) );
		}
	,	"!"		//	Eval
	)
,	MS< Prefix >(
		[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			auto v = Eval( C, _ );
			if( auto bits = Cast< Bits >( v ) ) return MS< Bits >( ~bits->$ );
			_Z( "Illegal operand type" );
		}
	,	"~"		//	Bit not
	)
,	MS< Prefix >(
		[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			return IsNil( Eval( C, _ ) ) ? T : _;
		}
	,	"¬"		//	Logical not
	)
,	MS< Prefix >(
		[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
			auto literal = Cast< Literal >( _ );
			return literal
			?	literal
			:	MS< Literal >( _->REPR() , U'`' )
			;
		}
	,	"¶"		//	Convert to literal
	)
,	MS< Unary >(
		[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
			if( auto list = Cast< List >( _ ) ) return MS< Bits >( list->$.size() );
			if( auto literal = Cast< Literal >( _ ) ) return MS< Bits >( literal->$.length() );
			_Z( "Illegal operand type" );
		}
	,	"#"		//	Number of elements
	)
,	MS< Unary >(
		[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
			if( auto list = Cast< List >( _ ) ) {
				vector< SP< SliP > > $;
				$.reserve( list->$.size() - 1 );
				$.insert( $.end(), list->$.begin() + 1, list->$.end() );
				if( Cast< Matrix		>( _ ) ) return MS< Matrix		>( $ );
				if( Cast< Parallel		>( _ ) ) return MS< Parallel	>( $ );
				if( Cast< Sentence		>( _ ) ) return MS< Sentence	>( $ );
				if( Cast< Procedure	>( _ ) ) return MS< Procedure	>( $ );
				return MS< List >( $ );
			}
			_Z( "Illegal operand type" );
		}
	,	"*"		//	CDR
	)
,	MS< Unary >(
		[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
			auto list = Z( "Illegal operand type", Cast< List >( _ ) );
			auto size = list->$.size();
			Z( "Insufficient list", size );
			return list->$[ size - 1 ];
		}
	,	"$"		//	Last element
	)
,	MS< Unary >(
		[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
			cout << _->REPR() << endl;
			return _;
		}
	,	";"		//	stdout
	)
,	MS< Unary >(
		[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
			cerr << _->REPR() << endl;
			return _;
		}
	,	"¦"		//	stderr
	)

//	:	10	Apply		2 x [ 3, 4 ]:1			2 x 4
//	×	20	Multi/Div	2 + 3 x 4				2 + 12
//	+	30	Plus/Minus	2 | 3 + 4				2 | 7
//	|	40	Binary		2 , 3 | 4				2 < 7
//	,	50	Cons		[ 2 3 ] == 2 , [ 3 ]	[ 2 3 ] == [ 2, 3 ]
//	<	60	Compare		1 || 2 < 3				1 || T
//	∈	60	Member		1 || 2 ∈ [ 1, 2, 3 ]	1 || T
//	||	70	Logical		'a = [ 2 ] || T			a = T
//	?	80	IfElse
//	=	90	Define

,	MS< Infix >(
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
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return C->dict[
				Z( "Only name can be assigned.", Cast< Name >( l ) )->$
			] = r;
		}
	,	"="		//	assign
	,	90
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return Eval(
				C
			,	Z( "Right operand must be two element List.", Cast< List >( r ) )->$[ IsNil( l ) ? 1 : 0 ]
			);
		}
	,	"?"		//	if else
	,	80
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return IsT( l ) ? Eval( C, r ) : Nil;
		}
	,	"¿"		//	if
	,	80
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return ( IsT( l ) && IsT( r ) ) ? T : Nil;
		}
	,	"&&"	//	Logical and
	,	70
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return ( IsT( l ) || IsT( r ) ) ? T: Nil;
		}
	,	"||"	//	Logical or
	,	70
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return ( IsT( l ) ^ IsT( r ) ) ? T: Nil;
		}
	,	"^^"	//	Logical exclusive or
	,	70
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return has(
				Z( "Right operand must be List", Cast< List >( r ) )->$
			,	l
			) ? T : Nil;
		}
	,	"∈"		//	Member of
	,	60
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return has(
				Z( "Left operand must be List", Cast< List >( l ) )->$
			,	r
			) ? T : Nil;
		}
	,	"∋"		//	Includes
	,	60
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return _Compare( l, r ) ? T : Nil;
		}
	,	"=="	//	Equal
	,	60
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return _Compare( l, r ) ? Nil : T;
		}
	,	"<>"	//	Not Equal
	,	60
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return _Compare( l, r ) == -1 ? T : Nil;
		}
	,	"<"		//	Less than
	,	60
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return _Compare( l, r ) == 1 ? T : Nil;
		}
	,	">"		//	Greater than
	,	60
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return _Compare( l, r ) != 1 ? T : Nil;
		}
	,	"<="	//	Less equal
	,	60
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return _Compare( l, r ) != -1 ? T : Nil;
		}
	,	">="	//	Greater equal
	,	60
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto list = Z( "Right operand must be List", Cast< List >( r ) );
			vector< SP< SliP > > $;
			$.reserve( list->$.size() + 1 );
			$.push_back( l );
			$.insert( $.end(), list->$.begin(), list->$.end() );
			if( Cast< Matrix	>( r ) ) return MS< Matrix		>( $ );
			if( Cast< Parallel	>( r ) ) return MS< Parallel	>( $ );
			if( Cast< Sentence	>( r ) ) return MS< Sentence	>( $ );
			if( Cast< Procedure	>( r ) ) return MS< Procedure	>( $ );
			return MS< List >( $ );
		}
	,	","		//	[ l, ...r ]
	,	50
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto L = Cast< Bits >( l ), R = Cast< Bits >( r );
			Z( "Illegal operand type", L && R );
			return MS< Bits >( L->$ & R->$ );
		}
	,	"&"		//	And
	,	40
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto L = Cast< Bits >( l ), R = Cast< Bits >( r );
			Z( "Illegal operand type", L && R );
			return MS< Bits >( L->$ | R->$ );
		}
	,	"|"		//	Or
	,	40
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto L = Cast< Bits >( l ), R = Cast< Bits >( r );
			Z( "Illegal operand type", L && R );
			return MS< Bits >( L->$ ^ R->$ );
		}
	,	"^"		//	Exclusive or
	,	40
	)
,	MS< Infix >(
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
			auto L = Cast< List >( l ), R = Cast< List >( r );
			Z( "Illegal operand type", L && R );

			{	auto L = Cast< Matrix		>( l ), R = Cast< Matrix	>( r );
				if( L && R ) return MS< Matrix		>( L->$ + R->$ );
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
			return MS< List >( L->$ + R->$ );
		}
	,	"+"		//	Plus
	,	30
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			{	auto L = Cast< Bits	>( l ), R = Cast< Bits	>( r );
				if( L && R ) {
					int64_t	$;
					if( !ckd_sub( &$, L->$, R->$ ) ) return MS< Bits >( $ );
				}
			}
			auto L = Cast< Numeric	>( l ), R = Cast< Numeric	>( r );
			Z( "Illegal operand type", L && R );
			return MS< Float >( L->Double() - R->Double() );
		}
	,	"-"		//	Minus
	,	30
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto L = Cast< Matrix >( l ), R = Cast< Matrix >( r );
			Z( "Illegal operand type", L && R );
			auto nColsL = L->NumCols();
			auto nRowsR = R->NumRows();
			if( nColsL != nRowsR ) _Z( "DOT: Matrix size error" );
			if(	nColsL == 0 ) return MS< Float >( Numeric::Dot( L->$, R->$ ) );	//	Vector·Vector

			auto nRowsL = L->NumRows();
			auto nColsR = R->NumCols();

			vector< SP< SliP > >	$( nRowsL * nColsR );
			for ( size_t row = 0; row < nRowsL; row++ ) {
				for ( size_t col = 0; col < nColsR; col++ ) {
					double _ = 0.0;
					for ( size_t k = 0; k < nColsL; k++ ) {
						auto lN = Cast< Numeric >( (*L)( row, k ) );
						auto rN = Cast< Numeric >( (*R)( k, col ) );
						_ += lN->Double() * rN->Double();
					}
					$[ row * nColsR + col ] = MS< Float >( _ );
				}
			}
			return MS< Matrix >( $, nRowsL );
		}
	,	"·"		//	Dot product
	,	20
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return Mul( l, r );
		}
	,	"×"		//	Multiple
	,	20
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto L = Cast< Numeric >( l ), R = Cast< Numeric >( r );
			Z( "Illegal operand type", L && R );
			return MS< Float >( L->Double() / R->Double() );
		}
	,	"÷"		//	Div
	,	20
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto L = Cast< Bits >( l ), R = Cast< Bits >( r );
			Z( "Illegal operand type", L && R );
			return MS< Bits >( L->$ / R->$ );
		}
	,	"/"		//	iDiv
	,	20
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto L = Cast< Bits >( l ), R = Cast< Bits >( r );
			Z( "Illegal operand type", L && R );
			return MS< Bits >( L->$ % R->$ );
		}
	,	"%"		//	Remainder
	,	20
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			{	auto R = Cast< Bits >( r );
				if ( R ) return Z( "lhs must be List", Cast< List >( l ) )->$.at( R->$ );
			}
			{	auto R = Cast< Name >( r );
				if ( R ) return Z( "lhs must be Dict", Cast< Dict >( l ) )->$.at( R->$ );
			}
			{	auto R = Cast< Unary >( r );
				if ( R ) return R->$( C, l );
			}

			Pusher _( l );
			auto $ = Eval( C, r );
			return $;
		}
	,	":"		//	Apply
	,	10
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto L = Cast< Dict >( l );
			auto R = Cast< Name >( r );
			Z( "Illegal operand type", L && R );
			return L->$[ R->$ ];
		}
	,	"."		//	Dict element
	,	100
	)
};

inline static auto
prefixPlus = MS< Prefix >(
	[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
		return Z( "Not numeric", Cast< Numeric >( _ ) );
	}
,	"+"
);

inline static auto
prefixMinus = MS< Prefix >(
	[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
		return Z( "Not numeric", Cast< Numeric >( _ ) )->Negate();
	}
,	"-"
);


inline void
BuildUp() {
	for ( auto const& _: NumericConstants )	Builtins[ _->$ ]		= _;
	for ( auto const& _: Functions )		Builtins[ _->label ]	= _;
}
