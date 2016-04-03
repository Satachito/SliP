//	Written by Satoru Ogura, Tokyo.
//
import Foundation

enum
Error				:	ErrorType {
	case				ReadError( String )
	case				RuntimeError( String )
	case				UserException
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Context {
	var
	stack				:	Cell< Object >?

	func
	Push( o: Object ) {
		stack = Cell( o, stack )
	}
	func
	Pop() {
		stack = stack!.next
	}

	var
	dicts				:	Cell< [ String: Object ] >

	init( _ c: Context ) {
		dicts = c.dicts
		stack = c.stack
	}
	
	init() {
		dicts = Cell< [ String: Object ] >( [ String: Object ]() )
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
					} else {
						throw Error.RuntimeError( "\(a):for" )
					}
				} else {
					throw Error.RuntimeError( "\(a):for" )
				}
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
				return RealNumber( f( w0.Value(), w1.m ) )
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
				return RealNumber( f( w0.m, w1.Value() ) )
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
				return IntNumber( f( w.Value() ) )
			} else {
				throw Error.RuntimeError( "\(a):\(name)" )
			}
		}
	)
}

//protocol
//String -> () {
//	func
//	Print( a: String )
//}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Object				:	NSObject {
	func
	Eval( c: Context, _ p: String -> () ) throws -> Object { return self }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Name				:	Object {
	let
	m				:	String
	init(	_ a		:	String ) { m = a }
	override var
	description 	:	String { return m }
	override func
	Eval( c: Context, _ p: String -> () )	throws	->	Object {
		var	wAL = c.dicts as Cell< [ String: Object ] >?
		while let w = wAL {
			if let v = w.m[ m ] { return v }
			wAL = w.next
		}
		throw Error.RuntimeError( "UndefinedName \(m)" )
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
NumberL				:	Object {
	func
	Value()			->	Float64 { fatalError() }
}

class
IntNumber			:	NumberL {
	let
	m				:	Int
	init(	_ a		:	Int ) { m = a }
	override var
	description		:	String { return "\(m)" }
	override func
	Value()			->	Float64 { return Float64( m ) }
}
class
RealNumber			:	NumberL {
	let
	m				:	Float64
	init(	_ a		:	Float64 ) { m = a }
	override var
	description		:	String { return "\(m)" }
	override func
	Value()			->	Float64 { return m }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
StringL				:	Object {
	let
	m				:	String
	init(	_ a		:	String ) { m = a }
	override var
	description	:	String { return "\"\(m)\"" }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Primitive			:	Object {
	var
	m				:	Context throws -> Object
	init(	_ a		:	Context throws -> Object ) { m = a }
	override var
	description		:	String { return "P" }
	override func
	Eval( c: Context, _ p: String -> () ) throws	->	Object { return try m( c ) }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Builtin				:	Object {
	var
	m				:	( Context, String -> (), Object ) throws -> Object
	init(	_ a		:	( Context, String -> (), Object ) throws -> Object ) { m = a }
	override var
	description		:	String { return "B" }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Operator			:	Object {
	var
	bond			:	Int
	var
	m				:	( Context, String -> (), Object, Object ) throws -> Object
	init(	_ a		:	( Context, String -> (), Object, Object ) throws -> Object, _ pBond: Int ) { m = a; bond = pBond }
	override var
	description		:	String { return ":\(bond)" }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Quote				:	Object {
	let
	m				:	Object
	init(	_ a		:	Object ) { m = a }
	override var
	description		:	String { return "'\(m)" }
	override func
	Eval( c: Context, _ p: String -> () ) throws	->	Object { return m }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Combiner			:	Object {
	let
	m				:	Object
	init(	_ a		:	Object ) { m = a }
	override var
	description		:	String { return "`\(m)" }
	override func
	Eval( c: Context, _ p: String -> () ) throws	->	Object { return EvalAssoc( m, c.dicts.m ) }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
EvalAssoc			:	Object {
	let
	m				:	Object
	let
	dict			:	[ String: Object ]
	init(	_ a		:	Object, _ pDict: [ String: Object ] ) { m = a; dict = pDict }
	override var
	description		:	String { return "<\(dict):\(m)>" }
	override func
	Eval( c: Context, _ p: String -> () ) throws	->	Object {
		c.dicts = Cell< [ String: Object ] >( dict, c.dicts )
		defer{ c.dicts = c.dicts.next! }
		return try m.Eval( c, p )
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class
Dict				:	Object {
	let
	m				:	[ String: Object ]
	init(	_ a		:	[ String: Object ] ) { m = a }
	override var
	description		:	String { return "<\(m)>" }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
var
sObject = Object()

class
List				:	Object {
	func
	EvalSentence( c: Context, _ p: String -> (), _ o: [ Object ] ) throws -> Object {
		switch o.count {
		case  1:	return try o[ 0 ].Eval( c, p )
		case  0, 2:	throw Error.RuntimeError( "No operator in \(List( o, .Sentence ))" )
		default:	break
		}
		var wTarget :	Operator?
		var wIndex  =	0
		for i in 1 ..< o.count - 1 {
			if let w = o[ i ] as? Operator {
				if let wCurrent = wTarget {
					if w.bond >= wCurrent.bond {
						wTarget = w
						wIndex = i
					}
				} else {
					wTarget = w
					wIndex = i
				}
			}
		}
		if let w = wTarget {
			return try w.m( c, p, EvalSentence( c, p, Array( o[ 0 ..< wIndex ] ) ), EvalSentence( c, p, Array( o[ wIndex+1 ..< o.count ] ) ) )
		} else {
			throw Error.RuntimeError( "No operator in \(List( o, .Sentence ))" )
		}
	}
	enum
	Type {
		case Literal
		case Parallel
		case Block
		case Sentence
	}
	let
	type			:	Type
	let
	m				:	[ Object ]
	init(	_ a		:	[ Object ], _ pType: Type ) { m = a; type = pType }
	func
	_Str()			->	String {
 		var	v = " "
		for w in m { v += "\(w) " }
		return v
	}
	override	var
	description		:	String {
		switch type {
		case .Literal:	return "[" + _Str() + "]"
		case .Parallel:	return "«" + _Str() + "»"
		case .Block:	return "{" + _Str() + "}"
		case .Sentence:	return "(" + _Str() + ")"
		}
	}
	override	func
	Eval( c: Context, _ p: String -> () ) throws	->	Object {
		switch type {
		case .Literal:
			return self
		case .Parallel:
			var	v = [ Object ]( count: m.count, repeatedValue: sObject )
			let	wG = dispatch_group_create()
			let	wQ = dispatch_get_global_queue( DISPATCH_QUEUE_PRIORITY_DEFAULT, 0 )
			
			for ( i, w ) in m.enumerate() {
				dispatch_group_async( wG, wQ ) {
					do {
						v[ i ] = try w.Eval( Context( c ), p )
					} catch let e {
						print( e )
					}
				}
			}
			dispatch_group_wait( wG, DISPATCH_TIME_FOREVER )
			for w in m {
				if w == sObject { throw Error.RuntimeError( "Parallel" ) }	//	UC
			}
			return List( v, .Literal )
		case .Block:
			c.dicts = Cell< [ String: Object ] >( [ String: Object ](), c.dicts )
			defer{ c.dicts = c.dicts.next! }
			var	v = [ Object ]( count: m.count, repeatedValue:Object() )
			for ( i, w ) in m.enumerate() { v[ i ] = try w.Eval( c, p ) }
			return List( v, .Literal )
		case .Sentence:
			return try EvalSentence( c, p, m )
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
func
IsNil( o: Object )	->	Bool {
	if let w = o as? List { return w.m.count == 0 }
	return false
}

let
T	=	StringL( "T" )

let
Nil	=	List( [], .Literal )

let
StackTop = Primitive(
	{	c in
		if let v = c.stack { return v.m }
		throw Error.RuntimeError( "StackUnderflow" )
	}
)

let
CurrentDict = Primitive( { c in return Dict( c.dicts.m ) } )

let
Exception = Primitive( { c in throw Error.UserException } )

let
Cdr = Builtin(
	{	c, p, a in
		if let w = a as? List where w.m.count > 0 { return List( Array( w.m.dropFirst() ), w.type ) }
		throw Error.RuntimeError( "\(a):*" )
	}
)

let
Count = Builtin(
	{	c, p, a in
		if let w = a as? List { return IntNumber( w.m.count ) }
		throw Error.RuntimeError( "\(a):#" )
	}
)

let
Last = Builtin(
	{	c, p, a in
		if let w = a as? List where w.m.count > 0 { return w.m.last! }
		throw Error.RuntimeError( "\(a):$" )
	}
)

let
Print = Builtin(
	{	c, p, a in
		switch a {
		case let w as IntNumber	:	p( "\(a)" )
		case let w as RealNumber:	p( "\(a)" )
		case let w as StringL	:	p( w.m )
		case let w as Name		:	p( w.m )
		default:					break
		}
		return a
	}
)

let
If = Operator(
	{	c, p, l, r in
		return try IsNil( l ) ? Nil : r.Eval( c, p )
	}
,	9
)

let
IfElse = Operator(
	{	c, p, l, r in
		if let w = r as? List where w.type == .Literal && w.m.count == 2 {
			return try IsNil( l ) ? w.m[ 1 ].Eval( c, p ) : w.m[ 0 ].Eval( c, p )
		}
		throw Error.RuntimeError( "\(l) ? \(r)" )
	}
,	9
)

let
Cons = Operator(
	{	c, p, l, r in
		if let w = r as? List {
			var	wM = w.m
			wM.insert( l, atIndex: 0 )
			return List( wM, w.type )
		}
		throw Error.RuntimeError( "\(l) , \(r)" )
	}
,	7
)

let
Apply = Operator(
	{	c, p, l, r in
		switch r {
		case let w as IntNumber:
			if let wL = l as? List {
				if w.m < wL.m.count { return wL.m[ w.m ] } else { throw Error.RuntimeError( "Index operation \(w) to \(l)" ) }
			}
			throw Error.RuntimeError( "Index operation \(w) to \(l)" )
		case let w as Name:
			let	wR = w.m
			if let wL = l as? Dict {
				return wL.m[ w.m ] ?? Nil
			}
			throw Error.RuntimeError( "Dict operation \(w) to \(l)" )
		case let w as Builtin:
			return try w.m( c, p, l )
		case let w as List:
			c.Push( l )
			defer{ c.Pop() }
			return try w.Eval( c, p )
		case let w as EvalAssoc:
			c.Push( l )
			defer{ c.Pop() }
			return try w.Eval( c, p )
		default:
			throw Error.RuntimeError( "\(l) : \(r)" )
		}
	}
,	0
)

let
And = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m & wR.m ) }
		return IsNil( l ) && IsNil( r ) ? T : Nil
	}
,	4
)

let
Xor = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m ^ wR.m ) }
		return IsNil( l ) != IsNil( r ) ? T : Nil
	}
,	4
)

let
Or = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m | wR.m ) }
		return IsNil( l ) || IsNil( r ) ? T : Nil
	}
,	4
)

let
Mul = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m * wR.m ) }
		if let wL = l as? NumberL,	let wR = r as? NumberL { return RealNumber( wL.Value() * wR.Value() ) }
		throw Error.RuntimeError( "\(l) × \(r)" )
	}
