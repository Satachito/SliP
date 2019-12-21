const fs = require( 'fs' )

stack = []

class
Context {
	constructor( _, next ) {
		this.dict = _
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
Dict extends SliP {
	constructor( _ ) { super( _ ) }
	get
	string() {
		const v = Object.keys( this._ ).reduce(
			( dict, key ) => {
				const value = this._[ key ]
				dict[ key ] = value instanceof Literal ? value._ : value.string
				return dict
			}
		,	{}
		)
		return `${ JSON.stringify( v ) }`
	}
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
Primitive extends SliP {
	constructor( _ ) {
		super( _ )
		this.label = ''
	}
	get
	string() { return this.label }

	Eval( c ) { return this._( c ) }
}

class
Prefix extends SliP {
	constructor( _, target, label ) {
		super( _ )
		this.target = target
		this.label = label
	}
	get
	string() { return this.label + this.target.string }

	Eval( c ) { return this._( c, this.target ) }
}
class
PrefixFactory {
	constructor( _ ) {
		this._ = _
		this.label = ''
	}
	Create( _ ) {
		return new Prefix( this._, _, this.label )
	}
}

class
Unary extends SliP {
	constructor( _ ) {
		super( _ )
		this.label = ''
	}
	get
	string() { return this.label }
}

class
Infix extends SliP {
	constructor( _, priority ) {
		super( _ )
		this.priority = priority
		this.label = ''
	}
	get
	string() { return this.label }
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

	Eval( c ) { return new List( this._.map( _ => _.Eval( c ) ) ) }
}

class
Procedure extends _List {
	constructor( _ ) { super( _ ) }
	get
	string() { return `{ ${super.string} }` }

	Eval( c ) {
		c = new Context( {}, c )
		return new List( this._.map( _ => _.Eval( c ) ) )
	}
}

const
_EvalSentence = ( c, _ ) => {
	switch ( _.length ) {
	case  0:
		return new List( _ )
	case  1:
		return _[ 0 ].Eval( c )
	case  2:
		break
	default:
		const wInfixIndices = Object.keys( _ ).filter( i => _[ i ] instanceof Infix ).map( _ => Number( _ ) )
		if ( wInfixIndices.length ) {
			const i = wInfixIndices.reduce(
				( a, c ) => _[ c ].priority >= _[ a ].priority ? c : a
			,	wInfixIndices[ 0 ]
			)
			if ( ( 0 < i ) && ( i < _.length - 1 ) ) {
				return _[ i ]._( c, _EvalSentence( c, _.slice( 0, i ) ), _EvalSentence( c, _.slice( i + 1 ) ) )
			}
		}
		break
	}
	throw `Syntax error: ${new _List( _ ).string}`
}

class
Sentence extends _List {
	constructor( _ ) { super( _ ) }
	get
	string() { return `( ${super.string} )` }

	Eval( c ) {
		return _EvalSentence( c, this._ )
	}
}

////////////////////////////////////////////////////////////////
//	Builtins
////////////////////////////////////////////////////////////////

const
T = new SliP()

const
Nil = new _List( [] )

const
_IsT = _ => !( _ instanceof _List ) || _._.length

const
_IsNil = _ => _ instanceof _List && !_._.length

const
_EQ = ( p, q ) => {
	if ( p instanceof Numeric	&& q instanceof Numeric	) return p._ == q._
	if ( p instanceof Literal	&& q instanceof Literal	) return p._ == q._
	if ( p instanceof Name		&& q instanceof Name	) return p._ == q._
	if ( p instanceof _List		&& q instanceof _List	) {
		if ( p.constructor.name != q.constructor.name ) return false
		if ( p._.length != q._.length ) return false
		for ( let i = 0; i < p._.length; i++ ) if ( !_EQ( p._[ i ], q._[ i ] ) ) return false
		return true
	}
	return p === q
}

const
Functions = {
	'@'		: new Primitive(
		c => {
			if ( stack.length ) return stack[ 0 ]
			throw new Error( 'Stack underflow' )
		}
	)
,	'@@'	: new Primitive(
		c => new _List( stack )
	)
,	'¤'		: new Primitive(
		c => new Dict( c.dict )
	)
,	'`'		: new PrefixFactory(
		( c, _ ) => _._[ 1 ].Eval( new Context( _._[ 0 ].Eval( c )._, c ) )
	)
,	'\''	: new PrefixFactory(
		( c, _ ) => _
	)
,	'¡'		: new PrefixFactory(
		( c, _ ) => { throw _ }
	)
,	'~'		: new PrefixFactory(
		( c, _ ) => {
			const v = _.Eval( c )
			if ( v instanceof Numeric ) return new Numeric( ~v._ )
			throw `~${_.string}:`
		}
	)
,	'¬'		: new PrefixFactory(
		( c, _ ) => {
			const v = _.Eval( c )
			return _IsT( v ) ? Nil : T
		}
	)
,	'!'		: new Unary(
		( c, _ ) => _.Eval( c )
	)
,	'#'		: new Unary(
		( c, _ ) => {
			if ( _ instanceof _List		) return new Numeric( _._.length )
			if ( _ instanceof Literal	) return new Numeric( _._.length )
			throw `#${_.string}`
		}
	)
,	'*'		: new Unary(
		( c, _ ) => {
			if ( _ instanceof _List && _._.length ) {
				switch ( _.constructor.name ) {
				case '_List'	: return new _List		( _._.slice( 1 ) )
				case 'List'		: return new List		( _._.slice( 1 ) )
				case 'Parallel'	: return new Parallel	( _._.slice( 1 ) )
				case 'Sentence'	: return new Sentence	( _._.slice( 1 ) )
				case 'Procedure': return new Procedure	( _._.slice( 1 ) )
				}
			}
			throw `*${_.string}`
		}
	)
,	'$'		: new Unary(
		( c, _ ) => {
			const length = _._.length
			if ( _ instanceof _List && length ) return _._[ length - 1 ]
			throw `$${_.string}`
		}
	)
,	'·'		: new Unary(
		( c, _ ) => _ instanceof Literal ? _ : new Literal( _.string )
	)
,	'.'		: new Unary(
		( c, _ ) => {
			process.stdout.write( _.string )
			return _
		}
	)
,	'¦'		: new Unary(
		( c, _ ) => {
			process.stderr.write( _.string + '\n' )
			return _
		}
	)
/*
	:	10	Apply		2 x [ 3, 4 ]:1			2 x 4
	×	20	Multi/Div	2 + 3 x 4				2 + 12
	+	30	Plus/Minus	2 | 3 + 4				2 | 7
	|	40	Binary		2 , 3 | 4				2 < 7
	,	50	Cons		[ 2 3 ] == 2 , [ 3 ]	[ 2 3 ] == [ 2, 3 ]
	<	60	Compare		1 || 2 < 3				1 || T
	∈	60	Member		1 || 2 ∈ [ 1, 2, 3 ]	1 || T
	||	70	Logical		'a = [ 2 ] || T			a = T
	?	80	IfElse		
	=	90	Define		
*/
,	'='		: new Infix(
		( c, l, r ) => {
			if ( l instanceof Name ) {
				c.dict[ l._ ] = r
				return r
			}
			throw `${l.string}=${r.string}`
		}
	,	90
	)
,	'?'		: new Infix(
		( c, l, r ) => {
			if ( r instanceof List && r._.length == 2 ) return r._[ _IsT( l ) ? 0 : 1 ].Eval( c )
			throw `${l.string}?${r.string}`
		}
	,	80
	)
,	'¿'		: new Infix(
		( c, l, r ) => _IsT( l ) ? r.Eval( c ) : Nil
	,	80
	)
,	'&&'	: new Infix(
		( c, l, r ) => {
			return _IsT( l ) & _IsT( r ) ? T : Nil
		}
	,	70
	)
,	'||'	: new Infix(
		( c, l, r ) => _IsT( l ) | _IsT( r ) ? T : Nil
	,	70
	)
,	'^^'	: new Infix(
		( c, l, r ) => _IsT( l ) ^ _IsT( r ) ? T : Nil
	,	70
	)
,	'∈'		: new Infix(
		( c, l, r ) => {
			if ( r instanceof _List ) return r._.filter( _ => _EQ( _, l ) ).length ? T : Nil
			throw `${l.string}∈${r.string}`
		}
	,	60
	)
,	'∋'		: new Infix(
		( c, l, r ) => {
			if ( l instanceof _List ) return l._.filter( _ => _EQ( _, r ) ).length ? T : Nil
			throw `${l.string}∋${r.string}`
		}
	,	60
	)
,	'=='	: new Infix(
		( c, l, r ) => _EQ( l, r ) ? T : Nil
	,	60
	)
,	'<>'	: new Infix(
		( c, l, r ) => _EQ( l, r ) ? Nil : T
	,	60
	)
,	'<'		: new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric && r instanceof Numeric ) return l._ < r._ ? T : Nil
			throw `${l.string}<${r.string}`
		}
	,	60
	)
,	'>'		: new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric && r instanceof Numeric ) return l._ > r._ ? T : Nil
			throw `${l.string}>${r.string}`
		}
	,	60
	)
