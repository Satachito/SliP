package main

import	"fmt"

import	"bufio"
import	"os"

type
Object		interface {
	Eval()	Object
}

type
Map			struct {	//	¡
	u		map[ interface{} ]Object
}

type
Quote		struct {	//	'
	u		Object
}

type
Combiner	struct {
	u		Object
}

type
EvalAssoc	struct {	//	`
	mapList	*mapList
	u		Object
}

type
Number		struct {
	u		interface{}
}

type
Name		struct {
	u		interface{}
}

type
String		struct {
	u		string
}

type
Primitive	struct {
	tag		interface{}	//	DEBUGGING PURPOSE ONLY
	u		func() Object	
}

type
Builtin		struct {
	tag		interface{}	//	DEBUGGING PURPOSE ONLY
	u		func( Object ) Object	
}

type
Operator	struct {
	tag		interface{}	//	DEBUGGING PURPOSE ONLY
	bond	int
	u		func( Object, Object ) Object	
}

type
Slice		struct {	//	[]
	u		[]Object
}

type
Block		struct {	//	{}
	u		[]*Sentence
}

type
Sentence	struct {	//	()	;
	u		[]Object
}

////////////////////////////////////////////////////////////////	Stack
type
stack struct {
	u		Object
	next	*stack
}

var	sStack	*stack

func
Push( p Object ) {
//	fmt.Printf( "Pushed:%v\n", p )
	sStack = &stack{ p, sStack }
}

func
Pop() {
//	fmt.Printf( "Poped:%v\n", sStack.u )
	sStack = sStack.next
}

////////////////////////////////////////////////////////////////	AssocList
type
mapList struct {
	u		map[ interface{} ]Object
	next	*mapList
}

type
assocList struct {
	u		*mapList
	next	*assocList
}

var	sAssocList = &assocList{ &mapList{ map[ interface{} ]Object{}, nil }, nil }

func
begin() {
	sAssocList.u = &mapList{ map[ interface{} ]Object{}, sAssocList.u }
}

func
end() {
	sAssocList.u = sAssocList.u.next
	if sAssocList.u == nil { panic( "Global dictionary popped." ) }
}

func
assoc( p *Name ) Object {
	for wAL := sAssocList; wAL != nil; wAL = wAL.next {
		for wML := wAL.u; wML != nil; wML = wML.next {
			if v, ok := wML.u[ p.u ]; ok { return v }
		}
	}
	panic( fmt.Sprintf( "No such key:%v", p.u ) )
}

////////////////////////////////////////////////////////////////	Prepared functions
func
get( l Object, r int ) Object {
	switch t := l.(type) {
	case *Slice:		return t.u[ r ]
	case *Block:		return t.u[ r ]
	case *Sentence:		return t.u[ r ]
	default:			panic( fmt.Sprintf( "get %d from %T:%v", r, l, l ) )
	}
}

func 
apply( l Object, r Object ) Object {
//	fmt.Printf( "-----------apply %v to %v\n", l, r )
	switch t := r.(type) {
	case *Number:
		return get( l, t.u.(int) )
	case *Name:
		return l.(*Map).u[ t.u ]
	case *Builtin:
		return t.u( l )
	case *Block, *Sentence, *EvalAssoc:
		Push( l )
		defer Pop()
		return t.Eval()
	case *Slice:
		return t.u[ l.(*Number).u.(int) ].Eval()
	case *Map:
		return t.u[ l.(*Name).u.(int) ].Eval()
	default:
		panic( fmt.Sprintf( "%v applied to %v", t, l ) )
	}
}

func
def( l Object, r Object ) Object {
	sAssocList.u.u[ l.(*Name).u ] = r
//println( ">>ASSOC LIST" )
//fmt.Printf( "%v\n", sAssocList.u.u )
//println( "<<ASSOC LIST" )
	return r
}

func
cons( l Object, r Object ) Object {
	switch t := r.(type) {
	case *Slice:	return &Slice{ append( []Object{ l }, t.u... ) }
	default:		panic( "Cons to non array" )
	}
}

