#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <regex>
#include <functional>
#include <memory>
#include <limits>
#include <ranges>
#include <algorithm>
#include <numbers>

using namespace std;
using namespace views;

////////////////////////////////////////////////////////////////

#define	Cast	dynamic_pointer_cast

////////////////////////////////////////////////////////////////

template < typename T > vector< T >
concat( const vector< T >& l, const vector< T >& r ) {
	vector<T> $;
	$.reserve( l.size() + r.size() );  // メモリ予約（性能向上）
	$.insert( $.end(), l.begin(), l.end());
	$.insert( $.end(), r.begin(), r.end());
	return $;
}

template < typename Range, typename T > bool
contains( const Range& range, const T& value ) {
	return ranges::find( range, value ) != ranges::end( range );
}

////////////////////////////////////////////////////////////////

string
string_char32s( const vector< char32_t >& char32s ) {

	string $;

	for( char32_t _ : char32s ) {
		if ( _ <= 0x7F ) {				// 1バイト (ASCII)
			$ += static_cast<char>( _ );
		} else if ( _ <= 0x7FF ) {		// 2バイト
			$ += static_cast<char>( 0b11000000 | ( _ >> 6) );
			$ += static_cast<char>( 0b10000000 | ( _ & 0b00111111 ) );
		} else if ( _ <= 0xFFFF ) {		// 3バイト
			$ += static_cast<char>( 0b11100000 | ( _ >> 12 ) );
			$ += static_cast<char>( 0b10000000 | ( ( _ >> 6 ) & 0b00111111 ) );
			$ += static_cast<char>( 0b10000000 | ( _ & 0b00111111 ) );
		} else if ( _ <= 0x10FFFF ) {	// 4バイト
			$ += static_cast<char>( 0b11110000 | ( _ >> 18 ) );
			$ += static_cast<char>( 0b10000000 | ( ( _ >> 12 ) & 0b00111111 ) );
			$ += static_cast<char>( 0b10000000 | ( ( _ >> 6 ) & 0b00111111 ) );
			$ += static_cast<char>( 0b10000000 | ( _ & 0b00111111 ) );
		} else {
			$ += '?';
		}
	}

	return $;
}

bool
IsDigit( char32_t _ ) {
	return u'0' <= _ && _ <= u'9';
}
////////////////////////////////////////////////////////////////

int
nSliPs = 0;

struct
SliP {
	SliP() {
//		cout << '+' << ':' << ++nSliPs << endl;
	}

	virtual	~
	SliP() {
//		cout << '-' << ':' << --nSliPs << endl;
	}

	virtual string
	REP() const { return "SliP"; }
};

vector< shared_ptr< SliP > >
Stack;

struct
Context {
	unordered_map< string, shared_ptr< SliP > >	dict;
	shared_ptr< Context >						next;
	Context(
		unordered_map< string, shared_ptr< SliP > > dict = unordered_map< string, shared_ptr< SliP > >{}
	,	shared_ptr< Context > next = 0
	)
	:	dict( dict )
	,	next( next ) {
	}
};

struct
Numeric	: SliP {
	virtual	shared_ptr< Numeric >
	Negate() const = 0;

	virtual	int64_t
	Bits64() const = 0;

	virtual	double
	Double() const = 0;

	static	double
	Dot( vector< shared_ptr< SliP > >& l, vector< shared_ptr< SliP > >& r ) {
		auto size = l.size();
		if(	size != r.size() ) throw runtime_error( "Numeric::Dot: vector size unmatch" );
		double $ = 0;
		while( size-- ) {
			auto L = Cast< Numeric >( l[ size ] );
			if( !L ) throw runtime_error( "Numeric::Dot: lhs not a number" );
			auto R = Cast< Numeric >( r[ size ] );
			if( !R ) throw runtime_error( "Numeric::Dot: rhs not a number" );
			$ += L->Double() * R->Double();
		}
		return $;
	}
};

