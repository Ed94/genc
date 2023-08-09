/*
	genc: An attempt at simple staged metaprogramming for C.

	See Readme.md for more information from the project repository.

	Public Address:
	https://github.com/Ed94/genc
*/
#if ! defined(GEN_DONT_ENFORCE_GEN_TIME_GUARD) && ! defined(GEN_TIME)
#	error gen.h : GEN_TIME not defined
#endif

//! If its desired to roll your own dependencies, define GEN_ROLL_OWN_DEPENDENCIES before including this file.
// Dependencies are derived from the c-zpl library: https://github.com/zpl-c/zpl
#ifndef GEN_ROLL_OWN_DEPENDENCIES
#	include "genc.dep.hpp"
#endif

