const
RESERVED_CHARACTERS = [
	'\n'	//	0A
,	'\r'	//	0D
,	' '		//	20
,	'!'		//	21	Eval
,	'"'		//	22	Literal
,	'#'		//	23	Num
,	'$'		//	24	Last
,	'%'		//	25	Reminder
,	'&'		//	26	AND
,	'\''	//	27	Quote
,	'('		//	28	Open sentence
,	')'		//	29	Close sentence
,	'*'		//	2A	CDR
,	'+'		//	2B	Plus
,	','		//	2C	CONS
,	'-'		//	2D	Minus
,	'.'		//	2E	標準出力に表示します。引数をそのまま返します。
,	'/'		//	2F													RESERVED for rational number
,	':'		//	3A	Apply
,	';'		//	3B													RESERVED for END OF SENTENCE
,	'<'		//	3C	COMPARATOR
,	'='		//	3D	COMPARATOR
,	'>'		//	3E	COMPARATOR
,	'?'		//	3F	l が Nil でなければ r[ 0 ] を、そうでなければ r[ 1 ] を評価する
,	'@'		//	40	Stack top, Stack
,	'['		//	5B	Open list
,	']'		//	5D	Close list
,	'^'		//	5E	排他的論理和
,	'`'		//	60													RESERVED
,	'{'		//	7B	Open procedure
,	'|'		//	7C	OR
,	'}'		//	7D	Close procedure
,	'~'		//	7E	反転
,	'¡'		//	A1	Throw
,	'¤'		//	A4	Dict
,	'¦'		//	A6	標準エラー出力に改行をつけて表示します。引数をそのまま返します。
,	'§'		//	A7	右辺に2要素のリストをとり、0番目の要素である辞書をコンテクスト辞書チェーンの最初に付け加え、1番目の要素を評価したもの
,	'«'		//	AB	Open parallel
,	'¬'		//	AC	論理的に反転したもの
,	'±'		//	B1													RESERVED
,	'¶'		//	B6													RESERVED
,	'·'		//	B7	Stringify
,	'»'		//	BB	Close parallel
,	'¿'		//	BF	l が Nil でなければ r を評価、そうでなければ Nil
,	'×'		//	D7	MUL
,	'÷'		//	F7	DIV
,	'∈'		//	2208	要素
,	'∋'		//	220B	要素として含む
,	'⊂'		//														RESERVED
,	'⊃'		//														RESERVED
,	'∩'		//														RESERVED
,	'∪'		//														RESERVED
,	'∅'		//														RESERVED
]

const
stack = []

class
Context {
	constructor( dict, next = null ) {
		this.dict = dict
		this.next = next
	}
}

class
SliP {
	constructor( _ ) { this._ = _ }
	string()	{ return `${this._}` }
	Eval( c )	{ return this }
}

class
Numeric extends SliP {
}

class
Literal extends SliP {
	string()	{ return `"${this._}"` }
}

class
Dict extends SliP {		//	_ : Entries, key type is Name -> Object
	constructor( _ ) { super( Object.fromEntries( _.map( ( [ k, v ] ) => [ k.string(), v ] ) ) ) }
	string()	{
		let initChar = '{'
		return Object.entries( this._ ).reduce(
			( $, [ key, value ] ) => (
				$ = $ + initChar + '\t' + key + '\t: ' + value.string() + '\n'
			,	initChar = ','
			,	$
			)
		,	''
		) + '}'
	}
}

class
Name extends SliP {
	string()	{ return this._ }
	Eval( c )	{
		while ( c ) {
			const $ = c.dict[ this._ ]
			if ( $ !== void 0 ) return $
			c = c.next
		}
		throw `Undefined:${this._}`
	}
}

class
_Func extends SliP {
	constructor( _, label ) {
		super( _ )
		this.label = label
	}
}

class
Primitive extends _Func {
	string() { return this.label }
	Eval( c ) { return this._( c ) }
}

class
Prefix extends _Func {
	constructor( _, label, target ) {
		super( _, label )
		this.target = target
	}
	string()	{ return this.label }
	Eval( c )	{ return this._( c, this.target ) }
}

