#include	<iostream>

#include	"SliP.hpp"

pair<UI8, UI8>
Mul( UI8 p, UI8 q ) {

	UI8 pL = p & 0xFFFFFFFF;
	UI8 pH = p >> 32;
	UI8 qL = q & 0xFFFFFFFF;
	UI8 qH = q >> 32;

	UI8 L_L = pL * qL;
	UI8 L_H = pL * qH;
	UI8 H_L = pH * qL;
	UI8 H_H = pH * qH;

	UI8 L = L_L + ( L_H << 32 ) + ( H_L << 32 );
	UI8 C = L < L_L ? 1 : 0;

	return {
		H_H + ( L_H >> 32 ) + ( H_L >> 32 ) + C
	,	L
	};
}
pair<UI8, UI8>
Mul( UI8 p, UI4 q ) {

	UI8 $ = ( p & 0xFFFFFFFF ) * q;
	
	UI8 $H = $ >> 32;
	
	UI8 L_L = pL * qL;
	UI8 L_H = pL * qH;
	UI8 H_L = pH * qL;
	UI8 H_H = pH * qH;

	UI8 L = L_L + ( L_H << 32 ) + ( H_L << 32 );
	UI8 C = L < L_L ? 1 : 0;

	return {
		H_H + ( L_H >> 32 ) + ( H_L >> 32 ) + C
	,	L
	};
/*
	FF*F
	E10
	 E1
 */
}

enum {
	ILLEGAL_RADIX = 1
};
struct
SliPException {
	int		code;
	string	supp;
	SliPException( int code, string const& supp )
	:	code( code )
	,	supp( supp ) {
	}
};

inline vector< UI8 >
MakeBigInteger( string const& _, UI8 radix ) {

	if ( radix > 36 ) throw SliPException( ILLEGAL_RADIX, to_string( radix ) );

	auto
	ToDigit = [ & ]( UTF32 _ ) -> UI1 {
		switch ( _ ) {
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
			return _ - '0';
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
		case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
			return _ - 'a' + 10;
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
		case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
			return _ - 'A' + 10;
		}
		return radix;
	};

	UnicodeReader ur( _ );
	vector< UI8 > $;
	UI8	A = 0;
	while ( ur.Avail() ) {
		auto c = ur.Get();
		auto _ = ToDigit( c );	//	UI1
		if ( _ >= radix ) break;
		auto preA = A;
		A *= radix;
cerr << hex << A << ':';
		A += _;
cerr << hex << A << endl;
		if ( A < preA ) {
			UI8	_ = 0;
			while ( _ < $.size() ) if ( ++$[ _++ ] ) break;
			if ( _ == $.size() ) $.emplace_back( 1 );
		}
	}
	$.insert( $.begin(), A );
	return $;
}
inline void
TEST_MakeBigInteger() {
	auto $ = MakeBigInteger( "1ffffffffffffffff", 16 );
	A( $.size() == 2 );
	A( $[ 0 ] == 0xffffffffffffffff );
	A( $[ 1 ] == 1 );
}

inline string
HexString( vector< UI8 > const& _ ) {
	string $ = "0x";
	for ( auto& _: _ ) $ += EncodeHex( _ );
	return $;
}

inline double
Double( vector< UI8 > const& _ ) {
	auto $ = 0.0;
	for ( auto const& _: _ ) {
		$ *= 0x100000000;
		$ *= 0x100000000;
		$ += double( _ );
	}
	return $;
}

inline vector< UI8 >
Add( vector< UI8 > const& p, vector< UI8 > const& q ) {
	vector< UI8 >	$;
	return $;
}

inline vector< UI8 >
Sub( vector< UI8 > const& p, vector< UI8 > const& q ) {
	vector< UI8 >	$;
	return $;
}

int
GLE( vector< UI8 > const& p, vector< UI8 > const& q ) {
	return 0;
}

#define		LITERAL_BIT			0x80

