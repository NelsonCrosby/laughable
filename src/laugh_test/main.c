
#include <stdio.h>

#include <lua.h>
#include <lauxlib.h>

#include "laf/util.h"
#include "laf/class.h"


int luaopen_foo(lua_State *L);


int main(int argc, char **argv)
{
    lua_State *L = laf_newstate_m();
    lua_pushcfunction(L, luaopen_foo)
    laf_newmodule(L, "foo");

    if (luaL_loadfile(L, "lua/laugh_test/main.lua"))
        goto _err;
    
    if (laf_pcallh(L, 0, 1))
        goto _err;

    lua_pushstring(L, argv[0]);

    lua_createtable(L, argc, 0);
    for (int i = 1; i < argc; i += 1) {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i);
    }

    if (laf_pcallh(L, 2, 1))
        goto _err;

    return lua_tointeger(L, -1);
_err:
    fprintf(stderr, "%s\n", lua_tostring(L, -1));
    return -1;
}
