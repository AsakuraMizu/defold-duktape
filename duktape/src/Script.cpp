#include "Script.h"
#include "binder.h"

static int call(lua_State *L)
{
    int top, i, err;
    const char *name;
    Script *script;
    duk_context *ctx;

    name = luaL_checkstring(L, -1);
    if (!lua_istable(L, 1))
    {
        goto type_error;
    }
    lua_getfield(L, 1, "__userdata");
    script = (Script *)lua_touserdata(L, -1);
    if (!script)
    {
        goto type_error;
    }
    ctx = script->ctx;
    lua_pop(L, 2);

    duk_get_global_string(ctx, "instance");
    duk_push_string(ctx, name);
    top = lua_gettop(L);
    for (i = 2; i <= top; ++i)
    {
        duk_push_lua(script->ctx, i);
    }
    err = duk_pcall_prop(ctx, -(top + 1), top - 1);
    if (err)
    {
        duk_dump_err(ctx, -1);
    }
    lua_pushduk(ctx, -1);
    duk_pop_2(ctx);
    return err == 0;

type_error:
    luaL_typerror(L, 1, "duktape.Script");
}

static int __init(lua_State *L)
{
    lua_pushstring(L, "init");
    return call(L);
}

static int __final(lua_State *L)
{
    lua_pushstring(L, "final");
    return call(L);
}

static int __update(lua_State *L)
{
    lua_pushstring(L, "update");
    return call(L);
}

static int __on_message(lua_State *L)
{
    lua_pushstring(L, "on_message");
    return call(L);
}

static int __on_input(lua_State *L)
{
    lua_pushstring(L, "on_input");
    return call(L);
}

static int __on_reload(lua_State *L)
{
    lua_pushstring(L, "on_reload");
    return call(L);
}

void Script::push(lua_State *L)
{
    lua_newtable(L);

    lua_pushlightuserdata(L, this);
    lua_setfield(L, -2, "__userdata");
    lua_pushcfunction(L, __init);
    lua_setfield(L, -2, "init");
    lua_pushcfunction(L, __final);
    lua_setfield(L, -2, "final");
    lua_pushcfunction(L, __update);
    lua_setfield(L, -2, "update");
    lua_pushcfunction(L, __on_message);
    lua_setfield(L, -2, "on_message");
    lua_pushcfunction(L, __on_input);
    lua_setfield(L, -2, "on_input");
    lua_pushcfunction(L, __on_reload);
    lua_setfield(L, -2, "on_reload");
}