
#include <lua.h>
#include <lauxlib.h>

#include "laf/class.h"


static int foo_init(lua_State *L);
static int foo_speak(lua_State *L);


int luaopen_foo(lua_State *L)
{
    static luaL_Reg foo_cls[] = {
        {"init", foo_init},
        {"speak", foo_speak},
        {NULL, NULL}
    };

    laf_class(L, 0, 0, foo_cls, 0);
    return 1;
}


static int foo_init(lua_State *L)
{
    lua_pushlightuserdata(L, foo_speak);
    lua_pushvalue(L, 2);
    lua_settable(L, 1);
    return 0;
}

static int foo_speak(lua_State *L)
{
    lua_pushlightuserdata(L, foo_speak);
    lua_gettable(L, 1);
    return 1;
}
