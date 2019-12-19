const fs = require( 'fs' )

class
Context {
	constructor( next ) {
		this.dict = {}
		this.stack = []
		this.next = next
	}
}

class
SliP {
	constructor( _ ) { this._ = _ }
	get
	string() { return 'T' }

	Eval( c ) { return this }
}

class
Numeric extends SliP {
	constructor( _ ) { super( _ ) }
	get
	string() { return String( this._ ) }
}

class
Literal extends SliP {
	constructor( _ ) { super( _ ) }
	get
	string() { return `"${this._}"` }
}

class
Name extends SliP {
	constructor( _ ) { super( _ ) }
	get
	string() { return this._ }

	Eval( c ) {
		while ( c ) {
			const v = c.dict[ this._ ]
			if ( v ) return v
			c = c.next
		}
		throw `Undefined:${this._}`
	}
}

class
Dict extends SliP {
	constructor( _ ) { super( _ ) }
	get
	string() { return '(Dict)' }
}

class
Primitive extends SliP {
	constructor( _, name ) {
		super( _ )
		this.name = name
	}
	get
	string() { return this.name }

	Eval( c ) { return this._( c ) }
}

class
Prefix extends SliP {
	constructor( _, target, name ) {
		super( _ )
		this.target = target
		this.name = name
	}
	get
	string() { return this.name + this.target.string }

	Eval( c ) { return this._( c, this.target ) }
}

class
Unary extends SliP {
	constructor( _, name ) {
		super( _ )
		this.name = name
	}
	get
	string() { return this.name }
}

class
Dyadic extends SliP {
	constructor( _, name, priority ) {
		super( _ )
		this.name = name
		this.priority = priority
	}
	get
	string() { return this.name }
}

class
_List extends SliP {
	constructor( _ ) { super( _ ) }
	get
	string() { return this._.map( _ => _.string ).join( ' ' ) }
}

class
List extends _List {
	constructor( _ ) { super( _ ) }
	get
	string() { return `[ ${super.string} ]` }
}

class
Parallel extends _List {
	constructor( _ ) { super( _ ) }
	get
	string() { return `« ${super.string} »` }

	Eval( c ) { return new List( this._.map( _ => _.Eval( new Context( c ) ) ) ) }
}

class
Procedure extends _List {
	constructor( _ ) { super( _ ) }
	get
	string() { return `{ ${super.string} }` }

	Eval( c ) {
		c = new Context( c )
		return new List( this._.map( _ => _.Eval( c ) ) )
	}
}

const
EvalSentence = ( c, _ ) => {
	switch ( _.length ) {
	case  0:
		return new List( _ )
	case  1:
		return _[ 0 ].Eval( c )
	case  2:
		break
	default:
		const wDyadicIndices = Object.keys( _ ).filter( i => _[ i ] instanceof Dyadic ).map( _ => Number( _ ) )
		if ( wDyadicIndices.length ) {
			const i = wDyadicIndices.reduce(
				( a, c ) => _[ c ].priority >= _[ a ].priority ? c : a
			,	wDyadicIndices[ 0 ]
			)
			if ( ( 0 < i ) && ( i < _.length - 1 ) ) {
				return _[ i ]._( c, EvalSentence( c, _.slice( 0, i ) ), EvalSentence( c, _.slice( i + 1 ) ) )
			}
		}
		break
	}
	throw `Syntax error: ${new Sentence( _ ).string}`
}

class
Sentence extends _List {
	constructor( _ ) { super( _ ) }
	get
	string() { return `( ${super.string} )` }

	Eval( _ ) { return EvalSentence( _, this._ ) }
}

////////////////////////////////////////////////////////////////
//	Builtins
////////////////////////////////////////////////////////////////

const
T = new SliP()

const
Nil = new List( [] )

const
_IsT = _ => !( _ instanceof _List ) || _._.length

const
_IsNil = _ => _ instanceof _List && !_._.length

