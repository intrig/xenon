#pragma once

// lua.hpp
// Lua header files for C++
// <<extern "C">> not supplied automatically because Lua also compiles as C++

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS // Added by MAB for winblows
#endif

namespace xenon {
namespace lua {
#include "lauxlib.h"
#include "lualib.h"
#include <lua.h>
} // namespace lua
} // namespace xenon
