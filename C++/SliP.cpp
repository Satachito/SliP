#include "SliP.hpp"

#ifndef	SLIP_NO_THREADS
#include	<future>
#endif

#if defined( ESP_PLATFORM )
#include	"esp_random.h"
#elif defined( PICO_ON_DEVICE )
#include	"pico/rand.h"
#endif

extern SP< SliP > Eval( SP< Context >, SP< SliP > );

//	std::random_device has no entropy source on a bare chip: libstdc++ reads
//	/dev/urandom, which neither ESP-IDF nor a bare-metal RP2350 provides, and
//	throws when it cannot.  Both chips have a hardware RNG; that is the seed.
static uint64_t
RandomSeed() {
#if defined( ESP_PLATFORM )
	return ( (uint64_t)esp_random() << 32 ) | esp_random();
#elif defined( PICO_ON_DEVICE )
	return get_rand_64();
#else
	return random_device{}();
#endif
}

int
RoundPrecision = 15;

//	Each thread owns its argument stack.  A mutex could not have made this
//	shared: `:` is Push / Eval / Pop, so concurrent applications would keep the
//	container intact while destroying the invariant that Top() is *my*
//	argument.  ∥ seeds a branch with a copy of the spawning thread's stack, so
//	@ inside a branch still reads the argument of the enclosing function.
//
//	Where there are no threads there is one stack, and saying so is not an
//	optimisation: thread_local on bare metal wants __aeabi_read_tp, which nothing
//	provides when the target has no threading library to provide it.
#ifdef	SLIP_NO_THREADS
	#define	SLIP_PER_THREAD
#else
	#define	SLIP_PER_THREAD	thread_local
#endif

SLIP_PER_THREAD V< SP< SliP > >
theStack;

void
Push( SP< SliP > _ ) {
	theStack.push_back( _ );
}
SP< SliP >
Pop() {
	if ( theStack.empty() ) _Z( "Stack underflow" );
	auto $ = theStack.back();
	theStack.pop_back();
	return $;
}
SP< SliP >
Top() {
	if ( theStack.empty() ) _Z( "Stack underflow" );
	return theStack.back();
}
V< SP< SliP > >
StackCopy() {
	return theStack;
}
void
SeedStack( V< SP< SliP > > const& _ ) {
	theStack = _;
}

void
ClearStack() {
	theStack.clear();
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
T = MS< Verum >();

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
			if( L->Size() == R->Size() ) {
				if( L->nCols == 0 && R->nCols == 0 ) {
					for( size_t _ = 0; _ < L->Size(); _++ ) {
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
				return L->Size() < R->Size() ? -1 : 1;
			}
		}
	}
	return l == r ? 0 : l < r ? -1 : 1;
}