,	'<='	: new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric && r instanceof Numeric ) return l._ <= r._ ? T : Nil
			throw `${l.string}<=${r.string}`
		}
	,	60
	)
,	'>='	: new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric && r instanceof Numeric ) return l._ >= r._ ? T : Nil
			throw `${l.string}>=${r.string}`
		}
	,	60
	)
,	','		: new Infix(
		( c, l, r ) => {
			if ( r instanceof _List ) {
				switch ( r.constructor.name ) {
				case '_List'		: return new _List		( [ l, ...r._ ] )
				case 'List'			: return new List		( [ l, ...r._ ] )
				case 'Parallel'		: return new Parallel	( [ l, ...r._ ] )
				case 'Sentence'		: return new Sentence	( [ l, ...r._ ] )
				case 'Procedure'	: return new Procedure	( [ l, ...r._ ] )
				default				: throw 'internal error'
				}
			}
			throw `${l.string},${r.string}`
		}
	,	50
	)
,	'&'		: new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric && r instanceof Numeric ) return new Numeric( l._ & r._ )
			throw `${l.string}&${r.string}`
		}
	,	40
	)
,	'|'		: new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric && r instanceof Numeric ) return new Numeric( l._ | r._ )
			throw `${l.string}|${r.string}`
		}
	,	40
	)
