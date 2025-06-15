#include	"SliP.hpp"

#include <unordered_set>
#include <unordered_map>

bool	//	Excluding NO-BREAK SPACE i.e. a0, feff
IsBreakingSpace( char32_t _ ) {
	if( _ <= 0x20 ) return true;
	if( 0x7f <= _ && _ < 0xA0 ) return true;
	switch ( _ ) {
	case 0x1680:
	case 0x2000:
	case 0x2001:
	case 0x2002:
	case 0x2003:
	case 0x2004:
	case 0x2005:
	case 0x2006:
	case 0x2007:
	case 0x2008:
	case 0x2009:
	case 0x200A:
	case 0x2028:
	case 0x2029:
	case 0x202F:
	case 0x205F:
	case 0x3000:
		return true;
	default:
		return false;
	}
}

const unordered_set<char32_t>
SoloChars = {
	U'Α'	,U'Β'	,U'Γ'	,U'Δ'	,U'Ε'	,U'Ζ'	,U'Η'	,U'Θ'	,U'Ι'	,U'Κ'	,U'Λ'	,U'Μ'
,	U'Ν'	,U'Ξ'	,U'Ο'	,U'Π'	,U'Ρ'	,U'Σ'	,U'Τ'	,U'Υ'	,U'Φ'	,U'Χ'	,U'Ψ'	,U'Ω'
,	U'α'	,U'β'	,U'γ'	,U'δ'	,U'ε'	,U'ζ'	,U'η'	,U'θ'	,U'ι'	,U'κ'	,U'λ'	,U'μ'
,	U'ν'	,U'ξ'	,U'ο'	,U'π'	,U'ρ'	,U'σ'	,U'τ'	,U'υ'	,U'φ'	,U'χ'	,U'ψ'	,U'ω'
,	U'ς'	//	Σの語尾系
,	U'𝑒'	//	U'\U0001D452'
,	U'∞'
};

const unordered_set<char32_t>
BreakingChars = {
	U'!'	,U'"'	,U'#'	,U'$'	,U'%'	,U'&'	,U'\\'	,U'('	,U')'	,U'*'	,U'+'	,U','	,U'-'	,U'.'	,U'/'	,U':'	,U';'
,	U'<'	,U'='	,U'>'	,U'?'	,U'@'	,U'['	,U']'	,U'^'	,U'`'	,U'{'	,U'|'	,U'}'	,U'~'	,U'¡'	,U'¤'	,U'¦'	,U'§'
,	U'«'	,U'¬'	,U'±'	,U'µ'	,U'¶'	,U'·'	,U'»'	,U'¿'	,U'×'	,U'÷'	,U'∈'	,U'∋'	,U'⊂'	,U'⊃'	,U'∩'	,U'∪'	,U'∅'
};


