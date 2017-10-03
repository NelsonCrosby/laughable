
#include "util.h"
#include "class.h"


static int l_new(lua_State *L);
static int l_class(lua_State *L);
static int l_ctor(lua_State *L);


void laf_ctor(lua_State *L, int clsidx)
{
    // > local v = rawget(cls, laf_ctor)
    lua_rawgetp(L, -1, laf_ctor);
    // > del mt
    lua_remove(L, -2);
}

void laf_class(lua_State *L, int ctoridx, int nparents,
                   luaL_Reg methods[], int nup)
{
    if (ctoridx < 0)
        ctoridx = (lua_gettop(L) + ctoridx) + 1;

    // > local cls = {}
    int mlen = 0;
    if (methods)
        while (methods[mlen].func)
            mlen += 1;
    lua_createtable(L, 0, mlen + 2);
    lua_insert(L, -(nparents + 1));

    int parentidx = lua_gettop(L) - nup;
    int clsidx = parentidx - nparents;
    // Populate from parent classes
    for (; parentidx > clsidx; lua_remove(L, parentidx--)) {
        lua_pushnil(L);
        // > for name, value in pairs(parent) do
        while (lua_next(L, parentidx)) {
            // Duplicate key so lua_next() still knows what's up
            lua_pushvalue(L, -2);
            lua_insert(L, -2);
            // > cls[name] = value
            lua_settable(L, clsidx);
        }
        // > end
    }
    // Parents are now removed from between cls and upvalues

    // > cls.__index = cls
    lua_pushvalue(L, clsidx);
    lua_setfield(L, clsidx, "__index");

    // Set up methods
    if (methods) luaL_setfuncs(L, methods, nup);

    // Set up constructor
    if (ctoridx == 0) {
        //~> cls[laf_ctor] = function (...) return ctor(cls, ...) end
        //  ~> function (...) return ctor(cls, ...) end
        lua_pushvalue(L, clsidx);
        lua_pushcclosure(L, l_ctor, 1);
        //   > rawset(cls, laf_ctor, _)
        lua_rawsetp(L, clsidx, laf_ctor);
    } else {
        // Set first upvalue of ctor as cls,
        //  reset if no upvalues are allocated
        lua_pushvalue(L, clsidx);
        if (!lua_setupvalue(L, ctoridx, 1))
            lua_pop(L, 1);
        // > rawset(cls, laf_ctor, ctor)
        lua_pushvalue(L, ctoridx);
        lua_rawsetp(L, clsidx, laf_ctor);
    }
}


void laf_new(lua_State *L, int clsidx, int nargs)
{
    laf_ctor(L, clsidx);
    lua_insert(L, -(nargs + 1));
    lua_call(L, nargs, 1);
}
void laf_newk(lua_State *L, int clsidx, int nargs,
              lua_KContext ctx, lua_KFunction k)
{
    laf_ctor(L, clsidx);
    lua_insert(L, -(nargs + 1));
    lua_callk(L, nargs, 1, ctx, k);
}

int laf_pnew(lua_State *L, int clsidx, int nargs, int msgh)
{
    lua_pushcfunction(L, l_new);
    lua_pushvalue(L, clsidx);

    int status = lua_pcall(L, 1, 1, msgh);
    lua_insert(L, -(nargs + 1));

    if (status) {
        lua_pop(L, nargs);
        return status;
    }

    status = lua_pcall(L, nargs, 1, msgh);
    return status;
}

int laf_pnewk(lua_State *L, int clsidx, int nargs, int msgh,
              lua_KContext ctx, lua_KFunction k)
{
    lua_pushcfunction(L, l_new);
    lua_pushvalue(L, clsidx);

    int status = lua_pcall(L, 1, 1, msgh);
    lua_insert(L, -(nargs + 1));

    if (status) {
        lua_pop(L, nargs);
        return status;
    }

    return lua_pcallk(L, nargs, 1, msgh, ctx, k);
}

int laf_pnewh(lua_State *L, int clsidx, int nargs)
{
    lua_pushcfunction(L, l_new);
    lua_pushvalue(L, clsidx);

    int status = laf_pcall(L, 1, 1);
    lua_insert(L, -(nargs + 1));

    if (status) {
        lua_pop(L, nargs);
        return status;
    }

    status = laf_pcall(L, nargs, 1);
    return status;
}

