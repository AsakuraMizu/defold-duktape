#ifndef __LUA_OBJECT_H__
#define __LUA_OBJECT_H__

#include <dmsdk/sdk.h>
#include "duktape.h"

// Check if duk value at idx is a LuaObject
bool duk_check_lua_object(duk_context *ctx, duk_idx_t idx);

// Push LuaObject at idx onto lua stack
void lua_pushluaobject(lua_State *L, duk_context *ctx, duk_idx_t idx);

// Convert lua value at idx to LuaObject and push onto duk stack
void duk_push_lua_object(duk_context *ctx, lua_State *L, int idx);

#endif // __LUA_OBJECT_H__