package	main

import	"os"
import	"log"
import	"io"
import	"bufio"
import	"unicode"
import	"strconv"
import	"fmt"
import	"sync"

func
Rune( r *bufio.Reader ) rune {
	v, _, err := r.ReadRune()
	if	err != nil { panic( err ) }
	return v
}

func
SkipWhite( r *bufio.Reader ) {
	for {
		w, _, err := r.ReadRune()
		if err != nil { panic( err ) }
		if unicode.IsSpace( w ) { continue }
		r.UnreadRune()
		return
	}
}

func
tagString( p interface{} ) string {
    switch t := p.(type) {
    case byte:      return fmt.Sprintf( "%c", t )
    case rune:      return fmt.Sprintf( "%c", t )
    case string:    return t
    default:        return fmt.Sprintf( "%v", t )
    }
}

func
objectsString( p []Object ) string {
	v := " ";
	for _, w := range( p ) {
		v = fmt.Sprintf( "%v%v ", v, w )
	}
	return v
}

func
evalParallel( p []Object ) []Object {
	v := make( []Object, len( p ) )
	var	wg sync.WaitGroup
	for i, o := range p {
		wg.Add( 1 )
		go func() {
			defer wg.Done()
			v[ i ] = o.Eval()
		}()
	}
	wg.Wait()
	return v
}

func
evalSentence( p []Object ) Object {
	switch len( p ) {
	case 0, 2:	panic( "Syntax error" )
	case	1:	return p[ 0 ]
	case	3:	return p[ 1 ].(*Operator).u( p[ 0 ], p[ 2 ] )
	}
    var target  *Operator = nil
    var index   = 0
    for i, w := range p {
        switch t := w.(type) {
        case *Operator:
            if target == nil {
                target = t
                index = i
            } else {
                if t.bond >= target.bond {
                    target = t
                    index = i
                } 
            }
        }
    }       
    if target == nil { panic( "Nothing to do." ) }
    if index == 0 || index == len( p ) - 1 { panic( fmt.Sprintf( "No argument for operator:%v", target ) ) }
    return target.u( evalSentence( p[ :index ] ), evalSentence( p[ index+1: ] ) )
}

////////////////////////////////////////////////////////////////    Stack
type
stack struct {
    u       Object
    next    *stack
}

var sStack  *stack

////////////////////////////////////////////////////////////////    AssocList
type
mapList struct {
    u       map[ interface{} ]Object
    next    *mapList
}

type
assocList struct {
    u       *mapList
    next    *assocList
}

var sAssocList = &assocList{ &mapList{ map[ interface{} ]Object{}, nil }, nil }

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

////////////////////////////////////////////////////////////////    Prepared functions
func
get( l Object, r int ) Object {
    switch t := l.(type) {
    case *Slice:        return t.u[ r ] 
    case *Block:        return t.u[ r ] 
    case *Sentence:     return t.u[ r ] 
    default:            panic( fmt.Sprintf( "get %d from %T:%v", r, l, l ) ) 
    }   
}

func
apply( l Object, r Object ) Object {
//  fmt.Printf( "-----------apply %v to %v\n", l, r ) 
    switch t := r.(type) {
    case *Number:
        return get( l, int( t.u.(float64) ) ) 
    case *Name:
        return l.(*Map).u[ t.u ]
    case *Builtin:
        return t.u( l ) 
    case *Block, *Sentence, *EvalAssoc:
		sStack = &stack{ l, sStack }
        defer func() { sStack = sStack.next }()
        return t.Eval()
    case *Slice:
        return t.u[ int( l.(*Number).u.(float64) ) ].Eval()
    case *Map:
        return t.u[ int( l.(*Name).u.(float64) ) ].Eval()
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
    case *Slice:    return &Slice{ append( []Object{ l }, t.u... ) } 
    default:        panic( "Cons to non array" )
    }   
}

func
concat( l Object, r Object ) Object {
    switch tL := l.(type) {
    case *Slice:
        switch tR := r.(type) {
        case *Slice:    return &Slice{ append( tL.u, tR.u... ) } 
        default:        return &Slice{ append( tL.u, r ) } 
        }   
    default:
        switch tR := r.(type) {
        case *Slice:    return &Slice{ append( []Object{ l }, tR.u... ) } 
        default:        return &Slice{ []Object{ l, r } } 
        }   
    }   
}

