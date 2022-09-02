const
LogBack = _ => ( console.log( _ ), _ )

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
	string() { return `${this._}` }
}

const
Round = _ => {
	if ( _ === 1 ) return 1
	const
	roundPrecision = (
		() => {
			try {
				return Number( ROUND_PRECISION.value )
			} catch {
				return 15
			}
		}
	)()
	if ( _ < 1 ) {
		const coef	= Math.pow( 10, roundPrecision )
		return Math.round( _ * coef ) / coef
	//	const $		= Math.round( _ * coef ) / coef
	//	console.log( _, $ )
	//	return $
	} else {
		const coef = Math.pow( 10, roundPrecision - Math.ceil( Math.log10( _ ) ) )
		return Math.round( _ * coef ) / coef
	//	console.log(
	//		_
	//	,	Math.log10( _ )
	//	,	Math.ceil( Math.log10( _ ) )
	//	,	roundPrecision - Math.ceil( Math.log10( _ ) )
	//	,	Math.pow( 10, roundPrecision - Math.ceil( Math.log10( _ ) ) )
	//	,	coef
	//	,	Math.round( _ * coef )
	//	,	Math.round( _ * coef ) / coef
	//	)
	//	const $ = Math.round( _ * coef ) / coef
	//	console.log( _, $ )
	//	return $
	}
}

class
Numeric extends SliP {
	string() {
		switch ( this._ ) {
		case  Infinity	: return '∞'
		case -Infinity	: return '-∞'
		default			:
		//	return this._
			return this._ === 0
			?	0
			:	this._ < 0 ? - Round( - this._ ) : Round( this._ )
		}
	}
}

class
Literal extends SliP {
	constructor( _, hint ) {
		super( _ )
		this.hint = hint ? hint : '"'
	}
	string() { return this.hint + this._ + this.hint }
}

class
Dict extends SliP {
	string() {
		let	$ = '{'
		let initChar = ''
		Object.entries( this._ ).forEach(
			( [ k, v ] ) => (
				$ = $ + initChar + '\t' + k + '\t: ' + v.string() + '\n'
			,	initChar = ','
			)
		)
		return $ + '}'
	}
}

class
Name extends SliP {
	string() { return this._ }
}

class
_Func extends SliP {
	constructor( _, label ) {
		super( _ )
		this.label = label
	}
	string() { return this.label }
//	string() { return this.label + '/' + this.constructor.name }
}

class
Primitive extends _Func {
}

class
Prefix extends _Func {
}

class
Unary extends _Func {
}

class
Infix extends _Func {
	constructor( _, label, priority ) {
		super( _, label )
		this.priority = priority
	}
}

class
_List extends SliP {
	string() { return this._.map( _ => _.string() ).join( ' ' ) }
}

class
List extends _List {
	string() {
		const $ = `[ ${super.string()} ]`
		return this.matrix
		?	this.matrix > 0
			?	$ + `(${this.matrix},${this._.length/this.matrix})`
			:	$ + `(${this._.length/-this.matrix},${-this.matrix})`
		:	$
	}
}

class
Parallel extends _List {
	string() { return `« ${super.string()} »` }
}

class
Procedure extends _List {
	string() { return `{ ${super.string()} }` }
}

class
Sentence extends _List {
	string() { return `( ${super.string()} )` }
}

const
_EvalSentence = ( c, _ ) => {
	switch ( _.length ) {
	case  0:
		throw `Syntax error(No operand)`
	case  1:
		return Eval( c, _[ 0 ] )
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
				let $ = `Evaluating: ${ new _List( _ ).string() }`
				if ( Array.isArray( ex ) ) {
					ex.push( $ )
					throw ex
				} else {
					throw [ ex, $ ]
				}
			}
		} else {
			let index = _.length - 1
			_ = _.slice()
			while ( index-- ) {
				let $ = _[ index ]
				$ instanceof Prefix || ( $ = Eval( c, $ ) )
				$ instanceof Prefix && _.splice( index, 2, $._( c, _[ index + 1 ] ) )
			}
			if ( _.length === 1 ) return _[ 0 ]
			const
			$ = _.map( _ => Eval( c, _ ) )
			if ( $.every( _ => _ instanceof Numeric ) ) return new Numeric( $.reduce( ( $, _ ) => $ * _._, 1 ) )
			throw `Syntax error: ${ new _List( _ ).string() }`
		}
		break
	}
}

