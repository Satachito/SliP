#include	<iostream>

#include	"SliP.hpp"

inline vector< UI8 >
MakeBigInteger( string const& _, UI8 radix ) {
	auto
	ToUI8 = []( UI1 _ ) {
		switch ( _ ) {
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
			return _ - '0';
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
			return _ - 'a' + 10;
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
			return _ - 'A' + 10;
		}
		throw "eh?: Input must be sanitalized.";
	};

	UnicodeReader ur( _ );
	vector< UI8 >	$;
	UI8	a = 0;
	try {
		while ( true ) {
			auto preA = a;
			a += ToUI8( ur.Get() ) * radix;
			if ( a < preA ) {
				UI8	_ = 0;
				while ( _ < $.size() ) if ( ++$[ _++ ] ) break;
				if ( _ == $.size() ) $.emplace_back( 1 );
			}
		}
	} catch ( ... ) {
		$.insert( $.begin(), a );
	}
	return $;
}
inline string
BigHexString( vector< UI8 > const& _ ) {
	string $ = "0x";
	for ( auto& _: _ ) $ += EncodeHex( _ );
	return $;
}
//inline vector< UI8 >
//BigRemainder( vector< UI8 > const& _, UI8 divisor ) {
//	return vector< UI8 >{ 0 };
//}
//inline string
//BigDecString ( vector< UI8 > const& _, UI8 radix ) {
//	string $;
//	auto buffer = _;
//	while ( buffer.size() ) {
//		$.insert( $.begin(),  '0' + BigDiv( buffer, 10 )[ 0 ];
//	}
//	return $;
//}

#define		LITERAL_BIT			0x08

#define		_STRING				0x00
#define		STRING_0			( _STRING	+ 0 + LITERAL_BIT )
#define		STRING_1			( _STRING	+ 1 + LITERAL_BIT )
#define		STRING_2			( _STRING	+ 2 + LITERAL_BIT )
#define		STRING_3			( _STRING	+ 3 + LITERAL_BIT )
#define		STRING_4			( _STRING	+ 4 + LITERAL_BIT )
#define		STRING				( _STRING	+ 7 + LITERAL_BIT )

#define		_NUMERIC			0x10
#define		DOUBLE				( _NUMERIC	+ 0 + LITERAL_BIT )
#define		INT_64				( _NUMERIC	+ 1 + LITERAL_BIT )
#define		POSITIVE_BIGINT		( _NUMERIC	+ 2 + LITERAL_BIT )
#define		NEGATIVE_BIGINT		( _NUMERIC	+ 3 + LITERAL_BIT )

#define		_CHAR				0x20
#define		UNICODE				( _CHAR		+ 0 + LITERAL_BIT )
#define		ASCII				( _CHAR		+ 1 + LITERAL_BIT )

#define		_MATRIX				0x30
#define		MATRIX_SINGLE		( _MATRIX	+ 0 + LITERAL_BIT )
#define		MATRIX_DOUBLE		( _MATRIX	+ 1 + LITERAL_BIT )

#define		_DICT				0x40
#define		STRING_KEY_DICT		( _DICT		+ 0 + LITERAL_BIT )

#define		_LIST				0x50
#define		LIST_LITERAL		( _LIST		+ 0 + LITERAL_BIT )
#define		LIST_PARALLEL		( _LIST		+ 1 )
#define		LIST_PROCEDURE		( _LIST		+ 2 )
#define		LIST_SENTENCE		( _LIST		+ 3 )

#define		_NAME				0x60
#define		NAME_1				( _NAME		+ 1 )
#define		NAME_2				( _NAME		+ 2 )
#define		NAME_3				( _NAME		+ 3 )
#define		NAME_4				( _NAME		+ 4 )
#define		NAME				( _NAME		+ 7 )

#define		_FUNCTION			0x70
#define		FUNCTION_PRIMITIVE	( _FUNCTION	+ 0 )
#define		FUNCTION_PREFIX		( _FUNCTION	+ 1 )
#define		FUNCTION_UNARY		( _FUNCTION	+ 2 )
#define		FUNCTION_INFIX		( _FUNCTION	+ 3 )

template	< typename T >	T*	//	//	//	//	//	//	//	//	CONSTRUCTIVE
Clone( T const* _, UI8 size ) {
	auto $ = new T[ size ];
	while ( size-- ) $[ size ] = _[ size ];
	return $;
}

template	< typename T >	T*	//	//	//	//	//	//	//	//	CONSTRUCTIVE
CloneData( vector< T > const& _ ) {
	return Clone( _.data(), _.size() );
}

static	int	counter = 0;

void
Delete( Object* );

