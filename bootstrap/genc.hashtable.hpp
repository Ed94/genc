#pragma once

#define GEN_DEFINE_LIBRARY_CODE_CONSTANTS
#define GEN_ENFORCE_STRONG_CODE_TYPES
#define GEN_EXPOSE_BACKEND
#include "gen.hpp"

using namespace gen;

CodeBody gen_array_base()
{

};

CodeBody gen_array()
{
	return CodeInvalid;
};