export const
Eval = ( c, _ ) => {
	switch ( _.constructor.name ) {
	case 'Name':
		while ( c ) {
			const $ = c.dict[ _._ ]
			if ( $ !== void 0 ) return $ instanceof Primitive ? $._( c ) : $
			c = c.next
		}
		throw `Undefined:${_._}`
	case 'Primitive':
		return _._( c )
	case 'Parallel':
		return new List( _._.map( _ => Eval( c, _ ) ) )
	case 'Procedure':
		{	c = new Context( {}, c )
			return new List( _._.map( _ => Eval( c, _ ) ) )
		}
	case 'Sentence':
		return _EvalSentence( c, _._ )
	default:
		return _
	}
}

////////////////////////////////////////////////////////////////
//	Builtins
////////////////////////////////////////////////////////////////

const
T = new SliP( 'T' )

const
Nil = new List( [] )

const
_IsT = _ => !( _ instanceof _List ) || _._.length

const
_IsNil = _ => _ instanceof _List && !_._.length

const
_EQ = ( p, q ) => {
	if ( p instanceof Numeric	&& q instanceof Numeric	) return p._ === q._
	if ( p instanceof Literal	&& q instanceof Literal	) return p._ === q._
	if ( p instanceof Name		&& q instanceof Name	) return p._ === q._
	if ( p instanceof _List		&& q instanceof _List	) {
		if ( p.constructor.name !== q.constructor.name ) return false
		if ( p._.length !== q._.length ) return false
		for ( let i = 0; i < p._.length; i++ ) if ( !_EQ( p._[ i ], q._[ i ] ) ) return false
		return true
	}
	return p === q
}

const
Plus = ( c, l, r ) => {
	if ( l instanceof Numeric	&& r instanceof Numeric		) return new Numeric	( l._ + r._ )
	if ( l instanceof Literal	&& r instanceof Literal		) return new Literal	( l._ + r._ )
	if ( l instanceof List		&& r instanceof List		) return new List		( [ ...l._, ...r._ ] )
	if ( l instanceof Parallel	&& r instanceof Parallel	) return new Parallel	( [ ...l._, ...r._ ] )
	if ( l instanceof Procedure	&& r instanceof Procedure	) return new Procedure	( [ ...l._, ...r._ ] )
	if ( l instanceof Sentence	&& r instanceof Sentence	) return new Sentence	( [ ...l._, ...r._ ] )
	throw `${l.string()}+${r.string()}`
}

const
Minus = ( c, l, r ) => {
	if ( l instanceof Numeric && r instanceof Numeric ) return new Numeric( l._ - r._ )
	throw `${l.string()}-${r.string()}`
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
		c => new List( stack.slice() )
	,	'@@'
	)
,	new Primitive(
		c => new Dict( c.dict )
	,	'¤'
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
			const v = Eval( c, _ )
			if ( v instanceof Numeric ) return new Numeric( ~v._ )
			throw `~${_.string()}`
		}
	,	'~'
	)
,	new Prefix(
		( c, _ ) => _IsT( Eval( c, _ ) ) ? Nil : T
	,	'¬'
	)
,	new Prefix(
		( c, _ ) => Eval( c, Eval( c, _ ) )
	,	'!'
	)
,	new Prefix(
		( c, _ ) => _ instanceof Literal ? _ : new Literal( _.string() )
	,	'¶'
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
		( c, _ ) => ( console.log( _.string() ), _ )
	,	'.'
	)
,	new Unary(
		( c, _ ) => ( console.error( _.string() ), _ )
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
		( c, l, r ) => Eval( new Context( l._, c ), r )
	,	'§'
	,	100
	)
,	new Infix(
		( c, l, r ) => {
			if ( l instanceof Name ) return c.dict[ l._ ] = r
			throw `${l.string()}=${r.string()}`
		}
	,	'='
	,	90
	)
,	new Infix(
		( c, l, r ) => {
			if ( r instanceof List && r._.length == 2 ) return Eval( c, r._[ _IsT( l ) ? 0 : 1 ] )
			throw `${l.string()}?${r.string()}`
		}
	,	'?'
	,	80
	)
,	new Infix(
		( c, l, r ) => _IsT( l ) ? Eval( c, r ) : Nil
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
			throw `${l.string()}∈${r.string()}()`
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
		Plus
	,	'+'
	,	30
	)
,	new Infix(
		Minus
	,	'-'
	,	30
	)