,	5
)

let
Div = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber {
			if wR.m != 0 {
				return wL.m % wR.m == 0 ? IntNumber( wL.m + wR.m ) : RealNumber( wL.Value() / wR.Value() )
			}
		} else if let wL = l as? NumberL, let wR = r as? NumberL {
			let wRv = wR.Value()
			if wRv != 0 {
				return RealNumber( wL.Value() / wRv )
			}
		}
		throw Error.RuntimeError( "\(l) ÷ \(r)" )
	}
,	5
)

let
IDiv = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m / wR.m ) }
		throw Error.RuntimeError( "\(l) / \(r)" )
	}
,	5
)

let
Remainder = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m % wR.m ) }
		throw Error.RuntimeError( "\(l) % \(r)" )
	}
,	5
)

let
Add = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m + wR.m ) }
		if let wL = l as? NumberL, let wR = r as? NumberL { return RealNumber( wL.Value() + wR.Value() ) }
		if let wL = l as? StringL, let wR = r as? StringL { return StringL( wL.m + wR.m ) }
		if let wL = l as? List, let wR = r as? List where wL.type == wR.type { return List( wL.m + wR.m, wL.type ) }
		throw Error.RuntimeError( "\(l) + \(r)" )
	}
,	6
)

let
Minus = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return IntNumber( wL.m - wR.m ) }
		if let wL = l as? NumberL, let wR = r as? NumberL { return RealNumber( wL.Value() - wR.Value() ) }
		throw Error.RuntimeError( "\(l) - \(r)" )
	}