func
xor( l Object, r Object ) Object {
    return &Number{ float64( int( l.(*Number).u.(float64) ) ^ int( r.(*Number).u.(float64) ) ) }
}

func
and( l Object, r Object ) Object {
    return &Number{ float64( int( l.(*Number).u.(float64) ) & int( r.(*Number).u.(float64) ) ) }
}

func
or( l Object, r Object ) Object {
    return &Number{ float64( int( l.(*Number).u.(float64) ) | int( r.(*Number).u.(float64) ) ) }
}

func
add( l Object, r Object ) Object {
    return &Number{ l.(*Number).u.(float64) + r.(*Number).u.(float64) }
}

func
sub( l Object, r Object ) Object {
    return &Number{ l.(*Number).u.(float64) - r.(*Number).u.(float64) }
}

func
mul( l Object, r Object ) Object {
    return &Number{ l.(*Number).u.(float64) * r.(*Number).u.(float64) }
}

func
div( l Object, r Object ) Object {
    return &Number{ l.(*Number).u.(float64) / r.(*Number).u.(float64) }
}

func
remainder( l Object, r Object ) Object {
    return &Number{ float64( int( l.(*Number).u.(float64) ) % int( r.(*Number).u.(float64) ) ) }
}

func
gt( l Object, r Object ) Object {
    if l.(*Number).u.(float64) > r.(*Number).u.(float64) { return &Number{ 1.0 } } else { return &Number{ 0.0 } }
}

func
ge( l Object, r Object ) Object {
    if l.(*Number).u.(float64) >= r.(*Number).u.(float64) { return &Number{ 1.0 } } else { return &Number{ 0.0 } }
}

func
lt( l Object, r Object ) Object {
    if l.(*Number).u.(float64) < r.(*Number).u.(float64) { return &Number{ 1.0 } } else { return &Number{ 0.0 } }
}

func
le( l Object, r Object ) Object {
    if l.(*Number).u.(float64) <= r.(*Number).u.(float64) { return &Number{ 1.0 } } else { return &Number{ 0.0 } }
}

func
eq( l Object, r Object ) Object {
    if l.(*Number).u.(float64) == r.(*Number).u.(float64) { return &Number{ 1.0 } } else { return &Number{ 0.0 } }
}

func
neq( l Object, r Object ) Object {
    if l.(*Number).u.(float64) != r.(*Number).u.(float64) { return &Number{ 1.0 } } else { return &Number{ 0.0 } }
}

func
size( p Object ) Object {
    switch t := p.(type) {
    case *Slice:        return &Number{ float64( len( t.u ) ) }
    case *Block:        return &Number{ float64( len( t.u ) ) }
    case *Sentence:     return &Number{ float64( len( t.u ) ) }
    case *Map:          return &Number{ float64( len( t.u ) ) }
    default:            panic( "Syntax Error" )
    }
}

func
car( p Object ) Object {
    return get( p, 0 )
}

func
cdr( p Object ) Object {
    switch t := p.(type) {
    case *Slice:        return &Slice{ t.u[ 1: ] }
    case *Block:        return &Block{ t.u[ 1: ] }
    case *Sentence:     return &Sentence{ t.u[ 1: ] }
    default:            panic( "Syntax Error" )
    }
}

func
last( p Object ) Object {
    switch t := p.(type) {
    case *Slice:        return t.u[ len( t.u ) - 1 ]
    case *Block:        return t.u[ len( t.u ) - 1 ]
    case *Sentence:     return t.u[ len( t.u ) - 1 ]
    default:            panic( "Syntax Error" )
    }
}

func
not( p Object ) Object {
    switch t := p.(type) {
    case *Number:   return &Number{ float64( ^int( t.u.(float64) ) ) }
    default:        panic( "Syntax Error" )
    }
}

func
_if( p Object ) Object {
    switch t := p.(type) {
    case *Slice:    if len( t.u ) == 0  { return &Number{ 0.0 } } else { return &Number{ 1.0 } }
    case *Block:    if len( t.u ) == 0  { return &Number{ 0.0 } } else { return &Number{ 1.0 } }
    case *Sentence: if len( t.u ) == 0  { return &Number{ 0.0 } } else { return &Number{ 1.0 } }
    case *Number:   if t.u.(float64) == 0.0         { return t } else { return &Number{ 1.0 } }
    default:        panic( fmt.Sprintf( "if applied to %T:%v", p, p ) )
    }
}