,	new Infix(
		( c, l, r ) => {
			if ( l instanceof List && r instanceof List ) {
				const [ numRowL, numColL ] = l.matrix
				?	l.matrix > 0
					?	[ l.matrix, l._.length / l.matrix ]
					:	[ l._.length / -l.matrix, -l.matrix ]
				:	[ 1, l._.length ]
				const [ numRowR, numColR ] = r.matrix
				?	r.matrix > 0
					?	[ r.matrix, r._.length / r.matrix ]
					:	[ r._.length / -r.matrix, -r.matrix ]
				:	[ r._.length, 1 ]
				if ( numColL !== numRowR ) throw `${l.string()}·${r.string()}`
				if ( numRowL === 1 && numColR === 1 ) {
					let $ = 0
					for ( let _ = 0; _ < numColL; _++ ) $ += l._[ _ ]._ * r._[ _ ]._
					return new Numeric( $ )
				} else {
					const $ = new Array( numRowL * numColR )
					for ( let row = 0; row < numRowL; row++ ) {
						const _$ = row * numColR
						for ( let col = 0; col < numColR; col++ ) {
							let _ = 0
							for ( let k = 0; k < numColL; k++ ) _ += l._[ row * numColL + k ]._ * r._[ col + k * numColR ]._
							$[ _$ + col ] = _
						}
					}
					const v = new List( $.map( _ => new Numeric( _ ) ) )
					v.matrix = numRowL
					return v
				}
			}
			throw `${l.string()}·${r.string()}`
		}
	,	'·'
	,	20
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
			switch ( r.constructor.name ) {
			case 'Numeric'	:
				{	if ( ! l instanceof _List ) throw `${l.string()}:${r.string()}`
					let v = l._[ r._ ]
					return v ? v : Nil
				}
			case 'Name'		:
			case 'Literal'	:
				{	if ( ! l instanceof Dict ) throw `${l.string()}:${r.string()}`
					let v = l._[ r._ ]
					return v ? v : Nil
				}
			case 'Unary'	:
				return r._( c, l )
			default:
				{	stack.unshift( l )
					const v = Eval( c, r )
					stack.shift()
					return v
				}
			}
		}
	,	':'
	,	10
	)
]

////////////////////////////////////////////////////////////////
//	Reader
////////////////////////////////////////////////////////////////

const
BuiltinLabels = Builtins.map( _ => _.label )
const
BuiltinLabel0s = BuiltinLabels.map( _ => _[ 0 ] )

const
SoloChars = [
	'Α', 'Β', 'Γ', 'Δ', 'Ε', 'Ζ', 'Η', 'Θ', 'Ι', 'Κ', 'Λ', 'Μ', 'Ν', 'Ξ', 'Ο', 'Π', 'Ρ', 'Σ', 'Τ', 'Υ', 'Φ', 'Χ', 'Ψ', 'Ω'
,	'α', 'β', 'γ', 'δ', 'ε', 'ζ', 'η', 'θ', 'ι', 'κ', 'λ', 'μ', 'ν', 'ξ', 'ο', 'π', 'ρ', 'ς', 'σ', 'τ', 'υ', 'φ', 'χ', 'ψ', 'ω'
,	'∞'
]
const
NAME_BREAKING_CHARACTERS = [
	...SoloChars
,	'!'		//	21	Eval
,	'"'		//	22	StringLiteral
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
,	'.'		//	2E	Console.log, returns argument
,	'/'		//	2F
,	':'		//	3A	Apply
,	';'		//	3B	Dummy sentence terminator for Sugared syntax
,	'<'		//	3C	COMPARATOR
,	'='		//	3D	COMPARATOR
,	'>'		//	3E	COMPARATOR
,	'?'		//	3F	if L is non-Nil evaluate R[ 0 ] otherwise evaluate R[ 1 ]
,	'@'		//	40	Stack top, Stack
,	'['		//	5B	Open list
,	']'		//	5D	Close list
,	'^'		//	5E	Exclusive logical OR
,	'`'		//	60	String Literal
,	'{'		//	7B	Open procedure
,	'|'		//	7C	OR
,	'}'		//	7D	Close procedure
,	'~'		//	7E	Bit flip
,	'¡'		//	A1	Throw
,	'¤'		//	A4	Dict
,	'¦'		//	A6	Console.error, returns argument
,	'§'		//	A7	Evaluate R in context of L
,	'«'		//	AB	Open parallel
,	'¬'		//	AC	NOT
,	'±'		//	B1
,	'µ'		//	B5
,	'¶'		//	B6	Stringify
,	'·'		//	B7	Dot
,	'»'		//	BB	Close parallel
,	'¿'		//	BF	If L is non-NIL evaluate R otherwise Nil
,	'×'		//	D7	MUL
,	'÷'		//	F7	DIV
,	'∈'		//	2208	Member
,	'∋'		//	220B	Includes
,	'⊂'		//
,	'⊃'		//
,	'∩'		//
,	'∪'		//
,	'∅'		//
]

