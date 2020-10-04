#define LIB_NAME "duktape"

#include <dmsdk/sdk.h>
#include "Engine.h"

static int load(lua_State *L)
{
    Script *script = Engine::getInstance()->load(L);
    if (script)
    {
        script->push(L);
        return 1;
    }
    else
    {
        return 0;
    }
}

static const luaL_reg Module_methods[] =
{
    {"load", load},
    {0, 0}
};

static void LuaInit(lua_State* L)
{
    int top = lua_gettop(L);

    luaL_register(L, LIB_NAME, Module_methods);

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

dmExtension::Result AppInitializeDuktape(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeDuktape(dmExtension::Params* params)
{
    LuaInit(params->m_L);
    return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalizeDuktape(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizeDuktape(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result OnUpdateDuktape(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(duktape, LIB_NAME, AppInitializeDuktape, AppFinalizeDuktape, InitializeDuktape, OnUpdateDuktape, 0, FinalizeDuktape)