struct
Negator : Numeric {
	shared_ptr< Numeric >															$;

	Negator( shared_ptr< Numeric > $ ) : $( $ ) {}

	string
	REP() const override { return '-' + $->REP(); }	//	TODO:かっこで囲む？

	shared_ptr< Numeric >
	Negate() const override {
		return $;
	}

	int64_t
	Bits64() const override {
		auto _ = $->Bits64();
		if( _ == numeric_limits< int64_t >::min() ) throw runtime_error( "Negator::Bits64 failed" );
		return -_;
	}

	double
	Double() const override {
		return -$->Double();
	}
};

struct
Float : Numeric {
	double																			$;

	Float( double $ ) : $( $ ) {}

	string
	REP() const override { return to_string( $ ); }

	shared_ptr< Numeric >
	Negate() const override {
		return make_shared< Float >( -$ );
	}

	int64_t
	Bits64() const override {
		return *(int64_t*)&$;
	}

	double
	Double() const override {
		return $;
	}
};

struct
Bits : Numeric {
	int64_t																			$;

	Bits( const int64_t& $ ) : $( $ ) {}

	string
	REP() const override { return to_string( $ ); }

	shared_ptr< Numeric >
	Negate() const override {
		return $ == numeric_limits< int64_t >::min()
		?	(shared_ptr< Numeric >)make_shared< Negator >( make_shared< Bits >( $ ) )
		:	(shared_ptr< Numeric >)make_shared< Bits >( -$ )
		;
	}

	int64_t
	Bits64() const override {
		return $;
	}

	double
	Double() const override {
		return $;
	}
};

struct //	∞, 𝑒, π
NumericConstants : Numeric {
	string																			$;

	NumericConstants( const string& $ ) : $( $ ) {}

	string
	REP() const override { return $; }

	shared_ptr< Numeric >
	Negate() const override {
		return make_shared< Negator >( make_shared< NumericConstants >( $ ) );
	}

	int64_t
	Bits64() const override {
		throw runtime_error( "Bits on NumericConstants" );
	}

	double
	Double() const override {
		if( $ == "∞"		) return numeric_limits< double >::infinity();
		if( $ == "𝑒"		) return numbers::e;
		if( $ == "π"		) return numbers::pi;
		if( $ == "log2e"	) return numbers::log2e;
		if( $ == "log10e"	) return numbers::log10e;
		if( $ == "ln2"		) return numbers::ln2;
		if( $ == "ln10"		) return numbers::ln10;
		if( $ == "γ"		) return numbers::egamma;
		if( $ == "φ"		) return numbers::phi;
		throw runtime_error( "eh?" );
	}
};

struct
Literal : SliP {
	string																			$;
	char32_t																		mark;

	Literal( const string& $, char32_t mark ) : $( $ ), mark( mark ) {}

	string
	REP() const override {
		auto _ = string_char32s( vector< char32_t >{ mark } );
		return _ + $ + _;
	}
};

struct
Dict : SliP {
	unordered_map< string, shared_ptr< SliP > >										$;

	Dict( const unordered_map< string, shared_ptr< SliP > >& $ ) : $( $ ) {}

	string
	REP() const override {
		string _ = "{";
		bool first = true;
		for ( const auto& [ K, V ] : $ ) {
			if( !first ) _ += ", ";
			_ += "\"" + K + "\": " + V->REP();
			first = false;
		}
		_ += "}";
		return _;
	}
};

struct
Name : SliP {
	string																			$;

	Name( const string& $ ) : $( $ ) {}

	string
	REP() const override { return $; }
};

struct
Function : SliP {
	string																			label;

	Function( string label ) : label( label ) {}

	string
	REP() const override { return label; }
};
struct
Primitive : Function {
	function< shared_ptr< SliP >( shared_ptr< Context > ) >							$;

	Primitive(
		function< shared_ptr< SliP >( shared_ptr< Context > ) >						$
	,	string label
	) :	Function( label ), $( $ ) {}
};
struct
Prefix : Function {
	function< shared_ptr< SliP >( shared_ptr< Context >, shared_ptr< SliP > ) >		$;
	Prefix(
		function< shared_ptr< SliP >( shared_ptr< Context >, shared_ptr< SliP > ) > $
	,	string label
	) :	Function( label ), $( $ ) {}
};
struct
Unary : Function {
	function< shared_ptr< SliP >( shared_ptr< Context >, shared_ptr< SliP > ) >		$;
	Unary(
		function< shared_ptr< SliP >( shared_ptr< Context >, shared_ptr< SliP > ) > $
	,	string label
	) :	Function( label ), $( $ ) {}
};