const
ReadNumber = ( r, _ ) => {
	let	v = _
	let	readDot = false
	while ( r.Avail() ) {
		const _ = r.Peek()
		if ( !_.match( /[\d.]/ ) ) break
		r.Forward()
		if ( _ === '.' ) {
			if ( readDot ) break
			else readDot = true
		}
		v += _
	}
	return new Numeric( Number( v ) )
}

const
ReadName = ( r, _ ) => {

	if ( SoloChars.includes( _ ) ) return new Name( _ )

	let	escaped = _ === '\\'

	let	v = escaped ? '' : _

	while ( r.Avail() ) {

		const _ = r.Peek()

		if ( escaped ) {
			r.Forward()
			escaped = false
			switch ( _ ) {
			case '0'		: v += '\0'			; break
			case 'f'		: v += '\f'			; break
			case 'n'		: v += '\n'			; break
			case 'r'		: v += '\r'			; break
			case 't'		: v += '\t'			; break
			case 'v'		: v += '\v'			; break
			default			: v += _			; break
			}
		} else {
			if ( NAME_BREAKING_CHARACTERS.includes( _ )	) break
			r.Forward()
			if ( _.match( /\s/ )						) break	//	[ \f\n\r\t\v\u00a0\u1680\u180e\u2000-\u200a\u2028\u2029\u202f\u205f\u3000\ufeff]
			switch ( _ ) {
			case '\\'		: escaped = true	; break
			default			: v += _			; break
			}
		}
	}
	return new Name( v )
}

const
ReadLiteral = ( r, terminator ) => {
	let	escaped = false
	let	v = ''
	while ( r.Avail() ) {
		const _ = r.Read()
		if ( escaped ) {
			escaped = false
			switch ( _ ) {
			case '0'		: v += '\0'			; break
			case 'f'		: v += '\f'			; break
			case 'n'		: v += '\n'			; break
			case 'r'		: v += '\r'			; break
			case 't'		: v += '\t'			; break
			case 'v'		: v += '\v'			; break
			default			: v += _			; break
			}
		} else {
			switch ( _ ) {
			case terminator	: return new Literal( v, terminator )
			case '\\'		: escaped = true	; break
			default			: v += _			; break
			}
		}
	}
	throw `Unterminated string: ${v}`
}

const
ReadList = ( r, terminator ) => {

	const $ = []
	while ( true ) {
		const slip = Read( r, terminator )
		if ( slip === void 0 )	break	//	Read terminator
		if ( slip === null )	throw $.reduce( ( $, _ ) => $ + ' ' + _.string(), 'Open list: ' )	//	 + $.map( _ => _.string() )
		$.push( slip )
	}

	const
	ModP = _ => $[ _ + 1 ] instanceof Numeric
	?	$.splice( _, 1 )
	:	$[ _ ] = new Prefix( ( c, _ ) => new Numeric( +Eval( c, _ )._ ), '+' )

	const
	ModM = _ => $[ _ + 1 ] instanceof Numeric
	?	$.splice( _, 2, new Numeric( -$[ _ + 1 ]._ ) )
	:	$[ _ ] = new Prefix( ( c, _ ) => new Numeric( -Eval( c, _ )._ ), '-' )
	
	if ( $.length > 1 ) {
		switch ( $[ 0 ]._ ) {
		case Plus	: ModP( 0 )	; break
		case Minus	: ModM( 0 )	; break
		}
	}

	{	let	i = $.length - 1
		while ( i-- > 1 ) {
			const _ = $[ i - 1 ]
			if ( _ instanceof Infix ) {
				switch ( $[ i ]._ ) {
				case Plus	: ModP( i )	; break
				case Minus	: ModM( i )	; break
				}
			}
		}
	}
	return $
}

const
BuiltinDict = Object.fromEntries( Builtins.map( _ => [ _.label, _ ] ) )

