#include	"../JP/JP_CPP/JP.h"

#define	Cast	dynamic_pointer_cast
#define	SP		shared_ptr
#define	MS		make_shared

////////////////////////////////////////////////////////////////
#include	<numbers>
////////////////////////////////////////////////////////////////


struct
SliP {
	static	inline	int
	nSliPs = 0;

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

SP< SliP >
Eval( const SP< Context >& C, const SP< SliP >& _ );

static	inline	vector< SP< SliP > >
theStack;
static	inline	mutex
stackMutex;

inline	auto
Push( SP< SliP > _ ) {
	lock_guard< mutex >	lock( stackMutex );
	theStack.push_back( _ );
}
inline	auto
Pop() {
	if ( theStack.empty() ) throw runtime_error( "Stack underflow" );
	lock_guard< mutex >	lock( stackMutex );
	auto $ = theStack.back();
	theStack.pop_back();
	return $;
}
inline	auto
Stack() {
	lock_guard< mutex >	lock( stackMutex );
	return theStack;
}
inline	auto
PushAndEval( SP< Context > C, SP< SliP > l, SP< SliP > r ) {
	lock_guard< mutex >	lock( stackMutex );
	theStack.push_back( l );
	auto $ = Eval( C, r );
	theStack.pop_back();
	return $;
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
	Dot( vector< SP< SliP > >& l, vector< SP< SliP > >& r ) {
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
Negator : Numeric {
	SP< Numeric >															$;

	Negator( SP< Numeric > $ ) : $( $ ) {}

	string
	REPR() const override { return '-' + $->REPR(); }	//	TODO:かっこで囲む？

	SP< Numeric >
	Negate() const override {
		return $;
	}

	int64_t
	Bits64() const override {
		auto _ = $->Bits64();
		if( _ == numeric_limits< int64_t >::min() ) throw runtime_error( "Negator::Bits64 cannot change sign of int64_t::min()" );
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
		?	(SP< Numeric >)MS< Negator >( MS< Bits >( $ ) )
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

struct //	∞, 𝑒, π
NumericConstants : Numeric {
	string																			$;

	NumericConstants( const string& $ ) : $( $ ) {}

	string
	REPR() const override { return $; }

	SP< Numeric >
	Negate() const override {
		return MS< Negator >( MS< NumericConstants >( $ ) );
	}

	int64_t
	Bits64() const override {
		throw runtime_error( "Bits on NumericConstants" );
	}

	double
	Double() const override {
		if( $ == "∞"		) return numeric_limits< double >::infinity();
		if( $ == "𝑒"			) return numbers::e;
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
	REPR() const override {
		auto _ = string_char32s( vector< char32_t >{ mark } );
		return _ + $ + _;
	}
};

struct
Dict : SliP {
	unordered_map< string, SP< SliP > >										$;

	Dict( const unordered_map< string, SP< SliP > >& $ ) : $( $ ) {}

	string
	REPR() const override {
		string _ = "{";
		bool first = true;
		for ( const auto& [ K, V ] : $ ) {
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
	function< SP< SliP >( SP< Context > ) >							$;

	Primitive(
		function< SP< SliP >( SP< Context > ) >						$
	,	string label
	) :	Function( label ), $( $ ) {}
};
struct
Prefix : Function {
	function< SP< SliP >( SP< Context >, SP< SliP > ) >		$;
	Prefix(
		function< SP< SliP >( SP< Context >, SP< SliP > ) > $
	,	string label
	) :	Function( label ), $( $ ) {}
};
struct
Unary : Function {
	function< SP< SliP >( SP< Context >, SP< SliP > ) >		$;
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
	vector< SP< SliP > >													$;

	List( const vector< SP< SliP > >& $ ) :	$( $ ) {}

	virtual string
	REPR() const override { return ListString( U'[', U']' ); }

	string
	ListString( char32_t o, char32_t c ) const {
		const auto O = string_char32( o );
		const auto C = string_char32( c );
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

	Matrix( const vector< SP< SliP > >& $, int direction = 0 )
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
	Sentence( const vector< SP< SliP > >& $ ) : List( $ ) {}

	string
	REPR() const override { return ListString( U'(', U')' ); }
};

struct
Procedure : List {
	Procedure( const vector< SP< SliP > >& $ ) : List( $ ) {}

	string
	REPR() const override { return ListString( U'{', U'}' ); }
};

struct
Parallel : List {
	Parallel( const vector< SP< SliP > >& $ ) : List( $ ) {}

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

static SP< SliP >
T = MS< SliP >();

static SP< SliP >
Nil = MS< List >( vector< SP< SliP > >{} );

////////////////////////////////////////////////////////////////

struct
iReader {
	virtual	bool		Avail()		= 0;
	virtual	char32_t	Read()		= 0;
	virtual	char32_t	Peek()		= 0;
	virtual	void		Forward()	= 0;
	virtual	void		Backward()	= 0;
};

SP< SliP >
Read( iReader& R, char32_t terminator );

struct
StringReader : iReader {
	string	$;
	size_t	_ = 0;

	StringReader( const string& $ ) : $( $ ) {}

	bool		Avail()		{ return _ < $.length(); }
	char32_t	Read()		{ return static_cast<unsigned char>( $[ _++ ] ); }
	char32_t	Peek()		{ return static_cast<unsigned char>( $[ _ ] ); }
	void		Forward()	{ _++; }
	void		Backward()	{ --_; }
};

