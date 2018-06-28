//	Written by Satoru Ogura, Tokyo.
//
import Foundation

func
SliPBuiltins() -> [ String: Object ] {
	var	v = [ String: Object ]()
	v[ "for" ] = Unary( "for" ) { c, p in
		if	let wArgs = p as? List
		,	wArgs.m.count >= 2
		,	let wList = wArgs.m[ 0 ] as? List {
			var	v = [ Object ]()
			for w in wList.m {
				c.Push( w )
				defer{ c.Pop() }
				v.append( try wArgs.m[ 1 ].Eval( c ) )
			}
			return ListL( v )
		}
		throw SliPError.RuntimeError( "\(p):for" )
	}
	v[ "int" ] = Unary( "int" ) { c, p in
		switch p {
		case let w as StringL:
			return IntNumber( MakeBigInteger( w.m ) )
		case let w as List:
			if let wString = w.m[ 0 ] as? StringL {
				let	wRadix = ( w.m[ 1 ] as? IntNumber ) ?? IntNumber( 10 )
				let	wMinus = w.m.count < 3 ? false : IsNil( w.m[ 2 ] )
				return IntNumber(
					MakeBigInteger(
						wString.m
					,	wRadix.m.NativeInt
					,	wMinus
					)
				)
			}
		default:
			break
		}
		throw SliPError.RuntimeError( "\(p):int" )
	}
	v[ "qr" ] = Unary( "qr" ) { c, p in
		if	let w = p as? List, w.m.count == 2
		,	let wL = w.m[ 0 ] as? IntNumber
		,	let wR = w.m[ 1 ] as? IntNumber {
			
			let	( vQuo, vRem ) = QR( ArraySlice( wL.m.m ), ArraySlice( wR.m.m ) )
			return ListL(
				[	IntNumber( BigInteger( vQuo, wL.m.minus != wR.m.minus ) )
				,	IntNumber( BigInteger( vRem, wL.m.minus != wR.m.minus ) )
				]
			)
		}
		throw SliPError.RuntimeError( "\(p):qr" )
	}
	v[ "string" ] = Unary( "string" ) {	c, p in
		if	let w = p as? List, w.m.count == 2
		,	let wL = w.m[ 0 ] as? IntNumber
		,	let wR = w.m[ 1 ] as? IntNumber {
			
			return StringL( wL.m.StringRepresentation( wR.m.NativeInt ) )
		}
		throw SliPError.RuntimeError( "\(p):string" )
	}
	v[ "M_E"		] = RealNumber	( M_E						)
	v[ "M_LOG2E"	] = RealNumber	( M_LOG2E					)
	v[ "M_LOG10E"	] = RealNumber	( M_LOG10E					)
	v[ "M_LN2"		] = RealNumber	( M_LN2						)
	v[ "M_LN10"		] = RealNumber	( M_LN10					)
	v[ "M_PI"		] = RealNumber	( Double.pi					)
	v[ "M_PI_2"		] = RealNumber	( Double.pi / 2				)
	v[ "M_PI_4"		] = RealNumber	( Double.pi / 4				)
	v[ "M_1_PI"		] = RealNumber	( M_1_PI					)
	v[ "M_2_PI"		] = RealNumber	( M_2_PI					)
	v[ "M_2_SQRTPI"	] = RealNumber	( M_2_SQRTPI				)
	v[ "M_SQRT2"	] = RealNumber	( 2.squareRoot()			)
	v[ "M_SQRT1_2"	] = RealNumber	( 0.5.squareRoot()			)

//	v[ "fpclassify"	] = IMathFunc	( fpclassify, "fpclassify"	)
//	v[ "isfinite"	] = IMathFunc	( isfinite	, "isfinite"	)
//	v[ "isinf"		] = IMathFunc	( isinf		, "isinf"		)
//	v[ "isnan"		] = IMathFunc	( isnan		, "isnan"		)
//	v[ "isnormal"	] = IMathFunc	( isnormal	, "isnormal"	)
//	v[ "sign"		] = IMathFunc	( sign		, "sign"		)
	v[ "ceil"		] = MathFunc	( ceil		, "ceil"		)
	v[ "floor"		] = MathFunc	( floor		, "floor"		)
	v[ "nearbyint"	] = MathFunc	( nearbyint	, "nearbyint"	)
	v[ "rint"		] = MathFunc	( rint		, "rint"		)
	v[ "round"		] = MathFunc	( round		, "round"		)
	v[ "lrint"		] = IMathFunc	( lrint		, "lrint"		)
	v[ "lround"		] = IMathFunc	( lrint		, "lround"		)
	v[ "trunc"		] = MathFunc	( trunc		, "trunc"		)
	v[ "fmod"		] = MathFuncFF	( fmod		, "fmod"		)
	v[ "remainder"	] = MathFuncFF	( remainder	, "remainder"	)
//	v[ "remquo"		] = MathFuncFF	( remquo	, "remquo"		)
	v[ "fdim"		] = MathFuncFF	( fdim		, "fdim"		)
	v[ "fmax"		] = MathFuncFF	( fmax		, "fmax"		)
	v[ "fmin"		] = MathFuncFF	( fmin		, "fmin"		)
	v[ "fma"		] = MathFuncFFF	( fma		, "fma"			)
	v[ "fabs"		] = MathFunc	( fabs		, "fabs"		)
	v[ "sqrt"		] = MathFunc	( sqrt		, "sqrt"		)
	v[ "cbrt"		] = MathFunc	( cbrt		, "cbrt"		)
	v[ "hypot"		] = MathFuncFF	( hypot		, "hypot"		)
	v[ "exp"		] = MathFunc	( exp		, "exp"			)
	v[ "exp2"		] = MathFunc	( exp2		, "exp2"		)
	v[ "__exp10"	] = MathFunc	( __exp10	, "__exp10"		)
	v[ "expm1"		] = MathFunc	( expm1		, "expm1"		)
	v[ "log"		] = MathFunc	( log		, "log"			)
	v[ "log2"		] = MathFunc	( log2		, "log2"		)
	v[ "log10"		] = MathFunc	( log10		, "log10"		)
	v[ "log1p"		] = MathFunc	( log1p		, "log1p"		)
	v[ "logb"		] = MathFunc	( logb		, "logb"		)
	v[ "scalbn"		] = MathFuncFI	( scalbn	, "scalbn"		)
	v[ "pow"		] = MathFuncFF	( pow		, "pow"			)
	v[ "cos"		] = MathFunc	( cos		, "cos"			)
	v[ "sin"		] = MathFunc	( sin		, "sin"			)
	v[ "tan"		] = MathFunc	( tan		, "tan"			)
	v[ "cosh"		] = MathFunc	( cosh		, "cosh"		)
	v[ "sinh"		] = MathFunc	( sinh		, "sinh"		)
	v[ "tanh"		] = MathFunc	( tanh		, "tanh"		)
	v[ "acos"		] = MathFunc	( acos		, "acos"		)
	v[ "asin"		] = MathFunc	( asin		, "asin"		)
	v[ "atan"		] = MathFunc	( atan		, "atan"		)
	v[ "atan2"		] = MathFuncFF	( atan2		, "atan2"		)
	v[ "acosh"		] = MathFunc	( acosh		, "acosh"		)
	v[ "asinh"		] = MathFunc	( asinh		, "asinh"		)
	v[ "atanh"		] = MathFunc	( atanh		, "atanh"		)
	v[ "tgamma"		] = MathFunc	( tgamma	, "tgamma"		)
	v[ "lgamma"		] = MathFunc	( lgamma	, "lgamma"		)
	v[ "j0"			] = MathFunc	( j0		, "j0"			)
	v[ "j1"			] = MathFunc	( j1		, "j1"			)
//	v[ "jn"			] = MathFunc	( jn		, "jn"			)
	v[ "y0"			] = MathFunc	( y0		, "y0"			)
	v[ "y1"			] = MathFunc	( y1		, "y1"			)
//	v[ "yn"			] = MathFunc	( yn		, "yn"			)
	v[ "erf"		] = MathFunc	( erf		, "erf"			)
	v[ "erfc"		] = MathFunc	( erfc		, "erfc"		)
	
	return v
}

