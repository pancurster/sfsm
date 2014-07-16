#include "sfsm.h"
#include <malloc.h>
#include <assert.h>
#include <string.h>

static state_t* get_raw_state(fsm_t* fsm, int sid);
static void make_default_tr(fsm_t* fsm);

event_t FSM_DEF_TR = {
    .evid = FSM_DEF_TR_ID,
};

static state_t* get_raw_state(fsm_t* fsm, int sid)
{
    return &fsm->states[sid];
}

static tr_t* find_possible_tr(fsm_t* fsm, event_t* ev)
{
    int i;
    for (i=0; i < fsm->tr_size; i++) {
        if (fsm->current_sid == fsm->tr[i].sid_from && fsm->tr[i].e.evid == ev->evid)
            return &fsm->tr[i];
    }
    return NULL;
}

static void make_default_tr(fsm_t* fsm)
{
    fsm_ev(fsm, &FSM_DEF_TR);
}

static void run_on_exit(fsm_t* fsm, int sid)
{
    state_t* st;
    st = get_raw_state(fsm, sid);
    if (st->on_exit)
        st->on_exit(st);
}

static void run_on_enter(fsm_t* fsm, int sid)
{
    state_t* st;
    st = get_raw_state(fsm, sid);
    if (st->on_enter)
        st->on_enter(st);
}

void fsm_init(fsm_t* fsm, state_t* statetab, int nstates)
{
    memset(fsm, 0, sizeof(fsm));

    fsm->states = statetab;
    fsm->states_size = nstates;
    fsm->current_sid = 0;
}

void* fsm_get_state_data(fsm_t* fsm)
{
    return get_raw_state(fsm, fsm->current_sid)->data;
}

void fsm_ev(fsm_t* fsm, event_t* ev)
{
    int i;
    tr_t* tr;
    if (tr = find_possible_tr(fsm, ev))
    {
        if (tr->sid_to != FSM_NO_STATE)
            run_on_exit(fsm, fsm->current_sid);

        if (tr->h)
            tr->h();

        if (tr->sid_to != FSM_NO_STATE) {
            fsm->current_sid = tr->sid_to;
            run_on_enter(fsm, fsm->current_sid);
        }

        make_default_tr(fsm);

    }
}

void fsm_start(fsm_t* fsm)
{
    fsm->current_sid = fsm->defstid;

    run_on_enter(fsm, fsm->current_sid);

    make_default_tr(fsm);
}