class
Unary extends _Func {
	string()	{ return this.label }
}

class
Infix extends _Func {
	constructor( _, label, priority ) {
		super( _, label )
		this.priority = priority
	}
	string()	{ return this.label }
}

class
_List extends SliP {
	string()	{ return this._.map( _ => _.string() ).join( ' ' ) }
}

class
List extends _List {
	string()	{ return `[ ${super.string()} ]` }
}

class
Parallel extends _List {
	string()	{ return `« ${super.string()} »` }
	Eval( c )	{ return new List( this._.map( _ => _.Eval( c ) ) ) }
}

class
Procedure extends _List {
	string()	{ return `{ ${super.string()} }` }
	Eval( c )	{
		c = new Context( {}, c )
		return new List( this._.map( _ => _.Eval( c ) ) )
	}
}

const
_EvalSentence = ( c, _ ) => {
//console.log( 'EvalSentence', _ )
	switch ( _.length ) {
	case  0:
		throw [ `No left or right operand for infix operator: ${ infix.label } : ${ new _List( _ ).string() }` ]
		break
	case  1:
		return _[ 0 ].Eval( c )
	default:
		const infixEntries = _.map( ( v, k ) => [ k, v ] ).filter( ( [ k, v ] ) => v instanceof Infix )
		if ( infixEntries.length ) {
			let $ = infixEntries[ 0 ]
			infixEntries.slice( 1 ).forEach(
				_ => _[ 1 ].priority >= $[ 1 ].priority && ( $ = _ )
			)
			const [ index, infix ] = $
			try {
				return infix._(
					c
				,	_EvalSentence( c, _.slice( 0, index ) )
				,	_EvalSentence( c, _.slice( index + 1 ) )
				)
			} catch ( ex ) {
//console.error( '_EvalSentence:ex', ex )
				let $ = `Syntax error: ${ new _List( _ ).string() }`
				if ( Array.isArray( ex ) ) {
					ex.push( $ )
					throw ex
				} else {
					throw [ $, ex ]
				}
			}
		} else {
			throw [ `Syntax error: No infix operators: ${ new _List( _ ).string() }` ]
		}
		break
	}
}
class
Sentence extends _List {
	string() { return `( ${super.string()} )` }
	Eval( c ) {
		return _EvalSentence( c, this._ )
	}
}

////////////////////////////////////////////////////////////////
//	Builtins
////////////////////////////////////////////////////////////////

const
Nil = new List( [] )

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
Builtins = [
	new Primitive(
		c => {
			if ( stack.length ) return stack[ 0 ]
			throw 'Stack underflow'
		}
	,	'@'		
	)
,	new Primitive(
		c => new List( stack )
	,	'@@'	
	)
,	new Primitive(
		c => new Dict( Object.entries( c.dict ).map( ( [ k, v ] ) => [ new Name( k ), v ] ) )
	,	'¤'		
	)
,	new Prefix(
		( c, _ ) => _._[ 1 ].Eval( new Context( _._[ 0 ].Eval( c )._, c ) )
	,	'§'
	)
,	new Prefix(
		( c, _ ) => _
	,	'\''	
	)
,	new Prefix(
		( c, _ ) => { throw _.string() }
	,	'¡'		
	)
,	new Prefix(
		( c, _ ) => {
			const v = _.Eval( c )
			if ( v instanceof Numeric ) return new Numeric( ~v._ )
			throw `~${_.string()}`
		}
	,	'~'		
	)
,	new Prefix(
		( c, _ ) => {
			const v = _.Eval( c )
			return _IsT( v ) ? Nil : T
		}
	,	'¬'		
	)
,	new Unary(
		( c, _ ) => _.Eval( c )
	,	'!'		
	)
,	new Unary(
		( c, _ ) => {
			if ( _ instanceof _List		) return new Numeric( _._.length )
			if ( _ instanceof Literal	) return new Numeric( _._.length )
			throw `#${_.string()}`
		}
	,	'#'		
	)
,	new Unary(
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
			throw `*${_.string()}`
		}
	,	'*'		
	)
