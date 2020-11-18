#include "JSObject.h"
#include "binder.h"
#include "Engine.h"

#define REFERENCE_NAME "__js_object_reference"
#define REGISTRY_NAME "__js_object_registry"

static uint32_t lastId = 0;

void duk_create_jsobject_registry(duk_context *ctx)
{
    duk_push_global_stash(ctx);
    duk_push_bare_object(ctx);
    duk_put_prop_string(ctx, -2, REGISTRY_NAME);
    duk_pop(ctx);
}

static int ref(duk_context *ctx)
{
    int id;

    duk_push_global_stash(ctx);
    duk_get_prop_string(ctx, -1, REGISTRY_NAME);
    duk_pull(ctx, -3);
    id = lastId++;
    duk_put_prop_index(ctx, -2, id);
    duk_pop_2(ctx);

    return id;
}

static void unref(duk_context *ctx, int id)
{
    duk_push_global_stash(ctx);
    duk_get_prop_string(ctx, -1, REGISTRY_NAME);
    duk_del_prop_index(ctx, -1, id);
    duk_pop_2(ctx);
}

static void reg_get(duk_context *ctx, int id)
{
    duk_push_global_stash(ctx);
    duk_get_prop_string(ctx, -1, REGISTRY_NAME);
    duk_get_prop_index(ctx, -1, id);
    duk_insert(ctx, -3);
    duk_pop_2(ctx);
}

bool lua_checkjsobject(lua_State *L, int idx)
{
    if (!lua_istable(L, idx))
    {
        return false;
    }
    lua_getfield(L, idx, REFERENCE_NAME);
    bool ret = lua_isnumber(L, -1);
    lua_pop(L, 1);
    return ret;
}

void duk_push_js_object(duk_context *ctx, lua_State *L, int idx)
{
    lua_getfield(L, idx, REFERENCE_NAME);
    int reference = lua_tointeger(L, -1);
    lua_pop(L, 1);
    reg_get(ctx, reference);
}

static int index(lua_State *L)
{
    duk_context *ctx = Engine::_ctx;
    duk_push_js_object(ctx, L, 0);
    duk_push_lua(ctx, L, 1);
    duk_get_prop(ctx, -2);
    lua_pushduk(L, ctx, -1);
    duk_pop_2(ctx);
    return 1;
}

static int newindex(lua_State *L)
{
    duk_context *ctx = Engine::_ctx;
    duk_push_js_object(ctx, L, 0);
    duk_push_lua(ctx, L, 1);
    duk_push_lua(ctx, L, 2);
    duk_put_prop(ctx, -3);
    duk_pop(ctx);
    return 0;
}

static int call(lua_State *L)
{
    int i, top, pcall_rc, nret = 0;
    duk_context *ctx = Engine::_ctx;

    top = lua_gettop(L);
    duk_push_js_object(ctx, L, -1);
    for (i = 2; i <= top; ++i)
    {
        duk_push_lua(ctx, L, i);
    }
    pcall_rc = duk_pcall(ctx, top - 1);
    if (pcall_rc != DUK_EXEC_SUCCESS)
    {
        duk_dump_err(ctx, -1);
    }
    else if (!duk_is_undefined(ctx, -1))
    {
        lua_pushduk(L, ctx, -1);
        nret = 1;
    }
    duk_pop(ctx);
    return nret;
}

static int gc(lua_State *L)
{
    int id;
    duk_context *ctx = Engine::_ctx;

    lua_getfield(L, 1, REFERENCE_NAME);
    id = lua_tointeger(L, -1);
    lua_pop(L, 1);
    unref(ctx, id);
    return 0;
}

void lua_pushjsobject(lua_State *L, duk_context *ctx, duk_idx_t idx)
{
    duk_dup(ctx, idx);
    int id = ref(ctx);

    lua_newtable(L);

    duk_enum(ctx, idx, 0);
    while (duk_next(ctx, -1, true))
    {
        lua_pushduk(L, ctx, -2);
        lua_pushduk(L, ctx, -1);
        lua_settable(L, -3);
        duk_pop_2(ctx);
    }
    duk_pop(ctx);

    lua_pushinteger(L, id);
    lua_setfield(L, -2, REFERENCE_NAME);

    lua_newtable(L);
    lua_pushcfunction(L, call);
    lua_setfield(L, -2, "__call");
    lua_pushcfunction(L, gc);
    lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, newindex);
    lua_setfield(L, -2, "__newindex");

    lua_setmetatable(L, -2);
}