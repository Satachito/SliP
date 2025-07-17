#pragma once

inline static unordered_set<char32_t>
SoloChars = {
	U'Α',	U'Β',	U'Γ',	U'Δ',	U'Ε',	U'Ζ',	U'Η',	U'Θ',	U'Ι',	U'Κ',	U'Λ',	U'Μ',	U'Ν',	U'Ξ',	U'Ο',	U'Π',	U'Ρ',	U'Σ',	U'Τ',	U'Υ',	U'Φ',	U'Χ',	U'Ψ',	U'Ω'
,	U'α',	U'β',	U'γ',	U'δ',	U'ε',	U'ζ',	U'η',	U'θ',	U'ι',	U'κ',	U'λ',	U'μ',	U'ν',	U'ξ',	U'ο',	U'π',	U'ρ',	U'σ',	U'τ',	U'υ',	U'φ',	U'χ',	U'ψ',	U'ω'
,	U'ς'	//	Σの語尾系
,	U'𝑒'	//	U'\U0001D452'
,	U'∞'
,	U'∅'	//	空集合
,	U'⊤'	//	Verum
,	U'⊥'	//	Falsum
};

inline static unordered_set<char32_t>
OperatorChars = {
	U'!'
,	U'#'
,	U'$'
,	U'%'
,	U'&'
,	U'*'
,	U'+'
,	U','
,	U'-'
,	U'.'
,	U'/'
,	U':'
,	U';'
,	U'<'
,	U'='
,	U'>'
,	U'?'
,	U'@'
,	U'^'
,	U'`'
,	U'|'
,	U'~'
,	U'¡'
,	U'¤'
,	U'¦'
,	U'§'
,	U'¬'
,	U'±'
,	U'¶'
,	U'·'
,	U'¿'
,	U'×'
,	U'÷'
,	U'∈'
,	U'∋'
,	U'⊂'
,	U'⊃'
,	U'∩'
,	U'∪'
};

inline static unordered_set<char32_t>
BreakingChars = {
	U'\\'
,	U'\''
,	U'"'
,	U'('
,	U')'
,	U'['
,	U']'
,	U'{'
,	U'}'
,	U'⟨'
,	U'⟩'
,	U'«'
,	U'»'
};

struct
iReader {
	virtual	bool		Avail()		= 0;
	virtual	char32_t	Read()		= 0;
	virtual	char32_t	Peek()		= 0;
};

inline SP< SliP >
Read( iReader& R, char32_t terminator );

inline vector< SP< SliP > >
ReadList( iReader& _, char32_t close ) {
	vector< SP< SliP > > $;

	auto slip = Read( _, close );
	if( !slip ) return $;
	auto infix = Cast< Infix >( slip );
	if( infix ) {
		if(	infix->label == "+" ) {
			$.push_back( prefixPlus );
		} else if( infix->label == "-" ) {
			$.push_back( prefixMinus );
		} else {
			$.push_back( slip );
		}
	} else {
		$.push_back( slip );
	}

	while ( SP< SliP > draft = Read( _, close ) ) {
		auto draftInfix = Cast< Infix >( draft );
		if( infix && draftInfix ) {
			if(	draftInfix->label == "+" ) {
				$.push_back( prefixPlus );
			} else if( draftInfix->label == "-" ) {
				$.push_back( prefixMinus );
			} else {
				throw runtime_error( "Syntax error: " + infix->label + ' ' + draftInfix->label );
			}
		} else {
			$.push_back( draft );
		}
		slip = draft;
		infix = draftInfix;
	}
//	DEBUG
//	for ( auto const& _: $ ) {
//		auto prefix = Cast< Prefix >( _ );
//		cout << ( prefix ? ( ':' + prefix->label + ':' ) : _->REPR() ) << ' ';
//	}
//	cout << endl;
//
	return $;
}

inline void
WhenEscaped( vector< char32_t >& $, char32_t _ ) {
	switch ( _ ) {
	case U'\\'	: $.push_back( '\\'	); break;
	case U'0'	: $.push_back( '\0'	); break;
	case U'f'	: $.push_back( '\f'	); break;
	case U'n'	: $.push_back( '\n'	); break;
	case U'r'	: $.push_back( '\r'	); break;
	case U't'	: $.push_back( '\t'	); break;
	case U'v'	: $.push_back( '\v'	); break;
	default		: $.push_back( _	); break;
	}
}

inline string
ReadNameRaw( iReader& R, char32_t initial ) {

	if( has( SoloChars, initial ) ) return string_Us( vector< char32_t >{ initial } );

	vector< char32_t >
	${ initial };

	auto
	readingOperator = has( OperatorChars, initial );

	while ( R.Avail() ) {
		if ( readingOperator ) {
			if( !has( OperatorChars	, R.Peek() ) ) break;
			$.push_back( R.Read() );
		} else {
			if( has( OperatorChars	, R.Peek() ) ) break;
			if( has( SoloChars		, R.Peek() ) ) break;
			if( has( BreakingChars	, R.Peek() ) ) break;

			auto _ = R.Read();

			if( IsBreakingWhite( _ ) ) break;
			$.push_back( _ );
		}
	}
	return string_Us( $ );
}

inline SP< Literal >
CreateLiteral( iReader& R, char32_t terminator ) {
	auto escaped = false;
	vector< char32_t >	$;
	while ( R.Avail() ) {
		auto _ = R.Read();
		if( escaped ) {
			escaped = false;
			WhenEscaped( $, _ );
		} else {
			if( _ == terminator ) return MS< Literal >( string_Us( $ ), terminator );
			if( _ == '\\' )	escaped = true;
			else $.push_back( _ );
		}
	}
	throw runtime_error( "Unterminated string: " + string_Us( $ ) );
}

inline SP< SliP >
Read( iReader& R, char32_t terminator ) {
	while ( R.Avail() ) {
		auto _ = R.Read();
		if( _ == terminator )		return 0;
		if( IsBreakingWhite( _ ) )	continue;
		if( IsDigit( _ ) ) {
			vector< char32_t > ${ _ };
			auto dotRead = false;
			while ( R.Avail() ) {
				if( R.Peek() == U'.' ) {
					if( dotRead ) break;
					dotRead = true;
				} else if( !IsDigit( R.Peek() ) ) {
					break;
				}
				$.push_back( R.Read() );
			}
			return dotRead
			?	Cast< SliP >( MS<Float	>( stof( string_Us( $ ) ) ) )
			:	Cast< SliP >( MS<Bits	>( stoi( string_Us( $ ) ) ) )
			;
		}
		switch ( _ ) {
		case U'\\'	: throw runtime_error( "Invalid escape" );
		case U']'	:
		case U'⟩'	:
		case U'}'	:
		case U')'	:
		case U'»'	: throw runtime_error( "Detect close parenthesis" );
		case U'"'	: return CreateLiteral( R, _ );
		case U'`'	: return CreateLiteral( R, _ );
		case U'['	: return MS< List		>( ReadList( R, U']' ) );
		case U'⟨'	: return MS< Matrix		>( ReadList( R, U'⟩' ) );
		case U'{'	: return MS< Procedure	>( ReadList( R, U'}' ) );
		case U'«'	: return MS< Parallel	>( ReadList( R, U'»' ) );
		case U'('	: return MS< Sentence	>( ReadList( R, U')' ) );
		default		:
			{	auto name = ReadNameRaw( R, _ );
				auto it = Builtins.find( name );
				return it == Builtins.end()
				?	MS< Name >( name )
				:	it->second
				;
			}
		}
	}
	return 0;
}