,	new Unary(
		( c, _ ) => {
			const length = _._.length
			if ( _ instanceof _List && length ) return _._[ length - 1 ]
			throw `$${_.string()}`
		}
	,	'$'		
	)
,	new Unary(
		( c, _ ) => _ instanceof Literal ? _ : new Literal( _.string() )
	,	'·'		
	)
,	new Unary(
		( c, _ ) => {
			process.stdout.write( _.string() )
			return _
		}
	,	'.'		
	)
,	new Unary(
		( c, _ ) => {
			process.stderr.write( _.string() + '\n' )
			return _
		}
	,	'¦'		
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
,	new Infix(
		( c, l, r ) => {
			if ( l instanceof Name ) {
				c.dict[ l._ ] = r
				return r
			}
			throw `${l.string()}=${r.string()}`
		}
	,	'='		
	,	90
	)
,	new Infix(
		( c, l, r ) => {
			if ( r instanceof List && r._.length == 2 ) return r._[ _IsT( l ) ? 0 : 1 ].Eval( c )
			throw `${l.string()}?${r.string()}`
		}
	,	'?'		
	,	80
	)
,	new Infix(
		( c, l, r ) => _IsT( l ) ? r.Eval( c ) : Nil
	,	'¿'		
	,	80
	)
,	new Infix(
		( c, l, r ) => _IsT( l ) & _IsT( r ) ? T : Nil
	,	'&&'	
	,	70
	)
,	new Infix(
		( c, l, r ) => _IsT( l ) | _IsT( r ) ? T : Nil
	,	'||'	
	,	70
	)
,	new Infix(
		( c, l, r ) => _IsT( l ) ^ _IsT( r ) ? T : Nil
	,	'^^'	
	,	70
	)
,	new Infix(
		( c, l, r ) => {
			if ( r instanceof _List ) return r._.filter( _ => _EQ( _, l ) ).length ? T : Nil
			throw `${l.string()}∈${r.string}()`
		}
	,	'∈'		
	,	60
	)
,	new Infix(
		( c, l, r ) => {
			if ( l instanceof _List ) return l._.filter( _ => _EQ( _, r ) ).length ? T : Nil
			throw `${l.string()}∋${r.string()}`
		}
	,	'∋'		
	,	60
	)
,	new Infix(
		( c, l, r ) => _EQ( l, r ) ? T : Nil
	,	'=='	
	,	60
	)
,	new Infix(
		( c, l, r ) => _EQ( l, r ) ? Nil : T
	,	'<>'	
	,	60
	)
,	new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric && r instanceof Numeric ) return l._ < r._ ? T : Nil
			throw `${l.string()}<${r.string()}`
		}
	,	'<'		
	,	60
	)
,	new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric && r instanceof Numeric ) return l._ > r._ ? T : Nil
			throw `${l.string()}>${r.string()}`
		}
	,	'>'		
	,	60
	)
,	new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric && r instanceof Numeric ) return l._ <= r._ ? T : Nil
			throw `${l.string()}<=${r.string()}`
		}
	,	'<='	
	,	60
	)
,	new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric && r instanceof Numeric ) return l._ >= r._ ? T : Nil
			throw `${l.string()}>=${r.string()}`
		}
	,	'>='	
	,	60
	)
,	new Infix(
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
			throw `${l.string()},${r.string()}`
		}
	,	','		
	,	50
	)
,	new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric && r instanceof Numeric ) return new Numeric( l._ & r._ )
			throw `${l.string()}&${r.string()}`
		}
	,	'&'		
	,	40
	)
,	new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric && r instanceof Numeric ) return new Numeric( l._ | r._ )
			throw `${l.string()}|${r.string()}`
		}
	,	'|'		
	,	40
	)
,	new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric && r instanceof Numeric ) return new Numeric( l._ ^ r._ )
			throw `${l.string()}^${r.string()}`
		}
	,	'^'		
	,	40
	)