,	'^'		: new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric && r instanceof Numeric ) return new Numeric( l._ ^ r._ )
			throw `${l.string}^${r.string}`
		}
	,	40
	)
,	'+'		: new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric	&& r instanceof Numeric		) return new Numeric	( l._ + r._ )
			if ( l instanceof Literal	&& r instanceof Literal		) return new Literal	( l._ + r._ )
			if ( l instanceof List		&& r instanceof List		) return new List		( [ ...l._, ...r._ ] )
			if ( l instanceof Parallel	&& r instanceof Parallel	) return new Parallel	( [ ...l._, ...r._ ] )
			if ( l instanceof Procedure	&& r instanceof Procedure	) return new Procedure	( [ ...l._, ...r._ ] )
			if ( l instanceof Sentence	&& r instanceof Sentence	) return new Sentence	( [ ...l._, ...r._ ] )
			throw `${l.string}+${r.string}`
		}
	,	30
	)
,	'-'		: new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric && r instanceof Numeric ) return new Numeric( l._ - r._ )
			throw `${l.string}-${r.string}`
		}
	,	30
	)
,	'×'		: new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric && r instanceof Numeric ) return new Numeric( l._ * r._ )
			throw `${l.string}*${r.string}`
		}
	,	20
	)
,	'÷'		: new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric && r instanceof Numeric ) return new Numeric( l._ / r._ )
			throw `${l.string}÷${r.string}`
		}
	,	20
	)
,	'%'		: new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric && r instanceof Numeric ) return new Numeric( l._ % r._ )
			throw `${l.string}%${r.string}`
		}
	,	20
	)
,	':'		: new Infix(
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
				{	stack.unshift( l )
					const v = r.Eval( c )
					stack.shift()
					return v
				}
			}
		}
	,	10
	)
,	'/'		: new Primitive( c => T )	//	Reserved for rational number
}

Object.keys( Functions ).forEach( key => Functions[ key ].label = key )
const
functionFirstChars = Object.keys( Functions ).map( _ => _[ 0 ] )

////////////////////////////////////////////////////////////////
//	Reader
////////////////////////////////////////////////////////////////

const
ReadNumber = ( r, neg ) => {
	let	v = ''
	while ( r.Avail() ) {
		const _ = r.Peek()
		if ( ! _.match( /\d/ ) && _ != '.' ) break
		r.Read()
		v += _
	}
	return new Numeric( Number( neg ? -v : v ) )
}

const
ReadName = r => {
	let	v = ''
	while ( r.Avail() ) {
		const _ = r.Peek()
		if ( _.match( /\s/ ) ) break
		if ( _.match( /[\]})»;]/ ) ) break
		if ( functionFirstChars.includes( _ ) ) break
		r.Read()
		v += _
	}
	return new Name( v )
}

