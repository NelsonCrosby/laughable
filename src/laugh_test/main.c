
#include <stdio.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "laf/util.h"
#include "laf/class.h"
#include "foo.h"


int luaopen_foo(lua_State *L);


int main(int argc, char **argv)
{
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaopen_class(L);
    laf_newmod(L, "foo", luaopen_foo);

    if (luaL_loadfile(L, "lua/laugh_test/main.lua"))
        goto _err;
    
    if (laf_pcall(L, 0, 1))
        goto _err;

    lua_pushstring(L, argv[0]);

    lua_createtable(L, argc, 0);
    for (int i = 1; i < argc; i += 1) {
        lua_pushstring(L, argv[i]);
        lua_rawseti(L, -2, i);
    }

    if (laf_pcall(L, 2, 1))
        goto _err;

    if (lua_isnil(L, -1))
        return 0;
    else
        return lua_tonumber(L, -1);
_err:
    fprintf(stderr, "%s\n", lua_tostring(L, -1));
    return 1;
}
