#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <dmsdk/sdk.h>
#include "Script.h"

namespace Engine
{
    extern duk_context *_ctx;
    extern lua_State *_L;

    void init(lua_State *L);
    Script *load(lua_State *L);

    void push_instance(duk_context *ctx, uint32_t id);

    void create_require(duk_context *ctx);
};

#endif // __ENGINE_H__