const
CurrentDict		= new Primitive(
	c => new Dict( c.stack.dict )
,	'·'
)

const
Stack			= new Primitive(
	c => new List( c.stack.slice() )
,	'@'
)

const
Count			= new Unary(
	( c, _ ) => {
		if ( _ instanceof _List		) return new Numeric( _._.length )
		if ( _ instanceof Literal	) return new Numeric( _._.length )
		throw `#${_.string}`
	}
,	'#'
)

const
Cdr				= new Unary(
	( c, _ ) => {
		if ( _ instanceof _List && _._.length ) {
			switch ( _.constructor.name ) {
			case 'List'		: return new List		( _._.slice( 1 ) )
			case 'Parallel'	: return new Parallel	( _._.slice( 1 ) )
			case 'Sentence'	: return new Sentence	( _._.slice( 1 ) )
			case 'Procedure': return new Procedure	( _._.slice( 1 ) )
			}
		}
		throw `*${_.string}`
	}
,	"*"
)

const
Last			= new Unary(
	( c, _ ) => {
		const length = _._.length
		if ( _ instanceof _List && length ) return _._[ length - 1 ]
		throw `$${_.string}`
	}
,	'$'
)

const
Print			= new Unary(
	( c, _ ) => {
		console.log( _.string )
		return _
	}
,	'.'
)

const
Log				= new Unary(
	( c, _ ) => {
		console.error( _.string )
		return _
	}
,	'|'
)

/*
	:	20	Apply		2 x [ 3, 4 ]:1			2 x 4
	×	30	Multi/Div	2 + 3 x 4				2 + 12
	+	40	Plus/Minus	2 | 3 + 4				2 | 7
	|	50	Binary		2 < 3 | 4				2 < 7
	<	60	Compare		1, 2 < 3				1, T
	∈	60	Member		1, 2 ∈ [ 1, 2, 3 ]		1, T
	,	70	Cons		T || 1, [ 2 ]			T || [ 1, 2 ]
	||	80	Logical		'a = [ 2 ] || T			a = T
	?	85	IfElse		
	=	90	Define		
*/
const
Define			= new Dyadic(
	( c, l, r ) => {
		if ( l instanceof Name ) {
			c.dict[ l._ ] = r
			return r
		}
		throw `${l.string}=${r.string}`
	}
,	'='
,	90
)

const
IfElse			= new Dyadic(
	( c, l, r ) => {
		if ( r instanceof List && r._.length == 2 ) return r._[ _IsT( l ) ? 0 : 1 ].Eval( c )
		throw `${l.string}?${r.string}`
	}
,	'?'
,	85
)

const
If				= new Dyadic(
	( c, l, r ) => _IsT( l ) ? r.Eval( c ) : Nil
,	'¿'
,	85
)


const
LogicalAnd		= new Dyadic(
	( c, l, r ) => {
		return _IsT( l ) & _IsT( r ) ? T : Nil
	}
,	'&&'
,	80
)

const
LogicalOr		= new Dyadic(
	( c, l, r ) => _IsT( l ) | _IsT( r ) ? T : Nil
,	'||'
,	80
)

const
LogicalXOr		= new Dyadic(
	( c, l, r ) => _IsT( l ) ^ _IsT( r ) ? T : Nil
,	'^^'
,	80
)

const
Cons			= new Dyadic(
	( c, l, r ) => {
		if ( r instanceof _List ) {
			switch ( r.constructor.name ) {
			case 'List'			: return List		( [ l, ...r._ ] )
			case 'Parallel'		: return Parallel	( [ l, ...r._ ] )
			case 'Sentence'		: return Sentence	( [ l, ...r._ ] )
			case 'Procedure'	: return Procedure	( [ l, ...r._ ] )
			default				: throw 'internal error'
			}
		}
		throw `${l.string},${r.string}`
	}
,	','
,	70
)

