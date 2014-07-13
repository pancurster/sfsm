#include "sfsm.h"
#include <malloc.h>
#include <assert.h>
#include <string.h>

static void fsm_change_state(fsm_t* fsm, int st_id)
{
    if (fsm->state[fsm->in_state].on_exit)
        fsm->state[fsm->in_state].on_exit(fsm);

    /* for now we assume that state id can't be higher than number of states */
    assert(st_id <= fsm->nstates);
    fsm->in_state = fsm->id2state[st_id];

    if (fsm->state[fsm->in_state].on_enter)
        fsm->state[fsm->in_state].on_enter(fsm);
}

static void map_id2state(const state_t* statetab, const int snum, int* smap)
{
    int i;
    for (i=0; i < snum; i++)
    {
        assert(statetab[i].id <= snum);
        smap[statetab[i].id] = i;
    }
}

static state_t* get_raw_state(fsm_t* fsm, int sid)
{
    return &fsm->state[fsm->id2state[sid]];
}

static state_t* get_acc_state(fsm_t* fsm)
{
    return get_raw_state(fsm, fsm->in_state);
}

void fsm_init(fsm_t* fsm, state_t* statetab, int nstates)
{
    memset(fsm, 0, sizeof(fsm));

    fsm->state = statetab;
    fsm->nstates = nstates;
    fsm->in_state = 0;

    fsm->id2state = malloc(sizeof(int) * nstates);
    map_id2state(fsm->state, fsm->nstates, fsm->id2state);
}

void fsm_run(fsm_t* fsm)
{
    fsm->in_state = 0;
    fsm->state[fsm->in_state].on_enter(fsm);
}

void fsm_go_state(fsm_t* fsm, int st_id)
{
    fsm_change_state(fsm, st_id);
}

void fsm_go_default(fsm_t* fsm)
{
    assert(fsm->state[fsm->in_state].default_transition_state_id != FSM_NO_DEF);

    if (fsm->state[fsm->in_state].default_transition_state_id != FSM_NO_DEF)
        fsm_change_state(fsm, fsm->state[fsm->in_state].default_transition_state_id);
}

void* fsm_get_state_data(fsm_t* fsm)
{
    return get_acc_state(fsm)->data;
}

