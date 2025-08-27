#include "SliP.hpp"

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
	lock_guard< mutex >	lock( stackMutex );
	if ( theStack.empty() ) _Z( "Stack underflow" );
	auto $ = theStack.back();
	theStack.pop_back();
	return $;
}
SP< SliP >
Top() {
    lock_guard< mutex > lock( stackMutex );
    if ( theStack.empty() ) _Z( "Stack underflow" );
    return theStack.back();
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
		return Z( "Not a numeric", Cast< Numeric >( _ ) );
	}
,	"+"
);

auto
prefixMinus = MS< Prefix >(
	[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
		return Z( "Not a numeric", Cast< Numeric >( _ ) )->Negate();
	}
,	"-"
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
	{	auto L = Cast< Name >( l ), R = Cast< Name >( r );
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

template < typename T, typename F > void	//	prefix, unary, primitive
Register( F $, string const& label ) { BUILTINS[ label ] = MS< T >( $, label ); }

template < typename F > void
RegisterInfix( F $, string const& label, int priority ) { BUILTINS[ label ] = MS< Infix >( $, label, priority ); }

void
RegisterNumericConstant( string const& label ) { BUILTINS[ label ] = MS< NumericConstant >( label ); }

template	< typename F >	void
RegisterFloatPrefix( string const& label, F f ) {
	BUILTINS[ label ] = MS< Prefix >(
		[ & ]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			return MS< Float >(
				f( Z( "Illegal operand type: " + _->REPR(), Cast< Numeric >( Eval( C, _ ) ) )->Double() )
			);
		}
	,	label
	);
}

SP< SliP >
EvalIsolated( SP< Context > C, SP< SliP > _ ) {
	return Eval( MS< Context >( C ), _ );
}

template < typename F > void
RegisterFloatPairPrefix( string const& label, F f ) {
	BUILTINS[ label ] = MS< Prefix >(
		[ & ]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			auto $ = Z( "Illegal operand type: " + _->REPR(), Cast< List >( _ ) )->$;
			return MS< Float >(
				f(	Z( "Illegal operand type: " + $[ 0 ]->REPR(), Cast< Numeric >( EvalIsolated( C, $[ 0 ] ) ) )->Double()
				,	Z( "Illegal operand type: " + $[ 1 ]->REPR(), Cast< Numeric >( EvalIsolated( C, $[ 1 ] ) ) )->Double()
				)
			);
		}
	,	label
	);
}

template < typename F > void
RegisterFloatListPrefix( string const& label, F f ) {
	BUILTINS[ label ] = MS< Prefix >(
		[ & ]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			return MS< Float >(
				f(	ranges::to< V >(
						project(
							Z( "Illegal operand type: " + _->REPR(), Cast< List >( _ ) )->$
						,	[ & ]( auto const& _ ){ return Z( "Illegal operand type: ", Cast< Numeric >( EvalIsolated( C, _ ) ) )->Double(); }
						)
					)
				)
			);
		}
	,	label
	);
}

