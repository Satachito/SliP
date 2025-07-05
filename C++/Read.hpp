#pragma once

inline static unordered_set<char32_t>
SoloChars = {
	U'Α'	,U'Β'	,U'Γ'	,U'Δ'	,U'Ε'	,U'Ζ'	,U'Η'	,U'Θ'	,U'Ι'	,U'Κ'	,U'Λ'	,U'Μ'
,	U'Ν'	,U'Ξ'	,U'Ο'	,U'Π'	,U'Ρ'	,U'Σ'	,U'Τ'	,U'Υ'	,U'Φ'	,U'Χ'	,U'Ψ'	,U'Ω'
,	U'α'	,U'β'	,U'γ'	,U'δ'	,U'ε'	,U'ζ'	,U'η'	,U'θ'	,U'ι'	,U'κ'	,U'λ'	,U'μ'
,	U'ν'	,U'ξ'	,U'ο'	,U'π'	,U'ρ'	,U'σ'	,U'τ'	,U'υ'	,U'φ'	,U'χ'	,U'ψ'	,U'ω'
,	U'ς'	//	Σの語尾系
,	U'𝑒'		//	U'\U0001D452'
,	U'∞'
};

inline static unordered_set<char32_t>
BreakingChars = {
	U'!'	,U'"'	,U'#'	,U'$'	,U'%'	,U'&'	,U'\\'	,U'('	,U')'	,U'*'	,U'+'	,U','	,U'-'	,U'.'	,U'/'	,U':'	,U';'
,	U'<'	,U'='	,U'>'	,U'?'	,U'@'	,U'['	,U']'	,U'^'	,U'`'	,U'{'	,U'|'	,U'}'	,U'~'	,U'¡'	,U'¤'	,U'¦'	,U'§'
,	U'«'	,U'¬'	,U'±'	,U'µ'	,U'¶'	,U'·'	,U'»'	,U'¿'	,U'×'	,U'÷'	,U'∈'	,U'∋'	,U'⊂'	,U'⊃'	,U'∩'	,U'∪'	,U'∅'
};

struct
iReader {
	virtual	bool		Avail()		= 0;
	virtual	char32_t	Read()		= 0;
	virtual	char32_t	Peek()		= 0;
	virtual	void		Forward()	= 0;
	virtual	void		Backward()	= 0;
};

inline SP< SliP >
Read( iReader& R, char32_t terminator );

inline vector< SP< SliP > >
ReadList( iReader& _, char32_t close ) {
	vector< SP< SliP > > $;

	auto slip = Read( _, close );
	$.push_back( slip );
	auto infix = Cast< Infix >( slip );
	while ( SP< SliP > draft = Read( _, close ) ) {
		auto draftInfix = Cast< Infix >( draft );
		if( infix && draftInfix ) {
			if(	draftInfix->label == "+" ) {
				$.push_back( prefixPlus );
			} else if ( draftInfix->label == "-" ) {
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
	return $;
}

inline SP< Name >
CreateName( iReader& R, char32_t initial ) {

	if( contains( SoloChars, initial ) ) return MS< Name >( string_Us( vector< char32_t >{ initial } ) );

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
			if( IsBreakingWhite( _ )			) break;
			if( _ == '\\' )	escaped = true;
			else $.push_back( _ );
		}
	}
	return MS<Name>( string_Us( $ ) );
}
inline SP< Literal >
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
			if( dotRead )	return MS<Float	>( stod( string_Us( $ ) ) );
			else			return MS<Bits		>( stoi( string_Us( $ ) ) );
		}
		switch ( _ ) {
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
			auto label0 = string_Us( vector< char32_t >{ _ } );
			auto it0 = find_if( Builtins.begin(), Builtins.end(), [ & ]( SP< Function > _ ){ return _->label == label0; } );
			if( it0 != Builtins.end() ) {
				auto label = label0 + string_Us( vector< char32_t >{ R.Peek() } );
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

