#ifndef __CALLBACK_BINDER_H__
#define __CALLBACK_BINDER_H__

#include "duktape.h"

// Convert a duk function at idx to lua callback and push onto stack
void lua_pushdukcallback(duk_context *ctx, duk_idx_t idx);

#endif // __CALLBACK_BINDER_H__