struct
Object {

	UI1			_;

	struct
	TwoUI4 {
		UI4 _1;
		UI4 _2;
		TwoUI4();
		TwoUI4( UI4 _1, UI4 _2 ) : _1( _1 ), _2( _2 ) {}
	};
	union
	TheUnion1 {
		double	d;
		long	i;
		TwoUI4	u;
		char	l_p[ 8 ];	//	Label and Priority
		TheUnion1() 						{}
		TheUnion1( double	_ )				: d( _ ) {}
		TheUnion1( long		_ )				: i( _ ) {}
		TheUnion1( UI4 _1, UI4 _2 = 0 )		: u( _1, _2 ) {}
		TheUnion1( char const* _ )			{ A( strlen( _ ) < 7 ); strcpy( l_p, _ ); }
		TheUnion1( char const* _, UI1 $ )	{ A( strlen( _ ) < 7 ); strcpy( l_p, _ ); l_p[ 7 ] = $; }
		
	}			_1;
	union
	TheUnion2 {
		TwoUI4					u;
		Object**				pObjectP;
		UTF32*					pUTF32;
		UI8*					pUI8;
		float*					pSingle;
		double*					pDouble;
		map< string, Object* >*	pMap;
		void*					p;
		TheUnion2( void* _ = 0 )		: p( _ ) {}
		TheUnion2( UI4 _1, UI4 _2 = 0 )	: u( _1, _2 ) {}

	}			_2;

#ifdef DEBUG
	Object( Object const& $ ) { throw "eh?"; }
#endif
	Object( UI1 _ )									: _( _ )								{ counter++; }
	Object( UI1 _, double	d )						: _( _ ), _1( d )						{ counter++; }
	Object( UI1 _, long		i )						: _( _ ), _1( i )						{ counter++; }
	Object( UI1 _, UI4 u1 )							: _( _ ), _1( u1 )						{ counter++; }
	Object( UI1 _, UI4 u1, UI4 u2 )					: _( _ ), _1( u1, u2 )					{ counter++; }
	Object( UI1 _, UI4 u1, UI4 u2, UI4 u3 )			: _( _ ), _1( u1, u2 )	, _2( u3 )		{ counter++; }
	Object( UI1 _, UI4 u1, UI4 u2, UI4 u3, UI4 u4 )	: _( _ ), _1( u1, u2 )	, _2( u3, u4 )	{ counter++; }
	Object( UI1 _, UI4 u1, UI4 u2, void* p )		: _( _ ), _1( u1, u2 )	, _2( p )		{ counter++; }
	Object( UI1 _, long i, void* p )				: _( _ ), _1( i )		, _2( p )		{ counter++; }
	Object( UI1 _, void* p )						: _( _ ), _1()			, _2( p )		{ counter++; }
	Object(
		UI1 _
	,	Object*( *f )( SliP* )
	,	char const* label
	) : _( _ ), _1( label ), _2( (void*)f )													{ counter++; }
	Object(
		UI1 _
	,	Object*( *f )( SliP*, Object* )
	,	char const* label
	) : _( _ ), _1( label ), _2( (void*)f )													{ counter++; }
	Object(
		UI1 _
	,	Object*( *f )( SliP*, Object*, Object* )
	,	char const* label
	,	UI1 priority
	) : _( _ ), _1( label, priority ), _2( (void*)f )										{ counter++; }
	~
	Object() {
		--counter;
		switch ( _ ) {
		case POSITIVE_BIGINT	:
		case NEGATIVE_BIGINT	: delete[]	UI8P()		; break;
		case STRING				:
		case NAME				: delete[]	UTF32P()	; break;
		case MATRIX_SINGLE		: delete[]	SingleP()	; break;
		case MATRIX_DOUBLE		: delete[]	DoubleP()	; break;
		case STRING_KEY_DICT	: delete	MapP()		; break;
		case LIST_LITERAL		:
		case LIST_PARALLEL		:
		case LIST_PROCEDURE		:
		case LIST_SENTENCE		:
			{	auto _ = Integer();
				auto $ = ObjectPP();
				while ( _-- ) Delete( $[ _ ] );
				delete[] $;
			}
			break;
		}
	}

	bool
	IsLiteral	() { return _ & LITERAL_BIT				; }

	bool
	IsString	() { return ( _ & 0xf0 ) == _STRING		; }
	bool
	IsNumeric	() { return ( _ & 0xf0 ) == _NUMERIC	; }
	bool
	IsChar		() { return ( _ & 0xf0 ) == _CHAR		; }
	bool
	IsMatrix	() { return ( _ & 0xf0 ) == _MATRIX		; }
	bool
	IsDict		() { return ( _ & 0xf0 ) == _DICT		; }
	bool
	IsList		() { return ( _ & 0xf0 ) == _LIST		; }
	bool
	IsName		() { return ( _ & 0xf0 ) == _NAME		; }
	bool
	IsFunction	() { return ( _ & 0xf0 ) == _FUNCTION	; }

	double
	Double		() { return _1.d; }
	long
	Integer		() { return _1.i; }
	UI4
	UI4_1		() { return _1.u._1; }
	UI4
	UI4_2		() { return _1.u._2; }
	UI4
	UI4_3		() { return _2.u._1; }
	UI4
	UI4_4		() { return _2.u._2; }
	string
	Label		() { return string( _1.l_p ); }
	UI8
	Priority	() { return _1.l_p[ 7 ]; }
	Object**
	ObjectPP	() { return _2.pObjectP; }
	UTF32*
	UTF32P		() { return _2.pUTF32; }
	UI8*
	UI8P		() { return _2.pUI8; }
	float*
	SingleP		() { return _2.pSingle; }
	double*
	DoubleP		() { return _2.pDouble; }
	map< string, Object* >*
	MapP		() { return _2.pMap; }
	void*
	P			() { return _2.p; }

	double
	Number() {
		switch ( _ ) {
		case DOUBLE:	return Double();
		case INT_64:	return Integer();
	//	TODO:
	//	case POSITIVE_BIGINT	: return ;
	//	case NEGATIVE_BIGINT	: return ;
		}
		throw "Not a number";
	}
	string
	GenericListString() {
		auto size = Integer();
		auto head = ObjectPP();
		string $;
		for ( UI8 _ = 0; _ < size; _++ ) {
			$ += ' ';
			$ += *head[ _ ];
		}
		return $ + ' ';
	}
	
	operator
	string() {
		switch ( _ ) {
		case INT_64:
			return to_string( Integer() );
		case DOUBLE:
			{	auto _ = Double();
				return _ == ( 1.0 / 0.0 )
				?	"∞"
				:	_ == -( 1.0 / 0.0 )
					?	"-∞"
					:	EncodeDouble( _ )
				;
			}
		case POSITIVE_BIGINT:
			return string( "POSITIVE BIGINT, # of usigned long:" ) + to_string( Integer() );
		case NEGATIVE_BIGINT:
			return string( "NEGATIVE BIGINT, # of usigned long:" ) + to_string( Integer() );
		case UNICODE:
			return UTF8( UI4_1() );

		case STRING_0:
			return "\"\"";
		case STRING_1:
			return '"' + UTF8( UI4_1() ) + '"';
		case STRING_2:
			return '"' + UTF8( UI4_1() ) + UTF8( UI4_2() ) + '"';
		case STRING_3:
			return '"' + UTF8( UI4_1() ) + UTF8( UI4_2() ) + UTF8( UI4_3() ) + '"';
		case STRING_4:
			return '"' + UTF8( UI4_1() ) + UTF8( UI4_2() ) + UTF8( UI4_3() ) + UTF8( UI4_4() ) + '"';
		case STRING:
			{	auto _ = UTF32P();
				auto size = Integer();
				return '"' + UTF8( vector< UTF32 >( _, _ + size ) ) + '"';
			}
		case NAME_1:
			return UTF8( UI4_1() );
		case NAME_2:
			return UTF8( UI4_1() ) + UTF8( UI4_2() );
		case NAME_3:
			return UTF8( UI4_1() ) + UTF8( UI4_2() ) + UTF8( UI4_3() );
		case NAME_4:
			return UTF8( UI4_1() ) + UTF8( UI4_2() ) + UTF8( UI4_3() ) + UTF8( UI4_4() );
		case NAME:
			{	auto _ = UTF32P();
				auto size = Integer();
				return UTF8( vector< UTF32 >( _, _ + size ) );
			}
		case FUNCTION_PRIMITIVE:
		case FUNCTION_PREFIX:
		case FUNCTION_UNARY:
		case FUNCTION_INFIX:
			return Label();
//			return Label() + '(' + to_string( Priority() ) + ')';
		case LIST_LITERAL:
			return '[' + GenericListString() + ']';
		case LIST_PARALLEL:
			return "«" + GenericListString() + "»";
		case LIST_PROCEDURE:
			return '{' + GenericListString() + '}';
		case LIST_SENTENCE:
			return '(' + GenericListString() + ')';

		case MATRIX_SINGLE:
			{	string ${ "⟨" };
				auto _ = Integer();
				auto nR = _ >> 32;
				auto nC = _ & 0xffffffff;
				for ( UI8 _R = 0; _R < nR; _R++ ) {
					auto _ = _R * nC;
					for ( UI8 _C = 0; _C < nC; _C++ ) {
						$ += '\t';
						$ += SingleP()[ _ + _C ];
					}
					$ += '\n';
				}
				return $ + "⟩";
			}

		case MATRIX_DOUBLE:
			{	string ${ "⟪" };
				auto _ = Integer();
				auto nR = _ >> 32;
				auto nC = _ & 0xffffffff;
				for ( UI8 _R = 0; _R < nR; _R++ ) {
					auto _ = _R * nC;
					for ( UI8 _C = 0; _C < nC; _C++ ) {
						$ += '\t';
						$ += DoubleP()[ _ + _C ];
					}
					$ += '\n';
				}
				return $ + "⟫";
			}

		case STRING_KEY_DICT:
			{	string ${ "⟦" };
				for ( auto& _: *MapP() ) {
					$ += '\t';
					$ += _.first + '\t' + (string)*_.second + "\n";
				}
				return $ + "⟧";
			}
		}

		throw "eh?";
	}
};