auto
Build() {
	
	Register< Primitive >(
		[]( SP< Context > ) -> SP< SliP > {
			return Top();
		}
	,	"@"	//	Stack top
	);
	Register< Primitive >(
		[]( SP< Context > ) -> SP< SliP > {
			return MS< List >( StackCopy() );
		}
	,	"£"	//	Stack list
	);
	Register< Primitive >(
		[]( SP< Context > C ) -> SP< SliP > {
			return MS< Dict >( C->$ );
		}
	,	"¶"		//	make Dict
	);
	Register< Primitive >(
		[]( SP< Context > ) -> SP< SliP > {
			return Nil;
		}
	,	"∅"
	);
	Register< Primitive >(
		[]( SP< Context > C ) -> SP< SliP > {
			static mt19937_64 RANGE{ random_device{}() };
			uniform_real_distribution<double> dist( 0, 1 );
			double $ = dist( RANGE );
			return MS< Float >( $ );
		}
	,	"¤"
	);
	Register< Quote >(
		[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
			return _;
		}
	,	"'"		//	Quote
	);
	Register< Prefix >(
		[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
			_Z( _->REPR() );
		}
	,	"¡"		//	Throw
	);
	Register< Prefix >(
		[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			return MS< Bits >(
				~Z( "Illegal operand type: " + _->REPR(), Cast< Bits >( _ ) )->$
			);
		}
	,	"~"		//	Bit not
	);
	Register< Prefix >(
		[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			return IsNil( _ ) ? T : Nil;
		}
	,	"¬"		//	Logical not
	);

	Register< Unary >(
		[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			return Eval( C, _ );
		}
	,	"!"		//	Eval
	);

	Register< Unary >(
		[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
			if( auto list = Cast< List >( _ ) ) return MS< Bits >( list->$.size() );
			return MS< Bits >( Z( "Illegal operand type", Cast< Literal >( _ ) )->$.length() );
		}
	,	"#"		//	Number of elements
	);
	Register< Unary >(
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
	);
	Register< Unary >(
		[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
			auto list = Z( "Illegal operand type", Cast< List >( _ ) );
			auto size = list->$.size();
			Z( "Insufficient list", size );
			return list->$[ size - 1 ];
		}
	,	"$"		//	Last element
	);
	Register< Unary >(
		[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
			cout << _->REPR() << endl;
			return _;
		}
	,	";"		//	stdout
	);
	Register< Unary >(
		[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
			cerr << _->REPR() << endl;
			return _;
		}
	,	"¦"		//	stderr
	);

	//	INFIX

	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return C->$[
				Z( "Only name can be assigned.", Cast< Name >( l ) )->$
			] = r;
		}
	,	"="		//	assign
	,	0
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto list = Z( "rhs must be a list", Cast< List >( r ) );
			if( list->$.size() != 2 ) _Z( "rhs list must be 2 element" );
			return Eval( C, list->$[ IsNil( l ) ? 1 : 0 ] );
		}
	,	"?"		//	if else
	,	10
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return IsT( l ) ? Eval( C, r ) : Nil;
		}
	,	"¿"		//	if
	,	10
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto list = Z( "Right operand must be List", Cast< List >( r ) );
			for( auto const& _: list->$ ) {
				if( _Compare( _, l ) == 0 ) return T;
			}
			return Nil;
		}
	,	"∈"		//	Member of
	,	30
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto list = Z( "Left operand must be List", Cast< List >( l ) );
			for( auto const& _: list->$ ) {
				if( _Compare( _, r ) == 0 ) return T;
			}
			return Nil;
		}
	,	"∋"		//	Includes
	,	30
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return _Compare( l, r ) ? Nil : T;
		}
	,	"=="	//	Equal
	,	30
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return _Compare( l, r ) ? T : Nil;
		}
	,	"<>"	//	Not Equal
	,	30
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return _Compare( l, r ) == -1 ? T : Nil;
		}
	,	"<"		//	Less than
	,	30
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return _Compare( l, r ) == 1 ? T : Nil;
		}
	,	">"		//	Greater than
	,	30
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return _Compare( l, r ) != 1 ? T : Nil;
		}
	,	"<="	//	Less equal
	,	30
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return _Compare( l, r ) != -1 ? T : Nil;
		}
	,	">="	//	Greater equal
	,	30
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return ( IsT( l ) && IsT( r ) ) ? T : Nil;
		}
	,	"&&"	//	Logical and
	,	40
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return ( IsT( l ) || IsT( r ) ) ? T: Nil;
		}
	,	"||"	//	Logical or
	,	40
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return ( IsT( l ) != IsT( r ) ) ? T: Nil;
		}
	,	"^^"	//	Logical exclusive or
	,	40
	);

	RegisterInfix(
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
	,	50
	);
	RegisterInfix(
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
	);

	RegisterInfix(
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
	,	60
	);
	RegisterInfix(
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
	,	60
	);
	RegisterInfix(
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
	,	70
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return Mul( l, r );
		}
	,	"×"		//	Multiple
	,	70
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto L = Z( "Illegal operand type: " + l->REPR(), Cast< Numeric >( l ) );
			auto R = Z( "Illegal operand type: " + r->REPR(), Cast< Numeric >( r ) );
			return MS< Float >( L->Double() / R->Double() );
		}
	,	"÷"		//	Div
	,	70
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto L = Z( "Illegal operand type: " + l->REPR(), Cast< Bits >( l ) );
			auto R = Z( "Illegal operand type: " + r->REPR(), Cast< Bits >( r ) );
			return MS< Bits >( L->$ / R->$ );
		}
	,	"/"		//	iDiv
	,	70
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto L = Z( "Illegal operand type: " + l->REPR(), Cast< Bits >( l ) );
			auto R = Z( "Illegal operand type: " + r->REPR(), Cast< Bits >( r ) );
			return MS< Bits >( L->$ % R->$ );
		}
	,	"%"		//	Remainder
	,	70
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto L = Z( "Illegal operand type: " + l->REPR(), Cast< Bits >( l ) );
			auto R = Z( "Illegal operand type: " + r->REPR(), Cast< Bits >( r ) );
			return MS< Bits >( L->$ & R->$ );
		}
	,	"&"		//	And
	,	80
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto L = Z( "Illegal operand type: " + l->REPR(), Cast< Bits >( l ) );
			auto R = Z( "Illegal operand type: " + r->REPR(), Cast< Bits >( r ) );
			return MS< Bits >( L->$ | R->$ );
		}
	,	"|"		//	Or
	,	80
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto L = Z( "Illegal operand type: " + l->REPR(), Cast< Bits >( l ) );
			auto R = Z( "Illegal operand type: " + r->REPR(), Cast< Bits >( r ) );
			return MS< Bits >( L->$ ^ R->$ );
		}
	,	"^"		//	Exclusive or
	,	80
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			if( auto R = Cast< Unary >( r ) ) return R->$( C, l );

			Push( l );
			auto $ = Eval( C, r );
			Pop();
			return $;
		}
	,	":"		//	Apply
	,	90
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto L = Z( "Illegal operand type: " + l->REPR(), Cast< Matrix >( l ) );
			auto R = Z( "Illegal operand type: " + r->REPR(), Cast< Bits >( r ) );
			L->nCols = R->$;
			return L;
		}
	,	"±"		//	Set nCols
	,	100
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			{	auto L = Cast< Dict >( l );
				auto R = Cast< Name >( r );
				if( L && R ) {
					if( !L->$.contains( R->$ ) ) _Z( "No such key in dict: " + R->$ );
					return L->$[ R->$ ];
				}
			}
			{	auto L = Cast< List >( l );
				auto R = Cast< Bits >( r );
				if( L && R ) {
					if( R->$ < 0 || L->$.size() <= R->$ ) _Z( "Index out of bounds: " + to_string( R->$ ) );
					return L->$[ R->$ ];
				}
			}
			_Z( "Illegal operand combination" );	//	+ l->REPR() + " :" + r->REPR() );
		}
	,	"."		//	element
	,	100
	);
	RegisterNumericConstant( "∞"		);
	RegisterNumericConstant( "𝑒"		);
	RegisterNumericConstant( "π"		);
	RegisterNumericConstant( "γ"		);
	RegisterNumericConstant( "φ"		);
	RegisterNumericConstant( "log2e"	);
	RegisterNumericConstant( "log10e"	);
	RegisterNumericConstant( "ln2"		);
	RegisterNumericConstant( "ln10"		);