export const
Read = ( r, terminator ) => {
	while ( r.Avail() ) {
		let _ = r.Read()
		if ( _.match( /\d/ ) ) return ReadNumber( r, _ )
		if ( _.match( /\s/ ) ) continue	//	[ \f\n\r\t\v\u00a0\u1680\u180e\u2000-\u200a\u2028\u2029\u202f\u205f\u3000\ufeff]
		switch ( _ ) {
		case	terminator	: return void 0	//	Must be before close brace
		case	']'			:
		case	'}'			:
		case	')'			:
		case	'»'			:
			return null
		//	throw `Unexpected brace closer: ${_}`
		case	'['			: return new List		( ReadList( r, ']' ) )
		case	'('			: return new Sentence	( ReadList( r, ')' ) )
		case	'{'			: return new Procedure	( ReadList( r, '}' ) )
		case	'«'			: return new Parallel	( ReadList( r, '»' ) )
		case	'"'			: return ReadLiteral( r, _ )
		case	'`'			: return ReadLiteral( r, _ )
		default				:
			if ( BuiltinLabel0s.includes( _ ) ) {
				const c2 = r.Peek()
				return BuiltinLabels.includes( _ + c2 )
				?	(	r.Forward()
					,	BuiltinDict[ _ + c2 ]
					)
				:	BuiltinDict[ _ ]
			} else {
				return ReadName( r, _ )
			}
		}
	}
	return null
}

////////////////////////////////////////////////////////////////
//	REPL
////////////////////////////////////////////////////////////////

export class
StringReader {
	constructor( $ ) {
		this.$ = $
		this._ = 0
	}
	Avail()		{ return this._ < this.$.length }
	Read()		{ return this.$[ this._++ ] }
	Peek()		{ return this.$[ this._ ] }
	Forward()	{ this._++ }
	Backward()	{ --this._ }
}

const
SliP_JObject = _ => {
	switch ( typeof _ ) {
	case 'number'	: return new Numeric( _ )
	case 'string'	: return new Literal( _ )
	case 'object'	:
		if ( _ === null ) return new List( [] )
		switch ( _.constructor ) {
		case Array:
			return new List( _.map( _ => SliP_JObject( _ ) ) )
		case Object:
			return new Dict( Object.fromEntries( Object.entries( _ ).map( ( [ k, v ] ) => [ k, SliP_JObject( v ) ] ) ) )
		case String:
			return new Literal( _ )
		}
	}
}

const
JObject_SliP = _ => {
	switch ( _.constructor ) {
	case Numeric:
	case Literal:
		return _._
	case List:
		return _._.length
		?	_._.map( _ => JObject_SliP( _ ) )
		:	null
	case Dict:
		return Object.fromEntries( Object.entries( _._ ).map( ( [ k, v ] ) => [ k, JObject_SliP( v ) ] ) )
	}
	throw _.string() + ' can not be converted to JSON'
}

const
NEval = ( c, _ ) => {
	if ( _ === void 0 ) throw [ 'Insufficient argument:' ]
	const $ = Eval( c, _ )
	if ( $.constructor !== Numeric ) throw [ 'Not a number:' + $.string() ]
	if ( Number.isNaN( $._ ) ) throw [ 'Argument is NaN' ]
	return $
}

