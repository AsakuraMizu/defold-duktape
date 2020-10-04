#include "Engine.h"
#include "binder.h"

#define EVAL_NAME_LIMIT 256
#define CODE_LIMIT 1024

Engine *Engine::instance = nullptr;

Engine *Engine::getInstance()
{
    if (!instance)
    {
        instance = new Engine;
    }
    return instance;
}

Script *Engine::load(lua_State *L)
{
    int err;
    const char *filename;
    char code[CODE_LIMIT];
    char evalname[EVAL_NAME_LIMIT];
    Script *script;
    duk_context *ctx;

    filename = luaL_checkstring(L, 1);
    script = new Script;
    ctx = duk_create_heap_default();
    script->ctx = ctx;
    duk_init_lua_binder(ctx, L);

    snprintf(
        code, sizeof(code),
        "const script = require('%s');\n"
        "const names = ['init', 'final', 'update', 'on_message', 'on_input', 'on_reload'];\n"
        "var instance;\n"
        "if (typeof script === 'function') {\n"
            "instance = new script;\n"
        "} else {\n"
            "instance = {};\n"
            "names.forEach(function (name) {\n"
                "if (typeof script[name] === 'function') {\n"
                    "instance[name] = script[name];\n"
                "} else {\n"
                    "instance[name] = function() {};\n"
                "}\n"
            "});\n"
        "}\n"
        "globalThis.instance = instance;",
        filename
    );
    duk_push_string(ctx, code);

    snprintf(evalname, sizeof(evalname), "<script:%s>", filename);
    duk_push_string(ctx, evalname);

    err = duk_pcompile(ctx, 0);
    if (err)
    {
        duk_dump_err(ctx, -1);
        delete script;
        duk_destroy_heap(ctx);
        return nullptr;
    }

    err = duk_pcall(ctx, 0);
    if (err)
    {
        duk_dump_err(ctx, -1);
        delete script;
        duk_destroy_heap(ctx);
        return nullptr;
    }

    return script;
}