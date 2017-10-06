
#include <lua.h>

#include "../evl.h"


struct evl_qaction {
    laf_evl_action_t action;
    void *ud;
};

struct evl_qnode {
    struct evl_qaction action;
    struct evl_qnode *next;
};

struct evl_queue {
    struct evl_qnode *front;
    struct evl_qnode *back;
};

struct laf_evl {
    lua_State *L;
    lua_Alloc allocf;
    void *allocud;
    struct evl_queue *current;
    struct evl_queue *next;
};


void *evl_alloc(laf_evl_t *evl, size_t sz)
{
    return evl->allocf(evl->allocud, NULL, LUA_TUSERDATA, sz);
}

void evl_free(laf_evl_t *evl, void *data, size_t sz)
{
    evl->allocf(evl->allocud, data, sz, 0);
}


static struct evl_queue *evl_qnew(laf_evl_t *evl)
{
    struct evl_queue *newq = evl_alloc(evl, sizeof (*newq));
    newq->front = NULL;
    newq->back = NULL;
}

static int evl_qempty(struct evl_queue *q)
{
    return q->front == NULL;
}

static struct evl_qnode *evl_qpop(laf_evl_t *evl, struct evl_queue *q)
{
    struct evl_qnode *node = q->front;
    q->front = node->next;
    return node;
}

static void evl_qclear(laf_evl_t *evl, struct evl_queue *q)
{
    while (!evl_qempty(q)) {
        evl_free(evl, evl_qpop(evl, q), sizeof (struct evl_qnode));
    }
    evl_free(evl, q, sizeof (*q));
}


laf_evl_t *laf_evl_new(lua_State *L)
{
    void *allocud;
    lua_Alloc allocf = lua_getallocf(L, &allocud);
    laf_evl_t *evl = allocf(allocud, NULL, LUA_TUSERDATA, sizeof (*evl));

    evl->L = L;
    evl->allocf = allocf;
    evl->allocud = allocud;
    evl->current = evl_qnew(evl);
    evl->next = evl_qnew(evl);

    return evl;
}

void laf_evl_delete(laf_evl_t *evl)
{
    evl_qclear(evl, evl->current);
    evl_qclear(evl, evl->next);
    
    evl_free(evl, evl, sizeof (*evl));
}

int laf_evl_empty(laf_evl_t *evl)
{
    return evl_qempty(evl->current);
}

void laf_evl_push(laf_evl_t *evl, laf_evl_action_t action, void *ud)
{
    struct evl_qnode *node = evl_alloc(evl, sizeof (*node));
    node->action.action = action;
    node->action.ud = ud;
    node->next = NULL;

    if (evl->next->back)
        evl->next->back->next = node;
    evl->next->back = node;
}

void laf_evl_rotate(laf_evl_t *evl)
{
    evl_qclear(evl, evl->current);
    evl->current = evl->next;
    evl->next = evl_qnew(evl);
}

static int evl_run_next(lua_State *L)
{
    laf_evl_t *evl = lua_touserdata(L, 1);
    struct evl_qnode *node = lua_touserdata(L, 2);
    lua_pop(L, 2);
    node->action.action(evl, L, node->action.ud);
}

void laf_evl_next(laf_evl_t *evl)
{
    struct evl_qnode *node = evl_qpop(evl, evl->current);
    lua_pushcfunction(evl->L, evl_run_next);
    lua_pushlightuserdata(evl->L, evl);
    lua_pushlightuserdata(evl->L, node);
    lua_call(evl->L, 2, 0);
    evl_free(evl, node, sizeof (*node));
}

void laf_evl_cycle(laf_evl_t *evl)
{
    while (!laf_evl_empty(evl)) {
        laf_evl_next(evl);
    }
    laf_evl_rotate(evl);
}