UM< string, SP< SliP > > BUILTINS;

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
			static mt19937_64 RANGE{ RandomSeed() };
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
	//	∥ '{ … } — evaluate the elements concurrently, each in its own child
	//	context, collecting the results in source order.  Isolation plus source
	//	ordering makes the value identical to evaluating them one at a time, so
	//	the single-threaded fallback below is the same language, not a dialect.
	//	Branches cannot see each other's bindings; use « » when they must.
	Register< Prefix >(
		[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			auto const&	Ss = Z( "Illegal operand type: " + _->REPR(), Cast< List >( _ ) )->$;
			V< SP< SliP > >	$( Ss.size() );
#ifdef	SLIP_NO_THREADS
			//	Sequential on hosts without usable threads — see SLIP_NO_THREADS
			//	in SliP.hpp for which those are and why.
			for( size_t I = 0; I < Ss.size(); I++ ) $[ I ] = Eval( MS< Context >( C ), Ss[ I ] );
#else
			auto	seed = StackCopy();
			V< future< SP< SliP > > >	fs;
			fs.reserve( Ss.size() );
			for( auto const& S: Ss ) fs.push_back(
				async(
					launch::async
				,	[ C, &seed, S ]() {
						SeedStack( seed );
						return Eval( MS< Context >( C ), S );
					}
				)
			);
			//	Collected in source order, so a failing branch reports the
			//	earliest error rather than whichever thread lost the race.
			for( size_t I = 0; I < fs.size(); I++ ) $[ I ] = fs[ I ].get();
#endif
			return MS< List >( $ );
		}
	,	"∥"		//	Parallel evaluation
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
	,	true	//	right-associative: 'a = 'b = 2
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
	RegisterLazyInfix(
		[]( SP< Context > C, SP< SliP > l, function< SP< SliP >() > const& r ) -> SP< SliP > {
			return IsT( l ) ? Eval( C, r() ) : Nil;
		}
	,	"¿"		//	if — rhs is not touched when lhs is Nil
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
	//	Logical operators bind looser than comparisons (priority 20 < 30) so
	//	`x > 0 && y > 0` reads as ( x > 0 ) && ( y > 0 ).
	RegisterLazyInfix(
		[]( SP< Context > C, SP< SliP > l, function< SP< SliP >() > const& r ) -> SP< SliP > {
			return IsT( l ) ? ( IsT( r() ) ? T : Nil ) : Nil;
		}
	,	"&&"	//	Logical and — short-circuits
	,	20
	);
	RegisterLazyInfix(
		[]( SP< Context > C, SP< SliP > l, function< SP< SliP >() > const& r ) -> SP< SliP > {
			return IsT( l ) ? T : ( IsT( r() ) ? T : Nil );
		}
	,	"||"	//	Logical or — short-circuits
	,	20
	);
	RegisterInfix(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return ( IsT( l ) != IsT( r ) ) ? T: Nil;
		}
	,	"^^"	//	Logical exclusive or
	,	20
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
	,	true	//	right-associative: 1 , 2 , [ 3 ] == [ 1 2 3 ]
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
				if( L->Size() != R->Size() ) _Z( "The number of elements must mutch in Vector." );
				auto $ = (double)0;
				for( size_t _ = 0; _ < L->Size(); _++ ) $+= L->$[ _ ] * R->$[ _ ];
				return MS< Float >( $ );
			}

			auto [ nRows, nCols ] = L->Shape();
			auto [ rNRows, rNCols ] = R->Shape();

			if( nCols != rNRows ) _Z( "The number of columns in the left matrix must match the number of rows in the right matrix." );

			V< double >	$( nRows * rNCols );
			for ( size_t row = 0; row < nRows; row++ ) {
				for ( size_t col = 0; col < rNCols; col++ ) {
					double _ = 0.0;
					for ( size_t k = 0; k < nCols; k++ ) _ += (*L)( row, k ) * (*R)( k, col );
					$[ row * rNCols + col ] = _;
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
			if( auto L = Cast< List >( l ) ) {		//	list : index -> element, Nil when out of range
				if( auto R = Cast< Numeric >( r ) ) {
					auto $ = R->Double();
					auto i = (int64_t)$;
					return ( i == $ && 0 <= i && i < (int64_t)L->$.size() ) ? L->$[ i ] : Nil;
				}
			}
			if( auto L = Cast< Dict >( l ) ) {		//	dict : key -> value, Nil when missing
				if( Cast< Name >( r ) || Cast< Literal >( r ) ) {
					auto K = Cast< Name >( r ) ? Cast< Name >( r )->$ : Cast< Literal >( r )->$;
					return L->$.contains( K ) ? L->$[ K ] : Nil;
				}
			}
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
			return MS< Matrix >( L->$, R->$ );	//	New value: never reshape the bound operand
		}
	,	"±"		//	Shape as nCols columns
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
			_Z( "Illegal operand combination: " + l->REPR() + " . " + r->REPR() );
		}
	,	"."		//	element
	,	100
	);
	RegisterNumericConstant( "∞"		);
	RegisterNumericConstant( "𝑒"		);
	RegisterNumericConstant( "π"		);
	BUILTINS[ "inf" ]	= MS< NumericConstant >( "∞" );
	BUILTINS[ "euler" ]	= MS< NumericConstant >( "𝑒" );
	BUILTINS[ "pi" ]	= MS< NumericConstant >( "π" );
	RegisterNumericConstant( "γ"		);
	RegisterNumericConstant( "φ"		);
	RegisterNumericConstant( "log2e"	);
	RegisterNumericConstant( "log10e"	);
	RegisterNumericConstant( "ln2"		);
	RegisterNumericConstant( "ln10"		);

//	String <-> Int Conversion
	static auto
	StoInt = []( string const& digits, int radix = 10 ) -> int64_t {
		try {
			return stoll( digits, nullptr, radix );
		} catch( out_of_range const& ) {
			throw out_of_range( "integer out of range" );
		} catch( invalid_argument const& ) {
			throw invalid_argument( "invalid integer" );
		}
	};
	Register< Unary >(
		[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			if( auto literal = Cast< Literal >( _ ) ) {
				return MS< Bits >( StoInt( literal->$ ) );
			}
			auto list = Z( "Illegal operand type: " + _->REPR(), Cast< List >( _ ) );
			auto radix = (int)Z( "Illegal operand type: " + list->$[ 1 ]->REPR(), Cast< Bits >( list->$[ 1 ] ) )->$;
			if( radix < 2 || radix > 36 ) throw invalid_argument( "base must be 2..36" );
			return MS< Bits >(
				StoInt(
					Z( "Illegal operand type: " + list->$[ 0 ]->REPR(), Cast< Literal >( list->$[ 0 ] ) )->$
				,	radix
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
			static mt19937_64 RANGE{ RandomSeed() };
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