Object*
Eval( SliP* s, Object* _ ) {
	return _;
}

bool
IsT( Object* _ ) {
	return !_->IsList() || _->Integer();
}

Object*
MakeString( vector< UTF32 > const& $ ) {
	switch ( $.size() ) {
	case 0	: return new Object( STRING_0 );
	case 1	: return new Object( STRING_1	, $[ 0 ] );
	case 2	: return new Object( STRING_2	, $[ 0 ], $[ 1 ] );
	case 3	: return new Object( STRING_3	, $[ 0 ], $[ 1 ], $[ 2 ] );
	case 4	: return new Object( STRING_4	, $[ 0 ], $[ 1 ], $[ 2 ], $[ 3 ] );
	default	: return new Object( STRING		, $.size(), CloneData( $ ) );
	}
}

typedef Object* (*Primitive	)( SliP* );
typedef Object* (*Prefix	)( SliP*, Object* );
typedef Object* (*Unary		)( SliP*, Object* );
typedef Object* (*Infix		)( SliP*, Object*, Object* );

Object*
Plus( SliP* s, Object* l, Object* r ) {
	if ( l->_ == INT_64	) {
		switch( r->_ ) {
		case INT_64: return new Object( INT_64, l->Integer() + r->Integer() );	//	TODO: Overflow
		case DOUBLE: return new Object( DOUBLE, l->Integer() + r->Double() );
		//	TODO: BIGINT
		}
	} else if ( l->_ == DOUBLE	) {
		switch( r->_ ) {
		case INT_64: return new Object( INT_64, l->Double() + r->Integer() );
		case DOUBLE: return new Object( DOUBLE, l->Double() + r->Double() );
		//	TODO: BIGINT
		}
	} else if ( l->IsString() && r->IsString() ) {
		UI8 lSize = l->_ & 3;
		if ( lSize == 7 ) lSize = l->Integer();
		UI8 rSize = r->_ & 3;
		if ( rSize == 7 ) rSize = r->Integer();
		auto size = lSize + rSize;
		auto $ = vector< UTF32 >( size );
		switch ( lSize ) {
		case 0:
			break;
		case 4:	$[ 3 ] = l->UI4_4();
		case 3:	$[ 2 ] = l->UI4_3();
		case 2:	$[ 1 ] = l->UI4_2();
		case 1:	$[ 0 ] = l->UI4_1();
			break;
		default:
			{	auto dP = l->DoubleP();
				for ( UI8 _ = 0; _ < lSize; _++ ) $[ _ ] = dP[ _ ];
			}
		}
		switch ( rSize ) {
		case 0:
			break;
		case 4:	$[ lSize + 3 ] = r->UI4_4();
		case 3:	$[ lSize + 2 ] = r->UI4_3();
		case 2:	$[ lSize + 1 ] = r->UI4_2();
		case 1:	$[ lSize + 0 ] = r->UI4_1();
			break;
		default:
			{	auto dP = r->DoubleP();
				for ( UI8 _ = 0; _ < lSize; _++ ) $[ lSize + _ ] = dP[ _ ];
			}
		}
		return MakeString( $ );
	} else if ( l->_ == r->_ && l->IsList() && r->IsList() ) {
		auto lSize = l->Integer();
		auto rSize = r->Integer();
		auto size = lSize + rSize;
		auto $ = new Object*[ size ];
		for ( UI8 _ = 0; _ < lSize; _++ ) $[ _ ] = l->ObjectPP()[ _ ];
		for ( UI8 _ = 0; _ < rSize; _++ ) $[ lSize + _ ] = r->ObjectPP()[ _ ];
		return new Object( l->_, size, $ );
	}
	throw "Illegal type combination";
}

