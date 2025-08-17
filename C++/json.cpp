#include "SliP.hpp"

SP< SliP > ByJSON( iReader& R );

static bool
IsJSONSpace( char32_t _ ) {
	return _ == U' ' || _ == U'\t' || _ == U'\n' || _ == U'\r';
}

static string
Token( iReader& R ) {
	V< char32_t > $;
	while( R.Avail() ) {
		auto _ = R.Peek();
		if(	IsJSONSpace( _ ) || _ == ',' ) break;
		$.push_back( R.Read() );
	}
	return string_Us( $ );
}

static bool
SkipWS( iReader& R ) {
    while( R.Avail() ) {
		auto _ = R.Peek();
		if( !IsJSONSpace( _ ) ) return true;
		R.Read();
	}
	return false;
}

static SP< Numeric >
parse_number( char32_t initial, iReader& R ) {
    string $;
	$ += initial;
    while( R.Avail() && IsDigit( R.Peek() ) ) $ += R.Read();
    if( R.Avail() && R.Peek() == '.' ) {
        $ += R.Read();
        while( R.Avail() && IsDigit( R.Peek() ) ) $ += R.Read();
        return MS<Float>( stod( $ ) );
    }
    try {
        return MS<Bits>( stoll( $ ) );
    } catch( const out_of_range& ) {
		try {
			return MS<Float>( static_cast<double>( stoull( $ ) ) );
		} catch ( const out_of_range& ) {
			return MS<Float>( stod( $ ) );
		}
    }
}

static SP< Literal >
parse_string( iReader& R ) {
    string $;

	//	TODO: escape
    while ( R.Avail() ) {
		auto _ = R.Read();
		if( _ == U'"' ) return MS< Literal >( $, U'`' );
		$ += _;
	}
    _Z( "Invalid JSON String" );
}

static SP< List >
parse_array( iReader& R ) {
    V< SP< SliP > > $;
	auto has = false;
	while( SkipWS( R ) ) {
		auto _ = R.Peek();
		//	[:vim
		if( _ == U']' ) {
			R.Read();
			return MS< List >( $ );
		}
        if( has ) {
			if( _ != U',' ) 	break;
			R.Read();
		} else has = true;
        $.push_back( ByJSON( R ) );
	}
	_Z( "Invalid JSON Array" );
}

static SP< Dict >
parse_object( iReader& R ) {
    UM< string, SP< SliP > > $;
	auto has = false;
	while( SkipWS( R ) ) {
		auto _ = R.Read();
		//	{:vim
		if( _ == U'}' )			return MS< Dict >( $ );
        if( has ) {
			if( _ != U',' )		break;
			if( !SkipWS( R ) )	break;
			_ = R.Read();
		} else has = true;
		if( _ != U'"' )			break;
		auto K = parse_string( R )->$;
		if( !SkipWS( R ) )		break;
		if( R.Read() != U':' )	break;
		$[ K ] = ByJSON( R );
    }
	_Z( "Invalid JSON Object" );
}

extern	SP< SliP >	T;
extern	SP< SliP >	Nil;

SP<SliP> ByJSON( iReader& R ) {
	while( true ) {
		if( !SkipWS( R ) ) break;
		char32_t _ = R.Read();
		if ( _ == U'"' ) return parse_string( R );
		if ( _ == U'{' ) return parse_object( R );	//	}
		if ( _ == U'[' ) return parse_array( R );	//	]
		if ( _ == U't' ) { if ( Token( R ) != "rue"		) break; else return T	; }
		if ( _ == U'f' ) { if ( Token( R ) != "alse"	) break; else return Nil; }
		if ( _ == U'n' ) { if ( Token( R ) != "ull"		) break; else return Nil; }
		if ( IsDigit( _ ) || _ == U'+' || _ == U'-' ) return parse_number( _, R );
		break;
	}
	_Z( "Invalid JSON" );
}

string
ToJSON( SP< SliP > _ ) {
	if(	auto bits = Cast< Bits >( _ ) ) return to_string( bits->$ );
	if(	auto numeric = Cast< Numeric >( _ ) ) return double_to_string( numeric->Double() );
	if(	auto literal = Cast< Literal >( _ ) ) return "\"" + literal->$ + "\"";
	if(	auto list = Cast< List >( _ ) ) {
		auto count = 0;
		return reduce(
			list->$
		,	[ & ]( string const& $, SP< SliP > _, size_t index ){ return $ + ( count++ ? "," : "" ) + ToJSON( _ ); }
		,	string( "[" )
		) + "]";
	}
	if(	auto dict = Cast< Dict >( _ ) ) {
		auto count = 0;
		return reduce(
			dict->$
		,	[ & ]( string const& $, auto const& kv ){ return $ + ( count++ ? ",\"" : "\"" ) + get< 0 >( kv ) + "\":" + ToJSON( get< 1 >( kv ) ); }
		,	string( "{" )
		) + "}";
	}
	return "";
};
