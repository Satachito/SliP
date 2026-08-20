#include "BH.h"
#include "../C++/Embed.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>

//	The Web build sends these operators straight to a JavaScript Canvas.  Apple
//	hosts have no DOM, so they record a small, platform-neutral display list.
//	SwiftUI replays it after evaluation.  Keeping the display list here means the
//	language surface stays the same while no evaluator thread ever touches a view.

namespace {
	struct Point { double x, y; };
	struct Transform { double a = 1, b = 0, c = 0, d = 1, e = 0, f = 0; };
	struct Draw {
		string kind;
		string color;
		double width = 1;
		V< V< Point > > paths;
	};
	struct Canvas {
		double width = 0, height = 0;
		Transform transform;
		string strokeColor = "black", fillColor = "black";
		double lineWidth = 1;
		V< V< Point > > path;
		V< Draw > draws;
		struct State { Transform transform; string strokeColor, fillColor; double lineWidth; };
		V< State > states;
	};

	struct CanvasValue : SliP {
		SP< Canvas > canvas;
		explicit CanvasValue( SP< Canvas > canvas ) : canvas( std::move( canvas ) ) {}
		string REPR() const override {
			ostringstream out;
			out << "[Canvas " << canvas->width << "×" << canvas->height << "]";
			return out.str();
		}
	};

	struct AppleSession {
		EmbedSession* engine = nullptr;
		V< SP< Canvas > > canvases;
	};

	thread_local V< SP< Canvas > >* activeCanvases = nullptr;
	V< SP< Canvas > > globalCanvases;

	SP< Canvas > CanvasOf( SP< SliP > value ) {
		return Z( "Operand is not a Canvas: " + value->REPR(), Cast< CanvasValue >( value ) )->canvas;
	}

	double Number( SP< SliP > value ) {
		return Z( "Canvas argument is not numeric: " + value->REPR(), Cast< Numeric >( value ) )->Double();
	}

	V< SP< SliP > > Values( SP< Context > context, SP< SliP > operand ) {
		auto value = Eval( context, operand );
		if( auto list = Cast< List >( value ) ) {
			V< SP< SliP > > values;
			for( auto const& item : list->$ ) values.push_back( EvalIsolated( context, item ) );
			return values;
		}
		return { value };
	}

	Point Apply( Transform const& t, double x, double y ) {
		return { t.a * x + t.c * y + t.e, t.b * x + t.d * y + t.f };
	}

	void MoveTo( Canvas& canvas, double x, double y ) {
		canvas.path.push_back( { Apply( canvas.transform, x, y ) } );
	}

	void LineTo( Canvas& canvas, double x, double y ) {
		if( canvas.path.empty() ) MoveTo( canvas, x, y );
		else canvas.path.back().push_back( Apply( canvas.transform, x, y ) );
	}

	void RectPath( Canvas& canvas, double x, double y, double w, double h, bool newPath ) {
		V< Point > points {
			Apply( canvas.transform, x, y ), Apply( canvas.transform, x + w, y ),
			Apply( canvas.transform, x + w, y + h ), Apply( canvas.transform, x, y + h )
		};
		points.push_back( points.front() );
		if( newPath ) canvas.path.push_back( std::move( points ) );
	}

	void AppendDraw( Canvas& canvas, Draw draw, bool merge ) {
		if( merge && !canvas.draws.empty() ) {
			auto& previous = canvas.draws.back();
			if( previous.kind == draw.kind && previous.color == draw.color && previous.width == draw.width ) {
				previous.paths.insert( previous.paths.end(), draw.paths.begin(), draw.paths.end() );
				return;
			}
		}
		canvas.draws.push_back( std::move( draw ) );
	}

	string EscapeJSON( string const& value ) {
		string out;
		for( auto ch : value ) switch( ch ) {
			case '\\': out += "\\\\"; break;
			case '"': out += "\\\""; break;
			case '\n': out += "\\n"; break;
			case '\r': out += "\\r"; break;
			case '\t': out += "\\t"; break;
			default: out += ch;
		}
		return out;
	}

	string CanvasJSON( V< SP< Canvas > > const& canvases ) {
		ostringstream out;
		out << setprecision( 17 ) << '[';
		for( size_t ci = 0; ci < canvases.size(); ++ci ) {
			if( ci ) out << ',';
			auto const& canvas = *canvases[ ci ];
			out << "{\"width\":" << canvas.width << ",\"height\":" << canvas.height << ",\"commands\":[";
			for( size_t di = 0; di < canvas.draws.size(); ++di ) {
				if( di ) out << ',';
				auto const& draw = canvas.draws[ di ];
				out << "{\"kind\":\"" << EscapeJSON( draw.kind ) << "\",\"color\":\""
					<< EscapeJSON( draw.color ) << "\",\"width\":" << draw.width << ",\"paths\":[";
				for( size_t pi = 0; pi < draw.paths.size(); ++pi ) {
					if( pi ) out << ',';
					out << '[';
					for( size_t i = 0; i < draw.paths[ pi ].size(); ++i ) {
						if( i ) out << ',';
						out << '[' << draw.paths[ pi ][ i ].x << ',' << draw.paths[ pi ][ i ].y << ']';
					}
					out << ']';
				}
				out << "]}";
			}
			out << "]}";
		}
		return out.str() + ']';
	}