const
MemberOf		= new Dyadic(
	( c, l, r ) => {
		if ( r instanceof _List ) return r._.includes( l ) ? T : Nil
		throw `${l.string}∈${r.string}`
	}
,	'∈'
,	60
)

const
Contains		= new Dyadic(
	( c, l, r ) => {
		if ( l instanceof _List ) return l._.includes( r ) ? T : Nil
		throw `${l.string}∋${r.string}`
	}
,	'∋'
,	60
)

const
_EQ_List = ( p, q ) => {
	if ( p.length != q.length ) return false
	for ( let i = 0; i < p.length; i++ ) if ( !_EQ( p[ i ], q[ i ] ) ) return false
	return true
}
const
_EQ = ( p, q ) => {
	if ( p instanceof Numeric   && q instanceof Numeric   ) return p._ == q._
	if ( p instanceof Literal   && q instanceof Literal   ) return p._ == q._
	if ( p instanceof Name      && q instanceof Name      ) return p._ == q._
	if ( p instanceof List      && q instanceof List      ) return _EQ_List( p, q )
	if ( p instanceof Procedure && q instanceof Procedure ) return _EQ_List( p, q )
	if ( p instanceof Parallel  && q instanceof Parallel  ) return _EQ_List( p, q )
	if ( p instanceof Sentence  && q instanceof Sentence  ) return _EQ_List( p, q )
	return p === q
}
const
EQ				= new Dyadic(
	( c, l, r ) => _EQ( l, r ) ? T : Nil
,	'=='
,	60
)
const
NEQ				= new Dyadic(
	( c, l, r ) => _EQ( l, r ) ? Nil : T
,	'<>'
,	60
)
const
LT				= new Dyadic(
	( c, l, r ) => {
		if ( l instanceof Numeric && r instanceof Numeric ) return l._ < r._ ? T : Nil
		throw `${l.string}<${r.string}`
	}
,	'<'
,	60
)
const
GT				= new Dyadic(
	( c, l, r ) => {
		if ( l instanceof Numeric && r instanceof Numeric ) return l._ > r._ ? T : Nil
		throw `${l.string}>${r.string}`
	}
,	'>'
,	60
)
const
LE				= new Dyadic(
	( c, l, r ) => {
		if ( l instanceof Numeric && r instanceof Numeric ) return l._ <= r._ ? T : Nil
		throw `${l.string}<=${r.string}`
	}
,	'<='
,	60
)
const
GE				= new Dyadic(
	( c, l, r ) => {
		if ( l instanceof Numeric && r instanceof Numeric ) return l._ >= r._ ? T : Nil
		throw `${l.string}>=${r.string}`
	}
,	'>='
,	60
)
const
BitAnd			= new Dyadic(
	( c, l, r ) => {
		if ( l instanceof Numeric && r instanceof Numeric ) return new Numeric( l._ & r._ )
		throw `${l.string}&${r.string}`
	}
,	'&'
,	50
)

const
BitOr			= new Dyadic(
	( c, l, r ) => {
		if ( l instanceof Numeric && r instanceof Numeric ) return new Numeric( l._ | r._ )
		throw `${l.string}|${r.string}`
	}
,	'|'
,	50
)
const
BitXOr			= new Dyadic(
	( c, l, r ) => {
		if ( l instanceof Numeric && r instanceof Numeric ) return new Numeric( l._ ^ r._ )
		throw `${l.string}^${r.string}`
	}
,	'^'
,	50
)

const
Plus			= new Dyadic(
	( c, l, r ) => {
		if ( l instanceof Numeric	&& r instanceof Numeric		) return new Numeric	( l._ + r._ )
		if ( l instanceof Literal	&& r instanceof Literal		) return new Literal	( l._ + r._ )
		if ( l instanceof List		&& r instanceof List		) return new List		( [ ...l._, ...r._ ] )
		if ( l instanceof Parallel	&& r instanceof Parallel	) return new Parallel	( [ ...l._, ...r._ ] )
		if ( l instanceof Procedure	&& r instanceof Procedure	) return new Procedure	( [ ...l._, ...r._ ] )
		if ( l instanceof Sentence	&& r instanceof Sentence	) return new Sentence	( [ ...l._, ...r._ ] )
		throw `${l.string}+${r.string}`
	}
,	'+'
,	40
)

