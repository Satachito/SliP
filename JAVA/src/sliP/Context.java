package sliP;

import java.io.InputStreamReader;
import java.io.PushbackReader;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.function.BiFunction;
import java.util.function.Function;
import java.util.function.Supplier;

class
Cell<T> {
	T		u;
	Cell<T>	next;
	Cell( T p, Cell<T> pNext ) {
		u = p;
		next = pNext;
	}
}

public class 
Context {

	class
	Obj {
		Obj
		Eval() throws Exception { return this; }
	}
	
	Cell< Obj >
	stack						=	null;

	void
	Push( Obj p ) {
	//	print( "Push:", p )
		stack = new Cell<>( p, stack );
	}
	
	void
	Pop() {
	//print( "Pop:", sStack!.u )
		stack = stack.next;
	}
	
	Cell< Map< String, Obj > >
	dicts						=	new Cell<>( new HashMap< String, Obj >(), null );
	
	class
	Name						extends Obj {
		String
		u;
		Name( String p )		{ u = p; }
		public String
		toString()				{ return u; }
		Obj
		Eval() throws Exception	{
			for ( Cell< Map< String, Obj > > wAL = dicts; wAL != null; wAL = wAL.next ) {
				Obj v = wAL.u.get( u );
				if ( v != null ) { return v; }
			}
			throw new Exception( "UndefinedName " + u );
		}
	}
	
	class
	Number						extends Obj {
		double
		Value()					{ assert( false ); return 0; }
	}
//	func ==( l: Number, r: Number ) -> Bool {
//		return l.Value() == r.Value()
//	}
	
	
	class
	IntNumber					extends Number {
		int
		u;					
		IntNumber( int d )		{ u = d; }
		public String
		toString()				{ return "" + u; }
		double
		Value()					{ return u; }
	}
	
	class
	FloatNumber					extends Number {
		double
		u;
		FloatNumber( double p )	{ u = p; }
		public String
		toString()				{ return "" + u; }
		double
		Value()					{ return u; }
	}
	
	class
	Str							extends Obj {
		String
		u;
		Str( String p )			{ u = p; }
		public String
		toString()				{ return "\"" + u + "\""; }
	}
	
	class
	Primitive					extends Obj {
		Supplier<Obj>
		u;
		public 
		Primitive( Supplier<Obj> p ) { u = p; }
		public String
		toString()				{ return "P"; }
		Obj
		Eval() throws Exception	{
			return u.get(); 
		}
	}
	
	class
	Builtin						extends Obj {
		Function< Obj, Obj >
		u;
		Builtin( Function< Obj, Obj > p ) { u = p; }
		public String
		toString()				{ return "B"; }
	}
	
	class
	Operator					extends Obj {
		int
		bond;
		BiFunction< Obj, Obj, Obj >
		u;
		Operator( BiFunction< Obj, Obj, Obj > p, int pBond ) { u = p; bond = pBond; }
		public String
		toString()				{ return ":" + bond; }
	}
	
	class
	Quote						extends Obj {
		Obj
		u;
		Quote( Obj p )			{ u = p; }
		public String
		toString()				{ return "'" + u; }
		Obj
		Eval()					{ return u; }
	}
	
	class
	Combiner					extends Obj {
		Obj
		u;
		Combiner( Obj p )		{ u = p; }
		public String
		toString()				{ return "`" + u; }
		Obj
		Eval()					{ return new EvalAssoc( u, dicts.u ); }
	}
	
	class
	EvalAssoc					extends Obj {
		Obj
		u;
		Map< String, Obj >
		map;
		EvalAssoc( Obj p, Map< String, Obj > pMap ) { u = p; map = pMap; }
		public String
		toString()				{ return "<" + map + ":" + u + ">"; }
		Obj
		Eval() throws Exception {
			dicts = new Cell< Map< String, Obj > >( map, dicts );
			try {
				return u.Eval();
			} catch ( Exception e ) {
				throw e;
			} finally {
				dicts = dicts.next;
			}
		}
	}
	
	class
	Dict						extends Obj {
		Map< String, Obj >
		u;
		Dict( Map< String, Obj > p ) { u = p; }
		public String
		toString() { return "" + u; }
	}
	
	enum Type {
		 Literal
	,	 Parallel
	,	 Block
	,	 Sentence
	}

	Obj
	T	=	new Str( "T" );
	
	Obj
	Nil	=	new Linear( new Obj[ 0 ], Type.Literal );

	class
	Linear				extends Obj {
		Obj
		EvalSentence( Obj[] p ) throws Exception {
			switch ( p.length ) {
			case  0:	return Nil;
			case  1:	return p[ 0 ].Eval();
			case  2:	throw new Exception( "No operator in " + p );
			default:	break;
			}
			Operator wTarget = null;
			int wIndex  =	0;
			for ( int i = 1; i < p.length - 1; i++ ) {
				Obj w = p[ i ];
				if ( w instanceof Operator ) {
					if ( wTarget == null ) {
						wTarget = (Operator)w;
						wIndex = i;
					} else {
						if ( ((Operator)w).bond >= wTarget.bond ) {
							wTarget = (Operator)w;
							wIndex = i;
						}
					}
				}
			}
			if ( wTarget != null ) {
				return wTarget.u.apply( EvalSentence( Arrays.copyOfRange( p, 0, wIndex ) ), EvalSentence( Arrays.copyOfRange( p, wIndex + 1, p.length ) ) );
			} else {
				throw new Exception( "No operator in " + p );
			}
		}
		
		Type
		type;	
		Obj[]
		u;
		Linear( Obj[] p, Type pType ) { u = p; type = pType; }
		String
		_Str() {
			String	v = " ";
			for ( Obj w: u ) { v += "" + w + " "; }
			return v;
		}
		public String
		toString() {
			switch ( type ) {
			case Literal	:	return "[" + _Str() + "]";
			case Parallel	:	return "«" + _Str() + "»";
			case Block		:	return "{" + _Str() + "}";
			case Sentence	:	return "(" + _Str() + ")";
			default			:	return "";
			}
		}
		Obj
		Eval() throws Exception {
			switch ( type ) {
			case Literal:
				return this;
			case Parallel:
//				Obj[]	v = new Obj[ u.length ];
//				let	wG = dispatch_group_create()
//				let	wQ = dispatch_get_global_queue( DISPATCH_QUEUE_PRIORITY_DEFAULT, 0 )
//				for ( i, w ) in u.enumerate() {
//					dispatch_group_async( wG, wQ ) {
//						do {
//							v[ i ] = try w.Eval()
//						} catch let e {
//							print( e )
//						}
//					}
//				}
//				dispatch_group_wait( wG, DISPATCH_TIME_FOREVER )
//				for w in u {
//					if w == sObj { throw Error.RuntimeError( "Parallel" ) }	//	UC
//				}
//				return Linear( v, .Literal )
				return this;
			case Block:
				dicts = new Cell< Map< String, Obj > >( new HashMap< String, Obj >(), dicts );
//				defer{ dicts = dicts.next! }
				Obj[]	v = new Obj[ u.length ];
				for ( int i = 0; i < u.length; i++ ) v[ i ] = u[ i ].Eval();
				return new Linear( v, Type.Literal );
			case Sentence:
				return EvalSentence( u );
			}
			return Nil;
		}
	}

	boolean
	NilSliP( Obj p ) {
		return p instanceof Linear && ((Linear)p).u.length == 0;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	void
	SkipWhite( PushbackReader p ) {
		while ( true ) {
			int w = p.read();
			if ( w < 0 ) break;
			if ( Character.isWhitespace( w ) ) {
				p.unread( w );
				break;
			}
		}
	}
	
	Number
	ReadNumber( PushbackReader p, boolean neg ) throws Exception {
	 	String	v = "";
		Boolean	wFloatF = false;
		while ( true ) {
			int c = p.read();
			if ( c < 0 ) break;
			switch ( c ) {
			case	'0': case  '1': case  '2': case  '3': case  '4': case  '5': case  '6': case  '7': case  '8': case  '9': case  'x': case  'X': case  '+': case  '-':
				v += c;
			case	'.': case  'e': case  'E':
				v += c;
				wFloatF = true;
			default:
				p.unread( c );
				if ( wFloatF ) {
					double w = Double.parseDouble( v );
					return new FloatNumber( neg ? -w : w );
				} else {
					int w = Integer.parseInt( v );
					return new IntNumber( neg ? -w : w );
				}
			}
		}
		throw new Exception( "EOD" );
	}
	
	Name
	ReadName( PushbackReader p ) throws Exception {
	 	String	v = "";
		while ( true ) {
			int c = p.read();
			if ( c < 0 ) break;
			if ( ( '0' <= c && c <= '9' ) || ( 'a'<= c && c <= 'z' ) || ( 'A'<= c && c <= 'Z' ) || ( c == '_' ) ) {
				v += c;
			} else {
				p.unread( c );
				return new Name( v );
			}
		}
		throw new Exception( "EOD" );
	}
	
	Str
	ReadStr( PushbackReader p ) throws Exception {
		Boolean	wEscaped = false;
	 	String	v = "";
		while ( true ) {
			int c = p.read();
			if ( c < 0 ) break;
			if ( wEscaped ) {
				wEscaped = false;
				v += c;
			} else {
				switch ( c ) {
				case '"'	:	return new Str( v );
				case '\\'	:	wEscaped = true;
				default		:	v += c;
				}
			}
		}
		throw new Exception( "EOD" );
	}
	
	Obj[]
	ReadObjs( PushbackReader p, int terminator ) throws Exception {
		Obj[]	v = new Obj[ 0 ];
		while ( true ) {
			Obj w = Read( p, terminator );
			if ( w == null ) return break;
			v.append( w );
		}
		return v;
	}
	
	Primitive
	StackTop = new Primitive( () ->
		{	if ( stack != null ) { return stack.u; }
			throw new Exception( "StackUnderflow" );
		}
	);

	Primitive
	CurrentContext = new Primitive( () -> new Dict( dicts.u ) );
	
	Primitive
	Exception = new Primitive( () -> { new Exception( "UserException" ); return new Obj(); } );
	
	Builtin
	Cdr = new Builtin( p ->
		{	if ( p instanceof Linear )
			{	Linear w = (Linear)p;
				if ( w.u.length > 0 ) return new Linear( Arrays.copyOfRange( w.u, 0, w.u.length - 1 ), w.type );
			}
			throw new Exception( "" + p + ":*" );
		}
	);
	
	Builtin
	Count = new Builtin( p ->
		{	if ( p instanceof Linear ) return new IntNumber( ((Linear)p).u.length );
			throw new Exception( "" + p + ":#" );
		}
	);
	
	Builtin
	Last = new Builtin( p ->
		{	if ( p instanceof Linear ) 
			{	Linear w = (Linear)p;
				if ( w.u.length > 0 ) return w.u[ w.u.length - 1 ];
			}
			throw new Exception( "" + p + ":$" );
		}
	);
	
	Builtin
	Print = new Builtin( p ->
		{	System.out.println( p );
			return p;
		}
	);
	
	Operator
	If = new Operator( ( l, r ) -> NilSliP( l ) ? Nil : r.Eval()
	,	9
	);
	
	Operator
	IfElse = new Operator( ( l, r ) -> 
		{	if ( r instanceof Linear ) 
			{	Linear w = (Linear)r;
				if ( w.type == Type.Literal && w.u.length == 2 ) {
					return NilSliP( l ) ? w.u[ 1 ].Eval() : w.u[ 0 ].Eval();
				}
			}
			throw new Exception( "" + l + " ? " + r );
		}
	,	9
	);
	
	Operator
	Cons = new Operator( ( l, r ) -> 
		{	if ( r instanceof Linear ) 
			{	Linear w = (Linear)r;
				Obj[] wNew = new Obj[ w.u.length + 1 ];
				wNew[ 0 ] = l;
				for ( int i = 0; i < w.u.length; i++ ) wNew[ i + 1 ] = w.u[ i ];
				return new Linear( w.u, w.type );
			}
			throw new Exception( "" + l + " , " + r );
		}
	,	7
	);
	
	Operator
	Apply = new Operator( ( l, r ) -> 
		{	if ( l instanceof Linear && r instanceof IntNumber ) {
				int	wR = ((IntNumber)w).u;
				Linear wL = (Linear)l;
				if ( wL.u.count > wR ) { 
					return wL.u[ wR ];
				}
			} else if ( l instanceof Assoc && r instanceof Name ) {
				Obj w = ((Assoc)l).u[ ((Name)r).u ];
				return w ? w : Nil;
			} else if ( r instanceof Builtin ) {
				return ((Builtin).r).u( l );
			} else if ( r instanceof Linear ) {
				Push( l );
				try { return w.Eval(); } finally { Pop(); }
			} else if ( r instanceof EvalAssoc ) {
				Push( l );
				dicts = new Cell<>( ((EvalAssoc)r).u, dicts );
				try { 
					return w.Eval(); 
				} finally {
					dicts = dicts.next;
					Pop();
				}
			} else {
				throw new Exception( "" + l + " : " + r );
			}
		}
	,	0
	);
	
	Operator
	And = new Operator( ( l, r ) -> 
		{	if ( l instanceof Integer &&  r instanceof Integer ) return Integer( ((Integer)l).u & ((Integer)r).u );
			return NilSliP( l ) && NilSliP( r ) ? T : Nil;
		}
	,	4
	);
	
	Operator
	Xor = new Operator( ( l, r ) -> 
		{	if ( l instanceof Integer &&  r instanceof Integer ) return Integer( ((Integer)l).u ^ ((Integer)r).u );
			return NilSliP( l ) != NilSliP( r ) ? T : Nil;
		}
	,	4
	);
	
	Operator
	Or = new Operator( ( l, r ) -> 
		{	if ( l instanceof Integer &&  r instanceof Integer ) return Integer( ((Integer)l).u | ((Integer)r).u );
			return NilSliP( l ) || NilSliP( r ) ? T : Nil;
		}
	,	4
	);
	
	Operator
	Mul = new Operator( ( l, r ) -> 
		{	if ( l instanceof Integer &&  r instanceof Integer ) return Integer( ((Integer)l).u * ((Integer)r).u );
			if ( l instanceof Number &&  r instanceof Number ) return Float( ((Number)l).Value() * ((Number)r).Value() );
			throw new Exception( "" + l + " × " + r );
		}
	,	5
	);
	
						Operator
	Div = new Operator( ( l, r ) -> 
		{	if let wL = l as? Number, let wR = r as? Number { return Float( wL.Value() / wR.Value() ) }
			throw new Exception( "\(l) ÷ \(r)" )
		}
	,	5
	);
	
						Operator
	IDiv = new Operator( ( l, r ) -> 
		{	if let wL = l as? Integer, let wR = r as? Integer { return Integer( wL.u / wR.u ) }
			throw new Exception( "\(l) / \(r)" )
		}
	,	5
	);
	
						Operator
	Remainder = new Operator( ( l, r ) -> 
		{	if let wL = l as? Integer, let wR = r as? Integer { return Integer( wL.u % wR.u ) }
			throw new Exception( "\(l) % \(r)" )
		}
	,	5
	);
	
						Operator
	Add = new Operator(
		{	if let wL = l as? Integer, let wR = r as? Integer { return Integer( wL.u + wR.u ) }
			if let wL = l as? Number, let wR = r as? Number { return Float( wL.Value() + wR.Value() ) }
			if let wL = l as? Str, let wR = r as? Str { return Str( wL.u + wR.u ) }
			if let wL = l as? Linear, let wR = r as? Linear where wL.type == wR.type { return Linear( wL.u + wR.u, wL.type ) }
			throw new Exception( "\(l) + \(r)" )
		}
	,	6
	);
	
						Operator
	Minus = new Operator( ( l, r ) -> 
		{	if let wL = l as? Integer, let wR = r as? Integer { return Integer( wL.u - wR.u ) }
			if let wL = l as? Number, let wR = r as? Number { return Float( wL.Value() - wR.Value() ) }
			throw new Exception( "\(l) - \(r)" )
		}
	,	6
	);
	
	func
	Equal( l: Obj, _ r: Obj ) -> Obj {
		if let wL = l as? Number, let wR = r as? Number { return wL.Value() == wR.Value() ? T : Nil }
		if let wL = l as? Str, let wR = r as? Str { return wL.u == wR.u ? T : Nil }
		if let wL = l as? Name, let wR = r as? Name { return wL.u == wR.u ? T : Nil }
		if let wL = l as? Linear, let wR = r as? Linear {
			if wL.type != wR.type { return Nil }
			if wL.u.count != wR.u.count { return Nil }
			for i in 0 ..< wL.u.count { if Equal( wL.u[ i ], wR.u[ i ] ) == Nil { return Nil } }
			return T
		}
		return l == r ? T : Nil
	};
	
	Operator
	Eq = new Operator( ( l, r ) -> Equal( l, r }
	,	8
	);
	
	Operator
	GE = new Operator( ( l, r ) -> 
		{	if let wL = l as? Number, let wR = r as? Number { return wL.Value() >= wR.Value() ? T : Nil }
			throw new Exception( "\(l) >= \(r)" )
		}
	,	8
	);
	
	Operator
	LE = new Operator( ( l, r ) -> 
		{	if let wL = l as? Number, let wR = r as? Number { return wL.Value() <= wR.Value() ? T : Nil }
			throw new Exception( "\(l) <= \(r)" )
		}
	,	8
	);
	
	Operator
	Define = new Operator( ( l, r ) -> 
		{	if let w = l as? Name {
				dicts.u[ w.u ] = r
				return l
			}
			throw new Exception( "\(l) = \(r)" )
		}
	,	10
	);
	
	Operator
	Neq = new Operator( ( l, r ) -> 
		{	if let wL = l as? Number, let wR = r as? Number { return wL.Value() != wR.Value() ? T : Nil }
			if let wL = l as? Str, let wR = r as? Str { return wL.u != wR.u ? T : Nil }
			return l != r ? T : Nil
		}
	,	8
	);
	
	Operator
	LT = new Operator( ( l, r ) -> 
		{	if let wL = l as? Number, let wR = r as? Number { return wL.Value() < wR.Value() ? T : Nil }
			throw new Exception( "\(l) < \(r)" )
		}
	,	8
	);
	
	Operator
	GT = new Operator( ( l, r ) -> 
		{	if let wL = l as? Number, let wR = r as? Number { return wL.Value() > wR.Value() ? T : Nil }
			throw new Exception( "\(l) > \(r)" )
		}
	,	8
	);
	
	Operator
	Contains = new Operator( ( l, r ) -> 
		{	if let wR = r as? Linear {
				return wR.u.contains( l ) ? T : Nil
			}
			throw new Exception( "\(l) ∈ \(r)" )
		}
	,	8
	);
	Operator
	ContainsR = new Operator( ( l, r ) -> 
		{	if ( l instanceof Linear ) { return wL.u.contains( r ) ? T : Nil }
			throw new Exception( "" + l + " ∋ " + r )
		}
	,	8
	);
	
	func
	Read( pReader: Reader< UnicodeScalar >, _ terminator: UnicodeScalar = UnicodeScalar( 0 ) ) throws Exception -> Obj? {
		SkipWhite( pReader )
		if let c = pReader.Read() {
			switch c {
			case terminator			:	return nil
			case "}", "]", ")"		:	throw Error.ReadError( "Unexpected \(c)" )
			case "["				:	return Linear( try ReadObjs( pReader, "]" as UnicodeScalar ), .Literal )
			case "«"				:	return Linear( try ReadObjs( pReader, "»" as UnicodeScalar ), .Parallel )
			case "("				:	return Linear( try ReadObjs( pReader, ")" as UnicodeScalar ), .Sentence )
			case "{"				:	return Linear( try ReadObjs( pReader, "}" as UnicodeScalar ), .Block )
			case "\""				:	return try ReadStr( pReader )
			case "'"				:	if let w = try Read( pReader ) { return Quote( w ) }; throw Error.ReadError( "No Obj to quote" )
			case "`"				:	if let w = try Read( pReader ) { return Combiner( w ) }; throw Error.ReadError( "No Obj to combine" )
			case "!"				:	return Exception
			case "@"				:	return StackTop
			case "·"				:	return CurrentContext
			case "*"				:	return next
			case "#"				:	return Count
			case "$"				:	return Last
			case "¦"				:	return Print
			case "∈"				:	return Contains
			case "∋"				:	return ContainsR
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
				if let w = pReader.Read() {
					pReader.Unread( w )
					switch w {
					case ( "0"..."9" ), ".":
										return try ReadNumber( pReader )
					default			:	return Add
					}
				}
				throw new Exception( "EOD" );
			case "-"				:
				if let w = pReader.Read() {
					pReader.Unread( w )
					switch w {
					case ( "0"..."9" ), ".":
										return try ReadNumber( pReader, true )
					default			:	return Minus
					}
				}
				throw new Exception( "EOD" );
			case "="			:
				if let w = pReader.Read() {
					switch w {
					case "="		:	return Eq
					case ">"		:	return GE
					case "<"		:	return LE
					default			:	pReader.Unread( w )
										return Define
					}
				}
				throw new Exception( "EOD" );
			case "<"				:
				if let w = pReader.Read() {
					switch w {
					case ">"		:	return Neq
					case "="		:	return LE
					default			:	pReader.Unread( w )
										return LT
					}
				}
				throw new Exception( "EOD" );
			case ">"				:
				if let w = pReader.Read() {
					switch w {
					case "<"		:	return Neq
					case "="		:	return GE
					default			:	pReader.Unread( w )
										return GT
					}
				}
				throw new Exception( "EOD" );
			case ( "0"..."9" )		:	pReader.Unread( c )
										return try ReadNumber( pReader )
			case ( "a"..."z" ), ( "A"..."Z" ), "_":
										pReader.Unread( c )
										return try ReadName( pReader )
			default					:	throw new Exception( "Invalid character \(c)" )
			}
		}
		return nil
	}
	
	
	func
	SetupBuiltin() {
		dicts.u[ "eval" ] = Builtin(
			{	p in
				let v = try p.Eval()
	print( "Eval:", p, "->", v )
				return v
			}
		)
		dicts.u[ "for" ] = Builtin(
			{	p in
				if let wArgs = p as? Linear where wArgs.u.count >= 2 {
					if let wLinear = wArgs.u[ 0 ] as? Linear {
						var	v = [ Obj ]()
						for w in wLinear.u {
							Push( w )
							defer{ Pop() }
							v.append( try wArgs.u[ 1 ].Eval() )
						}
						return Linear( v, wLinear.type )
					} else {
						throw new Exception( "\(p):for" )
					}
				} else {
					throw new Exception( "\(p):for" )
				}
			}
		)
		dicts.u[ "TwoElements" ] = Builtin(
			{	p in
				if let w = p as? Linear {
					switch w.u.count {
					case 0, 1	:	return Integer( 0 )
					case 2		:	return Integer( 1 )
					default		:	return Integer( 2 )
					}
				} else {
					throw new Exception( "\(p):TwoElements" )
				}
			}
		)
	}
}
