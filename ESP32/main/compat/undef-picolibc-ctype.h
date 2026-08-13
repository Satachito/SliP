#pragma once

//	Force-included ahead of every translation unit in this component.
//
//	picolibc's <ctype.h> defines eight single-letter macros — _U _L _N _S _P _C
//	_X _B — and only when compiling C++, with the comment "we need these legacy
//	symbols to build libstdc++".  They collide with two ordinary names in this
//	codebase: JP.h's error-checking helper _X, and the column index _C in
//	SliP.cpp's matrix comparison.  Neither is a reserved identifier at namespace
//	scope; the collision belongs to picolibc, not to them.
//
//	They cannot simply be undefined, and the order below is the whole point.
//	picolibc says the macros exist "to build libstdc++", which reads as though
//	only libstdc++'s own sources want them — but one libstdc++ *header* wants
//	them too.  bits/ctype_base.h bakes them into the ctype masks:
//
//	    static const mask alnum = _U | _L | _N;
//
//	and that header is reached from <iostream>, which JP.h includes.  So the
//	undefs have to come after it has been parsed, not before.  <locale> is what
//	pulls it in ( through bits/locale_facets.h ), and it is named here for that
//	reason alone.  Once parsed, the masks are constants and the macros are spent:
//	bits/ctype_base.h is the only header in the C++ tree that reads them, and the
//	classification macros in <ctype.h> — isalpha and friends — are defined only
//	#ifndef __cplusplus, and even there they read the __CTYPE_* constants these
//	eight are aliases of.  In C++ the classifiers are real functions and keep
//	working.
//
//	Both headers are included here so that their include guards are set: every
//	later inclusion, from anywhere in the build, is then a no-op, and these
//	undefs stand for the whole translation unit.

#include <ctype.h>
#include <locale>

#undef	_U
#undef	_L
#undef	_N
#undef	_S
#undef	_P
#undef	_C
#undef	_X
#undef	_B