const vector< SP< Function > >
Builtins = {
	MS< Primitive >(
		[]( SP< Context > ) -> SP< SliP > {
			return Pop();
		}
	,	"@"		//	Stack top
	)
,	MS< Primitive >(
		[]( SP< Context > ) -> SP< SliP > {
			return MS< List >( Stack() );
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
			throw runtime_error( _->REPR() );
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
			if( auto numeric = Cast< Numeric >( v ) ) return MS< Bits >( ~numeric->Bits64() );
			throw runtime_error( "Illegal operand type" );
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
			throw runtime_error( "Illegal operand type" );
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
			throw runtime_error( "Illegal operand type" );
		}
	,	"*"		//	CDR
	)
,	MS< Unary >(
		[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
			if( auto list = Cast< List >( _ ) ) {
				auto size = list->$.size();
				if( !size ) throw runtime_error( "Insufficient list" );
				return list->$[ size - 1 ];
			}
			throw runtime_error( "Illegal operand type" );
		}
	,	"$"		//	Last element
	)
,	MS< Unary >(
		[]( SP< Context >, SP< SliP > _ ) -> SP< SliP > {
			cout << _->REPR() << endl;
			return _;
		}
	,	"."		//	stdout
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
			if( auto dict = Cast< Dict >( l ) ) {
				return Eval( MS< Context >( C, dict->$ ), r );
			}
			throw runtime_error( "Left must be dict." );
		}
	,	"§"		//	Open new context with dict(l) then eval r
	,	100
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			if( auto name = Cast< Name >( l ) ) {
				return C->dict[ name->$ ] = r;
			}
			throw runtime_error( "Only name can be assigned." );
		}
	,	"="		//	assign
	,	90
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			if( auto list = Cast< List >( r ) ) {
				if( list->$.size() == 2 ) return Eval( C, list->$[ IsNil( l ) ? 1 : 0 ] );
			}
			throw runtime_error( "Right operand must be two element List." );
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
	,	"&&"
	,	70
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return ( IsT( l ) || IsT( r ) ) ? T: Nil;
		}
	,	"||"
	,	70
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return ( IsT( l ) ^ IsT( r ) ) ? T: Nil;
		}
	,	"^^"
	,	70
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			if( auto list = Cast< List >( r ) ) return contains( list->$, l ) ? T : Nil;
			throw runtime_error( "Right operand must be List" );
		}
	,	"∈"
	,	60
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			if( auto list = Cast< List >( l ) ) return contains( list->$, r ) ? T : Nil;
			throw runtime_error( "Right operand must be List" );
		}
	,	"∋"
	,	60
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return _Compare( l, r ) ? T : Nil;
		}
	,	"=="
	,	60
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return _Compare( l, r ) ? Nil : T;
		}
	,	"<>"
	,	60
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return _Compare( l, r ) == -1 ? T : Nil;
		}
	,	"<"
	,	60
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return _Compare( l, r ) == 1 ? T : Nil;
		}
	,	">"
	,	60
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return _Compare( l, r ) != 1 ? T : Nil;
		}
	,	"<="
	,	60
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			return _Compare( l, r ) != -1 ? T : Nil;
		}
	,	">="
	,	60
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			if( auto list = Cast< List >( r ) ) {
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
			throw runtime_error( "Right operand must be List" );
		}
	,	","		//	[ l, ...r ]
	,	50
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto lNumeric = Cast< Numeric >( l );
			auto rNumeric = Cast< Numeric >( r );
			if( lNumeric && rNumeric ) return MS< Bits >( lNumeric->Bits64() & rNumeric->Bits64() );
			throw runtime_error( "Illegal operand type" );
		}
	,	"&"
	,	40
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto lNumeric = Cast< Numeric >( l );
			auto rNumeric = Cast< Numeric >( r );
			if( lNumeric && rNumeric ) return MS< Bits >( lNumeric->Bits64() | rNumeric->Bits64() );
			throw runtime_error( "Illegal operand type" );
		}
	,	"|"
	,	40
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto lNumeric = Cast< Numeric >( l );
			auto rNumeric = Cast< Numeric >( r );
			if( lNumeric && rNumeric ) return MS< Bits >( lNumeric->Bits64() ^ rNumeric->Bits64() );
			throw runtime_error( "Illegal operand type" );
		}
	,	"^"
	,	40
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			{	auto L = Cast< Bits	>( l ), R = Cast< Bits	>( r );
				if( L && R ) {
					return (
						( R->$ > 0 && L->$ > numeric_limits<int64_t>::max() - R->$ )
					||	( R->$ < 0 && L->$ < numeric_limits<int64_t>::min() - R->$ )
					) ?	(SP< Numeric >)MS< Float	>( L->Double() + R->Double() )
					:	(SP< Numeric >)MS< Bits	>( L->$ + R->$ )
					;
				}
			}
			{	auto L = Cast< Numeric	>( l ), R = Cast< Numeric	>( r );
				if( L && R ) return MS< Float		>( L->Double() + R->Double() );
			}
			{	auto L = Cast< Literal	>( l ), R = Cast< Literal	>( r );
				if( L && R ) return MS< Literal	>( L->$ + R->$, U'`' );
			}
			{	auto L = Cast< List		>( l ), R = Cast< List		>( r );
				if( L && R ) {
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
			}
			throw runtime_error( "Illegal operand type" );
		}
	,	"+"
	,	30
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			{	auto L = Cast< Bits	>( l ), R = Cast< Bits	>( r );
				if( L && R ) {
					return (
						( R->$ > 0 && L->$ < numeric_limits<int64_t>::max() + R->$ )
					||	( R->$ < 0 && L->$ > numeric_limits<int64_t>::min() + R->$ )
					) ?	(SP< Numeric >)MS< Float	>( L->Double() - R->Double() )
					:	(SP< Numeric >)MS< Bits	>( L->$ - R->$ )
					;
				}
			}
			{	auto L = Cast< Numeric	>( l ), R = Cast< Numeric	>( r );
				if( L && R ) return MS< Float		>( L->Double() + R->Double() );
			}
			throw runtime_error( "Illegal operand type" );
		}
	,	"-"
	,	30
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto L = Cast< Matrix >( l );
			auto R = Cast< Matrix >( r );
			if( L && R ) {
				auto nColsL = L->NumCols();
				auto nRowsR = R->NumRows();
				if( nColsL != nRowsR ) throw runtime_error( "DOT: Matrix size error" );
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
			throw runtime_error( "Operands must be Matrix" );
		}
	,	"·"		//	Dot product
	,	20
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto L = Cast< Numeric >( l );
			auto R = Cast< Numeric >( r );
			if( L && R ) return MS< Float >( L->Double() * R->Double() );
			throw runtime_error( "Illegal operand type" );
		}
	,	"×"
	,	20
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto L = Cast< Numeric >( l );
			auto R = Cast< Numeric >( r );
			if( L && R ) return MS< Float >( L->Double() / R->Double() );
			throw runtime_error( "Illegal operand type" );
		}
	,	"÷"
	,	20
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			auto L = Cast< Numeric >( l );
			auto R = Cast< Numeric >( r );
			if( L && R ) return MS< Bits >( L->Bits64() / R->Bits64() );
			throw runtime_error( "Illegal operand type" );
		}
	,	"%"
	,	20
	)
