#include "binder.h"
#include "JSObject.h"
#include "LuaObject.h"

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
    duk_pop(ctx);
    duk_get_prop_string(ctx, idx, "stack");
    if (!duk_is_undefined(ctx, -1))
    {
        dmLogError("%s", duk_safe_to_string(ctx, -1));
    }
    duk_pop(ctx);
}

void lua_pushduk(lua_State *L, duk_context *ctx, duk_idx_t idx)
{
    bool bool_val;
    double double_val;
    const char *str_val;
    size_t len;
    int i;

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
            lua_pushluaobject(L, ctx, idx);
        }
        else if (duk_is_array(ctx, idx))
        {
            lua_newtable(L);
            len = duk_get_length(ctx, idx);
            for (i = 0; i < len; ++i)
            {
                duk_get_prop_index(ctx, idx, i);
                lua_pushduk(L, ctx, -1);
                lua_rawseti(L, -2, i + 1);
            }
        }
        else
        {
            lua_pushjsobject(L, ctx, idx);
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

void duk_push_lua(duk_context *ctx, lua_State *L, int idx)
{
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
        if (lua_checkjsobject(L, idx))
        {
            duk_push_js_object(ctx, L, idx);
        }
        else
        {
            duk_push_lua_object(ctx, L, idx);
        }
        break;
    }
}