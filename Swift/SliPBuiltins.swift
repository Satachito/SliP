//
//  SliPBuiltins.swift
//  SliP
//
//  Created by Satoru Ogura on 2016/04/21.
//  Copyright © 2016年 Satoru Ogura. All rights reserved.
//

import Foundation

func
Register( dicts: Cell< [ String: Object ] > ) {
	dicts.m[ "for" ] = Builtin(
		{	c, p, a in
			if let wArgs = a as? List where wArgs.m.count >= 2 {
				if let wList = wArgs.m[ 0 ] as? List {
					var	v = [ Object ]()
					for w in wList.m {
						c.Push( w )
						defer{ c.Pop() }
						v.append( try wArgs.m[ 1 ].Eval( c, p ) )
					}
					return List( v, wList.type )
				}
			}
			throw Error.RuntimeError( "\(a):for" )
		}
	)
	dicts.m[ "int" ] = Builtin(
		{	c, p, a in
			switch a {
			case let w as StringL:
				return IntNumber( MakeBigInteger( w.m ) )
			case let w as List:
				if let wString = w.m[ 0 ] as? StringL {
					let	wRadix = ( w.m[ 1 ] as? IntNumber ) ?? IntNumber( int: 10 )
					let	wMinus = w.m.count < 3 ? false : IsNil( w.m[ 2 ] )
					return IntNumber(
						MakeBigInteger(
							wString.m
						,	radix: wRadix.m.RemainderE()
						,	minus: wMinus
						)
					)
				}
			default:
				break
			}
			throw Error.RuntimeError( "\(a):int" )
		}
	)
	dicts.m[ "qr" ] = Builtin(
		{	c, p, a in
			if	let w = a as? List where w.m.count == 2
			,	let wL = w.m[ 0 ] as? IntNumber
			,	let wR = w.m[ 1 ] as? IntNumber {
				
				let	( vQuo, vRem ) = QR( wL.m.m, wR.m.m )
				return List(
					[	IntNumber( BigInteger( vQuo, wL.m.minus != wR.m.minus ) )
					,	IntNumber( BigInteger( vRem, wL.m.minus != wR.m.minus ) )
					]
				,	.Literal
				)
			}
			throw Error.RuntimeError( "\(a):qr" )
		}
	)
	dicts.m[ "string" ] = Builtin(
		{	c, p, a in
			if	let w = a as? List where w.m.count == 2
			,	let wL = w.m[ 0 ] as? IntNumber
			,	let wR = w.m[ 1 ] as? IntNumber {
				
				return StringL( wL.m.StringRepresentation( wR.m.RemainderE() ) )
			}
			throw Error.RuntimeError( "\(a):string" )
		}
	)
	dicts.m[ "M_E" ] = RealNumber( M_E )
	dicts.m[ "M_LOG2E" ] = RealNumber( M_LOG2E )
	dicts.m[ "M_LOG10E" ] = RealNumber( M_LOG10E )
	dicts.m[ "M_LN2" ] = RealNumber( M_LN2 )
	dicts.m[ "M_LN10" ] = RealNumber( M_LN10 )
	dicts.m[ "M_PI" ] = RealNumber( M_PI )
	dicts.m[ "M_PI_2" ] = RealNumber( M_PI_2 )
	dicts.m[ "M_PI_4" ] = RealNumber( M_PI_4 )
	dicts.m[ "M_1_PI" ] = RealNumber( M_1_PI )
	dicts.m[ "M_2_PI" ] = RealNumber( M_2_PI )
	dicts.m[ "M_2_SQRTPI" ] = RealNumber( M_2_SQRTPI )
	dicts.m[ "M_SQRT2" ] = RealNumber( M_SQRT2 )
	dicts.m[ "M_SQRT1_2" ] = RealNumber( M_SQRT1_2 )

	dicts.m[ "fpclassify" ] = IMathFunc( fpclassify, "fpclassify" )
//		dicts.m[ "isfinite" ] = IMathFunc( isfinite, "isfinite" )
//		dicts.m[ "isinf" ] = IMathFunc( isinf, "isinf" )
//		dicts.m[ "isnan" ] = IMathFunc( isnan, "isnan" )
//		dicts.m[ "isnormal" ] = IMathFunc( isnormal, "isnormal" )
	dicts.m[ "signbit" ] = IMathFunc( signbit, "signbit" )
	dicts.m[ "ceil" ] = MathFunc( ceil, "ceil" )
	dicts.m[ "floor" ] = MathFunc( floor, "floor" )
	dicts.m[ "nearbyint" ] = MathFunc( nearbyint, "nearbyint" )
	dicts.m[ "rint" ] = MathFunc( rint, "rint" )
	dicts.m[ "round" ] = MathFunc( round, "round" )
	dicts.m[ "lrint" ] = IMathFunc( lrint, "lrint" )
	dicts.m[ "lround" ] = IMathFunc( lrint, "lround" )
	dicts.m[ "trunc" ] = MathFunc( trunc, "trunc" )
	dicts.m[ "fmod" ] = MathFuncFF( fmod, "fmod" )
	dicts.m[ "remainder" ] = MathFuncFF( remainder, "remainder" )
//		dicts.m[ "remquo" ] = MathFuncFF( remquo, "remquo" )
	dicts.m[ "fdim" ] = MathFuncFF( fdim, "fdim" )
	dicts.m[ "fmax" ] = MathFuncFF( fmax, "fmax" )
	dicts.m[ "fmin" ] = MathFuncFF( fmin, "fmin" )
	dicts.m[ "fma" ] = MathFuncFFF( fma, "fma" )
	dicts.m[ "fabs" ] = MathFunc( fabs, "fabs" )
	dicts.m[ "sqrt" ] = MathFunc( sqrt, "sqrt" )
	dicts.m[ "cbrt" ] = MathFunc( cbrt, "cbrt" )
	dicts.m[ "hypot" ] = MathFuncFF( hypot, "hypot" )
	dicts.m[ "exp" ] = MathFunc( exp, "exp" )
	dicts.m[ "exp2" ] = MathFunc( exp2, "exp2" )
	dicts.m[ "__exp10" ] = MathFunc( __exp10, "__exp10" )
	dicts.m[ "expm1" ] = MathFunc( expm1, "expm1" )
	dicts.m[ "log" ] = MathFunc( log, "log" )
	dicts.m[ "log2" ] = MathFunc( log2, "log2" )
	dicts.m[ "log10" ] = MathFunc( log10, "log10" )
	dicts.m[ "log1p" ] = MathFunc( log1p, "log1p" )
	dicts.m[ "logb" ] = MathFunc( logb, "logb" )
	dicts.m[ "ilogb" ] = IMathFunc( ilogb, "ilogb" )
	dicts.m[ "ldexp" ] = MathFuncFI( ldexp, "ldexp" )
	dicts.m[ "scalbn" ] = MathFuncFI( scalbn, "scalbn" )
	dicts.m[ "pow" ] = MathFuncFF( pow, "pow" )
	dicts.m[ "cos" ] = MathFunc( cos, "cos" )
	dicts.m[ "sin" ] = MathFunc( sin, "sin" )
	dicts.m[ "tan" ] = MathFunc( tan, "tan" )
	dicts.m[ "cosh" ] = MathFunc( cosh, "cosh" )
	dicts.m[ "sinh" ] = MathFunc( sinh, "sinh" )
	dicts.m[ "tanh" ] = MathFunc( tanh, "tanh" )
	dicts.m[ "acos" ] = MathFunc( acos, "acos" )
	dicts.m[ "asin" ] = MathFunc( asin, "asin" )
	dicts.m[ "atan" ] = MathFunc( atan, "atan" )
	dicts.m[ "atan2" ] = MathFuncFF( atan2, "atan2" )
	dicts.m[ "acosh" ] = MathFunc( acosh, "acosh" )
	dicts.m[ "asinh" ] = MathFunc( asinh, "asinh" )
	dicts.m[ "atanh" ] = MathFunc( atanh, "atanh" )
	dicts.m[ "tgamma" ] = MathFunc( tgamma, "tgamma" )
	dicts.m[ "lgamma" ] = MathFunc( lgamma, "lgamma" )
	dicts.m[ "j0" ] = MathFunc( j0, "j0" )
	dicts.m[ "j1" ] = MathFunc( j1, "j1" )
//		dicts.m[ "jn" ] = MathFunc( jn, "jn" )
	dicts.m[ "y0" ] = MathFunc( y0, "y0" )
	dicts.m[ "y1" ] = MathFunc( y1, "y1" )
//		dicts.m[ "yn" ] = MathFunc( yn, "yn" )
	dicts.m[ "erf" ] = MathFunc( erf, "erf" )
	dicts.m[ "erfc" ] = MathFunc( erfc, "erfc" )
}