#define		_STRING				0x00
#define		STRING				UI1( _STRING	+ 0 + LITERAL_BIT )	//	80

#define		_NUMERIC			0x10
#define		ZERO				UI1( _NUMERIC	+ 0 + LITERAL_BIT )	//	90
#define		POSITIVE			UI1( _NUMERIC	+ 1 + LITERAL_BIT )	//	91
#define		NEGATIVE			UI1( _NUMERIC	+ 2 + LITERAL_BIT )	//	92
#define		DOUBLE				UI1( _NUMERIC	+ 3 + LITERAL_BIT )	//	93

#define		_CHAR				0x20
#define		UNICODE				UI1( _CHAR		+ 0 + LITERAL_BIT )	//	a0

#define		_MATRIX				0x30
#define		MATRIX_FLOAT		UI1( _MATRIX	+ 0 + LITERAL_BIT )	//	b0

#define		_DICT				0x40
#define		STRING_KEY_DICT		UI1( _DICT		+ 0 + LITERAL_BIT )	//	c0

#define		_LIST				0x50
#define		LIST_LITERAL		UI1( _LIST		+ 0 + LITERAL_BIT )	//	d0
#define		LIST_PARALLEL		UI1( _LIST		+ 1 )				//	51
#define		LIST_PROCEDURE		UI1( _LIST		+ 2 )				//	52
#define		LIST_SENTENCE		UI1( _LIST		+ 3 )				//	53

#define		_NAME				0x60
#define		NAME				UI1( _NAME		+ 0 )				//	60

#define		_FUNCTION			0x70
#define		FUNCTION_PRIMITIVE	UI1( _FUNCTION	+ 0 )				//	70
#define		FUNCTION_PREFIX		UI1( _FUNCTION	+ 1 )				//	71
#define		FUNCTION_UNARY		UI1( _FUNCTION	+ 2 )				//	72
#define		FUNCTION_INFIX		UI1( _FUNCTION	+ 3 )				//	73

static	int	counter = 0;

struct
Matrix : vector< double > {
	UI8		nRows;
	UI8		nCols;
	Matrix( UI8 nRows, UI8 nCols )
	:	vector< double >( nRows * nCols )
	,	nRows( nRows )
	,	nCols( nCols ) {
	}
};

typedef Object* (*Primitive	)( SliP* );
typedef Object* (*PreUna	)( SliP*, Object* );	//	Prefix, Unary
typedef Object* (*Infix		)( SliP*, Object*, Object* );

struct
Function {
	string	label;
	union {
		Primitive	primitive;
		PreUna		preuna;
		Infix		infix;
	};
	UI1	priority;
	
	Function( string const& label, Primitive _ )
	:	label( label )
	,	primitive( _ ) {
	}
	Function( string const& label, PreUna _ )
	:	label( label )
	,	preuna( _ ) {
	}
	Function( string const& label, Infix _, UI8 priority )
	:	label( label )
	,	infix( _ )
	,	priority( priority ) {
	}
};

