#include "CallbackBinder.h"
#include <dmsdk/sdk.h>
#include "binder.h"

#define CALLBACK_REGISTRY_NAME "__callback_registry"
#define REFERENCE_ID_NAME "__reference_id"
#define DUK_CONTEXT_NAME "__duk_context"

static int last_id;

/**
 * Note: These three functions are very UNSAFE but this feature(?) won't be fixed
 */

int duk_callback_ref(duk_context *ctx)
{
  int id;

  duk_push_global_stash(ctx);
  if (!duk_has_prop_string(ctx, -1, CALLBACK_REGISTRY_NAME))
  {
    duk_push_object(ctx);
    duk_put_prop_string(ctx, -2, CALLBACK_REGISTRY_NAME);
  }
  duk_get_prop_string(ctx, -1, CALLBACK_REGISTRY_NAME);
  duk_pull(ctx, -3);
  id = last_id++;
  duk_put_prop_index(ctx, -2, id);
  duk_pop_2(ctx);

  return id;
}

void duk_callback_unref(duk_context *ctx, int id)
{
  duk_push_global_stash(ctx);
  duk_get_prop_string(ctx, -1, CALLBACK_REGISTRY_NAME);
  duk_del_prop_index(ctx, -1, id);
  duk_pop_2(ctx);
}

void duk_callback_get(duk_context *ctx, int id)
{
  duk_push_global_stash(ctx);
  duk_get_prop_string(ctx, -1, CALLBACK_REGISTRY_NAME);
  duk_get_prop_index(ctx, -1, id);
  duk_insert(ctx, -3);
  duk_pop_2(ctx);
}

int lua_dukcallback(lua_State *L)
{
  int i, id, top, err;
  duk_context *ctx;

  lua_getfield(L, 1, REFERENCE_ID_NAME);
  id = lua_tointeger(L, -1);
  lua_pop(L, 1);

  lua_getfield(L, 1, DUK_CONTEXT_NAME);
  ctx = (duk_context *)lua_touserdata(L, -1);
  lua_pop(L, 1);

  top = lua_gettop(L);
  duk_callback_get(ctx, id);
  for (i = 2; i <= top; ++i)
  {
    duk_push_lua(ctx, i);
  }
  err = duk_pcall(ctx, top - 1);
  if (err)
  {
    duk_dump_err(ctx, -1);
    duk_pop_2(ctx);
    luaL_error(L, "callback failed");
    return 0;
  }
  lua_pushduk(ctx, -1);
  duk_pop_2(ctx);
  return 1;
}

int lua_dukcallback_gc(lua_State *L)
{
  int id;
  duk_context *ctx;

  lua_getfield(L, 1, REFERENCE_ID_NAME);
  id = lua_tointeger(L, -1);
  lua_pop(L, 1);

  lua_getfield(L, 1, DUK_CONTEXT_NAME);
  ctx = (duk_context *)lua_touserdata(L, -1);
  lua_pop(L, 1);

  duk_callback_unref(ctx, id);
  return 0;
}

void lua_pushdukcallback(duk_context *ctx, duk_idx_t idx)
{
  int id;
  lua_State *L;

  L = duk_require_lua_state(ctx);
  duk_dup(ctx, idx);
  id = duk_callback_ref(ctx);
  lua_newtable(L);
  lua_pushinteger(L, id);
  lua_setfield(L, -2, REFERENCE_ID_NAME);
  lua_pushlightuserdata(L, ctx);
  lua_setfield(L, -2, DUK_CONTEXT_NAME);

  lua_newtable(L);
  lua_pushcfunction(L, lua_dukcallback);
  lua_setfield(L, -2, "__call");
  lua_pushcfunction(L, lua_dukcallback_gc);
  lua_setfield(L, -2, "__gc");

  lua_setmetatable(L, -2);
}