//	String <-> Int Conversion
	Register< Unary >(
		[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			if( auto literal = Cast< Literal >( _ ) ) {
				return MS< Bits >( stoll( literal->$ ) );
			}
			auto list = Z( "Illegal operand type: " + _->REPR(), Cast< List >( _ ) );
			return MS< Bits >(
				stoll(
					Z( "Illegal operand type: " + list->$[ 0 ]->REPR(), Cast< Literal >( list->$[ 0 ] ) )->$
				,	nullptr
				,	(int)Z( "Illegal operand type: " + list->$[ 1 ]->REPR(), Cast< Bits >( list->$[ 1 ] ) )->$
				)
			);
		}
	,	"int"		//	parse Int 
	);
	Register< Unary >(
		[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			auto _Convert = []( uint64_t bits, uint64_t base ) {
				if ( base < 2 || base > 36 ) throw invalid_argument( "base must be 2..36" );
				const char* digits = "0123456789abcdefghijklmnopqrstuvwxyz";
				string $;
				while( bits > 0 ) {
					$ = digits[ bits % base ] + $;
					bits /= base;
				}
				return $;
			};
			//	TODO: when intmin
			auto Convert = [ & ]( int64_t bits, int64_t base = 10 ) {
				return bits == 0
				?	string( "0" )
				:	bits < 0
					?	string( "(-" ) + _Convert( -bits, base ) + string( ")" )
					:	_Convert( bits, base )
				;
			};
			if( auto bits = Cast< Bits >( _ ) ) {
				return MS< Literal >( Convert( bits->$ ), U'`' );
			}
			auto list = Z( "Illegal operand type: " + _->REPR(), Cast< List >( _ ) );
			return MS< Literal >(
				Convert(
					Z( "Illegal operand type: " + list->$[ 0 ]->REPR(), Cast< Bits >( list->$[ 0 ] ) )->$
				,	Z( "Illegal operand type: " + list->$[ 1 ]->REPR(), Cast< Bits >( list->$[ 1 ] ) )->$
				)
			,	U'`'
			);
		}
	,	"str"		//	stringify with hex
	);
	Register< Unary >(
		[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			return MS< Literal >( _->REPR(), U'"' );
		}
	,	"string"	//	stringify
	);