,	new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric	&& r instanceof Numeric		) return new Numeric	( l._ + r._ )
			if ( l instanceof Literal	&& r instanceof Literal		) return new Literal	( l._ + r._ )
			if ( l instanceof List		&& r instanceof List		) return new List		( [ ...l._, ...r._ ] )
			if ( l instanceof Parallel	&& r instanceof Parallel	) return new Parallel	( [ ...l._, ...r._ ] )
			if ( l instanceof Procedure	&& r instanceof Procedure	) return new Procedure	( [ ...l._, ...r._ ] )
			if ( l instanceof Sentence	&& r instanceof Sentence	) return new Sentence	( [ ...l._, ...r._ ] )
			throw `${l.string()}+${r.string()}`
		}
	,	'+'		
	,	30
	)
,	new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric && r instanceof Numeric ) return new Numeric( l._ - r._ )
			throw `${l.string()}-${r.string()}`
		}
	,	'-'		
	,	30
	)
,	new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric && r instanceof Numeric ) return new Numeric( l._ * r._ )
			throw `${l.string()}*${r.string()}`
		}
	,	'×'		
	,	20
	)
,	new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric && r instanceof Numeric ) return new Numeric( l._ / r._ )
			throw `${l.string()}÷${r.string()}`
		}
	,	'÷'		
	,	20
	)
,	new Infix(
		( c, l, r ) => {
			if ( l instanceof Numeric && r instanceof Numeric ) return new Numeric( l._ % r._ )
			throw `${l.string()}%${r.string()}`
		}
	,	'%'		
	,	20
	)
,	new Infix(
		( c, l, r ) => {
//console.log( c, l, r )
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
	,	':'		
	,	10
	)
	//	Reserved for rational number
,	new Primitive(
		c => T
	,	'/'		
	)
]
const
BuiltinDict = Builtins.reduce(
	( $, _ ) => ( $[ _.label ] = _, $ )
,	{}
)

////////////////////////////////////////////////////////////////
//	Reader
////////////////////////////////////////////////////////////////

const
ReadNumber = ( r, neg ) => {
	let	v = ''
	let	dotRead = false
	while ( r.Avail() ) {
		const _ = r.Peek()
		if ( ! _.match( /\d/ ) && _ != '.' ) break
		if ( _ === '.' ) {
			if ( dotRead ) break
			else dotRead = true
		}
		r.Forward()
		v += _
	}
	return new Numeric( Number( neg ? -v : v ) )
}

const
ReadName = r => {
	let	v = ''
	while ( r.Avail() ) {
		const _ = r.Peek()
		if ( RESERVED_CHARACTERS.includes( _ ) ) break
		r.Forward()
		v += _
	}
	return new Name( v )
}

const
ReadLiteral = ( r, terminator ) => {
	let	v = ''
	let	escaped = false
	while ( r.Avail() ) {
		const _ = r.Read()
		if ( escaped ) {
			escaped = false
			switch ( _ ) {
			case '0'		: v += '\0';		break
			case 't'		: v += '\t';		break
			case 'n'		: v += '\n';		break
			case 'r'		: v += '\r';		break
			default			: v += _;			break
			}
		} else {
			switch ( _ ) {
			case terminator	: return			new Literal( v )
			case '\\'		: escaped = true;	break
			default			: v += _;			break
			}
		}
	}
	throw `Syntax error: Unterminated string: ${v}`
}

let
level = 0

const
ReadList = ( r, terminator ) => {
	const	v = []
	while ( true ) {
		const _ = Read( r, terminator )
		if ( _ === void 0 ) return v	//	Read terminator
		if ( _ instanceof SliP ) v.push( _ )
		else break
	}
	throw 'Open list: ' + v
}

