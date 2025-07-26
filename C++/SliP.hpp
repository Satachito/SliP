#include	"../JP/JP_CPP/JP.h"

#define	Cast	dynamic_pointer_cast
#define	SP		shared_ptr
#define	MS		make_shared
#define	UM		unordered_map
#define	US		unordered_set
#define	V		vector

struct
SliP {

//	static	inline	int
//	nSliPs = 0;

//	SliP() {
//		cout << '+' << ':' << ++nSliPs << endl;
//	}

//	virtual	~
//	SliP() {
//		cout << '-' << ':' << --nSliPs << endl;
//	}

	virtual string
	REPR() const { return "SliP"; }
};

struct
Context {
	SP< Context >				next;
	UM< string, SP< SliP > >	dict;
	Context(
		SP< Context >				next = nullptr
	,	UM< string, SP< SliP > >	dict = {}
	) :	next( next )
	,	dict( dict ) {
	}
};

struct
Numeric	: SliP {
	virtual	SP< Numeric >
	Negate() const = 0;

	virtual	double
	Double() const = 0;
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

	double
	Double() const override {
		return $;
	}
};

struct //	∞, 𝑒, π
NumericConstant : Numeric {

	string																			$;
	bool																			negative;

	NumericConstant( const string& $, bool negative = false )
	:	$( $ )
	,	negative( negative ) {
	}

	string
	REPR() const override { return negative ? "(-" + $ + ")" : $; }

	SP< Numeric >
	Negate() const override {
		return MS< NumericConstant >( $, !negative );
	}

	double
	Double() const override {
		static UM< string, double >
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
		if( !numericConstants.count( $ ) ) _Z( "eh?" );
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
		auto _ = string_Us( V< char32_t >{ mark } );
		return _ + $ + _;
	}
};

struct
Dict : SliP {
	UM< string, SP< SliP > >														$;

	Dict( UM< string, SP< SliP > > const& $ ) : $( $ ) {}

	string
	REPR() const override {
		string _ = "{";
		bool first = true;
		for ( auto const& [ K, V ] : $ ) {
			_ += ( first ? "\t" : ",\t" ) + K + ": " + V->REPR() + "\n";
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
		function< SP< SliP >( SP< Context > ) >							const& $
	,	string label
	) :	Function( label ), $( $ ) {}
};
struct
Prefix : Function {
	function< SP< SliP >( SP< Context >, SP< SliP > ) >								$;
	Prefix(
		function< SP< SliP >( SP< Context >, SP< SliP > ) >				const& $
	,	string label
	) :	Function( label ), $( $ ) {}
};
struct
Unary : Function {
	function< SP< SliP >( SP< Context >, SP< SliP > ) >								$;
	Unary(
		function< SP< SliP >( SP< Context >, SP< SliP > ) >				const& $
	,	string label
	) :	Function( label ), $( $ ) {}
};

struct
Infix : Function {
	function< SP< SliP >( SP< Context >, SP< SliP >, SP< SliP > ) >					$;
	int																				priority;
	Infix(
		function< SP< SliP >( SP< Context >, SP< SliP >, SP< SliP > ) >	const&	$
	,	string	label
	,	int		priority
	) : Function( label ), $( $ ), priority( priority ) {}
};

struct
Matrix : SliP {
	double*																			$;
	int64_t																			nCols;
	uint64_t																		size;

	~
	Matrix() {
		delete[] $;
	}
	double*
	new$( V< SP< SliP > > const& Ss ) {
		auto $ = new double[ Ss.size() ];
		for( size_t _ = 0; _ < Ss.size(); _++ ) {
			if( auto numeric = Cast< Numeric >( Ss[ _ ] ) ) {
				$[ _ ] = numeric->Double();
			} else {
				_Z( "All elements of the matrix must be numeric." );
			}
		}
		return $;
	}
	Matrix( V< SP< SliP > > const& $, int64_t nCols = 0 )
	:	$( new$( $ ) )
	,	nCols( nCols )
	,	size( $.size() ) {
		if( nCols == numeric_limits< int64_t >::min() ) _Z( "nCols must not be numeric_limits< int64_t >::min()" );
	}

	string
	REPR() const override { return "TODO:MATRIX REPR()"; }

	uint64_t
	NCols() const {
		return nCols
		?	uint64_t( nCols > 0 ? nCols : -nCols )
		:	0
		;
	}
	uint64_t
	NRows() const {
		return nCols
		?	size / uint64_t( nCols > 0 ? nCols : -nCols )
		:	0
		;
	}
	SP< SliP >
	operator() ( size_t r, size_t c ) const {
		return MS< Float >( $[ r * NCols() + c ] );
	}
};


struct
List : SliP {
	V< SP< SliP > >																	$;

	List( V< SP< SliP > > const& $ ) :	$( $ ) {}

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
Sentence : List {
	Sentence( V< SP< SliP > > const& $ ) : List( $ ) {}

	string
	REPR() const override { return ListString( U'(', U')' ); }
};

struct
Procedure : List {
	Procedure( V< SP< SliP > > const& $ ) : List( $ ) {}

	string
	REPR() const override { return ListString( U'{', U'}' ); }
};

struct
Parallel : List {
	Parallel( V< SP< SliP > > const& $ ) : List( $ ) {}

	string
	REPR() const override { return ListString( U'«', U'»' ); }
};

struct
iReader {
	virtual	bool		Avail()		= 0;
	virtual	char32_t	Read()		= 0;
	virtual	char32_t	Peek()		= 0;
};

struct
StringReader : iReader {
	V< char32_t >	$;
	size_t			_ = 0;

	StringReader( const string& $ ) : $( Us_string( $ ) ) {}

	bool		Avail()		{ return _ < $.size()	; }
	char32_t	Read()		{ return $[ _++ ]		; }
	char32_t	Peek()		{ return $[ _ ]			; }
};

extern SP< SliP >
Read( iReader&, char32_t );

extern SP< SliP >
Eval( SP< Context >, SP< SliP > );

