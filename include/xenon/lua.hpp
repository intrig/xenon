#pragma once

// lua.hpp
// Lua header files for C++
// <<extern "C">> not supplied automatically because Lua also compiles as C++

#define _CRT_SECURE_NO_WARNINGS // Added by MAB for winblows

namespace xenon {
    namespace lua {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

    }

}
