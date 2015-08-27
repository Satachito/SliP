package main

import	"fmt"

func
arrayString( p []Object ) string {
	v := " ";
	for _, w := range( p ) {
		v = fmt.Sprintf( "%v%v ", v, w )
	}
	return v
}
func
arrayString2( p []*Sentence ) string {
	v := " ";
	for _, w := range( p ) {
		v = fmt.Sprintf( "%v%v ", v, w )
	}
	return v
}

func
tagString( p interface{} ) string {
	switch t := p.(type) {
	case byte:		return fmt.Sprintf( "%c", t )
	case rune:		return fmt.Sprintf( "%c", t )
	case string:	return t
	default:		return fmt.Sprintf( "%v", t )
	}
}

func	( p *Map )			String() string  { return fmt.Sprintf( "<%v>", p.u ) }
func	( p *Quote )		String() string  { return fmt.Sprintf( "'%v", p.u ) }
func	( p *Combiner )		String() string  { return fmt.Sprintf( "`%v", p.u ) }
func	( p *EvalAssoc )	String() string  { return fmt.Sprintf( "<%v:%v>", p.mapList, p.u ) }
func	( p *Number )		String() string  { return fmt.Sprintf( "%v", p.u ) }
func	( p *Name )			String() string  { return fmt.Sprintf( "%v", p.u ) }
func	( p *String )		String() string  { return fmt.Sprintf( "\"%v\"", p.u ) }
func	( p *Primitive )	String() string  { return fmt.Sprintf( "P_%v", tagString( p.tag ) ) }
func	( p *Builtin )		String() string  { return fmt.Sprintf( "B_%v", tagString( p.tag ) ) }
func	( p *Operator )		String() string  { return fmt.Sprintf( "O_%v", tagString( p.tag ) ) }
func	( p *Slice )		String() string  { return fmt.Sprintf( "[%v]", arrayString( p.u ) ) }
func	( p *Block )		String() string  { return fmt.Sprintf( "{%v}", arrayString2( p.u ) ) }
func	( p *Sentence )		String() string  { return fmt.Sprintf( "(%v)", arrayString( p.u ) ) }

func
Print( p Object ) Object {
	fmt.Printf( "%v", p )
	return p
}

func
Println( p Object ) Object {
	fmt.Printf( "%v\n", p )
	return p
}

