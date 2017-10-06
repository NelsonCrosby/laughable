#pragma once

#include <lua.h>


typedef struct laf_evl laf_evl_t;
typedef void (*laf_evl_action_t)(laf_evl_t *q, lua_State *L, void *ud);


laf_evl_t *laf_evl_new(lua_State *L);
void laf_evl_delete(laf_evl_t *evl);
int laf_evl_empty(laf_evl_t *evl);
void laf_evl_push(laf_evl_t *eq, laf_evl_action_t action, void *ud);
void laf_evl_rotate(laf_evl_t *evl);
void laf_evl_cycle(laf_evl_t *evl);

void laf_evl_push_thread(laf_evl_t *evl);
