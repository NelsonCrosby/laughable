#pragma once
/** Laughable async API */

#include <lua.h>


/** Calls a function asynchronously.                [-(nargs+1),+1,-]
 *  Working much like lua_call and the laf_callm[fsv] functions, these
 *  functions instead queue up the Lua call on the event loop. The functions
 *  will not start until the event loop takes control (e.g. an await occurs),
 *  meanwhile the current function will continue running.
 * 
 *  Instead of pushing the results (which aren't ready yet), these functions
 *  push a `Future`, which can be used to retrieve the results once they are
 *  available (by calling `laf_await`).
 * 
 *  Similarly, since the call doesn't happen in the current thread, these
 *  functions have no protected variants (as that would be pointless).
 *  However, since they can have invalid inputs, they do return an error
 *  code. Note that they also never yield, so continuation variations are
 *  also not available. */
int laf_acall(lua_State *L, int nargs);
int laf_acallm(lua_State *L, int oidx, int nargs);
int laf_acallmf(lua_State *L, int oidx, int nargs);
int laf_acallms(lua_State *L, int oidx, const char *method, int nargs);
int laf_acallmv(lua_State *L, int oidx, void *method, int nargs);

/** Await on some awaitables until all are ready. [-nawait,+nawait,-]
 *  This function yields, then runs the continuation function once all
 *  the results have come through. The awaitables to await on are the
 *  top `nawait` values on the stack. These will be replaced by `nawait`
 *  corresponding _result objects_, which can be used with `laf_aresult`
 *  and friends to get the actual results.
 * 
 *  Since the actual results and errors don't get processed until you
 *  call `laf_aresult`, there are no parameters for counting results or
 *  variants for handling errors. However, this function may return
 *  non-zero if any of the arguments are invalid. */
int laf_await(lua_State *L, int nawait,
              lua_KContext ctx, lua_KFunction k);
/** Await on some awaitables until one is ready.  [-nawait,+nawait,-]
 *  This function yields, then runs the continuation function once a result
 *  has come through. The awaitables to await on are the top `nawait` values
 *  on the stack. Upon continuation, one of these awaitables will be removed
 *  and the corresponding _result object_ (for use with `laf_aresult`) will
 *  be pushed. The status argument to the continuation function when negated
 *  is the stack index (from the top) where awaitable originally was (i.e.
 *  performing `lua_insert(L, -status)` will put the results object where
 *  awaitable originally was and the rest of the awaitables will all be in
 *  the same place).
 * 
 *  Since the actual results and errors don't get processed until you
 *  call `laf_aresult`, there are no parameters for counting results or
 *  variants for handling errors. However, this function may return
 *  non-zero if any of the arguments are invalid. */
int laf_await_any(lua_State *L, int nawait,
                  lua_KContext ctx, lua_KFunction k);

/** Get results from a result object.                    [-1,+nret,e]
 *  After calling `laf_await` or `laf_await_any`, you will need to obtain
 *  the results of the operation. It is at this point that, if the awaitable
 *  threw an error, it would be introduced into the current thread
 *  (`laf_aresult` will re-throw it, while the other two will catch it just
 *  like any other protected function).
 * 
 *  If there weren't any errors, this will pop the result object and push
 *  `nret` results, just like how `lua_call` normally would. If the operation
 *  was cancelled, the return value will be (some constant).
 * 
 *  Retrieving the results will never yield, so no continuation variants
 *  are required.
 */
int laf_aresult(lua_State *L, int nret);
int laf_paresult(lua_State *L, int nret, int msgh);
int laf_paresulth(lua_State *L, int nret);

/** Try to cancel the awaitable on the top of the stack.    [-0,+0,-]
 *  Returns some failure code if the awaitable cannot be cancelled,
 *  otherwise this awaitable will begin cancelling at some point in
 *  the future. */
int laf_acancel(lua_State *L);
/** Try to cancel `n` awaitables on the top of the stack.   [-0,+0,-]
 *  This is a convenience function. It may not be flexible enough for your
 *  needs; in which case, use `laf_acancel` in a loop.
 *  
 *  If any argument is not awaitable, the entire operation fails and nothing
 *  is cancelled. If any awaitable is not cancellable:
 *  - If `aon` (all-or-nothing) is non-zero, no awaitables will be cancelled;
 *  - If `aon` is zero, all awaitables that can be cancelled will be, and
 *    the return value will be negative (its positive will be the count of
 *    awaitables that were not cancellable).
 */
int laf_acancel_all(lua_State *L, int n, int aon);

/** Await on the results of an awaitable.                [-1,+nret,e]
 *  These functions are a shorthand for:
 *    laf_await(L, 1, ctx, k);
 *    // (in continuation)
 *    laf_aresult*(L, nret);
 */
int laf_awaitr(lua_State *L, int nret,
               lua_KContext ctx, lua_KFunction k);
int laf_pawaitr(lua_State *L, int nret, int msgh,
                lua_KContext ctx, lua_KFunction k);
int laf_pawaitrh(lua_State *L, int nret,
                 lua_KContext ctx, lua_KFunction k);

/** Wrap a function to make it behave like `laf_acall`.     [-1,+1,-]
 *  The pushed function is essentially equivalent to:
 *      lua_pushvalue(L, lua_upvalueindex(1));
 *      lua_insert(L, 1);
 *      laf_acall(L, lua_gettop(L) - 1);
 *  Note that in the `m` variants, the method is resolved right away
 *  and the object at oidx will be inserted as the first argument.
 *  It doesn't have an immediately obvious use case, though it is the basis
 *  of the Lua `async` function. */
int laf_awrap(lua_State *L);
int laf_awrapm(lua_State *L, int oidx);
int laf_awrapmf(lua_State *L, int oidx);
int laf_awrapms(lua_State *L, int oidx, const char *method);
int laf_awrapmv(lua_State *L, int oidx, void *method);