,	MS< Infix >(
		[]( SP< Context > C, SP< SliP > l, SP< SliP > r ) -> SP< SliP > {
			{	auto R = Cast< Bits >( r );
				if ( R ) {
					auto L = Cast< List >( l );
					if ( !L ) throw runtime_error( "lhs must be List" );
					return L->$.at( R->Bits64() );
				}
			}
			{	auto R = Cast< Name >( r );
				if ( R ) {
					auto L = Cast< Dict >( l );
					if ( !L ) throw runtime_error( "lhs must be List" );
					return L->$.at( R->$ );
				}
			}
			{	auto R = Cast< Unary >( r );
				if ( R ) return R->$( C, l );
			}

			return PushAndEval( C, l, r );
		}
	,	":"		//	Apply
	,	10
	)
};

vector< SP< SliP > >
ReadList( iReader& _, char32_t close ) {
	vector< SP< SliP > > $;
	while ( SP< SliP > slip = Read( _, close ) ) $.push_back( slip );
	return $;
}

SP< Name >
CreateName( iReader& R, char32_t initial ) {

	if( contains( SoloChars, initial ) ) return MS< Name >( string_char32s( vector< char32_t >{ initial } ) );

	auto
	escaped = initial == U'\\';

	vector< char32_t >
	$;

	if( !escaped ) $.push_back( initial );

	while ( R.Avail() ) {
		auto _ = R.Peek();
		if( escaped ) {
			R.Forward();
			escaped = false;
			switch ( _ ) {
			case U'0'	: $.push_back( '\0'	); break;
			case U'f'	: $.push_back( '\f'	); break;
			case U'n'	: $.push_back( '\n'	); break;
			case U'r'	: $.push_back( '\r'	); break;
			case U't'	: $.push_back( '\t'	); break;
			case U'v'	: $.push_back( '\v'	); break;
			default		: $.push_back( _	); break;
			}
		} else {
			if( contains( SoloChars	, _ )		) break;
			if( contains( BreakingChars, _ )	) break;
			R.Forward();
			if( IsBreakingSpace( _ )			) break;
			if( _ == '\\' )	escaped = true;
			else $.push_back( _ );
		}
	}
	return MS<Name>( string_char32s( $ ) );
}
SP< Literal >
CreateLiteral( iReader& R, char32_t terminator ) {
	auto				escaped = false;
	vector< char32_t >	$;
	while ( R.Avail() ) {
		auto _ = R.Read();
		if( escaped ) {
			escaped = false;
			switch ( _ ) {
			case U'0'	: $.push_back( '\0'	); break;
			case U'f'	: $.push_back( '\f'	); break;
			case U'n'	: $.push_back( '\n'	); break;
			case U'r'	: $.push_back( '\r'	); break;
			case U't'	: $.push_back( '\t'	); break;
			case U'v'	: $.push_back( '\v'	); break;
			default		: $.push_back( _	); break;
			}
		} else {
			if( _ == terminator ) return MS< Literal >( string_char32s( $ ), terminator );
			if( _ == '\\' )	escaped = true;
			else $.push_back( _ );
		}
	}
	throw runtime_error( "Unterminated string: " + string_char32s( $ ) );
}