//#define		_STRING				0x00
//#define		STRING_0			( _STRING	+ 0 + LITERAL_BIT )
//#define		STRING_1			( _STRING	+ 1 + LITERAL_BIT )
//#define		STRING_2			( _STRING	+ 2 + LITERAL_BIT )
//#define		STRING_3			( _STRING	+ 3 + LITERAL_BIT )
//#define		STRING_4			( _STRING	+ 4 + LITERAL_BIT )
//#define		STRING				( _STRING	+ 7 + LITERAL_BIT )
//
//#define		_NUMERIC			0x10
//#define		DOUBLE				( _NUMERIC	+ 0 + LITERAL_BIT )
//#define		INT_64				( _NUMERIC	+ 1 + LITERAL_BIT )
//#define		POSITIVE_BIGINT		( _NUMERIC	+ 2 + LITERAL_BIT )
//#define		NEGATIVE_BIGINT		( _NUMERIC	+ 3 + LITERAL_BIT )
//
//#define		_CHAR				0x20
//#define		UNICODE				( _CHAR		+ 0 + LITERAL_BIT )
//#define		ASCII				( _CHAR		+ 1 + LITERAL_BIT )
//
//#define		_MATRIX				0x30
//#define		MATRIX_SINGLE		( _MATRIX	+ 0 + LITERAL_BIT )
//#define		MATRIX_DOUBLE		( _MATRIX	+ 1 + LITERAL_BIT )
//
//#define		_DICT				0x40
//#define		STRING_KEY_DICT		( _DICT		+ 0 + LITERAL_BIT )
//
//#define		_LIST				0x50
//#define		LIST_LITERAL		( _LIST		+ 0 + LITERAL_BIT )
//#define		LIST_PARALLEL		( _LIST		+ 1 )
//#define		LIST_PROCEDURE		( _LIST		+ 2 )
//#define		LIST_SENTENCE		( _LIST		+ 3 )
//
//#define		_NAME				0x60
//#define		NAME_1				( _NAME		+ 1 )
//#define		NAME_2				( _NAME		+ 2 )
//#define		NAME_3				( _NAME		+ 3 )
//#define		NAME_4				( _NAME		+ 4 )
//#define		NAME				( _NAME		+ 7 )
//
//#define		_FUNCTION			0x70
//#define		FUNCTION_PRIMITIVE	( _FUNCTION	+ 0 )
//#define		FUNCTION_PREFIX		( _FUNCTION	+ 1 )
//#define		FUNCTION_UNARY		( _FUNCTION	+ 2 )
//#define		FUNCTION_INFIX		( _FUNCTION	+ 3 )
bool
EQ( Object* p, Object* q ) {
	if ( p->IsNumeric() ) {
		switch ( p->_ ) {
		case DOUBLE:
			switch ( q->_ ) {
			case DOUBLE: return p->Double() == q->Double();
			case INT_64: return p->Double() == q->Integer();
			}
			return false;
		case INT_64:
			switch ( q->_ ) {
			case DOUBLE: return p->Integer() == q->Double();
			case INT_64: return p->Integer() == q->Integer();
			}
			return false;
		case POSITIVE_BIGINT:
		case NEGATIVE_BIGINT:
			{	if ( p->_ != q->_ ) return false;
				auto _ = p->Integer();
				if ( _ != q->Integer() ) return false;
				auto pP = p->UI8P();
				auto pQ = q->UI8P();
				while ( _-- ) if ( pP[ _ ] != pQ[ _ ] ) return false;
				return true;
			}
		}
		throw "eh?";
	} else {
		if ( p->_ != q->_ ) return false;

		switch ( p->_ ) {
		
		case FUNCTION_PRIMITIVE:
		case FUNCTION_PREFIX:
		case FUNCTION_UNARY:
		case FUNCTION_INFIX:
			return p->P() == q->P();

		case LIST_LITERAL:
		case LIST_PARALLEL:
		case LIST_PROCEDURE:
		case LIST_SENTENCE:
			{	auto _ = p->Integer();
				if ( _ != q->Integer() ) return false;
				auto pP = p->ObjectPP();
				auto pQ = q->ObjectPP();
				while ( _-- ) if ( !EQ( pP[ _ ], pQ[ _ ] ) ) return false;
				return true;
			}
		case STRING_0:	return true;
		case UNICODE:
		case ASCII:
		case NAME_1:
		case STRING_1:	return p->UI4_1() == q->UI4_1();
		case NAME_2:
		case STRING_2:	return p->UI4_1() == q->UI4_1() && p->UI4_2() == q->UI4_2();
		case NAME_3:
		case STRING_3:	return p->UI4_1() == q->UI4_1() && p->UI4_2() == q->UI4_2() && p->UI4_3() == q->UI4_3();
		case NAME_4:
		case STRING_4:	return p->UI4_1() == q->UI4_1() && p->UI4_2() == q->UI4_2() && p->UI4_3() == q->UI4_3() && p->UI4_4() == q->UI4_4();
		case NAME:
		case STRING:
			{	auto _ = p->Integer();
				if ( _ != q->Integer() ) return false;
				auto pP = p->UTF32P();
				auto pQ = q->UTF32P();
				while ( _-- ) if ( pP[ _ ] != pQ[ _ ] ) return false;
				return true;
			}
		case STRING_KEY_DICT:
			{	if ( p->MapP()->size() != q->MapP()->size() ) return false;
				for ( auto& _: *p->MapP() ) {
					auto map = q->MapP();
					auto $ = map->find( _.first );
					if ( $ == map->end() ) return false;
					if ( !EQ( _.second, $->second ) ) return false;
				}
				return true;
			}
		case MATRIX_SINGLE:
			{	auto _ = p->Integer();
				if ( _ != q->Integer() ) return false;
				auto pP = p->SingleP();
				auto pQ = q->SingleP();
				while ( _-- ) if ( pP[ _ ] != pQ[ _ ] ) return false;
				return true;
			}
		case MATRIX_DOUBLE:
			{	auto _ = p->Integer();
				if ( _ != q->Integer() ) return false;
				auto pP = p->DoubleP();
				auto pQ = q->DoubleP();
				while ( _-- ) if ( pP[ _ ] != pQ[ _ ] ) return false;
				return true;
			}
		}
		throw "eh?";
	}
}

//	//	//	//	//	//	//	//	READ
bool
IsMathSymbol( UTF32 _ ) {

	static vector< UTF32 > $ {
		0x391, 0x392, 0x393, 0x394, 0x395, 0x396, 0x397, 0x398, 0x399, 0x39a, 0x39b, 0x39c, 0x39d, 0x39e, 0x39f
	,	0x3a0, 0x3a1,        0x3a3, 0x3a4, 0x3a5, 0x3a6, 0x3a7, 0x3a8, 0x3a9
	,	0x3b1, 0x3b2, 0x3b3, 0x3b4, 0x3b5, 0x3b6, 0x3b7, 0x3b8, 0x3b9, 0x3ba, 0x3bb, 0x3bc, 0x3bd, 0x3be, 0x3bf
	,	0x3c0, 0x3c1, 0x3c2, 0x3c3, 0x3c4, 0x3c5, 0x3c6, 0x3c7, 0x3c8, 0x3c9
	,	0x221e
	//    'Α', 'Β', 'Γ', 'Δ', 'Ε', 'Ζ', 'Η', 'Θ', 'Ι', 'Κ', 'Λ', 'Μ', 'Ν', 'Ξ', 'Ο', 'Π', 'Ρ',      'Σ', 'Τ', 'Υ', 'Φ', 'Χ', 'Ψ', 'Ω'
	//,   'α', 'β', 'γ', 'δ', 'ε', 'ζ', 'η', 'θ', 'ι', 'κ', 'λ', 'μ', 'ν', 'ξ', 'ο', 'π', 'ρ', 'ς', 'σ', 'τ', 'υ', 'φ', 'χ', 'ψ', 'ω'
	//,   '∞'
	};
	if ( Includes( $, _ ) )				return true;
	if ( 0x2200 <= _ && _ < 0x2300 )	return true;	//	Mathematical Operators
	if ( 0x27c0 <= _ && _ < 0x27f0 )	return true;	//	Miscellaneous Mathematical Symbols-A
	if ( 0x2980 <= _ && _ < 0x2a00 )	return true;	//	Miscellaneous Mathematical Symbols-B
	if ( 0x2a00 <= _ && _ < 0x2b00 )	return true;	//	Supplemental Mathematical Operators

	return false;
}