struct
Object {

	union {
		double					d;
		UTF32					u;

		vector< UTF32 >*		pStrName;	//	Shared: String, Name
		vector< UI8 >*			pBigInt;
		Matrix*					pMatrix;

		map< string, Object* >*	pDict;

		Function*				pFunction;

		vector< Object* >*		pList;
	};

	UI1	_;

	~
	Object() {
								--counter;
		switch ( _ & 0x7000 ) {
		case STRING				:
		case NAME				: delete pStrName	; break;

		case POSITIVE			:
		case NEGATIVE			: delete pBigInt	; break;

		case MATRIX_FLOAT		: delete pMatrix	; break;

		case STRING_KEY_DICT	: delete pDict		; break;

		case FUNCTION_PRIMITIVE	:
		case FUNCTION_PREFIX	:
		case FUNCTION_UNARY		:
		case FUNCTION_INFIX		: delete pFunction	; break;

		case LIST_LITERAL		:
		case LIST_PARALLEL		:
		case LIST_PROCEDURE		:
		case LIST_SENTENCE		:
			for ( auto const _: *pList ) delete _;
			delete pList;
			break;
		}
	}

#ifdef DEBUG
	Object( Object const& $ )	{ throw "eh?"; }
#endif
	Object()
	:	_( ZERO )				{ counter++; }
	
	Object( long	_ ) {
								counter++;
		switch ( Sign( _ ) ) {
		case -1:
			this->_ = NEGATIVE;
			pBigInt = new vector< UI8 >{ UI8( ~_ + 1 ) };
			break;
		case 1:
			this->_ = POSITIVE;
			pBigInt = new vector< UI8 >{ UI8( _ ) };
			break;
		default:
			this->_ = ZERO;
			break;
		}
	}
	Object( double	_ )
	:	_( DOUBLE )				{ counter++; d = _; }
	Object( UTF32	_ )
	:	_( UNICODE )			{ counter++; u = _; }
	
	Object( UI1 _, UI8 size )
	:	_( _ ) {
		switch ( _ ) {
		case NAME				:
		case STRING				: counter++; pStrName	= new vector< UTF32 >( size );
			break;

		case POSITIVE			:
		case NEGATIVE			: counter++; pBigInt	= new vector< UI8 >( size );
			break;

		case LIST_LITERAL		:
		case LIST_PARALLEL		:
		case LIST_PROCEDURE		:
		case LIST_SENTENCE		: counter++; pList		= new vector< Object* >( size );
			break;
		}
	}

	Object( UI8 nRows, UI8 nCols )						//	Matrix
	:	_( MATRIX_FLOAT )		{ counter++; pMatrix	= new Matrix( nRows, nCols );
	}

	Object( UI1 _, const vector< Object* >& $ )			//	List
	:	_( _ )					{ counter++; pList		= new vector< Object* >( $ ); }

	Object( UI1 _, const vector< UI8 >& $ )				//	BigInt
	:	_( _ )					{ counter++; pBigInt	= new vector< UI8 >( $ ); }

	Object( UI1 _, const vector< UTF32 >& $ )			//	String, Name
	:	_( _ )					{ counter++; pStrName	= new vector< UTF32 >( $ ); }

	Object( UI1 _, const map< string, Object* >& $ )	//	Dict
	:	_( _ )					{ counter++; pDict		= new map< string, Object* >( $ ); }

	Object( UI1 _, const Function& $ )					//	Function
	:	_( _ )					{ counter++; pFunction	= new Function( $ ); }

	Object( Primitive f, const char* label )
	:	_( FUNCTION_PRIMITIVE )	{ counter++; pFunction	=  new Function( label, f ); }
	
	Object( UI1 _, PreUna f, const char* label )
	:	_( _ )					{ counter++; pFunction	=  new Function( label, f ); }
	
	Object( Infix f, const char* label, UI1 priority )
	:	_( FUNCTION_INFIX )		{ counter++; pFunction	=  new Function( label, f, priority ); }

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
	Double() {
		switch ( _ ) {
		case ZERO		: return 0;
		case DOUBLE		: return d;
		case POSITIVE	: return ::Double( *pBigInt );
		case NEGATIVE	: return -::Double( *pBigInt );
		}
		throw "Not a number";
	}
	string
	GenericListString() {
		string $;
		for ( auto const& _: *pList ) {
			$ += ' ';
			$ += *_;
		}
		return $ + ' ';
	}
	
	operator
	string() {
		switch ( _ ) {
		case ZERO:
			return "0";
		case DOUBLE:
			{	auto _ = d;
				return _ == ( 1.0 / 0.0 )
				?	"∞"
				:	_ == -( 1.0 / 0.0 )
					?	"-∞"
					:	EncodeDouble( _ )
				;
			}
		case POSITIVE:
			return pBigInt->size() == 1
			?	to_string( (*pBigInt)[ 0 ] )
			:	string( "POSITIVE BIGINT, # of usigned long:" ) + to_string( pBigInt->size() );
		case NEGATIVE:
			return pBigInt->size() == 1
			?	"-" + to_string( (*pBigInt)[ 0 ] )
			:	string( "NEGATIVE BIGINT, # of usigned long:" ) + to_string( pBigInt->size() );
			
		case UNICODE:
			return UTF8( u );
		case STRING:
			return '"' + UTF8( *pStrName ) + '"';
		case NAME:
			return UTF8( *pStrName );

		case FUNCTION_PRIMITIVE:
		case FUNCTION_PREFIX:
		case FUNCTION_UNARY:
			return pFunction->label;
		case FUNCTION_INFIX:
			return pFunction->label + '(' + to_string( pFunction->priority ) + ')';

		case LIST_LITERAL:
			return '[' + GenericListString() + ']';
		case LIST_PARALLEL:
			return "«" + GenericListString() + "»";
		case LIST_PROCEDURE:
			return '{' + GenericListString() + '}';
		case LIST_SENTENCE:
			return '(' + GenericListString() + ')';

		case MATRIX_FLOAT:
			{	string ${ "⟪" };
				auto nRs = pMatrix->nRows;
				auto nCs = pMatrix->nCols;
				for ( UI8 _R = 0; _R < nRs; _R++ ) {
					auto _ = _R * nCs;
					for ( UI8 _C = 0; _C < nCs; _C++ ) {
						$ += '\t';
						$ += (*pMatrix)[ _ + _C ];
					}
					$ += '\n';
				}
				return $ + "⟫";
			}

		case STRING_KEY_DICT:
			{	string ${ "⟦" };
				for ( auto& _: *pDict ) {
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
	return !_->IsList() || _->pList->size();
}

Object*
Plus( SliP* s, Object* l, Object* r ) {	//	TODO: WIP
	switch ( l->_ ) {
	case ZERO:
		return new Object( *r );
	case DOUBLE:
		switch( r->_ ) {
		case ZERO		: return new Object( *l );
		case DOUBLE		: return new Object( l->d + r->d );
		case POSITIVE	: return new Object( l->d + Double( *r->pBigInt ) );
		case NEGATIVE	: return new Object( l->d - Double( *r->pBigInt ) );
		}
		break;
	case POSITIVE:
		switch( r->_ ) {
		case ZERO		: return new Object( *l );
		case DOUBLE		: return new Object( Double( *l->pBigInt ) + r->d );
		case POSITIVE	: return new Object( POSITIVE, Add( *l->pBigInt, *r->pBigInt ) );
		case NEGATIVE	: {
				auto _ = GLE( *l->pBigInt, *r->pBigInt );
				return _ == 0
				?	new Object()
				:	_ > 0	//	Left is bigger
					?	new Object( POSITIVE, Sub( *l->pBigInt, *r->pBigInt ) )
					:	new Object( NEGATIVE, Sub( *r->pBigInt, *l->pBigInt ) )
				;
			}
		}
		break;
	case NEGATIVE:
		switch( r->_ ) {
		case ZERO		: return new Object( *l );
		case DOUBLE		: return new Object( -Double( *l->pBigInt ) + r->d );
		case NEGATIVE	: return new Object( NEGATIVE, Add( *l->pBigInt, *r->pBigInt ) );
		case POSITIVE	: {
				auto _ = GLE( *l->pBigInt, *r->pBigInt );
				return _ == 0
				?	new Object()
				:	_ > 0	//	Left is bigger
					?	new Object( NEGATIVE, Sub( *l->pBigInt, *r->pBigInt ) )
					:	new Object( POSITIVE, Sub( *r->pBigInt, *l->pBigInt ) )
				;
			}
		}
		break;
	default:
		if ( l->IsString() && r->IsString() ) {
			UI8 lSize = l->pStrName->size();
			UI8 rSize = r->pStrName->size();
			auto size = lSize + rSize;
			auto $ = new Object( STRING, size );
			auto& ref = *$->pStrName;
			for ( UI8 _ = 0; _ < lSize; _++ ) ref[ _ ] = (*l->pStrName)[ _ ];
			for ( UI8 _ = 0; _ < rSize; _++ ) ref[ lSize + _ ] = (*r->pStrName)[ _ ];
			return $;
		} else if ( l->_ == r->_ && l->IsList() && r->IsList() ) {
			auto lSize = l->pList->size();
			auto rSize = r->pList->size();
			auto size = lSize + rSize;
			auto $ = new Object( l->_, size );
			auto& ref = *$->pList;
			for ( UI8 _ = 0; _ < lSize; _++ ) ref[ _ ] = (*l->pList)[ _ ];
			for ( UI8 _ = 0; _ < rSize; _++ ) ref[ lSize + _ ] = (*r->pList)[ _ ];
			return $;
		}
	}
	throw "Illegal type combination";
}

bool
EQ( Object* p, Object* q ) {
	if ( p->IsNumeric() ) {
		return p->Double() == q->Double();
	} else {
		if ( p->_ != q->_ ) return false;

		switch ( p->_ ) {
		
		case FUNCTION_PRIMITIVE:
		case FUNCTION_PREFIX:
		case FUNCTION_UNARY:
		case FUNCTION_INFIX:
			return p->pFunction == q->pFunction;

		case LIST_LITERAL:
		case LIST_PARALLEL:
		case LIST_PROCEDURE:
		case LIST_SENTENCE:
			{	auto _ = p->pList->size();
				if ( _ != q->pList->size() ) return false;
				auto pP = p->pList;
				auto pQ = q->pList;
				while ( _-- ) if ( pP[ _ ] != pQ[ _ ] ) return false;
				return true;
			}
		case NAME:
		case STRING:
			{	auto _ = p->pStrName->size();
				if ( _ != q->pStrName->size() ) return false;
				auto pP = p->pStrName;
				auto pQ = q->pStrName;
				while ( _-- ) if ( pP[ _ ] != pQ[ _ ] ) return false;
				return true;
			}
		case MATRIX_FLOAT:
			{	if ( p->pMatrix->nRows != q->pMatrix->nRows ) return false;
				if ( p->pMatrix->nCols != q->pMatrix->nCols ) return false;
				auto _ = p->pMatrix->size();
				if ( _ != q->pMatrix->size() ) return false;
				auto pP = p->pMatrix;
				auto pQ = q->pMatrix;
				while ( _-- ) if ( pP[ _ ] != pQ[ _ ] ) return false;
				return true;
			}
		case STRING_KEY_DICT:
			{	if ( p->pDict->size() != q->pDict->size() ) return false;
				for ( auto& _: *p->pDict ) {
					auto map = q->pDict;
					auto $ = map->find( _.first );
					if ( $ == map->end() ) return false;
					if ( !EQ( _.second, $->second ) ) return false;
				}
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
		return new Object( stol( _, 0, 16 ) );	//	TODO: BigInt
	} else {
		return new Object( stol( _, 0, 16 ) );
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
		return new Object( stol( _, 0, 8 ) );	//	TODO: BigInt
	} else {
		return new Object( stol( _, 0, 8 ) );
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
	return readE || readDot
	?	new Object( stod( _ ) )
	:	new Object( POSITIVE, MakeBigInteger( _, 10 ) )
	;
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
				return new Object();
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
	auto $ = new Object( _, 0 );
	Object*
	Read( UnicodeReader&, UTF32, UI8 );
	for ( Object* _; ( _ = Read( r, terminator, level + 1 ) ); ) $->pList->emplace_back( _ );
	return $;
}

Object*
ReadString( UnicodeReader& r, UTF32 terminator ) {
	auto $ = new Object( STRING, 0 );
	auto escaping = false;
	while ( r.Avail() ) {
		auto _ = r.Get();
		if ( escaping ) {
			escaping = false;
			switch ( _ ) {
			case '0': $->pStrName->emplace_back( '\0'	); break;
			case 'f': $->pStrName->emplace_back( '\f'	); break;
			case 'n': $->pStrName->emplace_back( '\n'	); break;
			case 'r': $->pStrName->emplace_back( '\r'	); break;
			case 't': $->pStrName->emplace_back( '\t'	); break;
			case 'v': $->pStrName->emplace_back( '\v'	); break;
			default	: $->pStrName->emplace_back( _		); break;
			}
		} else {
			if ( _ == terminator ) return $;
			if ( _ == '\\' )	escaping = true;
			else				$->pStrName->emplace_back( _ );
		}
	}
	throw "Unterminated string: ";	//	UC  + $
}

Object*
ReadName( UnicodeReader& r, UTF32 first ) {
	auto $ = new Object( NAME, 0 );
	if ( IsMathSymbol( first ) ) {
		$->pStrName->emplace_back( first );
		return $;
	}
	auto escaping = first == '\\';
	if ( !escaping ) $->pStrName->emplace_back( first );
	
	while ( r.Avail() ) {
		auto _ = r.Get();
		if ( escaping ) {
			escaping = false;
			switch ( _ ) {
			case '0': $->pStrName->emplace_back( '\0' ); break;
			case 'f': $->pStrName->emplace_back( '\f' ); break;
			case 'n': $->pStrName->emplace_back( '\n' ); break;
			case 'r': $->pStrName->emplace_back( '\r' ); break;
			case 't': $->pStrName->emplace_back( '\t' ); break;
			case 'v': $->pStrName->emplace_back( '\v' ); break;
			default	: $->pStrName->emplace_back( _	)	; break;
			}
		} else {
			if ( _ == '\\' ) escaping = true;
			else if ( !IsCharacter( _ ) ) {
				r.UnGet( _ );
				break;
			}
			else $->pStrName->emplace_back( _ );
		}
	}
	return $;
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
T = new Object( 0L );

vector< Object* >
builtins {
	new Object(
		[]( SliP* s ) {
			if ( s->stack.empty() ) throw "Stack empty";
			auto $ = s->stack.back();
			s->stack.pop_back();
			return $;
		}
	,	"@"
	)
,	new Object(
		[]( SliP* s ) {
			return new Object(
				LIST_LITERAL
			,	s->stack
			);
		}
	,	"@@"
	)
,	new Object(
		[]( SliP* s ) {
			return new Object(
				STRING_KEY_DICT
			,	*s->context->mapP
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
,	new Object(	//	TODO: WIP
		FUNCTION_PREFIX
	,	[]( SliP* s, Object* _ ) {
			auto $ = Eval( s, _ );
			switch ( $->_ ) {
			case POSITIVE:
				return new Object(
					NEGATIVE
				,	*$->pBigInt
				);
			case NEGATIVE:
				return new Object(
					POSITIVE
				,	*$->pBigInt
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
			return $->IsList() && $->pList->size() == 0 ? T : NiL;
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
			return new Object( STRING, UTF32s( (string)*_ ) );
		}
	,	"¶"
	)
,	new Object(
		FUNCTION_UNARY
	,	[]( SliP* s, Object* _ ) {
			if ( _->IsList()	) return new Object( (long)_->pList->size() );		//	TODO: Overflow
			if ( _->IsString()	) return new Object( (long)_->pStrName->size() );
			if ( _->IsName()	) return new Object( (long)_->pStrName->size() );
			throw "Illegal operand type";
		}
	,	"#"
	)
,	new Object(
		FUNCTION_UNARY
	,	[]( SliP* s, Object* _ ) {
			if ( !_->IsList() ) throw "Illegal operand type";
			auto size = _->pList->size();
			if ( !size ) throw "No elements";
			return new Object( _->_, vector< Object* >( _->pList->begin() + 1, _->pList->end() ) );
		}
	,	"*"
	)
,	new Object(
		FUNCTION_UNARY
	,	[]( SliP* s, Object* _ ) {
			if ( !_->IsList() ) throw "Illegal operand type";
			auto size = _->pList->size();
			if ( !size ) throw "No elements";
			return (*_->pList)[ size - 1 ];
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
		[]( SliP* s, Object* l, Object* r ) {
			if ( !l->IsDict() ) throw "Apply dict to §";
			s->context = new Context( new map< string, Object* >( *l->pDict ), s->context );
			return Eval( s, r );
		}
	,	"§"
	,	100
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			if ( !l->IsName() ) throw "Only name can be assigned.";
			(*s->context->mapP)[ (string)*l ] = r;
			return r;
		}
	,	"="
	,	90
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			if ( r->IsList() && r->pList->size() == 2 ) return Eval( s, (*r->pList)[ IsT( l ) ? 0 : 1 ] );
			throw "Right operand must be two element List.";
		}
	,	"?"
	,	80
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			return IsT( l ) ? Eval( s, r ) : NiL;
		}
	,	"¿"
	,	80
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			return IsT( l ) && IsT( r ) ? T : NiL;
		}
	,	"&&"
	,	70
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			return IsT( l ) || IsT( r ) ? T : NiL;
		}
	,	"||"
	,	70
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			return IsT( l ) ^ IsT( r ) ? T : NiL;
		}
	,	"^^"
	,	70
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			if ( !r->IsList() ) throw "Right operand must be List";
			return Includes( *r->pList, l ) ? T : NiL;
		}
	,	"∈"
	,	60
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			if ( !l->IsList() ) throw "Left operand must be List";
			return Includes( *l->pList, r ) ? T : NiL;
		}
	,	"∋"
	,	60
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			return EQ( l, r ) ? T : NiL;
		}
	,	"=="
	,	60
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			return EQ( l, r ) ? NiL : T;
		}
	,	"<>"
	,	60
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			if ( l->IsNumeric() && r->IsNumeric() ) return l->Double() < r->Double() ? T : NiL;
			return EQ( l, r ) ? NiL : T;
		}
	,	"<"
	,	60
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			if ( l->IsNumeric() && r->IsNumeric() ) return l->Double() > r->Double() ? T : NiL;
			return EQ( l, r ) ? NiL : T;
		}
	,	">"
	,	60
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			if ( l->IsNumeric() && r->IsNumeric() ) return l->Double() <= r->Double() ? T : NiL;
			return EQ( l, r ) ? NiL : T;
		}
	,	"<="
	,	60
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			if ( l->IsNumeric() && r->IsNumeric() ) return l->Double() >= r->Double() ? T : NiL;
			return EQ( l, r ) ? NiL : T;
		}
	,	">="
	,	60
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			if ( !r->IsList() ) throw "Right operand must be List";
			auto _ = r->pList->size();
			auto $ = new Object( r->_, _ + 1 );
			while ( _-- ) (*$->pList)[ _ + 1 ] = (*r->pList)[ _ ];
			return $;
		}
	,	","
	,	50
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			return new Object();	//	TODO:	WIP
		}
	,	"&"
	,	40
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			return new Object();	//	TODO:	WIP
		}
	,	"|"
	,	40
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			return new Object();	//	TODO:	WIP
		}
	,	"^"
	,	40
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			return new Object();	//	TODO:	WIP
		}
	,	"%"
	,	30
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			if ( !l->IsMatrix() || !r->IsMatrix() ) throw "Operands must be Matrix";
			auto nRL = l->pMatrix->nRows;
			auto nCL = l->pMatrix->nCols;
			auto nRR = r->pMatrix->nRows;
			auto nCR = r->pMatrix->nCols;
			if ( nCL != nRR ) throw "Matrix size error";
			if ( nRL == 1 && nCR == 1 ) {
				double Σ = 0;
				for ( UI4 _ = 0; _ < nCL; _++ ) Σ += (*l->pMatrix)[ _ ] * (*r->pMatrix)[ _ ];
				return new Object( DOUBLE, Σ );
			} else {
				auto $ = new Object( nRL, nCR );
				auto size = (UI8)nRL * (UI8)nCR;
				auto& L = *l->pMatrix;
				auto& R = *r->pMatrix;
				for ( UI4 row = 0; row < nRL; row++ ) {
					auto cHead = row + nCR;
					for ( UI4 col = 0; col < nCR; col++ ) {
						auto Σ = 0;
						for ( UI4 _ = 0; _ < nCL; _++ ) Σ +=L[ row * nCL + _ ] + R[ col + _ * nCR ];
						(*$->pMatrix)[ cHead + col ] = Σ;
					}
				}
				return $;
			}
		}
	,	"·"
	,	30
	)
