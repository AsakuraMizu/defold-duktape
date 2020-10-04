#include <dmsdk/sdk.h>
#include "binder.h"
#include "LuaObject.h"

#define REFERENCE_NAME "__lua_object_reference"

bool duk_check_lua_object(duk_context *ctx, duk_idx_t idx)
{
    bool ret = duk_get_prop_string(ctx, idx, REFERENCE_NAME);
    duk_pop(ctx);
    return ret;
}

void lua_pushluaobject(duk_context *ctx, duk_idx_t idx)
{
    lua_State *L;
    int reference;

    L = duk_require_lua_state(ctx);
    duk_get_prop_string(ctx, idx, REFERENCE_NAME);
    reference = duk_get_int(ctx, -1);
    duk_pop(ctx);
    lua_rawgeti(L, LUA_REGISTRYINDEX, reference);
}

static duk_ret_t get(duk_context *ctx)
{
    size_t len;
    const char *key;
    lua_State *L;
    int reference;

    key = duk_require_lstring(ctx, 1, &len);
    if (!strncmp(key, REFERENCE_NAME, len))
    {
        duk_get_prop_string(ctx, 0, REFERENCE_NAME);
        reference = duk_get_int(ctx, -1);
        duk_pop(ctx);
        duk_push_int(ctx, reference);
    }
    else
    {
        L = duk_require_lua_state(ctx);
        lua_pushluaobject(ctx, 0);
        lua_pushstring(L, key);
        lua_gettable(L, -2);
        duk_push_lua(ctx, -1);
        lua_pop(L, 2);
    }
    return 1;
}

static duk_ret_t set(duk_context *ctx)
{
    lua_State *L = duk_require_lua_state(ctx);
    lua_pushluaobject(ctx, 0);
    lua_pushduk(ctx, 1);
    lua_pushduk(ctx, 2);
    lua_settable(L, -3);
    lua_pop(L, 1);
    return 0;
}

static duk_ret_t has(duk_context *ctx)
{
    get(ctx);
    bool ret = duk_is_undefined(ctx, -1);
    duk_pop(ctx);
    duk_push_boolean(ctx, ret);
    return 1;
}

static duk_ret_t deleteProperty(duk_context *ctx)
{
    duk_push_undefined(ctx);
    set(ctx);
    return 0;
}

static duk_ret_t ownKeys(duk_context *ctx)
{
    lua_State *L = duk_require_lua_state(ctx);
    lua_pushluaobject(ctx, 0);
    lua_pushnil(L);
    duk_push_array(ctx);
    int cnt = 0;
    while (lua_next(L, -2))
    {
        lua_pop(L, 1);
        duk_push_lua(ctx, -1);
        duk_put_prop_index(ctx, -2, cnt++);
    }
    lua_pop(L, 1);
    return 1;
}

const duk_function_list_entry lua_object_proxy_funcs[] ={
    { "has", has, 2 },
    { "get", get, 2 },
    { "set", set, 3 },
    { "deleteProperty", deleteProperty, 2 },
    { "ownKeys", ownKeys, 1 },
    { nullptr, nullptr, 0 }
};

static duk_ret_t call(duk_context *ctx)
{
    int nret, top, i, err;
    lua_State *L;

    nret = duk_require_int(ctx, 0);
    top = duk_get_top(ctx);
    L = duk_require_lua_state(ctx);

    duk_push_current_function(ctx);
    lua_pushluaobject(ctx, -1);
    duk_pop(ctx);
    for (i = 1; i < top; ++i)
    {
        lua_pushduk(ctx, i);
    }

    err = dmScript::PCall(L, top - 1, nret);
    if (err)
    {
        return duk_type_error(ctx, "pcall failed");
    }

    if (nret == 1)
    {
        duk_push_lua(ctx, -1);
    }
    else if (nret > 1)
    {
        duk_push_array(ctx);
        for (i = 0; i < nret; ++i)
        {
            duk_push_lua(ctx, i - nret);
            duk_put_prop_index(ctx, -2, i);
        }
    }

    return nret > 0;
}

static duk_ret_t finalizer(duk_context *ctx)
{
    lua_State *L;
    int reference;

    L = duk_require_lua_state(ctx);
    duk_get_prop_string(ctx, 0, REFERENCE_NAME);
    reference = duk_get_int(ctx, -1);
    duk_pop(ctx);
    dmScript::Unref(L, LUA_REGISTRYINDEX, reference);
    return 0;
}

void duk_push_lua_object(duk_context *ctx, int idx)
{
    lua_State *L;
    int reference;

    L = duk_require_lua_state(ctx);
    lua_pushvalue(L, idx);
    reference = dmScript::Ref(L, LUA_REGISTRYINDEX);
    duk_push_c_function(ctx, call, DUK_VARARGS);    // [ target ]
    duk_push_c_function(ctx, finalizer, 2);         // [ taret, finalizer ]
    duk_set_finalizer(ctx, -2);                     // [ target ]
    duk_push_int(ctx, reference);                   // [ target, reference ]
    duk_put_prop_string(ctx, -2, REFERENCE_NAME);   // [ target ]
    duk_push_object(ctx);                           // [ target, handler ]
    duk_put_function_list(ctx, -1, lua_object_proxy_funcs);
    duk_push_proxy(ctx, 0);                         // [ proxy ]
}