bool
IsCharacter( UTF32 _ ) {
	if ( _ < '0' )				return false;	//	Symbols
	if ( _ < ':' )				return true;	//	Numerics
	if ( _ < 'A' )				return false;	//	Symbols
	if ( _ < '[' )				return true;	//	Uppers
	if ( _ < 'a' )				return false;	//	Symbols
	if ( _ < '{' )				return true;	//	Lowers
	if ( _ < 0xc0 )				return false;	//	Symbols		¡ ¤ ¦ § « ¬ ± µ ¶ · » ¿ × ÷
	if ( IsSpace( _ ) )			return false;
	if ( IsMathSymbol( _ ) )	return false;	//	⟦ ⟧ ⟨ ⟩ ⟪ ⟫

	return true;
}

bool
IsHexChar( UTF32 _ ) {
	switch ( _ ) {
	case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		return true;
	default:
		return false;
	}
}

bool
IsDecChar( UTF32 _ ) {
	switch ( _ ) {
	case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
		return true;
	default:
		return false;
	}
}

bool
IsOctChar( UTF32 _ ) {
	switch ( _ ) {
	case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
		return true;
	default:
		return false;
	}
}

Object*
ReadHex( UnicodeReader& r, UTF32 first ) {
	ostringstream $;
	$ << (char)first;
	while ( r.Avail() ) {
		auto _ = r.Get();
		if ( IsHexChar( _ ) ) {
			$ << (char)_;
		} else if ( !IsCharacter( _ ) ) {
			r.UnGet( _ );
			break;
		} else {
			throw "Hex number format error";
		}
	}
	//	TODO: Delete leading 0
	auto _ = $.str();
	if ( _.size() > 16 ) {	//	BigInt
		return new Object( INT_64, stol( _, 0, 16 ) );	//	TODO: BigInt
	} else {
		return new Object( INT_64, stol( _, 0, 16 ) );
	}
}
Object*
ReadOctal( UnicodeReader& r, UTF32 first ) {
	ostringstream $;
	$ << (char)first;
	while ( r.Avail() ) {
		auto _ = r.Get();
		if ( IsOctChar( _ ) ) {
			$ << (char)_;
		} else if ( !IsCharacter( _ ) ) {
			r.UnGet( _ );
			break;
		} else {
			throw "Octal number format error";
		}
	}
	//	TODO: Delete leading 0
	auto _ = $.str();
	if ( _.size() > 16 ) {	//	BigInt
		return new Object( INT_64, stol( _, 0, 8 ) );	//	TODO: BigInt
	} else {
		return new Object( INT_64, stol( _, 0, 8 ) );
	}
}
Object*
ReadDecimal( UnicodeReader& r, UTF32 first ) {

	ostringstream $;
	$ << (char)first;
	auto readDot = false;
	auto readE = false;

	while ( r.Avail() ) {
		auto _ = r.Get();
		auto breakMe = false;
		switch ( _ ) {
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
			$ << (char)_;
			break;
		case 'e': case 'E':
			if ( readE ) throw "Number format error";
			readE = true;
			$ << (char)_;
			break;
		case '.':
			if ( readDot || readE ) throw "Number format error";
			readDot = true;
			$ << (char)_;
			break;
		case ',':
			switch ( r.Peek() ) {
			case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
				break;
			default:
				throw "Number format error";
			}
			break;
		default:
			if ( IsCharacter( _ ) ) throw "Number format error";
			r.UnGet( _ );
			breakMe = true;
		}
		if ( breakMe ) break;
	}

	auto _ = $.str();
	switch ( _.back() ) {
	case '.': throw "Number format error:(Trailing .)";
	case 'e': throw "Number format error:(Trailing e)";
	case 'E': throw "Number format error:(Trailing E)";
	}
	if ( readE || readDot ) return new Object( DOUBLE, stod( _ ) );
	try {
		auto $ = stoul( _ );
		if ( $ > numeric_limits< long >::max() ) return new Object( POSITIVE_BIGINT, Clone( &$, 1 ) );
		return new Object( INT_64, long( $ ) );
	} catch ( ... ) {
		auto $ = MakeBigInteger( _, 10 );
		return new Object( POSITIVE_BIGINT, $.size(), CloneData( $ ) );
	}
}

Object*
ReadNumber( UnicodeReader& r, UTF32 first ) {
	switch ( first ) {
	case '0':
		{	auto second = r.Get();
			switch ( second ) {
			case 'x': case 'X':
				{	auto _ = r.Get();
					if ( HexChar( _ ) ) return ReadHex( r, _ );
					throw "Hex number format error";
				}
			case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
				return ReadOctal( r, second );
			case '.': case 'e': case 'E':
				r.UnGet( second );
				return ReadDecimal( r, first );
			default:
				if ( IsCharacter( second ) ) throw "Decimal number format error";
				r.UnGet( second );
				return new Object( INT_64, 0L );
			}
		}
		break;
	case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
		return ReadDecimal( r, first );
	}
	THROW;
}

Object*
ReadList( UnicodeReader& r, UTF32 terminator, UI8 _, UI8 level ) {
	vector< Object* >	$;
	Object*
	Read( UnicodeReader&, UTF32, UI8 );
	for ( Object* _; ( _ = Read( r, terminator, level + 1 ) ); ) $.emplace_back( _ );
	return new Object( _, $.size(), CloneData( $ ) );
}

Object*
ReadString( UnicodeReader& r, UTF32 terminator ) {
	auto escaping = false;
	vector< UTF32 > $;
	while ( r.Avail() ) {
		auto _ = r.Get();
		if ( escaping ) {
			escaping = false;
			switch ( _ ) {
			case '0': $.emplace_back( '\0'	); break;
			case 'f': $.emplace_back( '\f'	); break;
			case 'n': $.emplace_back( '\n'	); break;
			case 'r': $.emplace_back( '\r'	); break;
			case 't': $.emplace_back( '\t'	); break;
			case 'v': $.emplace_back( '\v'	); break;
			default	: $.emplace_back( _		); break;
			}
		} else {
			if ( _ == terminator ) return MakeString( $ );
			if ( _ == '\\' )	escaping = true;
			else				$.emplace_back( _ );
		}
	}
	throw "Unterminated string: ";	//	UC  + $
}