,	6
)

func
Equal( l: Object, _ r: Object ) -> Bool {
	if let wL = l as? NumberL, let wR = r as? NumberL {
		return wL.Value() == wR.Value()
	}
	if let wL = l as? StringL, let wR = r as? StringL { return wL.m == wR.m }
	if let wL = l as? Name, let wR = r as? Name { return wL.m == wR.m }
	if let wL = l as? List, let wR = r as? List {
		if wL.type != wR.type { return false }
		if wL.m.count != wR.m.count { return false }
		for i in 0 ..< wL.m.count { if Equal( wL.m[ i ], wR.m[ i ] ) == Nil { return false } }
		return true
	}
	return l == r
}

func
== ( l: Object, r: Object ) -> Bool {
//	return Equal( l, r )
	if let wL = l as? NumberL, let wR = r as? NumberL { return wL.Value() == wR.Value() }
	if let wL = l as? StringL, let wR = r as? StringL { return wL.m == wR.m }
	if let wL = l as? Name, let wR = r as? Name { return wL.m == wR.m }
	if let wL = l as? List, let wR = r as? List {
		if wL.type != wR.type { return false }
		if wL.m.count != wR.m.count { return false }
		for i in 0 ..< wL.m.count { if Equal( wL.m[ i ], wR.m[ i ] ) == Nil { return false } }
		return true
	}
	return l as NSObject == r as NSObject
}

