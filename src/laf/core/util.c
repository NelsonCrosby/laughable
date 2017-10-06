
#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "class.h"
#include "util.h"


#if LUA_KCONTEXT != intptr_t
#error "laf_pcallhk requires that lua_KContext is an intptr_t"
#endif


lua_State *laf_newstate(lua_Alloc f, void *ud)
{
    lua_State *L = f
        ? lua_newstate(f, ud)
        : luaL_newstate();
    laf_init(L);
    return L;
}

void laf_init(lua_State *L)
{
    luaL_openlibs(L);

    // This isn't optimization! Well, it might be, but more
    // importantly it's about making sure someone doesn't
    // accidentally break things by putting something else
    // in these variables.
    // Cache package.preload for laf_newmodule
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_rawsetp(L, LUA_REGISTRYINDEX, laf_newmodule);
    lua_pop(L, 1);
    // Cache require for laf_require
    lua_getglobal(L, "require");
    lua_rawsetp(L, LUA_REGISTRYINDEX, laf_require);

    // Set up class module and require it to put `new` into
    // the global namespace immediately.
    lua_pushcfunction(L, luaopen_class);
    laf_newmodule(L, "class");
    laf_require(L, "class");
    lua_pop(L, 1);
}


void laf_newmodule(lua_State *L, const char *modname)
{
    // Put the loader function into package.preload
    // (cached at _REG[laf_newmodule])
    lua_rawgetp(L, LUA_REGISTRYINDEX, laf_newmodule);
    lua_insert(L, -2);
    lua_setfield(L, -2, modname);
    lua_pop(L, 1);
}

void laf_require(lua_State *L, const char *modname)
{
    lua_rawgetp(L, LUA_REGISTRYINDEX, laf_require);
    lua_pushstring(L, modname);
    lua_call(L, 1, 1);
}


int laf_msgh(lua_State *L)
{
    luaL_traceback(L, L, lua_tostring(L, 1), 0);
    return 1;
}

int laf_pcallh(lua_State *L, int nargs, int nret)
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

static int laf_pcallhk_k(lua_State *L, int status, lua_KContext ctx);

int laf_pcallhk(lua_State *L, int nargs, int nret,
               lua_KContext ctx, lua_KFunction k)
{
    laf_msgh_ctx_t *lctx = malloc(sizeof (laf_msgh_ctx_t));
    lctx->ctx = ctx;
    lctx->k = k;

    lua_pushcfunction(L, laf_msgh);
    lua_insert(L, 1);
    int status = lua_pcallk(L, nargs, nret, 1, (lua_KContext) lctx, laf_pcallhk_k);
    lua_remove(L, 1);
    return status;
}

static int laf_pcallhk_k(lua_State *L, int status, lua_KContext ctx)
{
    lua_remove(L, 1);
    laf_msgh_ctx_t *lctx = (laf_msgh_ctx_t *) ctx;
    ctx = lctx->ctx;
    lua_KFunction k = lctx->k;
    free(lctx);
    return k(L, status, ctx);
}
