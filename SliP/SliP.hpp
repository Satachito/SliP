#pragma once

#include	"JP.h"

struct
Object;

struct
Context {
	map< string, Object* >*	mapP;
	Context*				next;
	Context( map< string, Object* >* $, Context* _ = 0 ) : mapP( $ ), next( _ ) {}
};

struct
SliP {
	Context*
	context = new Context(
		new map< string, Object* >()
	);
	
	vector< Object* >
	stack;
};

void
Interpret( SliP*, string const&, void (*)( string const& ) );

//#pragma GCC visibility push(hidden)
//#pragma GCC visibility pop
//#pragma GCC visibility push(default)
//#pragma GCC visibility pop