export const
NewContext = () => new Context(
	{}
,	new Context(
		[	new Unary(
				( c, _ ) => SliP_JObject( JSON.parse( _._ ) )
			,	'byJSON'
			)
		,	new Unary(
				( c, _ ) => new Literal( JSON.stringify( JObject_SliP( _ ) ) )
			,	'toJSON'
			)
		,	new Unary(
				( c, _ ) => new Numeric( parseInt( _._[ 0 ]._, _._[ 1 ]._ ) )
			,	'int'
			)
		,	new Unary(
				( c, _ ) => new Literal( _._[ 0 ]._.toString( _._[ 1 ]._ ) )
			,	'string'
			)
//	MATH EXTENSION
		,	new Prefix(
				( c, _ ) => new Numeric( Math.abs( NEval( c, _ )._ ) )
			,	'abs'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.acos( NEval( c, _ )._ ) )
			,	'acos'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.acosh( NEval( c, _ )._ ) )
			,	'acosh'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.asin( NEval( c, _ )._ ) )
			,	'asin'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.asinh( NEval( c, _ )._ ) )
			,	'asinh'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.atan( NEval( c, _ )._ ) )
			,	'atan'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.atanh( NEval( c, _ )._ ) )
			,	'atanh'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.atan2( NEval( c, _._[ 0 ] )._, NEval( c, _._[ 1 ] )._ ) )
			,	'atan2'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.cbrt( NEval( c, _ )._ ) )
			,	'cbrt'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.ceil( NEval( c, _ )._ ) )
			,	'ceil'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.cos( NEval( c, _ )._ ) )
			,	'cos'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.cosh( NEval( c, _ )._ ) )
			,	'cosh'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.exp( NEval( c, _ )._ ) )
			,	'exp'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.floor( NEval( c, _ )._ ) )
			,	'floor'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.hypot( ...Eval( c, _ )._.map( _ => NEval( c, _ )._ ) ) )
			,	'hypot'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.log( NEval( c, _ )._ ) )
			,	'log'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.log10( NEval( c, _ )._ ) )
			,	'log10'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.log2( NEval( c, _ )._ ) )
			,	'log2'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.max( ...Eval( c, _ )._.map( _ => NEval( c, _ )._ ) ) )
			,	'max'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.min( ...Eval( c, _ )._.map( _ => NEval( c, _ )._ ) ) )
			,	'min'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.pow( NEval( c, _._[ 0 ] )._, NEval( c, _._[ 1 ] )._ ) )
			,	'pow'
			)
		,	new Primitive(
				c => new Numeric( Math.random() )
			,	'random'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.round( NEval( c, _ )._ ) )
			,	'round'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.sign( NEval( c, _ )._ ) )
			,	'sign'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.sin( NEval( c, _ )._ ) )
			,	'sin'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.sinh( NEval( c, _ )._ ) )
			,	'sinh'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.sqrt( NEval( c, _ )._ ) )
			,	'sqrt'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.tan( NEval( c, _ )._ ) )
			,	'tan'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.tanh( NEval( c, _ )._ ) )
			,	'tanh'
			)
		,	new Prefix(
				( c, _ ) => new Numeric( Math.trunc( NEval( c, _ )._ ) )
			,	'trunc'
			)
		,	new Unary(
				( c, _ ) => {
					if ( ! _ instanceof List								) throw "matrix's argument must be List." 
					if ( _._.length !== 2									) throw "matrix's argument length must be 2." 
					const [ list, numeric ] = _._
					if ( ! list		instanceof List							) throw "matrix's 1st element must be List." 
					if ( ! numeric	instanceof Numeric						) throw "matrix's 2nd element must be Numeric." 
					if ( ! Number.isInteger( numeric._ )					) throw "matrix's 2nd element must be integer."
					if ( numeric._ === 0									) throw "matrix's 2nd element must be non zero."
					if ( ! Number.isInteger( list._.length / numeric._ )	) throw "matrix's size unmatch."
					const $ = new List( list._.slice() )
					$.matrix = numeric._
					return $
				}
			,	'matrix'
			)
//	ARRAY EXTENSION
		,	new Prefix(
				( c, _ ) => new Unary( ( c, array ) => new Numeric( array._[ Eval( c, _ )._ ] ) )
			,	'get'
			)
		,	new Prefix(
				( c, _ ) => new Unary(
					( c, array ) => {
						const [ offset, value ] = Eval( c, _ )._.map( _ => _._ )
						array._[ offset ] = value
						return array
					}
				)
			,	'set'
			)