int laf_pnewhk(lua_State *L, int clsidx, int nargs,
               lua_KContext ctx, lua_KFunction k)
{
    lua_pushcfunction(L, l_new);
    lua_pushvalue(L, clsidx);

    int status = laf_pcall(L, 1, 1);
    lua_insert(L, -(nargs + 1));

    if (status) {
        lua_pop(L, nargs);
        return status;
    }

    return laf_pcallk(L, nargs, 1, ctx, k);
}


/* Implements `require('class')`.
 * Returns `new`. Also sets the global `new`.
 * There should be no need to use `require('class')`,
 * as it is typically called by Laughable's init code
 * before running any Lua code. */
int luaopen_class(lua_State *L)
{
    static luaL_Reg new_l[] = {
        {"__call", l_new},
        {"class", l_class},
        {NULL, NULL}
    };

    // > local new_l = { class = l_class, __call = l_new }
    luaL_newlib(L, new_l);
    // > setmetatable(new_l, new_l)
    lua_pushvalue(L, -1);
    lua_setmetatable(L, -2);
    // > new = new_l
    lua_pushvalue(L, -1);
    lua_setglobal(L, "new");
    // > return new_l
    return 1;
}

/* Implements `new()`.
 * Gets the constructor and returns it.
 * Common usage is like `new(Foo)(arg1, arg2, ...)`. */
static int l_new(lua_State *L)
{
    laf_ctor(L, 1);
    return 1;
}

/* Implements `new.class()`.
 * Creates a new subclass using parents as mixin classes.
 * Common usage is like `local Bar = new.class(Foo, Baz)`. */
static int l_class(lua_State *L)
{
    laf_class(L, 0, lua_gettop(L), NULL, 0);
    // > return cls
    return 1;
}

static int l_ctor_k(lua_State *L, int status, lua_KContext ctx);
static int l_ctor(lua_State *L)
{
    // > local o = {}
    lua_newtable(L);
    lua_insert(L, 1);
    // > setmetatable(o, cls)
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_setmetatable(L, 1);

    /* The following code is essentially equivalent to
    lua_callmsk(L, 1, "init", lua_gettop(L) - 1, 0, 2, l_ctor_k);
    return l_ctor_k(L, LUA_OK, 1);
     * except it also checks whether o.init exists before trying to call
     * it. If you are implementing your own constructor and you know you
     * will have an init method, you can directly adapt the above two
     * lines instead of reproducing everything below. */

    lua_getfield(L, 1, "init");
    if (lua_isnil(L, -1)) {     // Skip trying to call init
        // > return o
        lua_pushvalue(L, 1);
        return 1;
    } else {
        lua_insert(L, 2);
        // > o.init(o, ...)
        lua_callmfk(L, 1,
                    lua_gettop(L) - 2, 0,
        // goto l_ctor_k
                    2, l_ctor_k);
        return l_ctor_k(L, LUA_OK, 1);
    }
}
static int l_ctor_k(lua_State *L, int status, lua_KContext ctx)
{
    // > return o
    return 1;
}


// The implementations of the callm* functions all follow a
// very regular pattern; as such, they can be implemented as
// macro calls. There is enough variation, however, that they
// can't easily be implemented as additional function calls
// (e.g. pcallm* functions need to return, while the others'
// don't). The variadic part is used as extra args for the
// underlying call after nret.