struct
Infix : Function {
	function<
		shared_ptr< SliP >(
			shared_ptr< Context >
		,	shared_ptr< SliP >
		,	shared_ptr< SliP >
		)
	>																				$;
	int																				priority;

	Infix(
		function<
			shared_ptr< SliP >(
				shared_ptr< Context >
			,	shared_ptr< SliP >
			,	shared_ptr< SliP >
			)
		> $
	,	string label
	,	int priority
	) : Function( label ), $( $ ), priority( priority ) {}
};


struct
List : SliP {
	vector< shared_ptr< SliP > >													$;

	List( const vector< shared_ptr< SliP > >& $ ) :	$( $ ) {}

	virtual string
	REP() const override { return ListString( "[", "]" ); }

	string
	ListString( string O, string C ) const {
		if( $.size() == 0 ) return O + C;
		string	_ = O + ' ' + $[ 0 ]->REP();
		for ( size_t i = 1; i < $.size(); i++ ) {
			_ += " ";
			_ += $[ i ]->REP();
		}
		return _ + ' ' + C;
	}
};

struct
Matrix : List {
	int																				direction;

	Matrix( const vector< shared_ptr< SliP > >& $, int direction = 0 )
	:	List( $ )
	,	direction( direction ) {
	}

	string
	REP() const override { return ListString( "[", "]" ); }

	uint64_t
	NumRows() {
		return direction
		?	direction > 0 ? direction : $.size() / -direction
		:	0
		;
	}
	uint64_t
	NumCols() {
		return direction
		?	direction > 0 ? $.size() / direction : -direction
		:	0
		;
	}
	shared_ptr< SliP >
	operator() ( size_t r, size_t c ) {
		return $[ r * NumCols() + c ];
	}
};

struct
Sentence : List {
	Sentence( const vector< shared_ptr< SliP > >& $ ) : List( $ ) {}

	string
	REP() const override { return ListString( "(", ")" ); }
};

struct
Procedure : List {
	Procedure( const vector< shared_ptr< SliP > >& $ ) : List( $ ) {}

	string
	REP() const override { return ListString( "{", "}" ); }
};

struct
Parallel : List {
	Parallel( const vector< shared_ptr< SliP > >& $ ) : List( $ ) {}

	string
	REP() const override { return ListString( "«", "»" ); }
};

////////////////////////////////////////////////////////////////
inline int
_Compare( shared_ptr< SliP > l, shared_ptr< SliP > r ) {
	return 0;
}
////////////////////////////////////////////////////////////////
inline shared_ptr< SliP >
Eval( shared_ptr< Context > C, shared_ptr< SliP > _ ) {
	return _;
}

inline bool
IsNil( shared_ptr< SliP > _ ) {
	auto list = Cast< List >( _ );
	return list
	?	list->$.size() == 0
	:	false
	;
}
inline bool
IsT( shared_ptr< SliP > _ ) {
	return !IsNil( _ );
}

shared_ptr< SliP >
T = make_shared< SliP >();

shared_ptr< SliP >
Nil = make_shared< List >( vector< shared_ptr< SliP > >{} );

////////////////////////////////////////////////////////////////