//	GRAPHIC EXTENSIONS	( WEB ONLY )
//		CANVAS
		,	new Unary(
				( c, _ ) => {
					const $ = document.body.appendChild( document.createElement( 'canvas' ) )
					$.width = _._[ 0 ]._
					$.height = _._[ 1 ]._
					$.setAttribute( 'title', 'Double click to close' )
					$.onmousedown = md => {
						const org = $.getBoundingClientRect()
						$.onmousemove = mm => (
							$.style.left = org.left + mm.clientX - md.clientX + 'px'
						,	$.style.top = org.top + mm.clientY - md.clientY + 'px'
						)
						$.onmouseup = $.onmouseleave = () => $.onmousemove = $.onmouseup = null
					}
					$.onclick = ev => ev.detail > 1 && $.parentNode.removeChild( $ )
					return new SliP( $.getContext( '2d' ) )
				}
			,	'canvas'
			)
		,	new Unary(
				( c, canvas ) => ( canvas._.save(), canvas )
			,	'save'
			)
		,	new Unary(
				( c, canvas ) => ( canvas._.restore(), canvas )
			,	'restore'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.translate( ...Eval( c, _ )._.map( _ => _._ ) ), canvas ), '' )
			,	'translate'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.scale( ...Eval( c, _ )._.map( _ => _._ ) ), canvas ), '' )
			,	'scale'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.rotate( Eval( c, _ )._ ), canvas ), '' )
			,	'rotate'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.setTransform( ...Eval( c, _ )._.map( _ => _._ ) ), canvas ), '' )
			,	'transform'
			)
		,	new Unary(
				( c, canvas ) => ( canvas._.resetTransform(), canvas )
			,	'resetTransform'
			)
		,	new Unary(
				( c, canvas ) => ( canvas._.beginPath(), canvas )
			,	'beginPath'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.moveTo( ...Eval( c, _ )._.map( _ => _._ ) ), canvas ), '' )
			,	'moveTo'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.lineTo( ...Eval( c, _ )._.map( _ => _._ ) ), canvas ), '' )
			,	'lineTo'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.quadraticCurveTo( ...Eval( c, _ )._.map( _ => _._ ) ), canvas ), '' )
			,	'quadraticCurveTo'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.bezierCurveTo( ...Eval( c, _ )._.map( _ => _._ ) ), canvas ), '' )
			,	'bezierCurveTo'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.arcTo( ...Eval( c, _ )._.map( _ => _._ ) ), canvas ), '' )
			,	'arcTo'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.arc( ...Eval( c, _ )._.map( _ => _._ ) ), canvas ) , '' )
			,	'arc'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.rect( ...Eval( c, _ )._.map( _ => _._ ) ), canvas ) , '' )
			,	'rect'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.ellipse( ...Eval( c, _ )._.map( _ => _._ ) ), canvas ) , '' )
			,	'ellipse'
			)
		,	new Unary(
				( c, canvas ) => ( canvas._.closePath(), canvas )
			,	'closePath'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.fillStyle = Eval( c, _ )._, canvas ), '' )
			,	'fillStyle'
			)
		,	new Unary(
				( c, canvas ) => ( canvas._.fill(), canvas )
			,	'fill'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.fill( ...Eval( c, _ )._.map( _ => _._ ) ), canvas ), '' )
			,	'fillWith'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.strokeStyle = Eval( c, _ )._, canvas ), '' )
			,	'strokeStyle'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.lineCap = Eval( c, _ )._, canvas ), '' )
			,	'lineCap'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.lineJoin = Eval( c, _ )._, canvas ), '' )
			,	'lineJoin'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.lineWidth = Eval( c, _ )._, canvas ), '' )
			,	'lineWidth'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.setLineDash( Eval( c, _ )._.map( _ => _._ ) ), canvas ), '' )
			,	'lineDash'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.lineDashOffset = Eval( c, _ )._, canvas ), '' )
			,	'lineDashOffset'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.miterLimit = Eval( c, _ )._, canvas ), '' )
			,	'miterLimit'
			)
		,	new Unary( ( c, canvas ) => ( canvas._.stroke(), canvas )
			,	'stroke'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.stroke( ...Eval( c, _ )._.map( _ => _._ ) ), canvas ), '' )
			,	'strokeWith'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.strokeRect( ...Eval( c, _ )._.map( _ => _._ ) ), canvas ), '' )
			,	'strokeRect'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.fillRect( ...Eval( c, _ )._.map( _ => _._ ) ), canvas ), '' )
			,	'fillRect'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.font = Eval( c, _ )._, canvas ), '' )
			,	'font'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.textAlign = Eval( c, _ )._, canvas ), '' )
			,	'textAlign'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.textBaseline = Eval( c, _ )._, canvas ), '' )
			,	'textBaseline'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.direction = Eval( c, _ )._, canvas ), '' )
			,	'direction'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.strokeText( ...Eval( c, _ )._.map( _ => _._ ) ), canvas ), '' )
			,	'strokeText'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.fillText( ...Eval( c, _ )._.map( _ => _._ ) ), canvas ), '' )
			,	'fillText'
			)
		,	new Unary(
				( c, canvas ) => ( canvas._.clip(), canvas )
			,	'clip'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.clip( ...Eval( c, _ )._.map( _ => _._ ) ), canvas ), '' )
			,	'clipWith'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.clearRect( ...Eval( c, _ )._.map( _ => _._ ) ), canvas ), '' )
			,	'clearRect'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.filter = Eval( c, _ )._, canvas ), '' )
			,	'filter'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.shadowColor = Eval( c, _ )._, canvas ), '' )
			,	'shadowColor'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.shadowBlur = Eval( c, _ )._, canvas ), '' )
			,	'shadowBlur'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.shadowOffsetX = Eval( c, _ )._, canvas ), '' )
			,	'shadowOffsetX'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.shadowOffsetY = Eval( c, _ )._, canvas ), '' )
			,	'shadowOffsetY'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.globalAlpha = Eval( c, _ )._, canvas ), '' )
			,	'globalAlpha'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.globalCompositeOperation = Eval( c, _ )._, canvas ), '' )
			,	'globalCompositeOperation'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.imageSmoothingEnabled = _IsT( Eval( c, _ ) ), canvas ), '' )
			,	'imageSmoothingEnabled'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.imageSmoothingQuality = Eval( c, _ )._, canvas ), '' )
			,	'imageSmoothingQuality'
			)