// This macro implements the *callmf functions
//  (lua_callmf, lua_callmfk, lua_pcallmf, lua_pcallmfk,
//      laf_pcallmf, laf_pkcallmfk).
// It pushes a copy of the object at oidx into the right
// location before calling the method.
#define CALLMF(call, ...)           \
{                                   \
    lua_pushvalue(L, oidx);         \
    lua_insert(L, -(nargs + 1));    \
    call(L, nargs + 1, nret, ##__VA_ARGS__);    \
}
// This macro implements the *callm functions. In addition
// to pushing the object into the right space, it also retrieves
// the function using the key on the top of the stack.
#define CALLM(call, ...)            \
{                                   \
    lua_pushvalue(L, oidx);         \
    lua_insert(L, -(nargs + 2));    \
    lua_gettable(L, -(nargs + 2));  \
    lua_insert(L, -(nargs + 2));    \
    call(L, nargs + 1, nret, ##__VA_ARGS__);    \
}
// This macro implements both the *callms and *callmv functions.
// The call argument this time is one of the *callm functions;
// these functions are simply a convenience wrapper that allow
// calling string and userdata functions directly.
#define CALLM_(push, call, ...)     \
{                                   \
    push(L, method);                \
    call(L, oidx < 0 ? oidx - 1 : oidx, nargs, nret, ##__VA_ARGS__);    \
}

void lua_callmf(lua_State *L, int oidx,
                int nargs, int nret)
    CALLMF(lua_call)

void lua_callm(lua_State *L, int oidx,
               int nargs, int nret)
    CALLM(lua_call)

void lua_callms(lua_State *L, int oidx, const char *method,
                int nargs, int nret)
    CALLM_(lua_pushstring, lua_callm)

void lua_callmv(lua_State *L, int oidx, void *method,
                int nargs, int nret)
    CALLM_(lua_pushlightuserdata, lua_callm)


void lua_callmfk(lua_State *L, int oidx,
                 int nargs, int nret,
                 lua_KContext ctx, lua_KFunction k)
    CALLMF(lua_callk, ctx, k)

void lua_callmk(lua_State *L, int oidx,
                int nargs, int nret,
                lua_KContext ctx, lua_KFunction k)
    CALLM(lua_callk, ctx, k)

void lua_callmsk(lua_State *L, int oidx, const char *method,
                 int nargs, int nret,
                 lua_KContext ctx, lua_KFunction k)
    CALLM_(lua_pushstring, lua_callmk, ctx, k)

void lua_callmvk(lua_State *L, int oidx, void *method,
                 int nargs, int nret,
                 lua_KContext ctx, lua_KFunction k)
    CALLM_(lua_pushlightuserdata, lua_callmk, ctx, k)


int lua_pcallmf(lua_State *L, int oidx,
                int nargs, int nret, int msgh)
    CALLMF(return lua_pcall, msgh)

int lua_pcallm(lua_State *L, int oidx,
               int nargs, int nret, int msgh)
    CALLM(return lua_pcall, msgh)

int lua_pcallms(lua_State *L, int oidx, const char *method,
                int nargs, int nret, int msgh)
    CALLM_(lua_pushstring, return lua_pcallm, msgh)

int lua_pcallmv(lua_State *L, int oidx, void *method,
                int nargs, int nret, int msgh)
    CALLM_(lua_pushlightuserdata, return lua_pcallm, msgh)


int lua_pcallmfk(lua_State *L, int oidx,
                 int nargs, int nret, int msgh,
                 lua_KContext ctx, lua_KFunction k)
    CALLMF(return lua_pcallk, msgh, ctx, k)

int lua_pcallmk(lua_State *L, int oidx,
                int nargs, int nret, int msgh,
                lua_KContext ctx, lua_KFunction k)
    CALLM(return lua_pcallk, msgh, ctx, k)

int lua_pcallmsk(lua_State *L, int oidx, const char *method,
                 int nargs, int nret, int msgh,
                 lua_KContext ctx, lua_KFunction k)
    CALLM_(lua_pushstring, return lua_pcallmk, msgh, ctx, k)

int lua_pcallmvk(lua_State *L, int oidx, void *method,
                 int nargs, int nret, int msgh,
                 lua_KContext ctx, lua_KFunction k)
    CALLM_(lua_pushlightuserdata, return lua_pcallmk, msgh, ctx, k)


int laf_pcallmf(lua_State *L, int oidx,
                int nargs, int nret)
    CALLMF(return laf_pcall)

int laf_pcallm(lua_State *L, int oidx,
               int nargs, int nret)
    CALLM(return laf_pcall)

int laf_pcallms(lua_State *L, int oidx, const char *method,
                int nargs, int nret)
    CALLM_(lua_pushstring, return laf_pcallm)

int laf_pcallmv(lua_State *L, int oidx, void *method,
                int nargs, int nret)
    CALLM_(lua_pushlightuserdata, return laf_pcallm)


int laf_pcallmfk(lua_State *L, int oidx,
                 int nargs, int nret,
                 lua_KContext ctx, lua_KFunction k)
    CALLMF(return laf_pcallk, ctx, k)

int laf_pcallmk(lua_State *L, int oidx,
                int nargs, int nret,
                lua_KContext ctx, lua_KFunction k)
    CALLM(return laf_pcallk, ctx, k)

int laf_pcallmsk(lua_State *L, int oidx, const char *method,
                 int nargs, int nret, int msgh,
                 lua_KContext ctx, lua_KFunction k)
    CALLM_(lua_pushstring, return laf_pcallmk, ctx, k)

int laf_pcallmvk(lua_State *L, int oidx, void *method,
                 int nargs, int nret, int msgh,
                 lua_KContext ctx, lua_KFunction k)
    CALLM_(lua_pushlightuserdata, return laf_pcallmk, ctx, k)