inline bool	//	Excluding NO-BREAK SPACE i.e. a0, feff
IsBreaking( char32_t _ ) {
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

const vector<char32_t>
SoloChars = {
	u'Α'	,u'Β'	,u'Γ'	,u'Δ'	,u'Ε'	,u'Ζ'	,u'Η'	,u'Θ'	,u'Ι'	,u'Κ'	,u'Λ'	,u'Μ'
,	u'Ν'	,u'Ξ'	,u'Ο'	,u'Π'	,u'Ρ'	,u'Σ'	,u'Τ'	,u'Υ'	,u'Φ'	,u'Χ'	,u'Ψ'	,u'Ω'
,	u'α'	,u'β'	,u'γ'	,u'δ'	,u'ε'	,u'ζ'	,u'η'	,u'θ'	,u'ι'	,u'κ'	,u'λ'	,u'μ'
,	u'ν'	,u'ξ'	,u'ο'	,u'π'	,u'ρ'	,u'σ'	,u'τ'	,u'υ'	,u'φ'	,u'χ'	,u'ψ'	,u'ω'
,	u'ς'	//	Σの語尾系
,	U'𝑒'	//	U'\U0001D452'
,	u'∞'
};

const vector<char32_t>
BreakingChars = {
	u'!'	,u'"'	,u'#'	,u'$'	,u'%'	,u'&'	,u'\\'	,u'('	,u')'	,u'*'	,u'+'	,u','	,u'-'	,u'.'	,u'/'	,u':'	,u';'
,	u'<'	,u'='	,u'>'	,u'?'	,u'@'	,u'['	,u']'	,u'^'	,u'`'	,u'{'	,u'|'	,u'}'	,u'~'	,u'¡'	,u'¤'	,u'¦'	,u'§'
,	u'«'	,u'¬'	,u'±'	,u'µ'	,u'¶'	,u'·'	,u'»'	,u'¿'	,u'×'	,u'÷'	,u'∈'	,u'∋'	,u'⊂'	,u'⊃'	,u'∩'	,u'∪'	,u'∅'
};


const vector< shared_ptr< Function > >
Builtins = {
	make_shared< Primitive >(
		[]( shared_ptr< Context > ) -> shared_ptr< SliP > {
			if( Stack.empty() ) throw runtime_error( "Stack underflow" );
			auto $ = Stack.back();
			Stack.pop_back();
			return $;
		}
	,	"@"		//	Stack top
	)
,	make_shared< Primitive >(
		[]( shared_ptr< Context > ) -> shared_ptr< SliP > {
			return make_shared< List >( Stack );
		}
	,	"@@"	//	Stack list
	)
,	make_shared< Primitive >(
		[]( shared_ptr< Context > C ) -> shared_ptr< SliP > {
			return make_shared< Dict >( C->dict );
		}
	,	"¤"		//	make Dict
	)
,	make_shared< Prefix >(
		[]( shared_ptr< Context >, shared_ptr< SliP > _ ) -> shared_ptr< SliP > {
			return _;
		}
	,	"'"		//	Quote
	)
,	make_shared< Prefix >(
		[]( shared_ptr< Context >, shared_ptr< SliP > _ ) -> shared_ptr< SliP > {
			throw runtime_error( _->REP() );
		}
	,	"¡"		//	Throw
	)
,	make_shared< Prefix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > _ ) -> shared_ptr< SliP > {
			return Eval( C, Eval( C, _ ) );
		}
	,	"!"		//	Eval
	)
,	make_shared< Prefix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > _ ) -> shared_ptr< SliP > {
			auto v = Eval( C, _ );
			if( auto numeric = Cast< Numeric >( v ) ) return make_shared< Bits >( ~numeric->Bits64() );
			throw runtime_error( "Illegal operand type" );
		}
	,	"~"		//	Bit not
	)
,	make_shared< Prefix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > _ ) -> shared_ptr< SliP > {
			return IsNil( Eval( C, _ ) ) ? T : _;
		}
	,	"¬"		//	Logical not
	)
,	make_shared< Prefix >(
		[]( shared_ptr< Context >, shared_ptr< SliP > _ ) -> shared_ptr< SliP > {
			auto literal = Cast< Literal >( _ );
			return literal
			?	literal
			:	make_shared< Literal >( _->REP() , u'`' )
			;
		}
	,	"¶"		//	Convert to literal
	)
,	make_shared< Unary >(
		[]( shared_ptr< Context >, shared_ptr< SliP > _ ) -> shared_ptr< SliP > {
			if( auto list = Cast< List >( _ ) ) return make_shared< Bits >( list->$.size() );
			if( auto literal = Cast< Literal >( _ ) ) return make_shared< Bits >( literal->$.length() );
			throw runtime_error( "Illegal operand type" );
		}
	,	"#"		//	Number of elements
	)