//	NON CANVAS VALUE
		,	new Unary(
				( c, canvas ) => new List( canvas._.getLineDash().map( _ => new Numeric( _ ) ) )
			,	'getLineDash'
			)
		,	new Unary(
				( c, canvas ) => {
					const $ = canvas._.getTransform()
					return new List( [ new Numeric( $.a ), new Numeric( $.b ), new Numeric( $.c ), new Numeric( $.d ), new Numeric( $.e ), new Numeric( $.f ) ] )
				}
			,	'getTransform'
			)
		,	new Prefix(
				( c, _ ) => new Unary(
					( c, canvas ) => {
						const textMetrics = canvas._.measureText( Eval( c, _ )._ )
						return new Dict(
							Object.keys( textMetrics.constructor.prototype ).reduce(
								( $, _ ) => ( $[ _ ] = new Numeric( textMetrics[ _ ] ), $ )
							,	{}
							)
						)
					}
				)
			,	'measureText'
			)
//		PATH
		,	new Primitive(
				( c, canvas ) => new SliP( new Path2D() )
			,	'path2D'
			)
		,	new Prefix(
				( c, _ ) => new SliP( new Path2D( ...Eval( c, _ )._.map( _ => _._ ) ) )
			,	'path2DWith'
			)
//		IMAGE
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => new SliP( canvas._.createImageData( ...Eval( c, _ )._.map( _ => _._ ) ) ) )
			,	'createImageData'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => new SliP( canvas._.getImageData( ...Eval( c, _ )._.map( _ => _._ ) ) ) )
			,	'getImageData'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => ( canvas._.putImageData( ...Eval( c, _ )._.map( _ => _._ ) ), canvas ) )
			,	'putImageData'
			)
		,	new Unary(
				( c, image ) => new Numeric( image._.width )
			,	'imageDataWidth'
			)
		,	new Unary(
				( c, image ) => new Numeric( image._.height )
			,	'imageDataHeight'
			)
		,	new Unary(
				( c, image ) => new SliP( image._.data )
			,	'imageDataArray'
			)
//		GRADIENT
		,	new Prefix(	//	ctx:createConicGradient[ startAngle x y ]
				( c, _ ) => new Unary( ( c, canvas ) => new SliP( canvas._.createConicGradient( ...Eval( c, _ )._.map( _ => _._ ) ) ), '' )
			,	'createConicGradient'
			)
		,	new Prefix(	//	ctx:createLinearGradient[ x0 y0 x1 y1 ]
				( c, _ ) => new Unary( ( c, canvas ) => new SliP( canvas._.createLinearGradient( ...Eval( c, _ )._.map( _ => _._ ) ) ), '' )
			,	'createLinearGradient'
			)
		,	new Prefix(	//	ctx:createRadialGradient[ x0 y0 r0 x1 y1 r1 ]
				( c, _ ) => new Unary( ( c, canvas ) => new SliP( canvas._.createRadialGradient( ...Eval( c, _ )._.map( _ => _._ ) ) ), '' )
			,	'createRadialGradient'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, gradient ) => ( gradient._.addColorStop( ...Eval( c, _ )._.map( _ => _._ ) ), gradient ), '' )
			,	'addColorStop'
			)
//		PointIn
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => canvas._.isPointInPath( ...Eval( c, _ )._.map( _ => _._ ) ) ? T : Nil, '' )
			,	'isPointInPath'
			)
		,	new Prefix(
				( c, _ ) => new Unary( ( c, canvas ) => canvas._.isPointInStroke( ...Eval( c, _ )._.map( _ => _._ ) ) ? T : Nil, '' )
			,	'isPointInStroke'
			)

//	TODO	element
		].reduce(
			( $, _ ) => ( $[ _.label ] = _, $ )
		,	{	π	: new Numeric( Math.PI )
			,	e	: new Numeric( Math.E )
			,	'∞'	: new Numeric( Infinity )
			}
		)
	)
)

export const
Sugared = _ => new Sentence( ReadList( new StringReader( _ + ')' ), ')' ) )
