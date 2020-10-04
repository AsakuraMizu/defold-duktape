#ifndef __BINDER_H__
#define __BINDER_H__

#include <dmsdk/sdk.h>
#include "duktape.h"

void duk_dump_obj(duk_context *ctx, duk_idx_t idx);
void duk_dump_err(duk_context *ctx, duk_idx_t idx);

// Get binded lua state
lua_State *duk_require_lua_state(duk_context *ctx);

// Convert and push duk value at idx onto lua stack
void lua_pushduk(duk_context *ctx, duk_idx_t idx);

// Convert and push lua value at idx onto duk stack
void duk_push_lua(duk_context *ctx, int idx);

// Initialize
void duk_init_lua_binder(duk_context *ctx, lua_State *L);

#endif // __BINDER_H__