func
concat( l Object, r Object ) Object {
	switch tL := l.(type) {
	case *Slice:
		switch tR := r.(type) {
		case *Slice:	return &Slice{ append( tL.u, tR.u... ) }
		default:		return &Slice{ append( tL.u, r ) }
		}
	default:
		switch tR := r.(type) {
		case *Slice:	return &Slice{ append( []Object{ l }, tR.u... ) }
		default:		return &Slice{ []Object{ l, r } }
		}
	}
}

func
xor( l Object, r Object ) Object {
	return &Number{ l.(*Number).u.(int) ^ r.(*Number).u.(int) }
}

func
and( l Object, r Object ) Object {
	return &Number{ l.(*Number).u.(int) & r.(*Number).u.(int) }
}

func
or( l Object, r Object ) Object {
	return &Number{ l.(*Number).u.(int) | r.(*Number).u.(int) }
}

func
add( l Object, r Object ) Object {
	return &Number{ l.(*Number).u.(int) + r.(*Number).u.(int) }
}

func
sub( l Object, r Object ) Object {
	return &Number{ l.(*Number).u.(int) - r.(*Number).u.(int) }
}

func
mul( l Object, r Object ) Object {
	return &Number{ l.(*Number).u.(int) * r.(*Number).u.(int) }
}

func
div( l Object, r Object ) Object {
	return &Number{ l.(*Number).u.(int) / r.(*Number).u.(int) }
}

func
remainder( l Object, r Object ) Object {
	return &Number{ l.(*Number).u.(int) % r.(*Number).u.(int) }
}

func
gt( l Object, r Object ) Object {
	if l.(*Number).u.(int) > r.(*Number).u.(int) { return &Number{ 1 } } else { return &Number{ 0 } }
}

func
ge( l Object, r Object ) Object {
	if l.(*Number).u.(int) >= r.(*Number).u.(int) { return &Number{ 1 } } else { return &Number{ 0 } }
}

func
lt( l Object, r Object ) Object {
	if l.(*Number).u.(int) < r.(*Number).u.(int) { return &Number{ 1 } } else { return &Number{ 0 } }
}

func
le( l Object, r Object ) Object {
	if l.(*Number).u.(int) <= r.(*Number).u.(int) { return &Number{ 1 } } else { return &Number{ 0 } }
}

func
eq( l Object, r Object ) Object {
	if l.(*Number).u.(int) == r.(*Number).u.(int) { return &Number{ 1 } } else { return &Number{ 0 } }
}

func
neq( l Object, r Object ) Object {
	if l.(*Number).u.(int) != r.(*Number).u.(int) { return &Number{ 1 } } else { return &Number{ 0 } }
}

func
size( p Object ) Object {
	switch t := p.(type) {
	case *Slice:		return &Number{ len( t.u ) }
	case *Block:		return &Number{ len( t.u ) }
	case *Sentence:		return &Number{ len( t.u ) }
	case *Map:			return &Number{ len( t.u ) }
	default:			panic( "Syntax Error" )
	}
}

func
car( p Object ) Object {
	return get( p, 0 )
}
	
func
cdr( p Object ) Object {
	switch t := p.(type) {
	case *Slice:		return &Slice{ t.u[ 1: ] }
	case *Block:		return &Block{ t.u[ 1: ] }
	case *Sentence:		return &Sentence{ t.u[ 1: ] }
	default:			panic( "Syntax Error" )
	}
}

func
last( p Object ) Object {
	switch t := p.(type) {
	case *Slice:		return t.u[ len( t.u ) - 1 ]
	case *Block:		return t.u[ len( t.u ) - 1 ]
	case *Sentence:		return t.u[ len( t.u ) - 1 ]
	default:			panic( "Syntax Error" )
	}
}

func
not( p Object ) Object {
	switch t := p.(type) {
	case *Number:	return &Number{ ^t.u.(int) }
	default:		panic( "Syntax Error" )
	}
}

func
_if( p Object ) Object {
	switch t := p.(type) {
	case *Slice:	if len( t.u ) == 0	{ return &Number{ 0 } } else { return &Number{ 1 } }
	case *Block:	if len( t.u ) == 0	{ return &Number{ 0 } } else { return &Number{ 1 } }
	case *Sentence:	if len( t.u ) == 0	{ return &Number{ 0 } } else { return &Number{ 1 } }
	case *Number:	if t.u == 0			{ return t } else { return &Number{ 1 } }
	default:		panic( fmt.Sprintf( "if applied to %T:%v", p, p ) )
	}
}