,	make_shared< Unary >(
		[]( shared_ptr< Context >, shared_ptr< SliP > _ ) -> shared_ptr< SliP > {
			if( auto list = Cast< List >( _ ) ) {
				vector< shared_ptr< SliP > > $;
				$.reserve( list->$.size() - 1 );
				$.insert( $.end(), list->$.begin() + 1, list->$.end() );
				if( Cast< Matrix		>( _ ) ) return make_shared< Matrix		>( $ );
				if( Cast< Parallel		>( _ ) ) return make_shared< Parallel	>( $ );
				if( Cast< Sentence		>( _ ) ) return make_shared< Sentence	>( $ );
				if( Cast< Procedure	>( _ ) ) return make_shared< Procedure	>( $ );
				return make_shared< List >( $ );
			}
			throw runtime_error( "Illegal operand type" );
		}
	,	"*"		//	CDR
	)
,	make_shared< Unary >(
		[]( shared_ptr< Context >, shared_ptr< SliP > _ ) -> shared_ptr< SliP > {
			if( auto list = Cast< List >( _ ) ) {
				auto size = list->$.size();
				if( !size ) throw runtime_error( "Insufficient list" );
				return list->$[ size - 1 ];
			}
			throw runtime_error( "Illegal operand type" );
		}
	,	"$"		//	Last element
	)
,	make_shared< Unary >(
		[]( shared_ptr< Context >, shared_ptr< SliP > _ ) -> shared_ptr< SliP > {
			cout << _->REP() << endl;
			return _;
		}
	,	"."		//	stdout
	)
