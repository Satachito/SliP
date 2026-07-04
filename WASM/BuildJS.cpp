//	GRAPHIC EXTENSIONS	( WEB ONLY )
//	Port of the graphic section of the JS engine ( JS/_.js ) on top of emscripten::val.

#include "../C++/SliP.hpp"

#include <emscripten/val.h>
using namespace emscripten;

extern SP< SliP >	T;
extern SP< SliP >	Nil;

struct
Val	: SliP {
	val	$;
	Val( val $ ) : $( $ ) {}
	string
	REPR() const override {
		return val::global( "String" )( $ ).as< string >();
	}
};

//	SliP -> JS value. Lists become JS arrays, elements evaluated like RegisterFloatPairPrefix does.
static val
ToVal( SP< Context > C, SP< SliP > _ ) {
	if( auto v = Cast< Val		>( _ ) )	return v->$;
	if( auto b = Cast< Bits		>( _ ) )	return val( (double)b->$ );
	if( auto n = Cast< Numeric	>( _ ) )	return val( n->Double() );
	if( auto l = Cast< Literal	>( _ ) )	return val( l->$ );
	if( auto l = Cast< List		>( _ ) ) {
		auto $ = val::array();
		for( auto const& e : l->$ ) $.call< void >( "push", ToVal( C, EvalIsolated( C, e ) ) );
		return $;
	}
	if( _ == T )							return val( true );
	_Z( "Cannot pass to JS: " + _->REPR() );
}

//	JS value -> SliP. Integral numbers become Bits so they work with bitwise infixes ( COLOR_BUFFER_BIT | ... ).
static SP< SliP >
ByVal( val _ ) {
	if( _.isUndefined() || _.isNull() )	return Nil;
	if( _.isTrue() )					return T;
	if( _.isFalse() )					return Nil;
	if( _.isNumber() ) {
		auto $ = _.as< double >();
		return trunc( $ ) == $ && abs( $ ) <= 0x1p53
		?	(SP< SliP >)MS< Bits	>( (int64_t)$ )
		:	(SP< SliP >)MS< Float	>( $ )
		;
	}
	if( _.isString() )					return MS< Literal >( _.as< string >(), U'"' );
	if( val::global( "Array" ).call< bool >( "isArray", _ ) ) {
		V< SP< SliP > > $;
		auto length = _[ "length" ].as< unsigned >();
		for( unsigned i = 0; i < length; i++ ) $.push_back( ByVal( _[ i ] ) );
		return MS< List >( $ );
	}
	return MS< Val >( _ );
}

static val
JSObject( SP< SliP > _ ) {
	return Z( "Operand is not a JS object: " + _->REPR(), Cast< Val >( _ ) )->$;
}

//	The evaluated operand of a prefix: a List spreads into arguments, anything else is a single argument.
static val
Args( SP< Context > C, SP< SliP > _ ) {
	auto v = Eval( C, _ );
	if( auto l = Cast< List >( v ) ) {
		auto $ = val::array();
		for( auto const& e : l->$ ) $.call< void >( "push", ToVal( C, EvalIsolated( C, e ) ) );
		return $;
	}
	auto $ = val::array();
	$.call< void >( "push", ToVal( C, v ) );
	return $;
}

static val
ApplyJS( val o, string const& method, val args ) {
	return val::global( "Reflect" ).call< val >( "apply", o[ method ], o, args );
}