func
fi( p Object ) Object {
    switch t := p.(type) {
    case *Slice:    if len( t.u ) != 0  { return &Number{ 0.0 } } else { return &Number{ 1.0 } }
    case *Block:    if len( t.u ) != 0  { return &Number{ 0.0 } } else { return &Number{ 1.0 } }
    case *Sentence: if len( t.u ) != 0  { return &Number{ 0.0 } } else { return &Number{ 1.0 } }
    case *Number:   if t.u.(float64) != 0.0         { return t } else { return &Number{ 1.0 } }
    default:        panic( fmt.Sprintf( "fi applied to %T:%v", p, p ) )
    }
}

////////////////////////////////////////////////////////////////    Builtins

func
binaryFunc( a Object, f func( Object, Object ) Object ) Object {
    w := a.(*Slice).u
    return f( w[ 0 ], w[ 1 ] )
}

var If          = &Builtin{ "?", _if }
var Fi          = &Builtin{ "¿", fi }
var Size        = &Builtin{ "#", size }
var Last        = &Builtin{ "$", last }
var Not         = &Builtin{ "~", not }

func
init() {
    sAssocList.u.u[ "print"     ] = &Builtin{ "print", func( p Object ) Object { fmt.Printf( "%v", p ); return p } }
    sAssocList.u.u[ "println"   ] = &Builtin{ "println", func( p Object ) Object { fmt.Printf( "%v\n", p ); return p } }
    sAssocList.u.u[ "car"       ] = &Builtin{ "car", car }
    sAssocList.u.u[ "cdr"       ] = &Builtin{ "cdr", cdr }
    sAssocList.u.u[ "fi"        ] = Fi
    sAssocList.u.u[ "if"        ] = If
    sAssocList.u.u[ "size"      ] = Size
    sAssocList.u.u[ "last"      ] = Last
    sAssocList.u.u[ "not"       ] = Not

    sAssocList.u.u[ "apply"     ] = &Builtin{ ".", func( p Object ) Object { return binaryFunc( p, apply ) } }
    sAssocList.u.u[ "def"       ] = &Builtin{ "=", func( p Object ) Object { return binaryFunc( p, def ) } }
    sAssocList.u.u[ "cons"      ] = &Builtin{ "cons", func( p Object ) Object { return binaryFunc( p, cons ) } }
    sAssocList.u.u[ "concat"    ] = &Builtin{ ",", func( p Object ) Object { return binaryFunc( p, concat ) } }
    sAssocList.u.u[ "add"       ] = &Builtin{ "+", func( p Object ) Object { return binaryFunc( p, add ) } }
    sAssocList.u.u[ "sub"       ] = &Builtin{ "-", func( p Object ) Object { return binaryFunc( p, sub ) } }
    sAssocList.u.u[ "mul"       ] = &Builtin{ "*", func( p Object ) Object { return binaryFunc( p, mul ) } }
    sAssocList.u.u[ "div"       ] = &Builtin{ "/", func( p Object ) Object { return binaryFunc( p, div ) } }
    sAssocList.u.u[ "remainder" ] = &Builtin{ "%", func( p Object ) Object { return binaryFunc( p, remainder ) } }

    sAssocList.u.u[ "gt"        ] = &Builtin{ ">" , func( p Object ) Object { return binaryFunc( p, gt ) } }
    sAssocList.u.u[ "ge"        ] = &Builtin{ ">=", func( p Object ) Object { return binaryFunc( p, ge ) } }
    sAssocList.u.u[ "lt"        ] = &Builtin{ "<" , func( p Object ) Object { return binaryFunc( p, lt ) } }
    sAssocList.u.u[ "le"        ] = &Builtin{ "<=", func( p Object ) Object { return binaryFunc( p, le ) } }
    sAssocList.u.u[ "eq"        ] = &Builtin{ "==", func( p Object ) Object { return binaryFunc( p, eq ) } }
}

////////////////////////////////////////////////////////////////    Object

