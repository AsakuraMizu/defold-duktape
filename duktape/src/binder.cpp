#include "binder.h"
#include "duk_module_duktape.h"
#include "LuaObject.h"

#define LUA_STATE_NAME "__lua_state"

void duk_dump_obj(duk_context *ctx, duk_idx_t idx)
{
    duk_dup(ctx, idx);
    dmLogInfo("%s", duk_safe_to_string(ctx, -1));
    duk_pop(ctx);
}

void duk_dump_err(duk_context *ctx, duk_idx_t idx)
{
    duk_dup(ctx, idx);
    dmLogError("%s", duk_safe_to_string(ctx, -1));
    if (duk_is_error(ctx, -1))
    {
        duk_get_prop_string(ctx, -1, "stack");
        dmLogError("%s", duk_safe_to_string(ctx, -1));
        duk_pop(ctx);
    }
    duk_pop(ctx);
}

lua_State *duk_require_lua_state(duk_context *ctx)
{
    lua_State *L;

    duk_push_global_stash(ctx);
    duk_get_prop_string(ctx, -1, LUA_STATE_NAME);
    if (!duk_is_pointer(ctx, -1))
        duk_type_error(ctx, "could not get lua state");
    L = (lua_State *)duk_get_pointer(ctx, -1);
    duk_pop_2(ctx);
    return L;
}

void lua_pushduk(duk_context *ctx, duk_idx_t idx)
{
    lua_State *L;
    union
    {
        bool bool_val;
        double double_val;
        struct
        {
            size_t len;
            const char *str_val;
        };
    };

    L = duk_require_lua_state(ctx);
    switch (duk_get_type(ctx, idx))
    {
    case DUK_TYPE_BOOLEAN:
    {
        bool_val = duk_get_boolean(ctx, idx);
        lua_pushboolean(L, bool_val);
        break;
    }

    case DUK_TYPE_NUMBER:
    {
        double_val = duk_get_number(ctx, idx);
        lua_pushnumber(L, double_val);
        break;
    }

    case DUK_TYPE_STRING:
    {
        str_val = duk_get_lstring(ctx, idx, &len);
        lua_pushlstring(L, str_val, len);
        break;
    }

    case DUK_TYPE_OBJECT:
    {
        if (duk_check_lua_object(ctx, idx))
        {
            lua_pushluaobject(ctx, idx);
        }
        else
        {
            duk_enum(ctx, idx, 0);
            lua_newtable(L);
            while (duk_next(ctx, -1, true))
            {
                lua_pushduk(ctx, -2);
                lua_pushduk(ctx, -1);
                lua_settable(L, -3);
                duk_pop_2(ctx);
            }
            duk_pop(ctx);
        }
        break;
    }
    
    default:
    {
        lua_pushnil(L);
        break;
    }
    }
}

void duk_push_lua(duk_context *ctx, int idx)
{
    lua_State *L = duk_require_lua_state(ctx);
    union
    {
        bool bool_val;
        double double_val;
        struct
        {
            size_t len;
            const char *str_val;
        };
    };

    switch (lua_type(L, idx))
    {
    case LUA_TNIL:
        duk_push_undefined(ctx);
        break;

    case LUA_TBOOLEAN:
        bool_val = lua_toboolean(L, idx);
        duk_push_boolean(ctx, bool_val);
        break;

    case LUA_TNUMBER:
        double_val = lua_tonumber(L, idx);
        duk_push_number(ctx, double_val);
        break;

    case LUA_TSTRING:
        str_val = lua_tolstring(L, idx, &len);
        duk_push_lstring(ctx, str_val, len);
        break;

    default:
        duk_push_lua_object(ctx, idx);
        break;
    }
}

static duk_ret_t mod_search(duk_context *ctx)
{
    int ret;
    size_t len;
    const char *id;
    char *buf;
    lua_State *L;
    
    L = duk_require_lua_state(ctx);
    id = duk_require_lstring(ctx, 0, &len);
    if (!strncmp(id, "res/", 4))
    {
        lua_getglobal(L, "sys");
        lua_getfield(L, -1, "load_resource");
        lua_pushstring(L, id + 3);
        ret = dmScript::PCall(L, 1, 1);
        if (ret == 0)
        {
            buf = (char *)lua_tolstring(L, -1, &len);
            duk_push_lstring(ctx, buf, len);
            lua_pop(L, 1);
        }
        lua_pop(L, 2);
        return ret == 0;
    }
    return 0;
}

void duk_init_lua_binder(duk_context *ctx, lua_State *L)
{
    // bind lua state
    duk_push_global_stash(ctx);
    duk_push_pointer(ctx, L);
    duk_put_prop_string(ctx, -2, LUA_STATE_NAME);
    duk_pop(ctx);

    // module loading
    duk_module_duktape_init(ctx);
    duk_get_global_string(ctx, "Duktape");
    duk_push_c_function(ctx, mod_search, 4);
    duk_put_prop_string(ctx, -2, "modSearch");
    duk_pop(ctx);

    // lua global table
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    duk_push_lua(ctx, -1);
    duk_put_global_string(ctx, "lua");
    lua_pop(L, 1);
}