,	new Object(
		Plus
	,	"+"
	,	30
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			if ( l->IsNumeric() && r->IsNumeric() ) return new Object( DOUBLE, l->Double() - r->Double() );
			throw "Illegal operand type";
		}
	,	"-"
	,	30
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			if ( l->IsNumeric() && r->IsNumeric() ) return new Object( DOUBLE, l->Double() * r->Double() );
			throw "Illegal operand type";
		}
	,	"×"
	,	20
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			if ( l->IsNumeric() && r->IsNumeric() ) return new Object( DOUBLE, l->Double() / r->Double() );
			throw "Illegal operand type";
		}
	,	"÷"
	,	20
	)
,	new Object(
		[]( SliP* s, Object* l, Object* r ) {
			switch ( r->_ ) {
			case ZERO:
				if ( l->IsList() ) return (*l->pList)[ 0 ];
				break;
			case POSITIVE:
				if ( l->IsList() ) return (*l->pList)[ (*r->pBigInt)[ 0 ] ];
				break;
			case NEGATIVE:
				if ( l->IsList() ) {	//	TODO: WIP
					return (*l->pList)[ (*r->pBigInt)[ 0 ] ];
				}
				break;
			case FUNCTION_UNARY:
				return r->pFunction->preuna( s, l );
			default:
				if ( l->IsDict() ) return (*l->pDict)[ (string)*r ];
				break;
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
	for ( auto const& _: builtins ) $[ _->pFunction->label ] = _;
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
	if ( sizeof( Object ) != 16 ) throw "Object size must be 24";
	UnicodeReader r( source );

	UI8	READ_START = 0;

auto
Check = [ & ]( Object* _ ) {
	auto source = UTF8( vector< UTF32 >( r._.begin() + READ_START, r._.begin() + r.index ) );
	auto result = string( *_ );
	if ( Shrink( Unescape( source ) ) != Shrink( result ) ) cerr
		<< ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl
		<< Shrink( Unescape( source ) ) << endl
		<< Shrink( result ) << endl
		<< "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl
	;
};
	Object*	_;
	try {
		do {
			try	{
				READ_START = r.index;
				_ = Read( r );
Check( _ );
				_ = Eval( s, _ );
				f( (string)*_ );
				Delete( _ );
			} catch ( string const& _ ) {
				cerr << _ << endl;
				cerr << "While reading: " << UTF8( vector< UTF32 >( r._.begin() + READ_START, r._.end() ) ) << endl;
			} catch ( char const* _ ) {
				cerr << _ << endl;
				cerr << "While reading: " << UTF8( vector< UTF32 >( r._.begin() + READ_START, r._.end() ) ) << endl;
			}
			cerr << endl;
		} while ( _ );
	} catch ( int _ ) {
		cerr << "Encounter EOD, counter: " << counter << endl;
	}
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

void
TEST_SliP() {
	TEST_MakeBigInteger();
}