Object*
ReadName( UnicodeReader& r, UTF32 first ) {
	if ( IsMathSymbol( first ) ) return new Object( NAME_1, first );
	vector< UTF32 > $;
	auto escaping = first == '\\';
	if ( !escaping ) $.emplace_back( first );
	
	while ( r.Avail() ) {
		auto _ = r.Get();
		if ( escaping ) {
			escaping = false;
			switch ( _ ) {
			case '0': $.emplace_back( '\0' ); break;
			case 'f': $.emplace_back( '\f' ); break;
			case 'n': $.emplace_back( '\n' ); break;
			case 'r': $.emplace_back( '\r' ); break;
			case 't': $.emplace_back( '\t' ); break;
			case 'v': $.emplace_back( '\v' ); break;
			default	: $.emplace_back( _	)	; break;
			}
		} else {
			if ( _ == '\\' ) escaping = true;
			else if ( !IsCharacter( _ ) ) {
				r.UnGet( _ );
				break;
			}
			else $.emplace_back( _ );
		}
	}

	switch ( $.size() ) {
	case 0	: throw "eh?";
	case 1	: return new Object( NAME_1	, $[ 0 ] );
	case 2	: return new Object( NAME_2	, $[ 0 ], $[ 1 ] );
	case 3	: return new Object( NAME_3	, $[ 0 ], $[ 1 ], $[ 2 ] );
	case 4	: return new Object( NAME_4	, $[ 0 ], $[ 1 ], $[ 2 ], $[ 3 ] );
	default	: return new Object( NAME	, $.size(), CloneData( $ ) );
	}
}

vector< UTF32 >
UTF32s( string const& _ ) {
	UnicodeReader ur( _ );
	vector< UTF32 > $;
	while ( ur.Avail() ) $.emplace_back( ur.Get() );
	return $;
}

Object*
NiL = new Object( LIST_LITERAL, 0L );

Object*
T = new Object( STRING_1, (UI4)'T' );

