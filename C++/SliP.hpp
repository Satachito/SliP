#pragma once

#include	"../JP/JP_CPP/JP.h"

#define	Cast	dynamic_pointer_cast
#define	SP		shared_ptr
#define	MS		make_shared

////////////////////////////////////////////////////////////////
#include	<numbers>
////////////////////////////////////////////////////////////////

struct
SliP {
//	static	inline	int
//	nSliPs = 0;

	SliP() {
//		cout << '+' << ':' << ++nSliPs << endl;
	}

	virtual	~
	SliP() {
//		cout << '-' << ':' << --nSliPs << endl;
	}

	virtual string
	REPR() const { return "SliP"; }
};

struct
Context {
	SP< Context >						next;
	unordered_map< string, SP< SliP > >	dict;
	Context(
		SP< Context > next = 0
	,	unordered_map< string, SP< SliP > > dict = unordered_map< string, SP< SliP > >{}
	) :	next( next )
	,	dict( dict ) {
	}
};

static	inline	vector< SP< SliP > >
theStack;
static	inline	mutex
stackMutex;

struct
Pusher {
	lock_guard< mutex >	lock;
	Pusher( SP< SliP > const& _ ) : lock( stackMutex ) {
		theStack.push_back( _ );
	}
	~
	Pusher() {
		theStack.pop_back();
	}
};

inline	void
Push( SP< SliP > _ ) {
	lock_guard< mutex >	lock( stackMutex );
	theStack.push_back( _ );
}
inline	void
Pop() {
	lock_guard< mutex >	lock( stackMutex );
	if ( theStack.empty() ) throw runtime_error( "Stack underflow" );
	theStack.pop_back();
}

inline	vector< SP< SliP > >
StackCopy() {
	lock_guard< mutex >	lock( stackMutex );
	return theStack;
}


struct
Numeric	: SliP {
	virtual	SP< Numeric >
	Negate() const = 0;

	virtual	int64_t
	Bits64() const = 0;

	virtual	double
	Double() const = 0;

	static	double
	Dot( vector< SP< SliP > > const& l, vector< SP< SliP > > const& r ) {
		auto index = l.size();
		if(	index != r.size() ) throw runtime_error( "Numeric::Dot: vector size unmatch" );
		double $ = 0;
		while( index-- ) {
			auto L = Cast< Numeric >( l[ index ] );
			if( !L ) throw runtime_error( "Numeric::Dot: lhs not a number" );
			auto R = Cast< Numeric >( r[ index ] );
			if( !R ) throw runtime_error( "Numeric::Dot: rhs not a number" );
			$ += L->Double() * R->Double();
		}
		return $;
	}
};

