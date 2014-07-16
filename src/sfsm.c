#include "sfsm.h"
#include <malloc.h>
#include <assert.h>
#include <string.h>

static state_t* get_raw_state(fsm_t* fsm, int sid);
static tr_t* is_tr_possible(fsm_t* fsm, state_t* from, state_t* to);
static tr_t* check_default_tr(fsm_t* fsm);
static void make_default_tr(fsm_t* fsm);
static void change_state(fsm_t* fsm, int sid);

event_t FSM_DEF_TR = {
    .evid = FSM_DEF_TR_ID,
};

static state_t* get_raw_state(fsm_t* fsm, int sid)
{
    return &fsm->states[sid];
}

static tr_t* is_tr_possible(fsm_t* fsm, state_t* from, state_t* to)
{
    int i;
    for (i=0; i < fsm->tr_size; i++) {
        if (fsm->tr[i].sid_from == from->sid && fsm->tr[i].sid_to == to->sid)
            return &fsm->tr[i];
    }
    return NULL;
}

static tr_t* check_default_tr(fsm_t* fsm)
{
    int i;
    for (i=0; i < fsm->tr_size; i++) {
        if (fsm->tr[i].sid_from == fsm->current_sid && fsm->tr[i].e.evid == FSM_DEF_TR.evid)
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

static void change_state(fsm_t* fsm, int sid)
{
    state_t* curr = get_raw_state(fsm, fsm->current_sid);
    state_t* to = get_raw_state(fsm, sid);

    tr_t* tr;
    if (tr = is_tr_possible(fsm, curr, to))
    {
        run_on_exit(fsm, fsm->current_sid);

        fsm->current_sid = sid;

        run_on_enter(fsm, fsm->current_sid);
    }

    make_default_tr(fsm);
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
    state_t* curr;
    state_t* st;
    for (i=0; i < fsm->tr_size; i++) {
        if (fsm->current_sid == fsm->tr[i].sid_from && fsm->tr[i].e.evid == ev->evid)
        {
            if (fsm->tr[i].sid_to != FSM_NO_TR)
                run_on_exit(fsm, fsm->current_sid);

            if (fsm->tr[i].h)
                fsm->tr[i].h();

            if (fsm->tr[i].sid_to != FSM_NO_TR) {
                fsm->current_sid = fsm->tr[i].sid_to;
                run_on_enter(fsm, fsm->current_sid);
            }
            break;
        }
    }
}

void fsm_start(fsm_t* fsm)
{
    fsm->current_sid = fsm->defstid;

    state_t* current = get_raw_state(fsm, fsm->current_sid);
    if (current->on_enter)
        current->on_enter(current);

    make_default_tr(fsm);
}