const
Minus			= new Dyadic(
	( c, l, r ) => {
		if ( l instanceof Numeric && r instanceof Numeric ) return new Numeric( l._ - r._ )
		throw `${l.string}-${r.string}`
	}
,	'-'
,	40
)

const
Mul				= new Dyadic(
	( c, l, r ) => {
		if ( l instanceof Numeric && r instanceof Numeric ) return new Numeric( l._ * r._ )
		throw `${l.string}*${r.string}`
	}
,	'×'
,	30
)

const
Div				= new Dyadic(
	( c, l, r ) => {
		if ( l instanceof Numeric && r instanceof Numeric ) return new Numeric( l._ / r._ )
		throw `${l.string}÷${r.string}`
	}
,	'÷'
,	30
)

const
Modulo			= new Dyadic(
	( c, l, r ) => {
		if ( l instanceof Numeric && r instanceof Numeric ) return new Numeric( l._ % r._ )
		throw `${l.string}%${r.string}`
	}
,	'%'
,	30
)

const
Apply			= new Dyadic(
	( c, l, r ) => {
		switch ( r.constructor.name ) {
		case 'Numeric'	:
			{	if ( ! l instanceof _List ) throw `${l.string}:${r.string}`
				let v = l._[ r._ ]
				return v ? v : Nil
			}
		case 'Name'		:
			{	if ( ! l instanceof Dict ) throw `${l.string}:${r.string}`
				let v = l._[ r._ ]
				return v ? v : Nil
			}
		case 'Unary'	:
			return r._( c, l )
		default:
			{	c.stack.push( l )
				const v = r.Eval( c )
				c.stack.pop()
				return v
			}
		}
	}
,	':'
,	20
)

////////////////////////////////////////////////////////////////
//	Reader
////////////////////////////////////////////////////////////////

class
StringReader {
	constructor( _ ) {
		this._ = _
		this.i = 0
	}
	Avail()		{ return this.i < this._.length }
	Peek()		{ return this.Avail() ? this._[ this.i ] : null }
	Read()		{ return this.Avail() ? this._[ this.i++ ] : null }
	Unread()	{ --this.i }
}

const
ReadNumber = ( r, neg ) => {
 	let	v = ""
	while ( r.Avail() ) {
		const _ = r.Read()
		if ( ! _.match( /\d/ ) && _ != '.' ) break
		v += _
	}
	return new Numeric( Number( neg ? -v : v ) )
}

const
ReadName = r => {

	let	v = ""
	while ( r.Avail() ) {
		const _ = r.Read()
		if ( _.match( /\s/ ) ) break
		v += _
	}
	return new Name( v )
}

const
ReadStr = r => {

	var	v = ""
	var	wEscaped = false
	while ( r.Avail() ) {
		const _ = r.Read()
		if ( wEscaped ) {
			wEscaped = false
			switch ( _ ) {
			case '0'	: v += '\0';	break
			case 't'	: v += '\t';	break
			case 'n'	: v += '\n';	break
			case 'r'	: v += '\r';	break
			default		: v += _;		break
			}
		} else {
			switch ( _ ) {
			case "\""	: return 			new Literal( v )
			case "\\"	: wEscaped = true;	break
			default		: v += _;			break
			}
		}
	}
	throw `Syntax error: Unterminated string: ${v}`
}

const
ReadList = ( r, terminator ) => {
	var	v = []
	for ( let w; w = Read( r, terminator ); ) v.push( w )
	return v
}