func
fi( p Object ) Object {
	switch t := p.(type) {
	case *Slice:	if len( t.u ) != 0	{ return &Number{ 0 } } else { return &Number{ 1 } }
	case *Block:	if len( t.u ) != 0	{ return &Number{ 0 } } else { return &Number{ 1 } }
	case *Sentence:	if len( t.u ) != 0	{ return &Number{ 0 } } else { return &Number{ 1 } }
	case *Number:	if t.u != 0			{ return t } else { return &Number{ 1 } }
	default:		panic( fmt.Sprintf( "fi applied to %T:%v", p, p ) )
	}
}

////////////////////////////////////////////////////////////////	Builtins

func
binaryFunc( a Object, f func( Object, Object ) Object ) Object {
	w := a.(*Slice).u
	return f( w[ 0 ], w[ 1 ] )
}

var	If			= &Builtin{ "?", _if }
var	Fi			= &Builtin{ "¿", fi }
var	Size		= &Builtin{ "#", size }
var	Last		= &Builtin{ "$", last }
var	Not			= &Builtin{ "~", not }

func
init() {
	sAssocList.u.u[ "print"		] = &Builtin{ "print", Print }
	sAssocList.u.u[ "println"	] = &Builtin{ "println", Println }
	sAssocList.u.u[ "car"		] = &Builtin{ "car", car }
	sAssocList.u.u[ "cdr"		] = &Builtin{ "cdr", cdr }
	sAssocList.u.u[ "fi"		] = Fi
	sAssocList.u.u[ "if"		] = If
	sAssocList.u.u[ "size"		] = Size
	sAssocList.u.u[ "last"		] = Last
	sAssocList.u.u[ "not"		] = Not

	sAssocList.u.u[ "apply"		] = &Builtin{ ".", func( p Object ) Object { return binaryFunc( p, apply ) } }
	sAssocList.u.u[ "def"		] = &Builtin{ "=", func( p Object ) Object { return binaryFunc( p, def ) } }
	sAssocList.u.u[ "cons"		] = &Builtin{ "cons", func( p Object ) Object { return binaryFunc( p, cons ) } }
	sAssocList.u.u[ "concat"	] = &Builtin{ ",", func( p Object ) Object { return binaryFunc( p, concat ) } }
	sAssocList.u.u[ "add"		] = &Builtin{ "+", func( p Object ) Object { return binaryFunc( p, add ) } }
	sAssocList.u.u[ "sub"		] = &Builtin{ "-", func( p Object ) Object { return binaryFunc( p, sub ) } }
	sAssocList.u.u[ "mul"		] = &Builtin{ "*", func( p Object ) Object { return binaryFunc( p, mul ) } }
	sAssocList.u.u[ "div"		] = &Builtin{ "/", func( p Object ) Object { return binaryFunc( p, div ) } }
	sAssocList.u.u[ "remainder"	] = &Builtin{ "%", func( p Object ) Object { return binaryFunc( p, remainder ) } }

	sAssocList.u.u[ "gt"		] = &Builtin{ ">" , func( p Object ) Object { return binaryFunc( p, gt ) } }
	sAssocList.u.u[ "ge"		] = &Builtin{ ">=", func( p Object ) Object { return binaryFunc( p, ge ) } }
	sAssocList.u.u[ "lt"		] = &Builtin{ "<" , func( p Object ) Object { return binaryFunc( p, lt ) } }
	sAssocList.u.u[ "le"		] = &Builtin{ "<=", func( p Object ) Object { return binaryFunc( p, le ) } }
	sAssocList.u.u[ "eq"		] = &Builtin{ "==", func( p Object ) Object { return binaryFunc( p, eq ) } }
}

func
body( r *bufio.Reader ) {
	for {
		Println( Read( r ).Eval() )
//		Println( Println( Read( r ) ).Eval() )
	}
}

func
main() {
/*
	defer func() {
		r := recover();
		switch t := r.(type) {
		case error:	
			if t != io.EOF { log.Printf( "ERROR: %T:%v\n", t, t ) }
		default:
			log.Printf( "OTHER: %v\n", t )
		}
	}()
*/
	body( bufio.NewReader( os.Stdin ) )
}

