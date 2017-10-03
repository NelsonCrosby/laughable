
#include <lua.h>

void laf_newmodule(lua_State *L, const char *modname, int openidx);
void laf_newmod(lua_State *L, const char *modname, lua_CFunction open);
void laf_newmodclosure(lua_State *L, const char *modname,
                       lua_CFunction open, int nup);

int laf_msgh(lua_State *L);
int laf_pcall(lua_State *L, int nargs, int nret);
int laf_pcallk(lua_State *L, int nargs, int nret,
               lua_KContext ctx, lua_KFunction k);