let
Eq = Operator(
	{ c, p, l, r in Equal( l, r ) ? T : Nil }
,	8
)

let
GE = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return wL.m >= wR.m ? T : Nil }
		if let wL = l as? NumberL, let wR = r as? NumberL { return wL.Value() >= wR.Value() ? T : Nil }
		throw Error.RuntimeError( "\(l) >= \(r)" )
	}
,	8
)

let
LE = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return wL.m <= wR.m ? T : Nil }
		if let wL = l as? NumberL, let wR = r as? NumberL { return wL.Value() <= wR.Value() ? T : Nil }
		throw Error.RuntimeError( "\(l) <= \(r)" )
	}
,	8
)

let
Neq = Operator(
	{ c, p, l, r in Equal( l, r ) ? Nil : T }
,	8
)

let
LT = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return wL.m < wR.m ? T : Nil }
		if let wL = l as? NumberL, let wR = r as? NumberL { return wL.Value() < wR.Value() ? T : Nil }
		throw Error.RuntimeError( "\(l) < \(r)" )
	}
,	8
)

let
GT = Operator(
	{	c, p, l, r in
		if let wL = l as? IntNumber, let wR = r as? IntNumber { return wL.m > wR.m ? T : Nil }
		if let wL = l as? NumberL, let wR = r as? NumberL { return wL.Value() > wR.Value() ? T : Nil }
		throw Error.RuntimeError( "\(l) > \(r)" )
	}
,	8
)

let
Define = Operator(
	{	c, p, l, r in
		if let w = l as? Name {
			c.dicts.m[ w.m ] = r
			return l
		}
		throw Error.RuntimeError( "\(l) = \(r)" )
	}
,	10
)

let
MemberOf = Operator(
	{	c, p, l, r in
		if let wR = r as? List {
			return wR.m.contains( { $0 == l } ) ? T : Nil
		}
		throw Error.RuntimeError( "\(l) ∈ \(r)" )
	}
,	8
)

let
Contains = Operator(
	{	c, p, l, r in
		if let wL = l as? List {
			return wL.m.contains( { $0 == r } ) ? T : Nil
		}
		throw Error.RuntimeError( "\(l) ∋ \(r)" )
	}
,	8
)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
func
SkipWhite( r: Reader< UnicodeScalar > ) {
	while let u = r.Read() {
		if u.value >= 0x10000 || !NSCharacterSet.whitespaceAndNewlineCharacterSet().characterIsMember( UInt16( u.value ) ) {
			r.Unread( u )
			break
		}
	}
}


func
ReadNumber( r: Reader< UnicodeScalar >, _ neg: Bool = false ) throws -> Object {
 	var	v = ""
	var	wRealF = false
	while let u = r.Read() {
		switch u {
		case	( "0"..."9" ), "x", "X", "+", "-":
			v += String( u )
		case	".", "e", "E":
			v += String( u )
			wRealF = true
		default:
			r.Unread( u )
			if wRealF	{ if let w = Float64( v ) { return RealNumber( neg ? -w : w ) } }
			else		{ if let w = Int( v ) { return IntNumber( neg ? -w : w ) } }
			throw Error.ReadError( "NumberFormat\(v)" )
		}
	}
	throw Error.ReadError( "EOD" )
}

func
ReadName( r: Reader< UnicodeScalar > ) throws -> Object {
	var	v = ""
	while let u = r.Read() {
		switch u {
		case ( "0" ... "9" ), ( "a" ... "z" ), ( "A" ... "Z" ), "_":
			v += String( u )
		default:
			r.Unread( u )
			return Name( v )
		}
	}
	throw Error.ReadError( "EOD" )
}

