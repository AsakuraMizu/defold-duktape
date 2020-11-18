#include "Engine.h"
#include "binder.h"
#include "JSObject.h"

namespace Engine
{
    duk_context *_ctx;
    lua_State *_L;
    static uint32_t lastId = 0;

    void init(lua_State *L)
    {
        _ctx = duk_create_heap_default();
        _L = L;

        duk_push_global_object(_ctx);

        // require
        duk_push_string(_ctx, "require");
        duk_push_bare_object(_ctx);
        create_require(_ctx);
        duk_def_prop(_ctx, 0, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WC);

        // lua
        duk_push_string(_ctx, "lua");
        lua_pushvalue(L, LUA_GLOBALSINDEX);
        duk_push_lua(_ctx, L, -1);
        lua_pop(L, 1);
        duk_def_prop(_ctx, 0, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WC);

        duk_push_global_stash(_ctx);

        // instances
        duk_push_bare_object(_ctx);
        duk_put_prop_string(_ctx, -2, "__instances");

        // jsobject registry
        duk_create_jsobject_registry(_ctx);

        duk_pop_2(_ctx);
    }

    Script *load(lua_State *L)
    {
        const char *filename;
        char evalname[MODULE_ID_LIMIT];
        duk_int_t pcall_rc;
        Script *script;

        filename = luaL_checkstring(L, 1);
        script = new Script;
        script->id = ++lastId;

        duk_push_string(_ctx,
            "(function (filename) {\n"
            "    const script = require(filename);\n"
            "    const instance = typeof script === 'function' ? new script : {};\n"
            "    const funcs = ['init', 'final', 'update', 'on_message', 'on_input', 'on_reload'];\n"
            "    funcs.forEach(function (name) {\n"
            "        instance[name] = typeof script[name] === 'function' ? script[name] : function () { };\n"
            "    });\n"
            "    return instance;\n"
            "})\n"
        );

        snprintf(evalname, sizeof(evalname), "<script:%s>", filename);
        duk_push_string(_ctx, evalname);

        pcall_rc = duk_pcompile(_ctx, DUK_COMPILE_EVAL);
        if (pcall_rc != DUK_EXEC_SUCCESS)
        {
            goto failed;
        }
        pcall_rc = duk_pcall(_ctx, 0);
        if (pcall_rc != DUK_EXEC_SUCCESS)
        {
            goto failed;
        }

        duk_push_string(_ctx, filename);
        pcall_rc = duk_pcall(_ctx, 1);
        if (pcall_rc != DUK_EXEC_SUCCESS)
        {
            goto failed;
        }

        duk_push_global_stash(_ctx);
        duk_get_prop_string(_ctx, -1, "__instances");
        duk_pull(_ctx, -3);
        duk_put_prop_index(_ctx, -2, script->id);
        duk_pop_2(_ctx);

        return script;

    failed:
        duk_dump_err(_ctx, -1);
        delete script;
        return nullptr;
    }

    void push_instance(duk_context *ctx, uint32_t id)
    {
        duk_push_global_stash(ctx);
        duk_get_prop_string(ctx, -1, "__instances");
        duk_get_prop_index(ctx, -1, id);
        duk_insert(ctx, -3);
        duk_pop_2(ctx);
    }

    static duk_ret_t require_load(duk_context *ctx)
    {
        int ret;
        size_t len;
        const char *id, *buf;
        lua_State *L = _L;

        id = duk_require_lstring(ctx, 0, &len);
        lua_getglobal(L, "sys");
        lua_getfield(L, -1, "load_resource");
        lua_pushstring(L, id);
        ret = dmScript::PCall(L, 1, 1);
        if (ret == 0)
        {
            buf = lua_tolstring(L, -1, &len);
            duk_push_lstring(ctx, buf, len);
            lua_pop(L, 3);
            return 1;
        }
        else
        {
            duk_type_error(ctx, "cannot load module: %s", id);
            return 0;
        }
    }

