
#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "util.h"

void laf_newmodule(lua_State *L, const char *modname, int openidx)
{
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_pushvalue(L, openidx < 0 ? openidx - 2 : openidx);
    lua_setfield(L, -2, modname);
    lua_pop(L, 2);
}

void laf_newmod(lua_State *L, const char *modname, lua_CFunction open)
{
    lua_pushcfunction(L, open);
    laf_newmodule(L, modname, -1);
    lua_pop(L, 1);
}

void laf_newmodclosure(lua_State *L, const char *modname,
                       lua_CFunction open, int nup)
{
    lua_pushcclosure(L, open, nup);
    laf_newmodule(L, modname, -1);
    lua_pop(L, 1);
}


int laf_msgh(lua_State *L)
{
    luaL_traceback(L, L, lua_tostring(L, 1), 0);
    return 1;
}

int laf_pcall(lua_State *L, int nargs, int nret)
{
    lua_pushcfunction(L, laf_msgh);
    lua_insert(L, 1);
    int status = lua_pcall(L, nargs, nret, 1);
    lua_remove(L, 1);
    return status;
}

typedef struct {
    lua_KContext ctx;
    lua_KFunction k;
} laf_msgh_ctx_t;

static int laf_pcallk_k(lua_State *L, int status, lua_KContext ctx);

int laf_pcallk(lua_State *L, int nargs, int nret,
               lua_KContext ctx, lua_KFunction k)
{
    laf_msgh_ctx_t *lctx = malloc(sizeof (laf_msgh_ctx_t));
    lctx->ctx = ctx;
    lctx->k = k;

    lua_pushcfunction(L, laf_msgh);
    lua_insert(L, 1);
    int status = lua_pcallk(L, nargs, nret, 1, (lua_KContext) lctx, laf_pcallk_k);
    lua_remove(L, 1);
    return status;
}

static int laf_pcallk_k(lua_State *L, int status, lua_KContext ctx)
{
    lua_remove(L, 1);
    laf_msgh_ctx_t *lctx = (laf_msgh_ctx_t *) ctx;
    ctx = lctx->ctx;
    lua_KFunction k = lctx->k;
    free(lctx);
    return k(L, status, ctx);
}
