#pragma once

#ifdef __cplusplus
extern "C" {
#endif
//#include "tolua++.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#ifdef __cplusplus
}
#endif

int  register_pbc_module(lua_State* L);