,	make_shared< Unary >(
		[]( shared_ptr< Context >, shared_ptr< SliP > _ ) -> shared_ptr< SliP > {
			cerr << _->REP() << endl;
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

,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			if( auto dict = Cast< Dict >( l ) ) {
				return Eval( make_shared< Context >( dict->$, C ), r );
			}
			throw runtime_error( "Left must be dict." );
		}
	,	"§"
	,	100
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			if( auto name = Cast< Name >( l ) ) {
				return C->dict[ name->$ ] = r;
			}
			throw runtime_error( "Only name can be assigned." );
		}
	,	"="
	,	90
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			if( auto list = Cast< List >( r ) ) {
				if( list->$.size() == 2 ) return Eval( C, list->$[ IsNil( l ) ? 1 : 0 ] );
			}
			throw runtime_error( "Right operand must be two element List." );
		}
	,	"?"
	,	80
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			return IsT( l ) ? Eval( C, r ) : Nil;
		}
	,	"¿"
	,	80
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			return ( IsT( l ) && IsT( r ) ) ? T : Nil;
		}
	,	"&&"
	,	70
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			return ( IsT( l ) || IsT( r ) ) ? T: Nil;
		}
	,	"||"
	,	70
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			return ( IsT( l ) ^ IsT( r ) ) ? T: Nil;
		}
	,	"^^"
	,	70
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			if( auto list = Cast< List >( r ) ) return contains( list->$, l ) ? T : Nil;
			throw runtime_error( "Right operand must be List" );
		}
	,	"∈"
	,	60
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			if( auto list = Cast< List >( l ) ) return contains( list->$, r ) ? T : Nil;
			throw runtime_error( "Right operand must be List" );
		}
	,	"∋"
	,	60
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			return _Compare( l, r ) ? T : Nil;
		}
	,	"=="
	,	60
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			return _Compare( l, r ) ? Nil : T;
		}
	,	"<>"
	,	60
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			return _Compare( l, r ) == -1 ? T : Nil;
		}
	,	"<"
	,	60
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			return _Compare( l, r ) == 1 ? T : Nil;
		}
	,	">"
	,	60
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			return _Compare( l, r ) != 1 ? T : Nil;
		}
	,	"<="
	,	60
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			return _Compare( l, r ) != -1 ? T : Nil;
		}
	,	">="
	,	60
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			if( auto list = Cast< List >( r ) ) {
				vector< shared_ptr< SliP > > $;
				$.reserve( list->$.size() + 1 );
				$.emplace_back( l );
				$.insert( $.end(), list->$.begin(), list->$.end() );
				if( Cast< Matrix	>( r ) ) return make_shared< Matrix		>( $ );
				if( Cast< Parallel	>( r ) ) return make_shared< Parallel	>( $ );
				if( Cast< Sentence	>( r ) ) return make_shared< Sentence	>( $ );
				if( Cast< Procedure	>( r ) ) return make_shared< Procedure	>( $ );
				return make_shared< List >( $ );
			}
			throw runtime_error( "Right operand must be List" );
		}
	,	","
	,	50
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			auto lNumeric = Cast< Numeric >( l );
			auto rNumeric = Cast< Numeric >( r );
			if( lNumeric && rNumeric ) return make_shared< Bits >( lNumeric->Bits64() & rNumeric->Bits64() );
			throw runtime_error( "Illegal operand type" );
		}
	,	"&"
	,	40
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			auto lNumeric = Cast< Numeric >( l );
			auto rNumeric = Cast< Numeric >( r );
			if( lNumeric && rNumeric ) return make_shared< Bits >( lNumeric->Bits64() | rNumeric->Bits64() );
			throw runtime_error( "Illegal operand type" );
		}
	,	"|"
	,	40
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			auto lNumeric = Cast< Numeric >( l );
			auto rNumeric = Cast< Numeric >( r );
			if( lNumeric && rNumeric ) return make_shared< Bits >( lNumeric->Bits64() ^ rNumeric->Bits64() );
			throw runtime_error( "Illegal operand type" );
		}
	,	"^"
	,	40
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			{	auto L = Cast< Bits	>( l ), R = Cast< Bits	>( r );
				if( L && R ) {
					return (
						R->$ > 0 && L->$ > numeric_limits<int64_t>::max() -  R->$
					||	R->$ < 0 && L->$ < numeric_limits<int64_t>::min() -  R->$
					) ?	(shared_ptr< Numeric >)make_shared< Float	>( L->Double() + R->Double() )
					:	(shared_ptr< Numeric >)make_shared< Bits	>( L->$ + R->$ )
					;
				}
			}
			{	auto L = Cast< Numeric	>( l ), R = Cast< Numeric	>( r );
				if( L && R ) return make_shared< Float		>( L->Double() + R->Double() );
			}
			{	auto L = Cast< Literal	>( l ), R = Cast< Literal	>( r );
				if( L && R ) return make_shared< Literal	>( L->$ + R->$, u'`' );
			}
			{	auto L = Cast< List		>( l ), R = Cast< List		>( r );
				if( L && R ) {
					{	auto L = Cast< Matrix		>( l ), R = Cast< Matrix		>( r );
						if( L && R ) return make_shared< Matrix	>( concat( L->$, R->$ ) );
					}
					{	auto L = Cast< Sentence		>( l ), R = Cast< Sentence		>( r );
						if( L && R ) return make_shared< Sentence	>( concat( L->$, R->$ ) );
					}
					{	auto L = Cast< Procedure	>( l ), R = Cast< Procedure	>( r );
						if( L && R ) return make_shared< Procedure	>( concat( L->$, R->$ ) );
					}
					{	auto L = Cast< Parallel		>( l ), R = Cast< Parallel		>( r );
						if( L && R ) return make_shared< Parallel	>( concat( L->$, R->$ ) );
					}
					return make_shared< List >( concat( L->$, R->$ ) );
				}
			}
			throw runtime_error( "Illegal operand type" );
		}
	,	"+"
	,	30
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			{	auto L = Cast< Bits	>( l ), R = Cast< Bits	>( r );
				if( L && R ) {
					return (
						R->$ > 0 && L->$ < numeric_limits<int64_t>::max() +  R->$
					||	R->$ < 0 && L->$ > numeric_limits<int64_t>::min() +  R->$
					) ?	(shared_ptr< Numeric >)make_shared< Float	>( L->Double() - R->Double() )
					:	(shared_ptr< Numeric >)make_shared< Bits	>( L->$ - R->$ )
					;
				}
			}
			{	auto L = Cast< Numeric	>( l ), R = Cast< Numeric	>( r );
				if( L && R ) return make_shared< Float		>( L->Double() + R->Double() );
			}
			throw runtime_error( "Illegal operand type" );
		}
	,	"-"
	,	30
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			auto L = Cast< Matrix >( l );
			auto R = Cast< Matrix >( r );
			if( L && R ) {
				auto nColsL = L->NumCols();
				auto nRowsR = R->NumRows();
				if( nColsL != nRowsR ) throw runtime_error( "DOT: Matrix size error" );
				if(	nColsL == 0 ) return make_shared< Float >( Numeric::Dot( L->$, R->$ ) );	//	Vector·Vector

				auto nRowsL = L->NumRows();
				auto nColsR = R->NumCols();

				vector< shared_ptr< SliP > >	$( nRowsL * nColsR );
				for ( size_t row = 0; row < nRowsL; row++ ) {
					for ( size_t col = 0; col < nColsR; col++ ) {
						double _ = 0.0;
						for ( size_t k = 0; k < nColsL; k++ ) {
							auto lN = Cast< Numeric >( (*L)( row, k ) );
							auto rN = Cast< Numeric >( (*R)( k, col ) );
							_ += lN->Double() * rN->Double();
						}
						$[ row * nColsR + col ] = make_shared< Float >( _ );
					}
				}
				return make_shared< Matrix >( $, nRowsL );
			}
			throw runtime_error( "Operands must be Matrix" );
		}
	,	"·"		//	Dot product
	,	20
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			auto L = Cast< Numeric >( l );
			auto R = Cast< Numeric >( r );
			if( L && R ) return make_shared< Float >( L->Double() * R->Double() );
			throw runtime_error( "Illegal operand type" );
		}
	,	"×"
	,	20
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			auto L = Cast< Numeric >( l );
			auto R = Cast< Numeric >( r );
			if( L && R ) return make_shared< Float >( L->Double() / R->Double() );
			throw runtime_error( "Illegal operand type" );
		}
	,	"÷"
	,	20
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
			auto L = Cast< Numeric >( l );
			auto R = Cast< Numeric >( r );
			if( L && R ) return make_shared< Bits >( L->Bits64() / R->Bits64() );
			throw runtime_error( "Illegal operand type" );
		}
	,	"%"
	,	20
	)