//	MATH EXTENTION
	RegisterFloatPrefix( "abs", []( double _ ) { return abs( _ ); } );

	RegisterFloatPrefix( "acos", []( double _ ) { return acos( _ ); } );
	RegisterFloatPrefix( "acosh", []( double _ ) { return acosh( _ ); } );
	RegisterFloatPrefix( "asin", []( double _ ) { return asin( _ ); } );
	RegisterFloatPrefix( "asinh", []( double _ ) { return asinh( _ ); } );
	RegisterFloatPrefix( "atan", []( double _ ) { return atan( _ ); } );
	RegisterFloatPrefix( "atanh", []( double _ ) { return atanh( _ ); } );
	RegisterFloatPairPrefix( "atan2", []( double y, double x ) { return atan2( y, x ); } );
	RegisterFloatPrefix( "cbrt", []( double _ ) { return cbrt( _ ); } );
	RegisterFloatPrefix( "ceil", []( double _ ) { return ceil( _ ); } );
	RegisterFloatPrefix( "cos", []( double _ ) { return cos( _ ); } );
	RegisterFloatPrefix( "cosh", []( double _ ) { return cosh( _ ); } );
	RegisterFloatPrefix( "exp", []( double _ ) { return exp( _ ); } );
	RegisterFloatPrefix( "floor", []( double _ ) { return floor( _ ); } );
	RegisterFloatListPrefix(
		"hypot"
	,	[]( V< double > const& _ ) {
			double $ = 0.0;
			for ( auto _ : _ ) $ += _ * _;
			return sqrt( $ );
		}
	);
	RegisterFloatPrefix( "log", []( double _ ) { return log( _ ); } );
	RegisterFloatPrefix( "log10", []( double _ ) { return log10( _ ); } );
	RegisterFloatPrefix( "log2", []( double _ ) { return log2( _ ); } );
	RegisterFloatListPrefix( "max", []( V< double > const& _ ) { return *std::max_element( _.begin(), _.end() ); } );
	RegisterFloatListPrefix( "min", []( V< double > const& _ ) { return *std::min_element( _.begin(), _.end() ); } );
	RegisterFloatPairPrefix( "pow", []( double _0, double _1 ) { return pow( _0, _1 ); } );
	RegisterFloatPairPrefix(
		"random"
	,	[]( double _0, double _1 ) {
			static mt19937_64 RANGE{ random_device{}() };
			uniform_real_distribution<double> dist( _0, _1 );
			return dist( RANGE );
		}
	);
	RegisterFloatPrefix( "round", []( double _ ) { return round( _ ); } );
	Register< Prefix >(
		[]( SP< Context > C, SP< SliP > _ ) {
			auto $ = Z( "Illegal operand type: " + _->REPR(), Cast< Numeric >( _ ) )->Double();
			return MS< Bits >( $ == 0 ? 0 : $ < 0 ? -1 : 1 );
		}
	,	"sign"
	);
	RegisterFloatPrefix( "sin", []( double _ ) { return sin( _ ); } );
	RegisterFloatPrefix( "sinh", []( double _ ) { return sinh( _ ); } );
	RegisterFloatPrefix( "sqrt", []( double _ ) { return sqrt( _ ); } );
	RegisterFloatPrefix( "tan", []( double _ ) { return tan( _ ); } );
	RegisterFloatPrefix( "tanh", []( double _ ) { return tanh( _ ); } );
	RegisterFloatPrefix( "trunc", []( double _ ) { return trunc( _ ); } );

//	llround
//	nearbyint
//	rint
//	expm1
//	log1p

//	JSON EXTENSION

	Register< Unary >(
		[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			extern string ToJSON( SP< SliP > );
			return MS< Literal >( ToJSON( _ ), U'`' );
		}
	,	"toJSON"
	);
	Register< Unary >(
		[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			StringReader R( 
				Z( "LHS must be Literal", Cast< Literal >( _ ) )->$
			);
			extern SP< SliP >ByJSON( iReader& );
			return ByJSON( R );
		}
	,	"byJSON"
	);
//	TODO: Graphic Extension
}
/*
is_null()
is_boolean()
is_number()
is_number_integer()
is_number_unsigned()
is_number_float()
is_string()
is_array()
is_object()
*/