struct
Float : Numeric {
	double																			$;

	Float( double $ ) : $( $ ) {}

	string
	REPR() const override { return to_string( $ ); }

	SP< Numeric >
	Negate() const override {
		return MS< Float >( -$ );
	}

	int64_t
	Bits64() const override {
		throw runtime_error( "Float::Bits64 failed" );
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
	REPR() const override { return to_string( $ ); }

	SP< Numeric >
	Negate() const override {
		return $ == numeric_limits< int64_t >::min()
		?	(SP< Numeric >)MS< Float >( -(double)numeric_limits< int64_t >::min() )
		:	(SP< Numeric >)MS< Bits >( -$ )
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

inline static unordered_map< string, double >
numericConstants = {
	{ "∞"		, numeric_limits< double >::infinity()	}
,	{ "𝑒"		, numbers::e							}
,	{ "π"		, numbers::pi							}
,	{ "log2e"	, numbers::log2e						}
,	{ "log10e"	, numbers::log10e						}
,	{ "ln2"		, numbers::ln2							}
,	{ "ln10"	, numbers::ln10							}
,	{ "γ"		, numbers::egamma						}
,	{ "φ"		, numbers::phi							}
};

struct //	∞, 𝑒, π
NumericConstants : Numeric {

	string																			$;
	bool																			negative;

	NumericConstants( const string& $, bool negative = false )
	:	$( $ )
	,	negative( negative ) {
	}

	string
	REPR() const override { return negative ? "-" + $ : $; }

	SP< Numeric >
	Negate() const override {
		return MS< NumericConstants >( $, !negative );
	}

	int64_t
	Bits64() const override {
		throw runtime_error( "Bits on NumericConstants" );
	}

	double
	Double() const override {
		if( !numericConstants.count( $ ) ) throw runtime_error( "eh?" );
		return numericConstants[ $ ];
	}
};

struct
Literal : SliP {
	string																			$;
	char32_t																		mark;

	Literal( const string& $, char32_t mark ) : $( $ ), mark( mark ) {}

	string
	REPR() const override {
		auto _ = string_Us( vector< char32_t >{ mark } );
		return _ + $ + _;
	}
};

struct
Dict : SliP {
	unordered_map< string, SP< SliP > >												$;

	Dict( unordered_map< string, SP< SliP > > const& $ ) : $( $ ) {}

	string
	REPR() const override {
		string _ = "{";
		bool first = true;
		for ( auto const& [ K, V ] : $ ) {
			if( !first ) _ += ", ";
			_ += "\"" + K + "\": " + V->REPR();
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
	REPR() const override { return $; }
};

struct
Function : SliP {
	string																			label;

	Function( string label ) : label( label ) {}

	string
	REPR() const override { return label; }
};
struct
Primitive : Function {
	function< SP< SliP >( SP< Context > ) >											$;
	Primitive(
		function< SP< SliP >( SP< Context > ) > $
	,	string label
	) :	Function( label ), $( $ ) {}
};
struct
Prefix : Function {
	function< SP< SliP >( SP< Context >, SP< SliP > ) >								$;
	Prefix(
		function< SP< SliP >( SP< Context >, SP< SliP > ) > $
	,	string label
	) :	Function( label ), $( $ ) {}
};
struct
Unary : Function {
	function< SP< SliP >( SP< Context >, SP< SliP > ) >								$;
	Unary(
		function< SP< SliP >( SP< Context >, SP< SliP > ) > $
	,	string label
	) :	Function( label ), $( $ ) {}
};

struct
Infix : Function {
	function<
		SP< SliP >(
			SP< Context >
		,	SP< SliP >
		,	SP< SliP >
		)
	>																				$;
	int																				priority;

	Infix(
		function<
			SP< SliP >(
				SP< Context >
			,	SP< SliP >
			,	SP< SliP >
			)
		> $
	,	string label
	,	int priority
	) : Function( label ), $( $ ), priority( priority ) {}
};


struct
List : SliP {
	vector< SP< SliP > >															$;

	List( vector< SP< SliP > > const& $ ) :	$( $ ) {}

	virtual string
	REPR() const override { return ListString( U'[', U']' ); }

	string
	ListString( char32_t o, char32_t c ) const {
		const auto O = string_U( o );
		const auto C = string_U( c );
		if( $.size() == 0 ) return O + C;
		string	_ = O + ' ' + $[ 0 ]->REPR();
		for ( size_t i = 1; i < $.size(); i++ ) {
			_ += " ";
			_ += $[ i ]->REPR();
		}
		return _ + ' ' + C;
	}
};

struct
Matrix : List {
	int																				direction;

	Matrix( vector< SP< SliP > > const& $, int direction = 0 )
	:	List( $ )
	,	direction( direction ) {
	}

	string
	REPR() const override { return ListString( U'⟨', U'⟩' ); }

	uint64_t
	NumRows() const {
		return direction
		?	direction > 0 ? direction : $.size() / -direction
		:	0
		;
	}
	uint64_t
	NumCols() const {
		return direction
		?	direction > 0 ? $.size() / direction : -direction
		:	0
		;
	}
	SP< SliP >
	operator() ( size_t r, size_t c ) const {
		return $[ r * NumCols() + c ];
	}
};

struct
Sentence : List {
	Sentence( vector< SP< SliP > > const& $ ) : List( $ ) {}

	string
	REPR() const override { return ListString( U'(', U')' ); }
};

struct
Procedure : List {
	Procedure( vector< SP< SliP > > const& $ ) : List( $ ) {}

	string
	REPR() const override { return ListString( U'{', U'}' ); }
};

struct
Parallel : List {
	Parallel( vector< SP< SliP > > const& $ ) : List( $ ) {}

	string
	REPR() const override { return ListString( U'«', U'»' ); }
};

////////////////////////////////////////////////////////////////
inline int
_Compare( SP< SliP > l, SP< SliP > r ) {
	return 0;
}
////////////////////////////////////////////////////////////////

inline bool
IsNil( SP< SliP > _ ) {
	auto list = Cast< List >( _ );
	return list
	?	list->$.size() == 0
	:	false
	;
}
inline bool
IsT( SP< SliP > _ ) {
	return !IsNil( _ );
}

inline static SP< SliP >
T = MS< SliP >();

inline static SP< SliP >
Nil = MS< List >( vector< SP< SliP > >{} );

////////////////////////////////////////////////////////////////
#include	"Eval.hpp"
#include	"Builtins.hpp"
#include	"Read.hpp"

struct
StringReader : iReader {
	vector< char32_t >	$;
	size_t				_ = 0;

	StringReader( const string& $ ) : $( Us_string( $ ) ) {}

	bool		Avail()		{ return _ < $.size(); }
	char32_t	Read()		{ return static_cast<unsigned char>( $[ _++ ] ); }
	char32_t	Peek()		{ return static_cast<unsigned char>( $[ _ ] ); }
	void		Forward()	{ _++; }
	void		Backward()	{ --_; }
};

template< ranges::range R > vector< string >
EvalSliPs( R&& _ ) {
	auto							C = MS< Context >();
	return ranges::to< vector >(
		project(
			_
		,	[ & ]( SP< SliP > const& slip ) {
				return Eval( C, slip )->REPR();
			}
		)
	);
}

inline vector< string >
CoreSyntaxLoop( string const& _ ) {
	StringReader					R( _ );
	vector< SP< SliP > >			slips;
	while( auto _ = Read( R, -1 ) ) slips.push_back( _ );

	return EvalSliPs( slips );
}

inline vector< string >
SugaredSyntaxLoop( string const& _ ) {
	return EvalSliPs(
		project(
			Split( _ )
		,	[ & ]( string const& line ) {
				StringReader			R( line + ')' );
				return MS< Sentence	>( ReadList( R, U')' ) );
			}
		)
	);
}