	void BuildAppleGraphics() {
		Register< Unary >(
			[]( SP< Context > context, SP< SliP > operand ) -> SP< SliP > {
				auto list = Z( "canvas: operand must be a list", Cast< List >( operand ) );
				V< double > numbers;
				for( auto const& item : list->$ ) {
					auto value = EvalIsolated( context, item );
					if( auto literal = Cast< Literal >( value ) ) {
						if( literal->$ == "webgl" ) _Z( "WebGL is not available in the native app" );
					} else numbers.push_back( Number( value ) );
				}
				if( numbers.size() < 2 ) _Z( "canvas: needs width and height" );
				if( !activeCanvases ) _Z( "Canvas is not attached to an Apple session" );
				auto canvas = MS< Canvas >();
				canvas->width = numbers[ 0 ]; canvas->height = numbers[ 1 ];
				activeCanvases->push_back( canvas );
				return MS< CanvasValue >( canvas );
			}, "canvas"
		);

		for( auto const& name : V< string >{ "beginPath", "closePath", "stroke", "fill", "save", "restore", "resetTransform" } )
			Register< Unary >(
				[ name ]( SP< Context >, SP< SliP > receiver ) -> SP< SliP > {
					auto canvas = CanvasOf( receiver );
					if( name == "beginPath" ) canvas->path.clear();
					else if( name == "closePath" ) {
						if( !canvas->path.empty() && !canvas->path.back().empty() ) canvas->path.back().push_back( canvas->path.back().front() );
					} else if( name == "stroke" ) AppendDraw( *canvas, { "stroke", canvas->strokeColor, canvas->lineWidth, canvas->path }, false );
					else if( name == "fill" ) AppendDraw( *canvas, { "fill", canvas->fillColor, 1, canvas->path }, false );
					else if( name == "save" ) canvas->states.push_back( { canvas->transform, canvas->strokeColor, canvas->fillColor, canvas->lineWidth } );
					else if( name == "restore" && !canvas->states.empty() ) {
						auto state = canvas->states.back(); canvas->states.pop_back();
						canvas->transform = state.transform; canvas->strokeColor = state.strokeColor;
						canvas->fillColor = state.fillColor; canvas->lineWidth = state.lineWidth;
					} else if( name == "resetTransform" ) canvas->transform = {};
					return receiver;
				}, name
			);

		for( auto const& name : V< string >{ "strokeStyle", "fillStyle", "lineWidth" } )
			Register< Prefix >(
				[ name ]( SP< Context > context, SP< SliP > operand ) -> SP< SliP > {
					auto value = Eval( context, operand );
					return MS< Unary >(
						[ name, value ]( SP< Context >, SP< SliP > receiver ) -> SP< SliP > {
							auto canvas = CanvasOf( receiver );
							if( name == "lineWidth" ) canvas->lineWidth = Number( value );
							else {
								auto literal = Z( "Canvas color must be a string", Cast< Literal >( value ) );
								( name == "strokeStyle" ? canvas->strokeColor : canvas->fillColor ) = literal->$;
							}
							return receiver;
						}, name
					);
				}, name
			);

		for( auto const& name : V< string >{ "moveTo", "lineTo", "translate", "scale", "rotate", "rect", "fillRect", "strokeRect", "clearRect" } )
			Register< Prefix >(
				[ name ]( SP< Context > context, SP< SliP > operand ) -> SP< SliP > {
					auto values = Values( context, operand );
					V< double > args; for( auto const& value : values ) args.push_back( Number( value ) );
					return MS< Unary >(
						[ name, args ]( SP< Context >, SP< SliP > receiver ) -> SP< SliP > {
							auto canvas = CanvasOf( receiver ); auto& t = canvas->transform;
							if( name == "moveTo" && args.size() >= 2 ) MoveTo( *canvas, args[ 0 ], args[ 1 ] );
							else if( name == "lineTo" && args.size() >= 2 ) LineTo( *canvas, args[ 0 ], args[ 1 ] );
							else if( name == "translate" && args.size() >= 2 ) { t.e += t.a * args[ 0 ] + t.c * args[ 1 ]; t.f += t.b * args[ 0 ] + t.d * args[ 1 ]; }
							else if( name == "scale" && args.size() >= 2 ) { t.a *= args[ 0 ]; t.b *= args[ 0 ]; t.c *= args[ 1 ]; t.d *= args[ 1 ]; }
							else if( name == "rotate" && !args.empty() ) {
								auto cs = cos( args[ 0 ] ), sn = sin( args[ 0 ] ); auto old = t;
								t.a = old.a * cs + old.c * sn; t.b = old.b * cs + old.d * sn;
								t.c = -old.a * sn + old.c * cs; t.d = -old.b * sn + old.d * cs;
							} else if( name == "rect" && args.size() >= 4 ) RectPath( *canvas, args[ 0 ], args[ 1 ], args[ 2 ], args[ 3 ], true );
							else if( ( name == "fillRect" || name == "strokeRect" || name == "clearRect" ) && args.size() >= 4 ) {
								V< V< Point > > saved = canvas->path; canvas->path.clear();
								RectPath( *canvas, args[ 0 ], args[ 1 ], args[ 2 ], args[ 3 ], true );
								AppendDraw( *canvas, { name == "strokeRect" ? "stroke" : name == "clearRect" ? "clear" : "fill",
									name == "strokeRect" ? canvas->strokeColor : canvas->fillColor, canvas->lineWidth, canvas->path }, true );
								canvas->path = std::move( saved );
							}
							return receiver;
						}, name
					);
				}, name
			);
	}
}