type
Object		interface {
	Eval()		Object	
	String()	string	
}

type
EvalAssoc   struct {    //  `
    mapList *mapList
    u       Object
}
func	( p *EvalAssoc )String()	string	{ return fmt.Sprintf( "<%v:%v>", p.mapList, p.u ) }
func	( p *EvalAssoc )Eval()		Object	{ return p.u }

type
Quote		struct {
	u		Object
}
func	( p *Quote )	String()	string	{ return "'" + p.u.String() }
func	( p *Quote )	Eval()		Object	{ return p.u }

type
Combiner    struct {
    u       Object
}
func	( p *Combiner )	String()	string	{ return "`" + p.u.String() }
func	( p *Combiner )	Eval()		Object	{ return &EvalAssoc{ sAssocList.u, p.u } }

type
String		struct {
	u		string
}
func	( p *String )	String()	string	{ return "\"" + p.u + "\"" }
func	( p *String )	Eval()		Object	{ return p }

type
Number		struct {
	u		interface{}
}
func	( p *Number )	String()	string	{ return fmt.Sprintf( "%v", p.u ) }
func	( p *Number )	Eval()		Object	{ return p }

type
Name		struct {
	u		interface{}
}
func	( p *Name )		String()	string	{ return fmt.Sprintf( "%v", p.u ) }
func	( p *Name )		Eval()		Object	{ return p }

type
Sentence	struct {
	u		[]Object
}
func	( p *Sentence )	String()	string	{ return "(" + objectsString( p.u ) + ")" }
func	( p *Sentence )	Eval()		Object	{ return evalSentence( evalParallel( p.u ) ) }

type
Slice		struct {
	u		[]Object
}
func	( p *Slice )	String()	string	{ return "[" + objectsString( p.u ) + "]" }
func	( p *Slice )	Eval()		Object	{ return &Slice{ evalParallel( p.u ) } }

type
Block		struct {
	u		[]Object
}
func	( p *Block )	String()	string	{ return "{" + objectsString( p.u ) + "}" }
func	( p *Block )	Eval()		Object	{ return p }

type
Map			struct {
	u		map[ interface{} ]Object
}
func	( p *Map )		String()	string	{ return fmt.Sprintf( "<%v>", p.u ) }
func	( p *Map )		Eval()		Object	{ return p }

type
Primitive   struct {
    tag     interface{} //  DEBUGGING PURPOSE ONLY
    u       func() Object
}
func	( p *Primitive )String()	string	{ return "P:" + tagString( p.tag ) }
func	( p *Primitive )Eval()		Object	{ return p }

type
Builtin     struct {
    tag     interface{} //  DEBUGGING PURPOSE ONLY
    u       func( Object ) Object
}
func	( p *Builtin )	String()	string	{ return "B:" + tagString( p.tag ) }
func	( p *Builtin )	Eval()		Object	{ return p }

type
Operator	struct {
    tag     interface{} //  DEBUGGING PURPOSE ONLY
	bond	int
	u		func( Object, Object ) Object
}
func	( p *Operator )	String()	string	{ return "O:" + tagString( p.tag ) }
func	( p *Operator )	Eval()		Object	{ return p }

func
readNumber( r *bufio.Reader ) float64 {
    v := ""
    for {
        c := Rune( r )
        switch c { 
        case '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.', 'x', 'e', '-', '+':
            v += string( c ) 
        default:
            r.UnreadRune()
            f, err := strconv.ParseFloat( v, 64 )
            if err != nil {
                panic( fmt.Sprintf( "ParseFloat Error: %v", v ) )
            }   
            return f
        }   
    }   
    return 0.0 
}

func
readString( r *bufio.Reader ) string {
	v := []rune{}
	escaped := false
	for {
        c := Rune( r )
		if escaped {
			escaped = false
			v = append( v, c )
		} else {
			if c == '"' {
				break
			} else if c == '\\' {
				escaped = true
			} else {
				v = append( v, c )
			}
		}
	}
	return string( v )
}

func
readName( r *bufio.Reader ) string {
	v := []rune{}
	for {
		c := Rune( r )
		if	( '0' <= c && c <= '9' ) ||	( 'A' <= c && c <= 'Z' ) ||	( 'a' <= c && c <= 'z' ) ||	( '_' == c ) {
			v = append( v, c ) 
		} else {
			r.UnreadRune()
			break
		}	
	}
	return string( v )
}

