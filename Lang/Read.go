package main

import	"bufio"
import	"fmt"

////////////////////////////////////////////////////////////////	Read

func
readInt( r *bufio.Reader ) int {
	v := 0
	for {
		c, _, err := r.ReadRune()
		if err != nil { break }
		if '0' <= c && c <= '9' { 
			v = v * 10 + int( c ) - int( '0' )
		} else {
			r.UnreadRune()
			break 
		}
	}
	return v
}

func
nameRuneP( p rune ) bool {
	if	'0' <= p && p <= '9' { return true }
	if	'A' <= p && p <= 'Z' { return true }
	if	'a' <= p && p <= 'z' { return true }
	if	'_' == p { return true }
	return false
}

func
ReadName( r *bufio.Reader ) *Name {
	v := []rune{}
	for {
		c, _, err := r.ReadRune()
		if err != nil { break }
		if nameRuneP( c ) {
			v = append( v, c )
		} else {
			r.UnreadRune()
			break
		}
	}
	return &Name{ string( v ) }
}

func
ReadString( r *bufio.Reader ) *String {
	v := []rune{}
	escaped := false
	for {
		c, _, err := r.ReadRune()
		if err != nil { break }
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
	return &String{ string( v ) }
}

func
skipWhite( r *bufio.Reader, terminator rune ) bool {
	for {
		c, _, err := r.ReadRune()
		if err != nil { panic( err ) }
		if c == terminator { return false }
		switch c { case ';', '}', ']', ')': panic( fmt.Sprintf( "Unexpected closing: '%c'", c ) ) }
		if c > ' ' { break }
	}
	r.UnreadRune()
	return true
}

func
object( r *bufio.Reader, terminator rune ) Object {
	if !skipWhite( r, terminator ) { return nil }
	c, _, _ := r.ReadRune()
	switch c {
	case '!':	return &Primitive{ c, func() Object { return sStack.u } }
	case '@':	return &Primitive{ c, func() Object { return car( sStack.u ) } }
	case ':':	return &Primitive{ c, func() Object { return cdr( sStack.u ) } }
	case '¡':	return &Primitive{ c, func() Object { return &Map{ sAssocList.u.u } } }
	case '\'':
		w := object( r, terminator )
		if w == nil { panic( "Nothing to Quote." ) }
		return &Quote{ w }
	case '`':
		w := object( r, terminator )
		if w == nil { panic( "Nothing to combine." ) }
		return &Combiner{ w }
	case '&':	return &Operator{ c, 100, and }
	case '^':	return &Operator{ c, 110, xor }
	case '|':	return &Operator{ c, 120, or }
	case '*':	return &Operator{ c, 50, mul }
	case '/':	return &Operator{ c, 50, div }
	case '%':	return &Operator{ c, 50, remainder }
	case '+':
		w, _, err := r.ReadRune()
		if err == nil {
			r.UnreadRune()
			if '0' <= w && w <= '9' { return &Number{ readInt( r ) } }
		}
		return &Operator{ c, 60, add }
	case '-':
		w, _, err := r.ReadRune()
		if err == nil {
			r.UnreadRune()
			if '0' <= w && w <= '9' { return &Number{ - readInt( r ) } }
		}
		return &Operator{ c, 60, sub }
	case '.':	return &Operator{ c, 20, apply }
	case ',':	return &Operator{ c, 155, concat }
	case '=':
		w, _, err := r.ReadRune()
		if err == nil {
			switch w {
			case '=':	return &Operator{ "==", 90, eq }
			case '>':	return &Operator{ "=>", 90, ge }
			case '<':	return &Operator{ "<=", 90, le }
			default:	r.UnreadRune()
			}
		}
		return &Operator{ c, 160, def }
	case '<':
		w, _, err := r.ReadRune()
		if err == nil {
			switch w {
			case '>':	return &Operator{ "<>", 90, neq }
			case '=':	return &Operator{ "<=", 80, le }
			default:	r.UnreadRune()
			}
		}
		return &Operator{ "<", 80, lt }
	case '>':
		w, _, err := r.ReadRune()
		if err == nil {
			switch w {
			case '<':	return &Operator{ "><", 90, neq }
			case '=':	return &Operator{ ">=", 80, ge }
			default:	r.UnreadRune()
			}
		}
		return &Operator{ ">", 80, gt }
	case '0', '1', '2', '3', '4', '5', '6', '7', '8', '9':
		r.UnreadRune()
		return &Number{ readInt( r ) }
	case '"':	return ReadString( r )
	case '[':	return &Slice		{ objects( r, ']' ) }
	case '(':	return &Sentence	{ objects( r, ')' ) }
	case '{':	return &Block		{ sentences( r, '}' ) }
	case '?':	return If
	case '¿':	return Fi
	case '#':	return Size
	case '$':	return Last
	case '~':	return Not
	default:
		r.UnreadRune()
		return ReadName( r )
	}
}

func
objects( r *bufio.Reader, terminator rune ) []Object {
	v := []Object{}
	for {
		w := object( r, terminator )
		if w == nil { break }
		v = append( v, w )
	}
	return v
}

func
sentences( r *bufio.Reader, terminator rune ) []*Sentence {
	v := []*Sentence{}
	for skipWhite( r, terminator ) { 
		v = append( v,  Read( r ) )
	}
	return v
}

func
Read( r *bufio.Reader ) *Sentence {
	return &Sentence{ objects( r, ';' ) }
}