void
BuildJS() {

	struct M { string label; string method; };

//	CANVAS: [ width height ( left ) ( top ) ( "webgl" ) ] : canvas
	Register< Unary >(
		[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			auto list = Z( "canvas: operand must be a list: " + _->REPR(), Cast< List >( _ ) );
			V< double >	nums;
			auto		webgl = false;
			for( auto const& e : list->$ ) {
				auto v = EvalIsolated( C, e );
				if( auto literal = Cast< Literal >( v ) )	webgl = webgl || literal->$ == "webgl";
				else if( auto numeric = Cast< Numeric >( v ) )	nums.push_back( numeric->Double() );
			}
			if( nums.size() < 2 ) _Z( "canvas: needs width and height" );
			val options = val::object();
			options.set( "width"	, nums[ 0 ] );
			options.set( "height"	, nums[ 1 ] );
			if( nums.size() > 2 ) options.set( "left"	, nums[ 2 ] );
			if( nums.size() > 3 ) options.set( "top"	, nums[ 3 ] );
			if( webgl ) options.set( "context", string( "webgl" ) );
			return MS< Val >( val::global( "window" ).call< val >( "createCanvas", options ) );
		}
	,	"canvas"
	);

//	Unary: call without arguments, return the receiver ( for chaining )
	for( auto const& _ : V< string >{
		"save", "restore", "resetTransform", "beginPath", "closePath", "fill", "stroke", "clip"
	} ) Register< Unary >(
		[ _ ]( SP< Context > C, SP< SliP > l ) -> SP< SliP > {
			ApplyJS( JSObject( l ), _, val::array() );
			return l;
		}
	,	_
	);

//	Unary: call without arguments, return the value
	for( auto const& _ : V< string >{
		"createProgram", "createBuffer", "getError", "getLineDash"
	} ) Register< Unary >(
		[ _ ]( SP< Context > C, SP< SliP > l ) -> SP< SliP > {
			return ByVal( ApplyJS( JSObject( l ), _, val::array() ) );
		}
	,	_
	);

//	Unary: read a property ( WebGL constants etc. ), return the value
	for( auto const& _ : V< string >{
		"DELETE_STATUS", "COMPILE_STATUS", "SHADER_TYPE"
	,	"VERTEX_SHADER", "FRAGMENT_SHADER"
	,	"LINK_STATUS", "VALIDATE_STATUS", "ATTACHED_SHADERS"
	,	"ACTIVE_ATTRIBUTES", "ACTIVE_UNIFORMS"
	,	"TRANSFORM_FEEDBACK_BUFFER_MODE", "TRANSFORM_FEEDBACK_VARYINGS", "ACTIVE_UNIFORM_BLOCKS"
	,	"BYTE", "SHORT", "UNSIGNED_BYTE", "UNSIGNED_SHORT", "FLOAT", "HALF_FLOAT"
	,	"ARRAY_BUFFER", "STATIC_DRAW"
	,	"COLOR_BUFFER_BIT", "DEPTH_BUFFER_BIT", "STENCIL_BUFFER_BIT"
	,	"POINTS", "LINE_STRIP", "LINE_LOOP", "LINES", "TRIANGLE_STRIP", "TRIANGLE_FAN", "TRIANGLES"
	} ) Register< Unary >(
		[ _ ]( SP< Context > C, SP< SliP > l ) -> SP< SliP > {
			return ByVal( JSObject( l )[ _ ] );
		}
	,	_
	);

//	Unary: ImageData properties
	for( auto const& _ : V< M >{
		{ "imageDataWidth"	, "width"	}
	,	{ "imageDataHeight"	, "height"	}
	,	{ "imageDataArray"	, "data"	}
	} ) Register< Unary >(
		[ _ ]( SP< Context > C, SP< SliP > l ) -> SP< SliP > {
			return ByVal( JSObject( l )[ _.method ] );
		}
	,	_.label
	);

//	Unary: getTransform -> [ a b c d e f ]
	Register< Unary >(
		[]( SP< Context > C, SP< SliP > l ) -> SP< SliP > {
			auto m = ApplyJS( JSObject( l ), "getTransform", val::array() );
			V< SP< SliP > > $;
			for( auto const& k : { "a", "b", "c", "d", "e", "f" } ) $.push_back( ByVal( m[ k ] ) );
			return MS< List >( $ );
		}
	,	"getTransform"
	);

//	Prefix -> Unary: call with arguments, return the receiver ( for chaining )
	for( auto const& _ : V< M >{
		{ "translate"				, ""				}
	,	{ "scale"					, ""				}
	,	{ "rotate"					, ""				}
	,	{ "transform"				, "setTransform"	}
	,	{ "moveTo"					, ""				}
	,	{ "lineTo"					, ""				}
	,	{ "quadraticCurveTo"		, ""				}
	,	{ "bezierCurveTo"			, ""				}
	,	{ "arcTo"					, ""				}
	,	{ "arc"						, ""				}
	,	{ "rect"					, ""				}
	,	{ "ellipse"					, ""				}
	,	{ "fillWith"				, "fill"			}
	,	{ "strokeWith"				, "stroke"			}
	,	{ "clipWith"				, "clip"			}
	,	{ "strokeRect"				, ""				}
	,	{ "fillRect"				, ""				}
	,	{ "clearRect"				, ""				}
	,	{ "strokeText"				, ""				}
	,	{ "fillText"				, ""				}
	,	{ "putImageData"			, ""				}
	,	{ "addColorStop"			, ""				}
	,	{ "shaderSource"			, ""				}
	,	{ "compileShader"			, ""				}
	,	{ "attachShader"			, ""				}
	,	{ "deleteShader"			, ""				}
	,	{ "linkProgram"				, ""				}
	,	{ "bindBuffer"				, ""				}
	,	{ "viewport"				, ""				}
	,	{ "clear"					, ""				}
	,	{ "useProgram"				, ""				}
	,	{ "uniform1f"				, ""				}
	,	{ "uniform1i"				, ""				}
	,	{ "uniform2f"				, ""				}
	,	{ "uniform2i"				, ""				}
	,	{ "uniform3f"				, ""				}
	,	{ "uniform3i"				, ""				}
	,	{ "uniform4f"				, ""				}
	,	{ "uniform4i"				, ""				}
	,	{ "uniform1fv"				, ""				}
	,	{ "uniform1iv"				, ""				}
	,	{ "uniform2fv"				, ""				}
	,	{ "uniform2iv"				, ""				}
	,	{ "uniform3fv"				, ""				}
	,	{ "uniform3iv"				, ""				}
	,	{ "uniform4fv"				, ""				}
	,	{ "uniform4iv"				, ""				}
	,	{ "vertexAttribPointer"		, ""				}
	,	{ "enableVertexAttribArray"	, ""				}
	,	{ "disableVertexAttribArray", ""				}
	,	{ "drawArrays"				, ""				}
	} ) Register< Prefix >(
		[ label = _.label, method = _.method.size() ? _.method : _.label ]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			auto args = Args( C, _ );
			return MS< Unary >(
				[ method, args ]( SP< Context > C, SP< SliP > l ) -> SP< SliP > {
					ApplyJS( JSObject( l ), method, args );
					return l;
				}
			,	label
			);
		}
	,	_.label
	);