func
read( r *bufio.Reader, terminator rune ) Object {
	SkipWhite( r )
	c := Rune( r )
	switch c {
	case terminator			:	return nil
	case ';', '}', ']', ')'	:	panic( "Unexpected closer" )
	case '['				:	return &Slice{ readObjects( r, ']' ) }
	case '('				:	return &Sentence{ readObjects( r, ')' ) }
	case '{'				:	return &Block{ readObjects( r, '}' ) }
	case '\''				:	return &Quote{ read( r, terminator ) }
    case '!'				:   return &Primitive{ c, func() Object { return sStack.u } }
    case '@'				:   return &Primitive{ c, func() Object { return car( sStack.u ) } }
    case ':'				:   return &Primitive{ c, func() Object { return cdr( sStack.u ) } }
    case '¡'				:   return &Primitive{ c, func() Object { return &Map{ sAssocList.u.u } } }
	case '`'				:
        w := read( r, terminator )
        if w == nil { panic( "Nothing to combine." ) }
        return &Combiner{ w }
    case '&'				:   return &Operator{ c, 100, and }
    case '^'				:   return &Operator{ c, 110, xor }
    case '|'				:   return &Operator{ c, 120, or }
    case '*'				:   return &Operator{ c, 50, mul }
    case '/'				:   return &Operator{ c, 50, div }
    case '%'				:   return &Operator{ c, 50, remainder }
    case '+'				:
        w, _, err := r.ReadRune()
        if err == nil {
            r.UnreadRune()
            if '0' <= w && w <= '9' { return &Number{ readNumber( r ) } }
        }
        return &Operator{ c, 60, add }
    case '-'				:
        w, _, err := r.ReadRune()
        if err == nil {
            r.UnreadRune()
            if '0' <= w && w <= '9' { return &Number{ - readNumber( r ) } }
        }
        return &Operator{ c, 60, sub }
    case '.'				:   return &Operator{ c, 20, apply }
    case ','				:   return &Operator{ c, 155, concat }
    case '='				:
        w, _, err := r.ReadRune()
        if err == nil {
            switch w {
            case '=':   return &Operator{ "==", 90, eq }
            case '>':   return &Operator{ "=>", 90, ge }
            case '<':   return &Operator{ "<=", 90, le }
            default:    r.UnreadRune()
            }
        }
        return &Operator{ c, 160, def }
    case '<'				:
        w, _, err := r.ReadRune()
        if err == nil {
            switch w {
            case '>':   return &Operator{ "<>", 90, neq }
            case '=':   return &Operator{ "<=", 80, le }
            default:    r.UnreadRune()
            }
        }
        return &Operator{ "<", 80, lt }
    case '>'				:
        w, _, err := r.ReadRune()
        if err == nil {
            switch w {
            case '<':   return &Operator{ "><", 90, neq }
            case '=':   return &Operator{ ">=", 80, ge }
            default:    r.UnreadRune()
            }
        }
        return &Operator{ ">", 80, gt }
    case '?'				:   return If
    case '¿'				:   return Fi
    case '#'				:   return Size
    case '$'				:   return Last
    case '~'				:   return Not
	case '"'				:	return &String{ readString( r ) }
    case '0', '1', '2', '3', '4', '5', '6', '7', '8', '9':
        r.UnreadRune()
        return &Number{ readNumber( r ) }
	default					:
		r.UnreadRune()
		return &Name{ readName( r ) }
	}
}

func
readObjects( r *bufio.Reader, terminator rune ) []Object {
	v := []Object{}
	for {
		w := read( r, terminator )
		if w == nil { break }
		v = append( v, w )
	}
	return v
}

func
main() {
	defer func() {
		r := recover();
		switch t := r.(type) {
		case error:
			if t != io.EOF { log.Printf( "ERROR: %T:%v\n", t, t ) }
		default:
			log.Printf( "%v\n", t )
		}
	}()
	wReader := bufio.NewReader( os.Stdin )
	for { fmt.Printf( "%v\n", ( &Sentence{ readObjects( wReader, ';' ) } ).Eval() ) }
}