//	The bridge to Swift.  Everything it exposes returns the JSON described in
//	Embed.hpp, so a SliP error arrives as data rather than as a C++ exception —
//	which is what matters here, because an exception thrown through this boundary
//	has nowhere to go but std::terminate.
//
//	Build() is latched on first use.  CLI, TEST and WASM each call it once from
//	their single entry point; the bridge has no such point, since Swift may call
//	any of these first and may call them again.  A file-scope initializer would
//	be a static-init-order race against BUILTINS in SliP.cpp.
static void
BuildOnce() {
	extern void Build();
	static auto
	$ = ( Build(), BuildAppleGraphics(), true );
	(void)$;
}

static char*
Copy( string const& _ ) {
	auto $ = new char[ _.length() + 1 ];
	strcpy( $, _.c_str() );
	return $;
}

extern "C" char*
BH_Version() {
	return Copy( Version() );
}

extern "C" char*
BH_REP( const char* source ) {
	BuildOnce();
	globalCanvases.clear(); activeCanvases = &globalCanvases;
	return Copy( REP( string( source ) ) );
}

extern "C" char*
BH_REPL( const char* source ) {
	BuildOnce();
	globalCanvases.clear(); activeCanvases = &globalCanvases;
	return Copy( REPL( string( source ) ) );
}

extern "C" char*
BH_Sugared( const char* source ) {
	BuildOnce();
	globalCanvases.clear(); activeCanvases = &globalCanvases;
	return Copy( Sugared( string( source ) ) );
}

extern "C" void
BH_Reset() {
	BuildOnce();
	Reset();
}

extern "C" void
BH_SetRoundPrecision( int _ ) {
	SetRoundPrecision( _ );
}

extern "C" BH_Session
BH_SessionCreate() {
	BuildOnce();
	return new AppleSession{ NewEmbedSession(), {} };
}

extern "C" void
BH_SessionDestroy( BH_Session session ) {
	auto apple = static_cast< AppleSession* >( session );
	if( activeCanvases == &apple->canvases ) activeCanvases = nullptr;
	DeleteEmbedSession( apple->engine );
	delete apple;
}

extern "C" char*
BH_SessionREPL( BH_Session session, const char* source ) {
	auto apple = static_cast< AppleSession* >( session );
	apple->canvases.clear(); activeCanvases = &apple->canvases;
	return Copy( SessionREPL( apple->engine, string( source ) ) );
}

extern "C" char*
BH_SessionSugared( BH_Session session, const char* source ) {
	auto apple = static_cast< AppleSession* >( session );
	apple->canvases.clear(); activeCanvases = &apple->canvases;
	return Copy( SessionSugared( apple->engine, string( source ) ) );
}

extern "C" void
BH_SessionReset( BH_Session session ) {
	auto apple = static_cast< AppleSession* >( session );
	apple->canvases.clear();
	ResetEmbedSession( apple->engine );
}

extern "C" char*
BH_SessionCanvases( BH_Session session ) {
	return Copy( CanvasJSON( static_cast< AppleSession* >( session )->canvases ) );
}

extern "C" void
BH_Free( char* _ ) {
	delete[] _;
}