const
ReadLiteral = r => {
	let	v = ''
	let	wEscaped = false
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
			case '"'	: return			new Literal( v )
			case '\\'	: wEscaped = true;	break
			default		: v += _;			break
			}
		}
	}
	throw `Syntax error: Unterminated string: ${v}`
}

const
ReadList = ( r, terminator ) => {
//	console.log( '>RL', terminator )
	const v = _ReadList( r, terminator )
//	console.log( '<RL', terminator, v )
	return v
}

const
_ReadList = ( r, terminator ) => {
	let	v = []
	while ( true ) {
		const _ = Read( r, terminator )
		if ( !_ ) {
			if ( v.length ) throw new Error( 'Open list' )
			return null
		}
		if ( Array.isArray( _ ) ) break
		v.push( _ )
	}
	return v
}

const
Read = ( r, terminator ) => {
	while ( r.Avail() ) {
		let _ = r.Read()
		if ( _.match( /\d/ ) )	{
			r.Unread()
			return ReadNumber( r, false )
		}
		if ( _.match( /\s/ ) )	continue
		switch ( _ ) {
		case	terminator	: return []	//	Must be before close brace
		case	']'			: throw `Unexpected ${_}`
		case	'}'			: throw `Unexpected ${_}`
		case	')'			: throw `Unexpected ${_}`
		case	'»'			: throw `Unexpected ${_}`
		case	'['			: return new List		( ReadList( r, ']' ) )
		case	'('			: return new Sentence	( ReadList( r, ')' ) )
		case	'{'			: return new Procedure	( ReadList( r, '}' ) )
		case	'«'			: return new Parallel	( ReadList( r, '»' ) )
		case	'"'			: return ReadLiteral( r )
		case	'+'			: return ( r.Peek().match( /\d/ ) ) ? ReadNumber( r, false ) : Functions[ _ ]
		case	'-'			: return ( r.Peek().match( /\d/ ) ) ? ReadNumber( r, true  ) : Functions[ _ ]
		default				:
			if ( functionFirstChars.includes( _ ) ) {
				let v = null
				const c2s = Object.keys( Functions ).filter( key => key[ 0 ] == _ && key.length > 1 ).map( _ => _[ 1 ] )
				if ( c2s.length ) {
					const c2 = r.Read()
					if ( c2s.includes( c2 ) ) {
						v = Functions[ _ + c2 ]
					} else {
						r.Unread()
						v = Functions[ _ ]
					}
				} else {
					v = Functions[ _ ]
				}
				if ( v instanceof PrefixFactory ) v = v.Create( Read( r ) )
				return v
			} else {
				r.Unread()
				return ReadName( r )
			}
		}
	}
	return null
}

////////////////////////////////////////////////////////////////
//	REPL
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

const c = new Context(
	{}
,	new Context(
		{	T
		,	dict	: new Unary(
				( c, _ ) => {
					const wJSON = JSON.parse( _._ )
					return new Dict(
						Object.keys( wJSON ).reduce(
							( dict, key ) => {
								dict[ key ] = Read( new StringReader( wJSON[ key ] ) )
								return dict
							}
						,	{}
						)
					)
				}
			)
		,	int		: new Unary(
				( c, _ ) => new Numeric( parseInt( _._[ 0 ]._, _._[ 1 ]._ ) )
			)
		,	string	: new Unary(
				( c, _ ) => new Literal( _._[ 0 ]._.toString( _._[ 1 ]._ ) )
			)
		,	cos		: new Unary(
				( c, _ ) => new Numeric( Math.cos( _._ ) )
			)
		}
	)
)

const r = new StringReader(
	fs.readFileSync( '/dev/stdin', 'utf8' ).split( '\n' ).map(
		_ => _.trim().startsWith( '//' )
		?	''
		:	_.endsWith( '=' )
			?	( _.substring( 0, _.length - 1 ) + ':¦;' )
			:	_ 
	).join( '\n' )
)
while ( true ) {
	try {
		const _ = ReadList( r, ';' )
		if ( !_ ) break
		new Sentence( _ ).Eval( c )
	} catch ( e ) {
		console.error( e )
	}
}

/*
//	Non sugar
const r = new StringReader( fs.readFileSync( '/dev/stdin', 'utf8' ) )
while ( true ) {
	try {
		const _ = Read( r )
		if ( !_ ) break
		console.log( _.Eval( c ).string )
	} catch ( e ) {
		console.error( 'EXCEPTION:', e )
	}
}

*/