,	make_shared< Infix >(
		[]( shared_ptr< Context > C, shared_ptr< SliP > l, shared_ptr< SliP > r ) -> shared_ptr< SliP > {
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

			Stack.emplace_back( l );
			auto $ = Eval( C, r );
			Stack.pop_back();
			return $;
		}
	,	":"
	,	10
	)
};

struct
iReader {
	virtual	bool		Avail()		= 0;
	virtual	char32_t	Read()		= 0;
	virtual	char32_t	Peek()		= 0;
	virtual	void		Forward()	= 0;
	virtual	void		Backward()	= 0;
};

struct
Reader : iReader {
	string	$;
	size_t	_ = 0;

	Reader( const string& $ ) : $( $ ) {}

	bool		Avail()		{ return _ < $.length(); }
	char32_t	Read()		{ return $[ _++ ]; }
	char32_t	Peek()		{ return $[ _ ]; }
	void		Forward()	{ _++; }
	void		Backward()	{ --_; }
};

inline shared_ptr< SliP >
Read( iReader& R, char32_t terminator );

inline shared_ptr< SliP >
Read( const string& _ ) {
	Reader R( _ );
	return Read( R, -1 );
}

inline vector< shared_ptr< SliP > >
ReadList( iReader& _, char32_t close ) {
	vector< shared_ptr< SliP > > $;
	while ( shared_ptr< SliP > slip = Read( _, close ) ) $.emplace_back( slip );
	return $;
}