//	Prefix -> Unary: bufferData needs its data argument as a Float32Array
	Register< Prefix >(
		[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			auto args = Args( C, _ );
			args.set( 1, val::global( "Float32Array" ).new_( args[ 1 ] ) );
			return MS< Unary >(
				[ args ]( SP< Context > C, SP< SliP > l ) -> SP< SliP > {
					ApplyJS( JSObject( l ), "bufferData", args );
					return l;
				}
			,	"bufferData"
			);
		}
	,	"bufferData"
	);

//	Prefix -> Unary: setLineDash takes the whole list as its single argument
	Register< Prefix >(
		[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			auto segments = ToVal( C, Eval( C, _ ) );
			return MS< Unary >(
				[ segments ]( SP< Context > C, SP< SliP > l ) -> SP< SliP > {
					JSObject( l ).call< void >( "setLineDash", segments );
					return l;
				}
			,	"lineDash"
			);
		}
	,	"lineDash"
	);

//	Prefix -> Unary: set a property, return the receiver ( for chaining )
	for( auto const& _ : V< string >{
		"fillStyle", "strokeStyle"
	,	"lineCap", "lineJoin", "lineWidth", "lineDashOffset", "miterLimit"
	,	"font", "textAlign", "textBaseline", "direction"
	,	"filter"
	,	"shadowColor", "shadowBlur", "shadowOffsetX", "shadowOffsetY"
	,	"globalAlpha", "globalCompositeOperation", "imageSmoothingQuality"
	} ) Register< Prefix >(
		[ _ ]( SP< Context > C, SP< SliP > operand ) -> SP< SliP > {
			auto v = ToVal( C, Eval( C, operand ) );
			return MS< Unary >(
				[ _, v ]( SP< Context > C, SP< SliP > l ) -> SP< SliP > {
					JSObject( l ).set( _, v );
					return l;
				}
			,	_
			);
		}
	,	_
	);
	Register< Prefix >(
		[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			extern bool IsT( SP< SliP > );
			auto v = IsT( Eval( C, _ ) );
			return MS< Unary >(
				[ v ]( SP< Context > C, SP< SliP > l ) -> SP< SliP > {
					JSObject( l ).set( "imageSmoothingEnabled", v );
					return l;
				}
			,	"imageSmoothingEnabled"
			);
		}
	,	"imageSmoothingEnabled"
	);

