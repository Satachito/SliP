package main

import	"fmt"
import	"sync"

func
evalParallel( p []Object ) []Object {
/*
	fmt.Printf( ">evalParallel:<%v>\n", arrayString( p ) )
	v := _evalParallel( p )
	fmt.Printf( "<evalParallel:<%v>\n", arrayString( v ) )
	return v
}

func
_evalParallel( p []Object ) []Object {
*/
	v := make( []Object, len( p ) )
	var wg sync.WaitGroup
	for i, o := range p {
		wg.Add( 1 )
		go func( i int, o Object ) {
			defer wg.Done()
			v[ i ] = o.Eval()
		} ( i, o )
	}
	wg.Wait()
	return v
}

func
evalSentence( p []Object ) Object {
/*
	fmt.Printf( ">evalSentence:<%v>\n", arrayString( p ) )
	v := _evalSentence( p )
	fmt.Printf( "<evalSentence:< %v >\n", v )
	return v
}

func
_evalSentence( p []Object ) Object {
*/
	switch len( p ) {
	case	0, 2:	panic( "Syntax Error" )
	case	   1:	return p[ 0 ]
	case	   3:	return p[ 1 ].(*Operator).u( p[ 0 ], p[ 2 ] )
	}
	var	target	*Operator = nil
	var	index	= 0
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

func	( p *Map )			Eval() Object {
	v := map[ interface{} ]Object{}
	for key, value := range( p.u ) { v[ key ] = value.Eval() }
	return &Map{ v }
}

func	( p *Number )		Eval() Object { return p }
func	( p *String )		Eval() Object { return p }
func	( p *Builtin )		Eval() Object { return p }
func	( p *Operator )		Eval() Object { return p }
func	( p *Quote )		Eval() Object { return p.u }
func	( p *Primitive )	Eval() Object { return p.u() }
func	( p *Name )			Eval() Object { return assoc( p ) }
func	( p *Slice )		Eval() Object {
//	fmt.Printf( "Slice.Eval:%v\n", p )
	return &Slice{ evalParallel( p.u ) }
}
func	( p *Sentence )		Eval() Object {
//	fmt.Printf( "Sentence.Eval:%v\n", p )
	return evalSentence( evalParallel( p.u ) )
}
func	( p *Block )		Eval() Object {
//	fmt.Printf( "Block.Eval:%v\n", p )
	begin()
	defer end()
	v := make( []Object, len( p.u ) )
	for i, w := range p.u { v[ i ] = w.Eval() }
	return &Slice{ v }
}
func	( p *Combiner )		Eval() Object {
	return &EvalAssoc{ sAssocList.u, p.u }
}
func	( p *EvalAssoc )	Eval() Object {
	sAssocList = &assocList{ p.mapList, sAssocList }
	defer func() { sAssocList = sAssocList.next }()
	return p.u.Eval()
}