func
MathFunc( _ f: @escaping ( Float64 ) -> Float64, _ name: String ) -> Unary {
	return Unary( name ) { c, p in
		guard let w = p as? Numeric else { throw SliPError.RuntimeError( "\(p):\(name)" ) }
		return RealNumber( f( w.v ) )
	}
}
func
MathFuncFF( _ f: @escaping ( Float64, Float64 ) -> Float64, _ name: String ) -> Unary {
	return Unary( name ) { c, p in
		guard let w = p as? List, let w0 = w.m[ 0 ] as? Numeric, let w1 = w.m[ 1 ] as? Numeric else { throw SliPError.RuntimeError( "\(p):\(name)" ) }
		return RealNumber( f( w0.v, w1.v ) )
	}
}
func
MathFuncFI( _ f: @escaping ( Float64, Int ) -> Float64, _ name: String ) -> Unary {
	return Unary( name ) { c, p in
		guard let w = p as? List, let w0 = w.m[ 0 ] as? Numeric, let w1 = w.m[ 1 ] as? IntNumber else { throw SliPError.RuntimeError( "\(p):\(name)" ) }
		return RealNumber( f( w0.v, w1.m.NativeInt ) )
	}
}
func
MathFuncIF( _ f: @escaping ( Int, Float64 ) -> Float64, _ name: String ) -> Unary {
	return Unary( name ) { c, p in
		guard let w = p as? List, let w0 = w.m[ 0 ] as? IntNumber, let w1 = w.m[ 1 ] as? Numeric else { throw SliPError.RuntimeError( "\(p):\(name)" ) }
		return RealNumber( f( w0.m.NativeInt, w1.v ) )
	}
}
func
MathFuncFFF( _ f: @escaping ( Float64, Float64, Float64 ) -> Float64, _ name: String ) -> Unary {
	return Unary( name ) { c, p in
		guard let w = p as? List, let w0 = w.m[ 0 ] as? Numeric, let w1 = w.m[ 1 ] as? Numeric, let w2 = w.m[ 2 ] as? Numeric else { throw SliPError.RuntimeError( "\(p):\(name)" ) }
		return RealNumber( f( w0.v, w1.v, w2.v ) )
	}
}
func
IMathFunc( _ f: @escaping ( Float64 ) -> Int, _ name: String ) -> Unary {
	return Unary( name ) { c, p in
		guard let w = p as? Numeric else { throw SliPError.RuntimeError( "\(p):\(name)" ) }
		return IntNumber( f( w.v ) )
	}
}

