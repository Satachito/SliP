#include "SliP.hpp"

US<char32_t>
SoloChars = {
	U'Α',	U'Β',	U'Γ',	U'Δ',	U'Ε',	U'Ζ',	U'Η',	U'Θ',	U'Ι',	U'Κ',	U'Λ',	U'Μ',	U'Ν',	U'Ξ',	U'Ο',	U'Π',	U'Ρ',	U'Σ',	U'Τ',	U'Υ',	U'Φ',	U'Χ',	U'Ψ',	U'Ω'
,	U'α',	U'β',	U'γ',	U'δ',	U'ε',	U'ζ',	U'η',	U'θ',	U'ι',	U'κ',	U'λ',	U'μ',	U'ν',	U'ξ',	U'ο',	U'π',	U'ρ',	U'σ',	U'τ',	U'υ',	U'φ',	U'χ',	U'ψ',	U'ω'
,	U'ς'	//	Σの語尾系
,	U'𝑒'	//	U'\U0001D452'
,	U'∞'
,	U'∅'	//	空集合
,	U'⊤'	//	Verum
,	U'⊥'	//	Falsum

,	U'!'
,	U'#'
,	U'$'
,	U'%'
,	U'\''
,	U'*'
,	U','
,	U'.'
,	U'/'
,	U':'
,	U';'
,	U'?'
,	U'`'
,	U'~'
,	U'¡'
,	U'¤'
,	U'¦'
,	U'§'
,	U'¬'
,	U'±'	//	nCols of matrix
,	U'¶'
,	U'·'
,	U'¿'
,	U'∈'
,	U'∋'
,	U'⊂'
,	U'⊃'
,	U'∩'
,	U'∪'
};

US<char32_t>
OperatorChars = {
	U'&'
,	U'|'
,	U'^'
,	U'+'
,	U'-'
,	U'×'
,	U'÷'
,	U'='
,	U'<'
,	U'>'
,	U'@'
};

US<char32_t>
BreakingChars = {
	U'\\'
,	U'"'
,	U'('
,	U')'
,	U'['
,	U']'
,	U'{'
,	U'}'
,	U'⟨'	//	27E8
,	U'⟩'	//	27E9
,	U'«'
,	U'»'
};

extern SP< SliP >
Read( SP< Context >, iReader&, char32_t );

extern SP< SliP >
Eval( SP< Context >, SP< SliP > );

V< SP< SliP > >
ReadList( SP< Context > C, iReader& R, char32_t close ) {

	V< SP< SliP > > $;
	while ( auto _ = Read( C, R, close ) ) $.push_back( _ );
	return $;
}

void
WhenEscaped( V< char32_t >& $, char32_t _ ) {
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

string
ReadNameRaw( iReader& R, char32_t initial ) {

	if( has( SoloChars, initial ) ) return string_Us( V< char32_t >{ initial } );

	V< char32_t >
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

SP< Literal >
CreateLiteral( iReader& R, char32_t terminator ) {
	auto escaped = false;
	V< char32_t >	$;
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
	_Z( "Unterminated string: " + string_Us( $ ) );
}

SP< SliP >
Read( SP< Context > C, iReader& R, char32_t terminator ) {

	while ( R.Avail() ) {
		auto _ = R.Read();
		if( _ == terminator )		return nullptr;
		if( IsBreakingWhite( _ ) )	continue;
		if( IsDigit( _ ) ) {
			V< char32_t > ${ _ };
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
			if( dotRead ) return MS<Float >( stof( string_Us( $ ) ) );
			try {
				return MS<Bits	>( stol( string_Us( $ ) ) );
			} catch( out_of_range const& ) {
				return MS<Float >( stof( string_Us( $ ) ) );
			}
		}
		switch ( _ ) {
		case U'\\'	: _Z( "Invalid escape" );
		case U']'	:
		case U'⟩'	:
		case U'}'	:
		case U')'	:
		case U'»'	: _Z( "Detect close parenthesis" );
		case U'"'	: return CreateLiteral( R, _ );
		case U'`'	: return CreateLiteral( R, _ );
		case U'['	: return MS< List		>( ReadList( C, R, U']' ) );
		case U'⟨'	: return MS< Matrix		>( ReadList( C, R, U'⟩' ) );
		case U'{'	: return MS< Procedure	>( ReadList( C, R, U'}' ) );
		case U'«'	: return MS< Parallel	>( ReadList( C, R, U'»' ) );
		case U'('	: return MS< Sentence	>( ReadList( C, R, U')' ) );
		default		:
			{	auto name = ReadNameRaw( R, _ );
				extern SP< Prefix >	prefixPlus;
				if(	name == "+" ) return prefixPlus;
				extern SP< Prefix >	prefixMinus;
				if(	name == "-" ) return prefixMinus;
			//	TODO: Follow context chain
				auto it = C->$.find( name );
				return it == C->$.end()
				?	MS< Name >( name )
				:	it->second
				;
			}
		}
	}
	return 0;
}

