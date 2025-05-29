#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <regex>
#include <functional>

using namespace std;


////////////////////////////////////////////////////////////////

//	clang が c++23 の対応して、range::contains ができるまで
template < typename T, typename U > bool
Contains( const T& range, const U& value ) {
	return find( range.begin(), range.end(), value ) != range.end();
}

////////////////////////////////////////////////////////////////

#include "utfcpp/source/utf8.h"
using namespace utf8;

inline string
string_char32s( const vector< char32_t >& _ ) {
	string $;
	utf32to8( _.begin(), _.end(), back_inserter( $ ) );
	return $;
}

bool
IsDigit( char32_t _ ) {
	return u'0' <= _ && _ <= u'9';
}
////////////////////////////////////////////////////////////////


//	import { NewContext, StringReader, Read, Eval, Sugared } from './SliP/_.js'

struct
SliP {
	virtual string
	REP() const = 0;

	virtual	~
	SliP() {}
};

struct
Numeric	: SliP {
};

struct
Integer64 : Numeric {
	int64_t																	$;
	Integer64( const int64_t& $ ) : $( $ ) {}
	string
	REP() const { return to_string( $ ); }
};

struct //	∞, -∞ ...
NumericConstant : Numeric {
	string																	$;
	NumericConstant( const string& $ ) : $( $ ) {}
	string
	REP() const { return $; }
};

struct
Float : Numeric {
	double																	$;
	Float( double $ ) : $( $ ) {}
	string
	REP() const { return to_string( $ ); }
};

//	TODO INTEGER

struct
Literal : SliP {
	string																	$;
	char32_t																mark;
	Literal( const string& $, char32_t mark ) : $( $ ), mark( mark ) {}
	string
	REP() const {
		auto _ = string_char32s( vector< char32_t >{ mark } );
		return _ + $ + _;
	}
};

struct
Dict : SliP {
	unordered_map< string, const SliP* >									$;
	string
	REP() const {
		string _ = "{";
		bool first = true;
		for ( const auto& [ K, V ] : $ ) {
			if ( !first ) _ += ", ";
			_ += "\"" + K + "\": " + V->REP();
			first = false;
		}
		_ += "}";
		return _;
	}
};

struct
Name : SliP {
	string																	$;
	Name( const string& $ ) : $( $ ) {}
	string
	REP() const { return $; }
};

struct
Func : SliP {
	function< SliP*() >														$;
	string																	label;
	Func( function< SliP*() > $, string label ) : $( $ ), label( label ) {}
	string
	REP() const { return label; }
};
struct
Primitive : Func {
};
struct
Prefix : Func {
};
struct
Unary : Func {
};
struct
Infix : Func {
	int																		priority;
	Infix( function< SliP*() > $, string label, int priority ) : Func( $, label ), priority( priority ) {}
};



struct
List : SliP {
	vector< SliP* >															$;
	List( const vector< SliP* >& $ ) : $( $ ) {}
	~
	List() {
		for ( SliP* _ : $ ) delete _;
	}
	static string
	Join( const vector< SliP* >& _, string O, string C ) {
		if ( _.size() == 0 ) return O + C;
		string	$ = O + ' ' + _[ 0 ]->REP();
		for ( size_t i = 1; i < _.size(); i++ ) {
			$ += " ";
			$ += _[ i ]->REP();
		}
		return $ + ' ' + C;
	}
};

struct
Matrix : List {
	int	nRows;
	Matrix( const vector< SliP* >& $, int nRows = 1 ) : List( $ ), nRows( nRows ) {}
	string
	REP() const { return List::Join( $, "[", "]" ); }
};

struct
Sentence : List {
	Sentence( const vector< SliP* >& $ ) : List( $ ) {}
	string
	REP() const { return List::Join( $, "(", ")" ); }
};

struct
Procedure : List {
	Procedure( const vector< SliP* >& $ ) : List( $ ) {}
	string
	REP() const { return List::Join( $, "{", "}" ); }
};

struct
Parallel : List {
	Parallel( const vector< SliP* >& $ ) : List( $ ) {}
	string
	REP() const { return List::Join( $, "«", "»" ); }
};

////////////////////////////////////////////////////////////////

inline bool	//	Excluding NO-BREAK SPACE i.e. a0, feff
IsBreaking( char32_t _ ) {
	if ( _ <= 0x20 ) return true;
	if ( 0x7f <= _ && _ < 0xA0 ) return true;
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
,	U'\U0001D452'	//	𝑒
,	u'∞'
};