vector< Object* >
builtins {
	new Object(
		FUNCTION_PRIMITIVE
	,	[]( SliP* s ) {
			if ( s->stack.empty() ) throw "Stack empty";
			auto $ = s->stack.back();
			s->stack.pop_back();
			return $;
		}
	,	"@"
	)
,	new Object(
		FUNCTION_PRIMITIVE
	,	[]( SliP* s ) {
			return new Object(
				LIST_LITERAL
			,	s->stack.size()
			,	CloneData( s->stack )
			);
		}
	,	"@@"
	)
,	new Object(
		FUNCTION_PRIMITIVE
	,	[]( SliP* s ) {
			return new Object(
				STRING_KEY_DICT
			,	new map< string, Object* >( *s->context->mapP )
			);
		}
	,	"¤"
	)
,	new Object(
		FUNCTION_PREFIX
	,	[]( SliP* s, Object* _ ) {
			return _;
		}
	,	"'"
	)
,	new Object(
		FUNCTION_PREFIX
	,	[]( SliP* s, Object* _ ) -> Object* {
			throw (string)*_;
		}
	,	"¡"
	)
,	new Object(
		FUNCTION_PREFIX
	,	[]( SliP* s, Object* _ ) {
			auto $ = Eval( s, _ );
			switch ( $->_ ) {
			case INT_64:
				return new Object( INT_64, (long)~$->Integer() );
			case POSITIVE_BIGINT:
				return new Object(
					POSITIVE_BIGINT
				,	$->Integer()
				,	Clone( $->UI8P(), $->Integer() )
				);
			case NEGATIVE_BIGINT:
				return new Object(
					NEGATIVE_BIGINT
				,	$->Integer()
				,	Clone( $->UI8P(), $->Integer() )
				);
			}
			throw "Illegal operand type";
		}
	,	"~"
	)
,	new Object(
		FUNCTION_PREFIX
	,	[]( SliP* s, Object* _ ) {
			auto $ = Eval( s, _ );
			return $->IsList() && $->Integer() == 0 ? T : NiL;
		}
	,	"¬"
	)
,	new Object(
		FUNCTION_PREFIX
	,	[]( SliP* s, Object* _ ) {
			return Eval( s, Eval( s, _ ) );
		}
	,	"!"
	)
,	new Object(
		FUNCTION_PREFIX
	,	[]( SliP* s, Object* _ ) {
			return MakeString( UTF32s( (string)*_ ) );
		}
	,	"¶"
	)
,	new Object(
		FUNCTION_UNARY
	,	[]( SliP* s, Object* _ ) {
			if ( _->IsList()	) return new Object( INT_64, _->Integer() );
			if ( _->IsString()	) return new Object( INT_64, _->Integer() );
			if ( _->IsName()	) return new Object( INT_64, _->Integer() );
			throw "Illegal operand type";
		}
	,	"#"
	)
,	new Object(
		FUNCTION_UNARY
	,	[]( SliP* s, Object* _ ) {
			if ( !_->IsList() ) throw "Illegal operand type";
			auto size = _->Integer();
			if ( !size ) throw "No elements";
			return new Object( _->_, size - 1, Clone( _->ObjectPP() + 1, size - 1 ) );
		}
	,	"*"
	)
,	new Object(
		FUNCTION_UNARY
	,	[]( SliP* s, Object* _ ) {
			if ( !_->IsList() ) throw "Illegal operand type";
			auto size = _->Integer();
			if ( !size ) throw "No elements";
			return _->ObjectPP()[ size - 1 ];
		}
	,	"$"
	)
,	new Object(
		FUNCTION_UNARY
	,	[]( SliP* s, Object* _ ) {
			cout << (string)*_;
			return _;
		}
	,	"."
	)
,	new Object(
		FUNCTION_UNARY
	,	[]( SliP* s, Object* _ ) {
			cerr << (string)*_;
			return _;
		}
	,	"¦"
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			if ( !l->IsDict() ) throw "Apply dict to §";
			s->context = new Context( l->MapP(), s->context );
			return Eval( s, r );
		}
	,	"§"
	,	100
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			if ( !l->IsName() ) throw "Only name can be assigned.";
			(*s->context->mapP)[ (string)*l ] = r;
			return r;
		}
	,	"="
	,	90
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			if ( r->IsList() && r->Integer() == 2 ) return Eval( s, r->ObjectPP()[ IsT( l ) ? 0 : 1 ] );
			throw "Right operand must be two element List.";
		}
	,	"?"
	,	80
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			return IsT( l ) ? Eval( s, r ) : NiL;
		}
	,	"¿"
	,	80
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			return IsT( l ) && IsT( r ) ? T : NiL;
		}
	,	"&&"
	,	70
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			return IsT( l ) || IsT( r ) ? T : NiL;
		}
	,	"||"
	,	70
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			return IsT( l ) ^ IsT( r ) ? T : NiL;
		}
	,	"^^"
	,	70
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			if ( !r->IsList() ) throw "Right operand must be List";
			auto $ = r->ObjectPP();
			auto _ = r->Integer();
			while ( _-- ) if ( $[ _ ] == l ) return T;
			return NiL;
		}
	,	"∈"
	,	60
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			if ( !l->IsList() ) throw "Left operand must be List";
			auto $ = l->ObjectPP();
			auto _ = l->Integer();
			while ( _-- ) if ( $[ _ ] == r ) return T;
			return NiL;
		}
	,	"∋"
	,	60
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			return EQ( l, r ) ? T : NiL;
		}
	,	"=="
	,	60
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			return EQ( l, r ) ? NiL : T;
		}
	,	"<>"
	,	60
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			if ( l->IsNumeric() && r->IsNumeric() ) return l->Number() < r->Number() ? T : NiL;
			return EQ( l, r ) ? NiL : T;
		}
	,	"<"
	,	60
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			if ( l->IsNumeric() && r->IsNumeric() ) return l->Number() > r->Number() ? T : NiL;
			return EQ( l, r ) ? NiL : T;
		}
	,	">"
	,	60
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			if ( l->IsNumeric() && r->IsNumeric() ) return l->Number() <= r->Number() ? T : NiL;
			return EQ( l, r ) ? NiL : T;
		}
	,	"<="
	,	60
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			if ( l->IsNumeric() && r->IsNumeric() ) return l->Number() >= r->Number() ? T : NiL;
			return EQ( l, r ) ? NiL : T;
		}
	,	">="
	,	60
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			if ( !r->IsList() ) throw "Right operand must be List";
			auto _ = r->Integer();
			auto $ = new Object*[ _ + 1 ];
			while ( _-- ) $[ _ + 1 ] = r->ObjectPP()[ _ ];
			return new Object(
				r->_
			,	r->Integer() + 1
			,	$
			);
		}
	,	","
	,	50
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			if ( l->_ == INT_64 && r->_ == INT_64 ) return new Object( INT_64, l->Integer() & r->Integer() );
			throw "Illegal operand type";
		}
	,	"&"
	,	40
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			if ( l->_ == INT_64 && r->_ == INT_64 ) return new Object( INT_64, l->Integer() | r->Integer() );
			throw "Illegal operand type";
		}
	,	"|"
	,	40
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			if ( l->_ == INT_64 && r->_ == INT_64 ) return new Object( INT_64, l->Integer() ^ r->Integer() );
			throw "Illegal operand type";
		}
	,	"^"
	,	40
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			if ( l->_ == INT_64 && r->_ == INT_64 ) return new Object( INT_64, l->Integer() % r->Integer() );
			throw "Illegal operand type";
		}
	,	"%"
	,	30
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			if ( !l->IsMatrix() || !r->IsMatrix() ) throw "Operands must be Matrix";
			auto nRL = l->UI4_1();
			auto nCL = l->UI4_2();
			auto nRR = r->UI4_1();
			auto nCR = r->UI4_2();
			if ( nCL != nRR ) throw "Matrix size error";
			if ( nRL == 1 && nCR == 1 ) {
				double $ = 0;
				for ( UI4 _ = 0; _ < nCL; _++ ) $ += l->Double() * r->Double();
				return new Object( DOUBLE, $ );
			} else {
				auto size = (UI8)nRL * (UI8)nCR;
				auto $ = new double[ size ];
				auto lP = l->DoubleP();
				auto rP = r->DoubleP();
				for ( UI4 row = 0; row < nRL; row++ ) {
					auto cHead = row + nCR;
					for ( UI4 col = 0; col < nCR; col++ ) {
						auto Σ = 0;
						for ( UI4 _ = 0; _ < nCL; _++ ) Σ += lP[ row * nCL + _ ] + rP[ col + _ * nCR ];
						$[ cHead + col ] = Σ;
					}
				}
				if ( l->_ == MATRIX_SINGLE && r->_ == MATRIX_SINGLE ) {
					auto singles = new float[ size ];
					for ( UI8 _ = 0; _ < size; _++ ) singles[ _ ] = $[ _ ];
					return new Object( MATRIX_SINGLE, nRL, nCR, singles );
				} else {
					return new Object( MATRIX_DOUBLE, nRL, nCR, $ );
				}
			}
		}
	,	"·"
	,	30
	)
,	new Object(
		FUNCTION_INFIX
	,	Plus
	,	"+"
	,	30
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			switch ( l->_ ) {
			case INT_64:
				switch ( r->_ ) {
				case INT_64:
					return new Object( INT_64, l->Integer() - r->Integer() );
				case DOUBLE:
					return new Object( DOUBLE, l->Integer() - r->Double() );
				}
				break;
			case DOUBLE:
				switch ( r->_ ) {
				case INT_64:
					return new Object( DOUBLE, l->Double() - r->Integer() );
				case DOUBLE:
					return new Object( DOUBLE, l->Double() - r->Double() );
				}
				break;
			}
			throw "Illegal operand type";
		}
	,	"-"
	,	30
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			switch ( l->_ ) {
			case INT_64:
				switch ( r->_ ) {
				case INT_64:
					return new Object( INT_64, l->Integer() * r->Integer() );	//	TODO: Overflow
				case DOUBLE:
					return new Object( DOUBLE, l->Integer() / r->Double() );
				}
				break;
			case DOUBLE:
				switch ( r->_ ) {
				case INT_64:
					return new Object( DOUBLE, l->Double() / r->Integer() );
				case DOUBLE:
					return new Object( DOUBLE, l->Double() / r->Double() );
				}
				break;
			}
			throw "Illegal operand type";
		}
	,	"×"
	,	20
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			switch ( l->_ ) {
			case INT_64:
				switch ( r->_ ) {
				case INT_64:
					{	auto iL = l->Integer();
						auto iR = r->Integer();
						return iL % iR == 0
						?	new Object( INT_64, iL / iR )
						:	new Object( DOUBLE, (double)iL / (double)iR );
						;
					}
				case DOUBLE:
					return new Object( DOUBLE, l->Integer() / r->Double() );
				}
				break;
			case DOUBLE:
				switch ( r->_ ) {
				case INT_64:
					return new Object( DOUBLE, l->Double() / r->Integer() );
				case DOUBLE:
					return new Object( DOUBLE, l->Double() / r->Double() );
				}
				break;
			}
			throw "Illegal operand type";
		}
	,	"÷"
	,	20
	)