const
Read = ( r, terminator ) => {
	while ( r.Avail() ) {
		let _ = r.Peek()
		if ( _.match( /\d/ ) )	return ReadNumber( r, false )
		_ = r.Read()
		if ( _.match( /\s/ ) )	continue
		switch ( _ ) {
		case	terminator	: return null	//	Must be before close brace
		case	']'			: throw `Unexpected ${_}`
		case	'}'			: throw `Unexpected ${_}`
		case	')'			: throw `Unexpected ${_}`
		case	'»'			: throw `Unexpected ${_}`
		case	'['			: return new List		( ReadList( r, ']' ) )
		case	'{'			: return new Procedure	( ReadList( r, '}' ) )
		case	'('			: return new Sentence	( ReadList( r, ')' ) )
		case	'«'			: return new Parallel	( ReadList( r, '»' ) )
		case	'"'			: return ReadStr( r )
		case	'!'			: return new Unary(
								( c, _ ) => _.Eval( c )
							,	_
							)
		case	'\''		: return new Prefix(
								( c, _ ) => _
							,	Read( r )
							,	_
							)
		case	'¡'			: return new Prefix(
								( c, _ ) => { throw _ }
							,	Read( r )
							,	_
							)
		case	'~'			: return new Prefix(
								( c, _ ) => {
									if ( _ instanceof Numeric ) return new Numeric( ~_._ )
									throw `~${_.string}:`
								}
							,	Read( r )
							,	_
							)
		case	'¬'			: return new Prefix(
								( c, _ ) => _IsT( _ ) ? Nil : T
							,	Read( r )
							,	_
							)
	//	case	'`'			:
	//	case	'¤'			:
	//	case	'/'			: //	Reserved for rational
		case	'@'			: return Stack
		case	'·'			: return CurretDict
		case	'#'			: return Count
		case	'*'			: return Cdr
		case	'$'			: return Last
		case	'.'			: return Print
		case	'¦'			: return Log
		case	'∈'			: return MemberOf
		case	'∋'			: return Contains
		case	'?'			: return IfElse
		case	'¿'			: return If
		case	','			: return Cons
		case	':'			: return Apply
		case	'&'			: switch ( r.Peek() ) {
								case _		: r.Read();	return LogicalAnd
								default		: 			return BitAnd
							}
		case	'|'			: switch ( r.Peek() ) {
								case _		: r.Read();	return LogicalOr
								default		:			return BitOr
							}
		case	'^'			: switch ( r.Peek() ) {
								case _		: r.Read();	return LogicalXOr
								default		:			return BitXOr
							}
		case	'×'			: return Mul
		case	'÷'			: return Div
		case	'%'			: return Modulo
		case	'+'			:
			{	let w = r.Peek(); if ( !w ) return Plus
				return w.match( /\d/ ) ? ReadNumber( r, false ) : Plus
			}
		case	'-'			:
			{	let w = r.Peek(); if ( !w ) return Minus
				return w.match( /\d/ ) ? ReadNumber( r, true ) : Minus
			}
		case	'='			:
			{	let w = r.Peek(); if ( !w ) return Define
				switch ( w ) {
				case '='		: r.Read(); return EQ
				case '<'		: r.Read(); return LE
				case '>'		: r.Read(); return GE
				}
				return Define
			}
		case	'<'			:
			{	let w = r.Peek(); if ( !w ) return LT
				switch ( w ) {
				case '>'		: r.Read(); return NE
				case '='		: r.Read(); return LE
				}
				return LT
			}
		case	'>'			:
			{	let w = r.Peek(); if ( !w ) return GT
				switch ( w ) {
				case '<'		: r.Read(); return NE
				case '='		: r.Read(); return GE
				}
				return GT
			}
		default				:
			r.Unread()
			return ReadName( r )
		}
	}
	return null
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const r = new StringReader( fs.readFileSync( '/dev/stdin', 'utf8' ) )
const c = new Context()
while ( true ) {
	const _ = Read( r )
	if ( !_ ) break
	try {
		console.log( _.Eval( c ).string )
	} catch ( e ) {
		console.error( 'EXCEPTION:', e.string )
	}
}