const vector<char32_t>
BreakingChars = {
	u'!'	,u'"'	,u'#'	,u'$'	,u'%'	,u'&'	,u'\\'	,u'('	,u')'	,u'*'	,u'+'	,u','	,u'-'	,u'.'	,u'/'	,u':'	,u';'
,	u'<'	,u'='	,u'>'	,u'?'	,u'@'	,u'['	,u']'	,u'^'	,u'`'	,u'{'	,u'|'	,u'}'	,u'~'	,u'¡'	,u'¤'	,u'¦'	,u'§'
,	u'«'	,u'¬'	,u'±'	,u'µ'	,u'¶'	,u'·'	,u'»'	,u'¿'	,u'×'	,u'÷'	,u'∈'	,u'∋'	,u'⊂'	,u'⊃'	,u'∩'	,u'∪'	,u'∅'
};

const unordered_map< string, function< SliP*() > >
BuiltinDict = {
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

inline SliP*
Read( iReader& R, char32_t terminator );

inline SliP*
Read( const string& _ ) {
	Reader R( _ );
	return Read( R, -1 );
}

inline vector< SliP* >
ReadList( iReader& _, char32_t terminator ) {
	vector< SliP* > $;
	while ( SliP* slip = Read( _, terminator ) ) $.emplace_back( slip );
	return $;
}

inline Name*
CreateName( iReader& R, char32_t initial ) {

	if ( Contains( SoloChars, initial ) ) return new Name( string_char32s( vector< char32_t >{ initial } ) );

	auto
	escaped = initial == u'\\';

	vector< char32_t >
	$;

	if ( !escaped ) $.emplace_back( initial );

	while ( R.Avail() ) {
		auto _ = R.Peek();
		if ( escaped ) {
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
			if ( Contains( SoloChars	, _ )	) break;
			if ( Contains( BreakingChars, _ )	) break;
			R.Forward();
			if ( IsBreaking( _ )				) break;
			if ( _ == '\\' )	escaped = true;
			else				$.emplace_back( _ );
		}
	}
	return new Name( string_char32s( $ ) );
}
inline Literal*
CreateLiteral( iReader& R, char32_t terminator ) {
	auto				escaped = false;
	vector< char32_t >	$;
	while ( R.Avail() ) {
		auto _ = R.Read();
		if ( escaped ) {
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
			if ( _ == terminator ) return new Literal( string_char32s( $ ), terminator );
			if ( _ == '\\' )	escaped = true;
			else				$.emplace_back( _ );
		}
	}
	throw runtime_error( "Unterminated string: " + string_char32s( $ ) );
}

inline SliP*
Read( iReader& R, char32_t terminator ) {
	while ( R.Avail() ) {
		auto _ = R.Read();
		if ( _ == terminator )	return 0;
		if ( IsBreaking( _ ) )	continue;
		if ( IsDigit( _ ) )	{
			vector< char32_t > $( _ );
			bool
			dotRead = false;
			while ( R.Avail() ) {
				auto _ = R.Peek();
				if ( _ == '.' ) {
					if ( dotRead ) break;
					dotRead = true;
					R.Forward();
				} else if ( IsDigit( _ ) ) {
					R.Forward();
				} else {
					break;
				}
				$.emplace_back( _ );
			}
			return dotRead
			?	(SliP*)new Float( stod( string_char32s( $ ) ) )
			:	(SliP*)new Integer64( stoi( string_char32s( $ ) ) )
			;
		}
		switch ( _ ) {
/*[*/	case u']'	:
/*{*/	case u'}'	:
/*(*/	case u')'	:
/*«*/	case u'»'	: throw runtime_error( "Detect close parenthesis" );
		case u'['	: return new Matrix		( ReadList( R, u']' ) );
		case u'('	: return new Sentence	( ReadList( R, u')' ) );
		case u'{'	: return new Procedure	( ReadList( R, u'}' ) );
		case u'«'	: return new Parallel	( ReadList( R, u'»' ) );
		case u'"'	: return CreateLiteral( R, _ );
		case u'`'	: return CreateLiteral( R, _ );
		default		:
			auto label0 = string_char32s( vector< char32_t >{ _ } );
			auto it0 = BuiltinDict.find( label0 );
			if ( it0 != BuiltinDict.end() ) {
				auto label = label0 + string_char32s( vector< char32_t >{ R.Peek() } );
				auto it = BuiltinDict.find( label );
				if ( it != BuiltinDict.end() ) {
					return it->second();
				} else {
					return it0->second();
				}
			} else {
				return CreateName( R, _ );
			}
		}
	}
	return 0;
}