export const
Read = ( r, terminator ) => {
	while ( r.Avail() ) {
		let _ = r.Peek()
		if ( _.match( /\d/ ) ) return ReadNumber( r, false )
		r.Forward()
		if ( _.match( /\s/ ) ) continue
		switch ( _ ) {
		case	terminator	: return void 0	//	Must be before close brace
		case	']'			:
		case	'}'			:
		case	')'			:
		case	'»'			:
			throw `Unexpected brace closer: ${_}`
		case	'['			: return new List		( ReadList( r, ']' ) )
		case	'('			: return new Sentence	( ReadList( r, ')' ) )
		case	'{'			: return new Procedure	( ReadList( r, '}' ) )
		case	'«'			: return new Parallel	( ReadList( r, '»' ) )
		case	'"'			: return ReadLiteral( r, _ )
		case	'`'			: return ReadLiteral( r, _ )
	//	case	'+'			: return ( r.Peek().match( /\d/ ) ) ? ReadNumber( r, false ) : BuiltinDict[ _ ]
	//	case	'-'			: return ( r.Peek().match( /\d/ ) ) ? ReadNumber( r, true ) : BuiltinDict[ _ ]
		default				:
			if ( RESERVED_CHARACTERS.includes( _ ) ) {
				let v = null
				const c2s = Object.keys( BuiltinDict ).filter( key => key[ 0 ] == _ && key.length > 1 ).map( _ => _[ 1 ] )
				if ( c2s.length ) {
					const c2 = r.Read()
					if ( c2s.includes( c2 ) ) {
						v = BuiltinDict[ _ + c2 ]
					} else {
						r.Backward()
						v = BuiltinDict[ _ ]
					}
				} else {
					v = BuiltinDict[ _ ]
				}
				if ( v instanceof Prefix ) v = new Prefix( v._, v.label, Read( r ) )
//console.log( 'Read:', v )
				return v
			} else {
				r.Backward()
				return ReadName( r )
			}
		}
	}
	return null
}

/*
const _SliP		= new SliP( 'T' )
const _Numeric	= new Numeric( 3.14 )
const _Literal	= new Literal( "theLiteral" )
const _Name		= new Name( "theName" )
const _Name2	= new Name( "theName2" )
const _Dict		= new Dict(
	[	[ _Name		, _Literal ]
	,	[ _Name2	, _SliP ]
	]
)
const _Prefix	= new Prefix(
	( c, target ) => new Literal( target.Eval( c )._.toUpperCase() )
,	'thePrefix'
)
_Prefix.target = _Name
const _Unary	= new Unary( _ => _SliP, 'theUnary' )
const _Infix	= new Unary( ( $, _ ) => _SliP, 'theInfix' )

const
Dump = _ => console.log( _.string() )
	Dump( _SliP )
	Dump( _Numeric )
	Dump( _Literal )
	Dump( _Name )
	Dump( _Dict )
	Dump( _Name.Eval( new Context( _Dict ) ) )
	Dump( _Name2.Eval( new Context( _Dict ) ) )
	Dump( _Prefix )
	Dump( _Prefix.Eval( new Context( _Dict ) ) )
	Dump( _Unary )
	Dump( _Unary._( _Name ) )
	Dump( _Infix )
	Dump( _Infix._( _Name, _Name2 ) )
//	6 / 2 * 3
*/

////////////////////////////////////////////////////////////////
//	REPL
////////////////////////////////////////////////////////////////

export class
StringReader {
	constructor( _ ) {
		this._ = _
		this.i = 0
	}
	Avail()		{ return this.i < this._.length }
	Read()		{ return this._[ this.i++ ] }
	Peek()		{ return this._[ this.i ] }
	Forward()	{ this.i++ }
	Backward()	{ --this.i }
}

export const
NewContext = () => new Context(
	{}
,	new Context(
		[	new Unary(
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
			,	'dict'
			)
		,	new Unary(
				( c, _ ) => new Numeric( parseInt( _._[ 0 ]._, _._[ 1 ]._ ) )
			,	'int'
			)
		,	new Unary(
				( c, _ ) => new Literal( _._[ 0 ]._.toString( _._[ 1 ]._ ) )
			,	'string'
			)
		,	new Unary(
				( c, _ ) => new Numeric( Math.cos( _._ ) )
			,	'cos'
			)
		].reduce(
			( $, _ ) => ( $[ _.label ] = _, $ )
		,	{}
		)
	)
)

export const
Sugared = _ => new Sentence( ReadList( new StringReader( _ + ';' ), ';' ) )