    // https://github.com/svaarala/duktape/blob/83517c8ddaff79d859fc9ec3f11ae6a917d0660b/extras/module-duktape/duk_module_duktape.c#L31
    static duk_ret_t require_resolve(duk_context *ctx)
    {
        duk_uint8_t buf[MODULE_ID_LIMIT];
        duk_uint8_t *p, *q, *q_last;
        duk_int_t int_rc;
        const char *req_id, *mod_id;

        req_id = duk_require_string(ctx, 0);
        duk_push_this(ctx);
        duk_get_prop_string(ctx, -1, "id");
        mod_id = duk_get_string(ctx, -1);
        duk_pop_2(ctx);

        if (mod_id != nullptr && req_id[0] == '.')
        {
            int_rc = snprintf((char *)buf, sizeof(buf), "%s/../%s", mod_id, req_id);
        }
        else
        {
            int_rc = snprintf((char *)buf, sizeof(buf), "%s", req_id);
        }
        if (int_rc >= (duk_int_t)sizeof(buf) || int_rc < 0)
        {
            goto resolve_error;
        }

        p = q = buf;
        while (true)
        {
            duk_uint_fast8_t c;

            q_last = q;

            c = *p++;
            if (c == 0)
            {
                goto resolve_error;
            }
            else if (c == '.')
            {
                c = *p++;
                if (c == '/')
                {
                    goto eat_dup_slashes;
                }
                if (c == '.' && *p == '/')
                {
                    ++p;
                    if (q == buf)
                    {
                        goto resolve_error;
                    }
                    q--;
                    while (true)
                    {
                        if (q == buf || *(q - 1) == '/')
                        {
                            break;
                        }
                        q--;
                    }
                    goto eat_dup_slashes;
                }
                goto resolve_error;
            }
            else
            {
                while (true)
                {
                    *q++ = c;
                    c = *p++;
                    if (c == 0)
                    {
                        goto loop_done;
                    }
                    else if (c == '/')
                    {
                        *q++ = '/';
                        break;
                    }
                }
            }

        eat_dup_slashes:
            while (true)
            {
                c = *p;
                if (c != '/')
                {
                    break;
                }
                p++;
            }
        }

    loop_done:
        duk_push_lstring(ctx, (const char *)buf, (size_t)(q - buf));
        return 1;

    resolve_error:
        duk_type_error(ctx, "cannot resolve module id: %s", (const char *)req_id);
    }

    static duk_ret_t require(duk_context *ctx)
    {
        const char *req_id;
        duk_int_t pcall_rc;

        req_id = duk_require_string(ctx, 0);
        duk_push_current_function(ctx);
        duk_push_string(ctx, "resolve");
        duk_dup(ctx, 0);
        duk_call_prop(ctx, -3, 1);

        // [ req_id require resolved_id ]

        duk_get_prop_string(ctx, -2, "cache");
        duk_require_type_mask(ctx, -1, DUK_TYPE_MASK_OBJECT);
        duk_dup(ctx, -2);
        if (duk_get_prop(ctx, -2))
        {
            duk_get_prop_string(ctx, -1, "exports");
            return 1;
        }
        duk_pop(ctx);

        // [ req_id require resolved_id cache ]

        duk_dup_top(ctx);
        create_require(ctx);

        duk_push_string(ctx, "id");
        duk_dup(ctx, -4);
        duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_CONFIGURABLE);

        // [ req_id require resolved_id cache fresh_require ]

        duk_push_object(ctx); // exports
        duk_push_object(ctx); // module

        duk_push_string(ctx, "exports");
        duk_dup(ctx, -3);
        duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);

        duk_push_string(ctx, "id");
        duk_dup(ctx, -6);
        duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE);

        duk_compact(ctx, -1);

        // [ req_id require resolved_id cache fresh_require exports module ]
        duk_dup(ctx, -5);
        duk_dup(ctx, -2);
        duk_put_prop(ctx, -6);

        duk_push_string(ctx, "(function(require,exports,module){");

        duk_push_string(ctx, "load");
        duk_dup(ctx, -7);
        pcall_rc = duk_pcall_prop(ctx, -9, 1);
        if (pcall_rc != DUK_EXEC_SUCCESS)
        {
            goto delete_rethrow;
        }

        duk_push_string(ctx, "\n})");
        duk_concat(ctx, 3);
        duk_dup(ctx, -6);
        pcall_rc = duk_pcompile(ctx, DUK_COMPILE_EVAL);
        if (pcall_rc != DUK_EXEC_SUCCESS)
        {
            goto delete_rethrow;
        }
        pcall_rc = duk_pcall(ctx, 0);
        if (pcall_rc != DUK_EXEC_SUCCESS)
        {
            goto delete_rethrow;
        }

        // [ req_id require resolved_id cache fresh_require exports module mod_func ]

        duk_push_string(ctx, "name");
        duk_dup(ctx, -7);
        duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_FORCE);

        duk_dup(ctx, -3);
        duk_dup(ctx, -5);
        duk_dup(ctx, -2);
        duk_dup(ctx, -5);

        // [ req_id require resolved_id cache fresh_require exports module mod_func exports fresh_require exports module ]

        pcall_rc = duk_pcall_method(ctx, 3);
        if (pcall_rc != DUK_EXEC_SUCCESS)
        {
            goto delete_rethrow;
        }
        duk_pop(ctx);

        // [ req_id require resolved_id cache fresh_require exports module ]

        duk_get_prop_string(ctx, -1, "exports");
        duk_compact(ctx, -1);
        return 1;

    delete_rethrow:
        duk_dup(ctx, 2);
        duk_del_prop(ctx, 3);
        duk_throw(ctx);
        return 0;
    }

    void create_require(duk_context *ctx)
    {
        duk_require_object(ctx, -1);    // require.cache
        duk_push_c_function(ctx, require, 1);

        duk_push_string(ctx, "name");
        duk_push_string(ctx, "require");
        duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE);

        duk_push_string(ctx, "resolve");
        duk_push_c_function(ctx, require_resolve, 1);
        duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_C);

        duk_push_string(ctx, "load");
        duk_push_c_function(ctx, require_load, 1);
        duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_C);

        duk_push_string(ctx, "cache");
        duk_pull(ctx, -3);
        duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WC);
    }
} // namespace Engine