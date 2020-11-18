#include "Script.h"
#include "binder.h"
#include "Engine.h"

namespace dmScript
{
    uint32_t GetUserType(lua_State *L, int user_data_index);
}

static void hack(lua_State *L)
{
    lua_State *tL = Engine::_L;

    dmScript::GetInstance(L);
    int reference = dmScript::Ref(L, LUA_REGISTRYINDEX);
    lua_rawgeti(tL, LUA_REGISTRYINDEX, reference);
    dmScript::SetInstance(tL);
    dmScript::Unref(L, LUA_REGISTRYINDEX, reference);
}

static int call(lua_State *L)
{
    int top, i, pcall_rc, nret = 0;
    const char *name;
    void *instance;
    Script *script;
    duk_context *ctx = Engine::_ctx;

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
    lua_pop(L, 2);

    Engine::push_instance(ctx, script->id);
    duk_push_string(ctx, name);
    top = lua_gettop(L);
    for (i = 2; i <= top; ++i)
    {
        duk_push_lua(ctx, L, i);
    }

    hack(L);

    pcall_rc = duk_pcall_prop(ctx, -(top + 1), top - 1);
    if (pcall_rc != DUK_EXEC_SUCCESS)
    {
        duk_dump_err(ctx, -1);
    }
    else if (!duk_is_undefined(ctx, -1))
    {
        lua_pushduk(L, ctx, -1);
        nret = 1;
    }
    duk_pop_2(ctx);
    return nret;

type_error:
    luaL_typerror(L, 1, "duktape.Script");
    return 0;
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