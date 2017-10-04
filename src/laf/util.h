#pragma once
/** Central utility functions */

#include <lua.h>

/** Shortcut to create a new state using the libc alloc functions. */
#define laf_newstate_m()    laf_newstate(NULL, NULL)
/** Create a new Lua state and intialise it for Laughable.
 *  The same options as `lua_newstate`, except that passing `NULL` for
 *  `f` is like `luaL_newstate`. */
lua_State *laf_newstate(lua_Alloc f, void *ud);
/** Initialise a Lua state with Laughable APIs and internals.
 *  Calling this function multiple times for the same Lua state is
 *  undefined. Calling any other API method without calling this
 *  first is undefined. You do not need to call this method if you
 *  created the lua_State using `laf_newstate` or `laf_newstate_m`. */
void laf_init(lua_State *L);

/** Set up a new module for use with `require`.             [-1,+0,-]
 *  This sets the function at the top of the stack in `package.preload`,
 *  so that when the module is first `require`d the function is called.
 *  Pops the function from the stack.
 */
void laf_newmodule(lua_State *L, const char *modname);
/** Equivalent of `require(modname)`.                       [-0,+1,-]
 *  Simply a helper function to reduce the amount of boilerplate to
 *  requiring something from C. */
void laf_require(lua_State *L, const char *modname);

/* Laughable message handling functions.
 * Laughable has a standard way of handling errors. These functions
 * simplify using this from C. */

/** Laughable standard error message handler.
 *  This function implements the standard and recommended message
 *  handler for Laughable. Right now, it just adds a traceback to
 *  a string error message; it will eventually support a much more
 *  featureful error object. */
int laf_msgh(lua_State *L);
/** Utility functions to make the usage of       [-(nargs+1),+nret,-]
 *  laf_msgh easier. They match the lua_pcall and lua_pcallk functions,
 *  but use laf_msg. */
int laf_pcallh(lua_State *L, int nargs, int nret);
int laf_pcallhk(lua_State *L, int nargs, int nret,
                lua_KContext ctx, lua_KFunction k);
