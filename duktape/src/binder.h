#ifndef __BINDER_H__
#define __BINDER_H__

#include <dmsdk/sdk.h>
#include "duktape.h"

#define MODULE_ID_LIMIT 256

void duk_dump_obj(duk_context *ctx, duk_idx_t idx);
void duk_dump_err(duk_context *ctx, duk_idx_t idx);

// Convert and push duk value at idx onto lua stack
void lua_pushduk(lua_State *L, duk_context *ctx, duk_idx_t idx);

// Convert and push lua value at idx onto duk stack
void duk_push_lua(duk_context *ctx, lua_State *L, int idx);

#endif // __BINDER_H__