func
ReadStr( r: Reader< UnicodeScalar > ) throws -> StringL {
	var	v = ""
	var	wEscaped = false
	while let u = r.Read() {
		if wEscaped {
			wEscaped = false
			switch u {
			case "0"	:	v.append( Character( "\0" ) )
			case "t"	:	v.append( Character( "\t" ) )
			case "n"	:	v.append( Character( "\n" ) )
			case "r"	:	v.append( Character( "\r" ) )
			default		:	v.append( Character( u ) )
			}
		} else {
			switch u {
			case "\""	:	return StringL( v )
			case "\\"	:	wEscaped = true
			default		:	v += String( u )
			}
		}
	}
	throw Error.ReadError( "EOD" )
}

func
ReadObjects( r: Reader< UnicodeScalar >, _ terminator: UnicodeScalar ) throws -> [ Object ] {
	var	v = [ Object ]()
	while let w = try Read( r, terminator ) { v.append( w ) }
	return v
}

func
Read( r: Reader< UnicodeScalar >, _ terminator: UnicodeScalar = UnicodeScalar( 0 ) ) throws -> Object? {
	SkipWhite( r )
	if let u = r.Read() {
		switch u {
		case terminator			:	return nil
		case "}", "]", ")"		:	throw Error.ReadError( "Unexpected \(u)" )
		case "["				:	return List( try ReadObjects( r, "]" as UnicodeScalar ), .Literal )
		case "«"				:	return List( try ReadObjects( r, "»" as UnicodeScalar ), .Parallel )
		case "("				:	return List( try ReadObjects( r, ")" as UnicodeScalar ), .Sentence )
		case "{"				:	return List( try ReadObjects( r, "}" as UnicodeScalar ), .Block )
		case "\""				:	return try ReadStr( r )
		case "'"				:	if let w = try Read( r ) { return Quote( w ) }; throw Error.ReadError( "No object to quote" )
		case "`"				:	if let w = try Read( r ) { return Combiner( w ) }; throw Error.ReadError( "No object to combine" )
		case "¡"				:	return Exception
		case "@"				:	return StackTop
		case "·"				:	return CurrentDict
		case "*"				:	return Cdr
		case "#"				:	return Count
		case "$"				:	return Last
		case "¦"				:	return Print
		case "∈"				:	return MemberOf
		case "∋"				:	return Contains
		case "?"				:	return IfElse
		case "¿"				:	return If
		case ","				:	return Cons
		case ":"				:	return Apply
		case "&"				:	return And
		case "^"				:	return Xor
		case "|"				:	return Or
		case "×"				:	return Mul
		case "÷"				:	return Div
		case "/"				:	return IDiv
		case "%"				:	return Remainder
		case "+"				:
			if let w = r.Read() {
				r.Unread( w )
				switch w {
				case ( "0"..."9" ), ".":
									return try ReadNumber( r )
				default			:	return Add
				}
			}
			throw Error.ReadError( "EOD" )
		case "-"				:
			if let w = r.Read() {
				r.Unread( w )
				switch w {
				case ( "0"..."9" ), ".":
									return try ReadNumber( r, true )
				default			:	return Minus
				}
			}
			throw Error.ReadError( "EOD" )
		case "="			:
			if let w = r.Read() {
				switch w {
				case "="		:	return Eq
				case ">"		:	return GE
				case "<"		:	return LE
				default			:	r.Unread( w )
									return Define
				}
			}
			throw Error.ReadError( "EOD" )
		case "<"				:
			if let w = r.Read() {
				switch w {
				case ">"		:	return Neq
				case "="		:	return LE
				default			:	r.Unread( w )
									return LT
				}
			}
			throw Error.ReadError( "EOD" )
		case ">"				:
			if let w = r.Read() {
				switch w {
				case "<"		:	return Neq
				case "="		:	return GE
				default			:	r.Unread( w )
									return GT
				}
			}
			throw Error.ReadError( "EOD" )
		case ( "0"..."9" )		:	r.Unread( u )
									return try ReadNumber( r )
		case ( "a"..."z" ), ( "A"..."Z" ), "_":
									r.Unread( u )
									return try ReadName( r )
		default					:	throw Error.ReadError( "Invalid character \(u):\(u.value)" )
		}
	}
	return nil
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class
GUIPreProcessor: Reader< UnicodeScalar > {
	var
	m	= String.UnicodeScalarView()
	init(	_ p	:	String ) {
		var wLines = p.componentsSeparatedByCharactersInSet( NSCharacterSet.newlineCharacterSet() )
		for ( i, w ) in wLines.enumerate() {
			wLines[ i ] = w.componentsSeparatedByString( "//" )[ 0 ]
		}
		m = ( wLines as NSArray ).componentsJoinedByString( "\n" ).unicodeScalars
	}
	override func
	_Read() -> UnicodeScalar? {
		if m.count == 0 { return nil }
		let v = m.first
		m = m.dropFirst()
		return v
	}
}

