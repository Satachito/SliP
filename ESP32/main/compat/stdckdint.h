#pragma once

//	<stdckdint.h> is a C23 header, and GCC guards its ckd_add / ckd_sub / ckd_mul
//	macros on __STDC_VERSION__ — which C++ never defines.  So the toolchain's own
//	copy, included from C++, declares nothing at all.
//
//	JP.h reaches for it on every host that is not Apple ( where it already
//	carries these three as templates ), and SliP.cpp's integer arithmetic needs
//	them to detect overflow before falling back to Float.  This shadows the
//	toolchain header with the same three operations for this component only:
//	"compat" is first in INCLUDE_DIRS, and nothing outside main includes it.

template< typename T > inline bool
ckd_add( T* $, T a, T b ) { return __builtin_add_overflow( a, b, $ ); }

template< typename T > inline bool
ckd_sub( T* $, T a, T b ) { return __builtin_sub_overflow( a, b, $ ); }

template< typename T > inline bool
ckd_mul( T* $, T a, T b ) { return __builtin_mul_overflow( a, b, $ ); }