func
MathFunc( f: Float64 -> Float64, _ name: String ) -> Builtin {
	return Builtin(
		{	c, p, a in
			if let w = a as? NumberL {
				return RealNumber( f( w.Value() ) )
			} else {
				throw Error.RuntimeError( "\(a):\(name)" )
			}
		}
	)
}
func
MathFuncFF( f: ( Float64, Float64 ) -> Float64, _ name: String ) -> Builtin {
	return Builtin(
		{	c, p, a in
			if let w = a as? List, let w0 = w.m[ 0 ] as? NumberL, let w1 = w.m[ 1 ] as? NumberL {
				return RealNumber( f( w0.Value(), w1.Value() ) )
			} else {
				throw Error.RuntimeError( "\(a):\(name)" )
			}
		}
	)
}
func
MathFuncFI( f: ( Float64, Int ) -> Float64, _ name: String ) -> Builtin {
	return Builtin(
		{	c, p, a in
			if let w = a as? List, let w0 = w.m[ 0 ] as? NumberL, let w1 = w.m[ 1 ] as? IntNumber {
				return RealNumber( f( w0.Value(), w1.m.RemainderE() ) )
			} else {
				throw Error.RuntimeError( "\(a):\(name)" )
			}
		}
	)
}
func
MathFuncIF( f: ( Int, Float64 ) -> Float64, _ name: String ) -> Builtin {
	return Builtin(
		{	c, p, a in
			if let w = a as? List, let w0 = w.m[ 0 ] as? IntNumber, let w1 = w.m[ 1 ] as? NumberL {
				return RealNumber( f( w0.m.RemainderE(), w1.Value() ) )
			} else {
				throw Error.RuntimeError( "\(a):\(name)" )
			}
		}
	)
}
func
MathFuncFFF( f: ( Float64, Float64, Float64 ) -> Float64, _ name: String ) -> Builtin {
	return Builtin(
		{	c, p, a in
			if let w = a as? List, let w0 = w.m[ 0 ] as? NumberL, let w1 = w.m[ 1 ] as? NumberL, let w2 = w.m[ 2 ] as? NumberL {
				return RealNumber( f( w0.Value(), w1.Value(), w2.Value() ) )
			} else {
				throw Error.RuntimeError( "\(a):\(name)" )
			}
		}
	)
}
func
IMathFunc( f: Float64 -> Int, _ name: String ) -> Builtin {
	return Builtin(
		{	c, p, a in
			if let w = a as? NumberL {
				return IntNumber( int: f( w.Value() ) )
			} else {
				throw Error.RuntimeError( "\(a):\(name)" )
			}
		}
	)
}

