#ifndef __JS_OBJECT_H__
#define __JS_OBJECT_H__

#include <dmsdk/sdk.h>
#include "duktape.h"

void duk_create_jsobject_registry(duk_context *ctx);

// Check if lua value at idx is a JSObject
bool lua_checkjsobject(lua_State *L, int idx);

// Convert lua value at idx to JSObject and push onto duk stack
void duk_push_js_object(duk_context *ctx, lua_State *L, int idx);

// Push JSObject at idx onto lua stack
void lua_pushjsobject(lua_State *L, duk_context *ctx, duk_idx_t idx);

#endif // __JS_OBJECT_H__