SP< SliP >
Read( iReader& R, char32_t terminator ) {
	while ( R.Avail() ) {
		auto _ = R.Read();
		if( _ == terminator )		return 0;
		if( IsBreakingSpace( _ ) )	continue;
		if( IsDigit( _ ) )	{
			vector< char32_t > ${ _ };
			bool
			dotRead = false;
			while ( R.Avail() ) {
				auto _ = R.Peek();
				if( _ == '.' ) {
					if( dotRead ) break;
					dotRead = true;
					R.Forward();
				} else if( IsDigit( _ ) ) {
					R.Forward();
				} else {
					break;
				}
				$.push_back( _ );
			}
			if( dotRead )	return MS<Float	>( stod( string_char32s( $ ) ) );
			else			return MS<Bits		>( stoi( string_char32s( $ ) ) );
		}
		switch ( _ ) {
		case U']'	:
		case U'⟩'	:
		case U'}'	:
		case U')'	:
		case U'»'	: throw runtime_error( "Detect close parenthesis" );
		case U'['	: return MS< List		>( ReadList( R, U']' ) );
		case U'⟨'	: return MS< Matrix		>( ReadList( R, U'⟩' ) );
		case U'('	: return MS< Sentence	>( ReadList( R, U')' ) );
		case U'{'	: return MS< Procedure	>( ReadList( R, U'}' ) );
		case U'«'	: return MS< Parallel	>( ReadList( R, U'»' ) );
		case U'"'	: return CreateLiteral( R, _ );
		case U'`'	: return CreateLiteral( R, _ );
		default		:
			auto label0 = string_char32s( vector< char32_t >{ _ } );
			auto it0 = find_if( Builtins.begin(), Builtins.end(), [ & ]( SP< Function > _ ){ return _->label == label0; } );
			if( it0 != Builtins.end() ) {
				auto label = label0 + string_char32s( vector< char32_t >{ R.Peek() } );
				auto it = find_if( Builtins.begin(), Builtins.end(), [ & ]( SP< Function > _ ){ return _->label == label; } );
				if( it != Builtins.end() )	return *it;
				else						return *it0;
			} else {
				return CreateName( R, _ );
			}
		}
	}
	return 0;
}