//	Prefix -> Unary: call with arguments, return the value
	for( auto const& _ : V< string >{
		"createShader", "getShaderParameter", "getShaderInfoLog", "getProgramParameter"
	,	"getUniformLocation"
	,	"createImageData", "getImageData"
	,	"createConicGradient", "createLinearGradient", "createRadialGradient"
	,	"isPointInPath", "isPointInStroke"
	} ) Register< Prefix >(
		[ _ ]( SP< Context > C, SP< SliP > operand ) -> SP< SliP > {
			auto args = Args( C, operand );
			return MS< Unary >(
				[ _, args ]( SP< Context > C, SP< SliP > l ) -> SP< SliP > {
					return ByVal( ApplyJS( JSObject( l ), _, args ) );
				}
			,	_
			);
		}
	,	_
	);

//	Prefix -> Unary: measureText -> Dict of TextMetrics
	Register< Prefix >(
		[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			auto args = Args( C, _ );
			return MS< Unary >(
				[ args ]( SP< Context > C, SP< SliP > l ) -> SP< SliP > {
					auto tm = ApplyJS( JSObject( l ), "measureText", args );
					auto keys = val::global( "Object" ).call< val >( "keys", tm[ "constructor" ][ "prototype" ] );
					UM< string, SP< SliP > > $;
					auto length = keys[ "length" ].as< unsigned >();
					for( unsigned i = 0; i < length; i++ ) {
						auto k = keys[ i ].as< string >();
						$[ k ] = ByVal( tm[ k ] );
					}
					return MS< Dict >( $ );
				}
			,	"measureText"
			);
		}
	,	"measureText"
	);

//	PATH
	Register< Primitive >(
		[]( SP< Context > ) -> SP< SliP > {
			return MS< Val >( val::global( "Path2D" ).new_() );
		}
	,	"path2D"
	);
	Register< Prefix >(
		[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			return MS< Val >(
				val::global( "Reflect" ).call< val >( "construct", val::global( "Path2D" ), Args( C, _ ) )
			);
		}
	,	"path2DWith"
	);

//	ARRAY EXTENSION ( ImageData.data etc. )
	Register< Prefix >(
		[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			auto index = ToVal( C, Eval( C, _ ) );
			return MS< Unary >(
				[ index ]( SP< Context > C, SP< SliP > l ) -> SP< SliP > {
					return ByVal( JSObject( l )[ index ] );
				}
			,	"get"
			);
		}
	,	"get"
	);
	Register< Prefix >(
		[]( SP< Context > C, SP< SliP > _ ) -> SP< SliP > {
			auto args = Args( C, _ );
			return MS< Unary >(
				[ args ]( SP< Context > C, SP< SliP > l ) -> SP< SliP > {
					JSObject( l ).set( args[ 0 ], args[ 1 ] );
					return l;
				}
			,	"set"
			);
		}
	,	"set"
	);
}