,	new Object(
		FUNCTION_INFIX
	,	[]( SliP* s, Object* l, Object* r ) {
			if ( r->_ == INT_64 ) {
				if ( l->IsList() ) return l->ObjectPP()[ r->Integer() ];
				throw "Left operand must be List";
			} else if ( r->IsString() ) {
				if ( l->IsDict() ) return (*l->MapP())[ (string)*r ];
				throw "Left operand must be Dict";
			} else if ( r->_ == FUNCTION_UNARY ) {
				return ((Unary)r->P())( s, l );
			}
			s->stack.emplace_back( l );
			auto $ = Eval( s, r );
			s->stack.pop_back();
			return $;
		}
	,	":"
	,	10
	)
};

void
Delete( Object* _ ) {
	if ( find( builtins.begin(), builtins.end(), _ ) != builtins.end() ) return;
	if ( _ == T || _ == NiL ) return;
	delete _;
}

map< string, Object* >
builtinDict = [](){
	map< string, Object* >	$;
	for ( auto const& _: builtins ) $[ _->Label() ] = _;
counter -= $.size() + 2;
	return $;
}();

Object*
Read( UnicodeReader& r, UTF32 terminator = 0, UI8 level = 0 ) {
	while ( r.Avail() ) {
		auto _ = r.Get();
		if ( _ == terminator ) return 0;
		if ( IsSpace( _ ) ) continue;
		if ( IsDecChar( _ ) ) return ReadNumber( r, _ );

		switch ( _ ) {
		case	']'		:
		case	'}'		:
		case	')'		:
		case	0xbb	:	//	'»'
			throw "Unexpected brace closer: " + UTF8( _ );
		case	'['		: return ReadList( r, ']'	, LIST_LITERAL		, level );
		case	'('		: return ReadList( r, ')'	, LIST_SENTENCE		, level );
		case	'{'		: return ReadList( r, '}'	, LIST_PROCEDURE	, level );
		case	0xab	: return ReadList( r, 0xbb	, LIST_PARALLEL		, level );	//	'«' '»'
		case	'"'		:
		case	'`'		: return ReadString( r, _ );
		default			:
			{	auto s = UTF8( _ );
				auto cand1 = builtinDict.find( s );
				if ( cand1 != builtinDict.end() ) {
					auto c2 = r.Get();
					s += UTF8( c2 );
					auto cand2 = builtinDict.find( s );
					if ( cand2 != builtinDict.end() ) return cand2->second;
					r.UnGet( c2 );
					return cand1->second;
				}
				return ReadName( r, _ );
			}
		}
	}
	throw 0;
}
//Object*
//Read( UnicodeReader& r, UTF32 terminator = 0 ) {
//	auto $ = _Read( r, terminator );
//	if ( $ ) cerr << (string)*$ << endl;
//	return $;
//}

void
Interpret( SliP* s, string const& source, void (*f)( string const& ) ) {

	cerr << "SliP ver0.0 Object size: " << sizeof( Object ) << endl;
	if ( sizeof( Object ) != 24 ) throw "Object size must be 24";
	UnicodeReader r( source );

	UI8	READ_START = 0;

auto
Check = [ & ]( Object* _ ) {
	auto source = UTF8( vector< UTF32 >( r._.begin() + READ_START, r._.begin() + r.index ) );
	auto result = string( *_ );
	if ( Shrink( Unescape( source ) ) != Shrink( result ) ) cerr
		<< ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl
		<< source << endl
		<< result << endl
		<< Shrink( Shrink( source ) ) << endl
		<< Shrink( result ) << endl
		<< "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl
	;
};
	try {

		READ_START = r.index;
		auto _ = Read( r );
Check( _ );

		while ( _ ) {
			_ = Eval( s, _ );
			f( (string)*_ );
			Delete( _ );
			
			READ_START = r.index;
			_ = Read( r );
Check( _ );
		}
	} catch ( string const* _ ) {
		cerr << _ << endl;
		cerr << "While reading: " << UTF8( vector< UTF32 >( r._.begin() + READ_START, r._.end() ) ) << endl;
		throw;
	} catch ( int _ ) {
	}
	cerr << "counter: " << counter << endl;
}
/*
,   '!'     //  21  Eval
,   '"'     //  22  StringLiteral
,   '#'     //  23  Num
,   '$'     //  24  Last
,   '%'     //  25  Reminder
,   '&'     //  26  AND
,   '\''    //  27  Quote
,   '('     //  28  Open sentence
,   ')'     //  29  Close sentence
,   '*'     //  2A  CDR
,   '+'     //  2B  Plus
,   ','     //  2C  CONS
,   '-'     //  2D  Minus
,   '.'     //  2E  Console.log, returns argument
,   '/'     //  2F
,   ':'     //  3A  Apply
,   ';'     //  3B  Dummy sentence terminator for Sugared syntax
,   '<'     //  3C  COMPARATOR
,   '='     //  3D  COMPARATOR
,   '>'     //  3E  COMPARATOR
,   '?'     //  3F  if L is non-Nil evaluate R[ 0 ] otherwise evaluate R[ 1 ]
,   '@'     //  40  Stack top, Stack
,   '['     //  5B  Open list
,   ']'     //  5D  Close list
,   '^'     //  5E  Exclusive logical OR
,   '`'     //  60  String Literal
,   '{'     //  7B  Open procedure
,   '|'     //  7C  OR
,   '}'     //  7D  Close procedure
,   '~'     //  7E  Bit flip
	0xa1	//	'¡'
,	0xa4	//	'¤'
,	0xa6	//	'¦'
,	0xa7	//	'§'
,	0xab	//	'«'
,	0xac	//	'¬'
,	0xb1	//	'±'
,	0xb5	//	'µ'
,	0xb6	//	'¶' Stringify
,	0xb7	//	'·' Dot
,	0xbb	//	'»' Close parallel
,	0xbf	//	'¿' If L is non-NIL evaluate R otherwise Nil
,	0xd7	//	'×' MUL
,	0xf7	//	'÷' DIV
,	0x2208	//	'∈' Member
,	0x220b	//	'∋' Includes
,	0x27e6	//	⟦
,	0x27e7	//	⟧
,	0x27e8	//	⟨
,	0x27e9	//	⟩
,	0x27e1	//	⟪
,	0x27eb	//	⟫
*/

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
