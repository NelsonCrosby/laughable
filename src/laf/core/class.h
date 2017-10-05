#pragma once
/** The Laughable class API */

#include <lua.h>
#include <lauxlib.h>


/** Implements `require('class')`. Returns `new`. Also sets the
 *  global `new`. There should be no need to use `require('class')`,
 *  as it is typically called by Laughable's init code before running
 *  any Lua code. */
int luaopen_class(lua_State *L);

/** Get the constructor for the class at clsidx.            [-0,+1,e]
 *  Pushes the constructor. Like `new(cls)`. */
void laf_ctor(lua_State *L, int clsidx);

/** Create a new class, using the constructor  [-(nparents+nup),+1,e]
 *  at ctoridx, nup items from the top of the stack as shared
 *  upvalues for methods (see luaL_setfuncs), the next nparents
 *  items on the stack as parents, and pre-filling the class with
 *  methods. Pops the parents and pushes the new class. Use 0 as
 *  ctoridx to use the default table constructor. If a custom
 *  constructor is used, its first upvalue is overwritten with the
 *  new class. Will not add any methods if parents == NULL. */
void laf_class(lua_State *L, int ctoridx, int nparents,
                luaL_Reg methods[], int nup);

/** Instantiate the class at clsidx, passing nargs      [-nargs,+1,e]
 *  arguments to the constructor. Pops the arguments and pushes
 *  the new instance. Like `new(cls)(args)`. There are six
 *  variants, each matching a corresponding variant of lua_call or
 *  laf_pcall (note that laf_pcall functions are suffixed with an
 *  h, since all these use the laf_ prefix). */
void laf_new(lua_State *L, int clsidx, int nargs);
void laf_newk(lua_State *L, int clsidx, int nargs,
              lua_KContext ctx, lua_KFunction k);
int laf_pnew(lua_State *L, int clsidx, int nargs, int msgh);
int laf_pnewk(lua_State *L, int clsidx, int nargs, int msgh,
              lua_KContext ctx, lua_KFunction k);
int laf_pnewh(lua_State *L, int clsidx, int nargs);
int laf_pnewhk(lua_State *L, int clsidx, int nargs,
               lua_KContext ctx, lua_KFunction k);

/** These functions are wrappers for their similarly-named counterparts.
 *  They allow for somewhat more concise handling of method calls. They
 *  find a function as per their variant (see below), then call it with
 *  o (the value at oidx on the stack) as the first argument (o remains
 *  on the stack in the same position it started).
 *
 *  There are four variants:
 *  - `m` functions take the method as o[i], where i is at -1 on the stack;
 *  - `mf` functions take the method from -(nargs+1) (i.e. where functions
 *         normally are);
 *  - `ms` and `mv` functions take the method as o[method] (pushing the value
 *         of method as a string or light userdata respectively).
 *
 *  Each of these variants also comes in the normal lua_call and laf_pcallh
 *  variants, for a total of 24 variants.
 */

void laf_callm(lua_State *L, int oidx,
               int nargs, int nret);
void laf_callmf(lua_State *L, int oidx,
                int nargs, int nret);
void laf_callms(lua_State *L, int oidx, const char *method,
                int nargs, int nret);
void laf_callmv(lua_State *L, int oidx, void *method,
                int nargs, int nret);

void laf_callmk(lua_State *L, int oidx,
                int nargs, int nret,
                lua_KContext ctx, lua_KFunction k);
void laf_callmfk(lua_State *L, int oidx,
                 int nargs, int nret,
                 lua_KContext ctx, lua_KFunction k);
void laf_callmsk(lua_State *L, int oidx, const char *method,
                 int nargs, int nret,
                 lua_KContext ctx, lua_KFunction k);
void laf_callmvk(lua_State *L, int oidx, void *method,
                 int nargs, int nret,
                 lua_KContext ctx, lua_KFunction k);

int laf_pcallm(lua_State *L, int oidx,
               int nargs, int nret, int msgh);
int laf_pcallmf(lua_State *L, int oidx,
                int nargs, int nret, int msgh);
int laf_pcallms(lua_State *L, int oidx, const char *method,
                int nargs, int nret, int msgh);
int laf_pcallmv(lua_State *L, int oidx, void *method,
                int nargs, int nret, int msgh);

int laf_pcallmk(lua_State *L, int oidx,
                int nargs, int nret, int msgh,
                lua_KContext ctx, lua_KFunction k);
int laf_pcallmfk(lua_State *L, int oidx,
                 int nargs, int nret, int msgh,
                 lua_KContext ctx, lua_KFunction k);
int laf_pcallmsk(lua_State *L, int oidx, const char *method,
                 int nargs, int nret, int msgh,
                 lua_KContext ctx, lua_KFunction k);
int laf_pcallmvk(lua_State *L, int oidx, void *method,
                 int nargs, int nret, int msgh,
                 lua_KContext ctx, lua_KFunction k);

int laf_pcallmh(lua_State *L, int oidx,
                int nargs, int nret);
int laf_pcallmfh(lua_State *L, int oidx,
                 int nargs, int nret);
int laf_pcallmsh(lua_State *L, int oidx, const char *method,
                 int nargs, int nret);
int laf_pcallmvh(lua_State *L, int oidx, void *method,
                 int nargs, int nret);

int laf_pcallmhk(lua_State *L, int oidx,
                 int nargs, int nret,
                 lua_KContext ctx, lua_KFunction k);
int laf_pcallmfhk(lua_State *L, int oidx,
                  int nargs, int nret,
                  lua_KContext ctx, lua_KFunction k);
int laf_pcallmshk(lua_State *L, int oidx, const char *method,
                  int nargs, int nret, int msgh,
                  lua_KContext ctx, lua_KFunction k);
int laf_pcallmvhk(lua_State *L, int oidx, void *method,
                  int nargs, int nret, int msgh,
                  lua_KContext ctx, lua_KFunction k);