inline shared_ptr< Name >
CreateName( iReader& R, char32_t initial ) {

	if( contains( SoloChars, initial ) ) return make_shared< Name >( string_char32s( vector< char32_t >{ initial } ) );

	auto
	escaped = initial == u'\\';

	vector< char32_t >
	$;

	if( !escaped ) $.emplace_back( initial );

	while ( R.Avail() ) {
		auto _ = R.Peek();
		if( escaped ) {
			R.Forward();
			escaped = false;
			switch ( _ ) {
			case u'0'	: $.emplace_back( '\0'	); break;
			case u'f'	: $.emplace_back( '\f'	); break;
			case u'n'	: $.emplace_back( '\n'	); break;
			case u'r'	: $.emplace_back( '\r'	); break;
			case u't'	: $.emplace_back( '\t'	); break;
			case u'v'	: $.emplace_back( '\v'	); break;
			default		: $.emplace_back( _		); break;
			}
		} else {
			if( contains( SoloChars	, _ )	) break;
			if( contains( BreakingChars, _ )	) break;
			R.Forward();
			if( IsBreaking( _ )				) break;
			if( _ == '\\' )	escaped = true;
			else				$.emplace_back( _ );
		}
	}
	return make_shared<Name>( string_char32s( $ ) );
}
inline shared_ptr< Literal >
CreateLiteral( iReader& R, char32_t terminator ) {
	auto				escaped = false;
	vector< char32_t >	$;
	while ( R.Avail() ) {
		auto _ = R.Read();
		if( escaped ) {
			escaped = false;
			switch ( _ ) {
			case u'0'	: $.emplace_back( '\0'	); break;
			case u'f'	: $.emplace_back( '\f'	); break;
			case u'n'	: $.emplace_back( '\n'	); break;
			case u'r'	: $.emplace_back( '\r'	); break;
			case u't'	: $.emplace_back( '\t'	); break;
			case u'v'	: $.emplace_back( '\v'	); break;
			default		: $.emplace_back( _		); break;
			}
		} else {
			if( _ == terminator ) return make_shared< Literal >( string_char32s( $ ), terminator );
			if( _ == '\\' )	escaped = true;
			else				$.emplace_back( _ );
		}
	}
	throw runtime_error( "Unterminated string: " + string_char32s( $ ) );
}

inline shared_ptr< SliP >
Read( iReader& R, char32_t terminator ) {
	while ( R.Avail() ) {
		auto _ = R.Read();
		if( _ == terminator )	return 0;
		if( IsBreaking( _ ) )	continue;
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
				$.emplace_back( _ );
			}
			if( dotRead )	return make_shared<Float	>( stod( string_char32s( $ ) ) );
			else			return make_shared<Bits		>( stoi( string_char32s( $ ) ) );
		}
		switch ( _ ) {
		case u']'	:
		case u'}'	:
		case u')'	:
		case u'»'	: throw runtime_error( "Detect close parenthesis" );
		case u'['	: return make_shared< Matrix	>( ReadList( R, u']' ) );
		case u'('	: return make_shared< Sentence	>( ReadList( R, u')' ) );
		case u'{'	: return make_shared< Procedure	>( ReadList( R, u'}' ) );
		case u'«'	: return make_shared< Parallel	>( ReadList( R, u'»' ) );
		case u'"'	: return CreateLiteral( R, _ );
		case u'`'	: return CreateLiteral( R, _ );
		default		:
			auto label0 = string_char32s( vector< char32_t >{ _ } );
			auto it0 = find_if( Builtins.begin(), Builtins.end(), [ & ]( shared_ptr< Function > _ ){ return _->label == label0; } );
			if( it0 != Builtins.end() ) {
				auto label = label0 + string_char32s( vector< char32_t >{ R.Peek() } );
				auto it = find_if( Builtins.begin(), Builtins.end(), [ & ]( shared_ptr< Function > _ ){ return _->label == label; } );
				if( it != Builtins.end() )	return *it;
				else						return *it0;
			} else {
				return CreateName( R, _ );
			}
		}
	}
	return 0;
}

inline shared_ptr< SliP >
Print( shared_ptr< SliP > _ ) {
	cout << _->REP() << endl;
	return _;
}

