#include "sfsm.h"
#include <malloc.h>
#include <assert.h>
#include <string.h>

static state_t* get_raw_state(fsm_t* fsm, int sid);
static tr_t* is_tr_possible(fsm_t* fsm, state_t* from, state_t* to);
static tr_t* check_default_tr(fsm_t* fsm);
static void make_default_tr(fsm_t* fsm);
static void change_state(fsm_t* fsm, int sid);

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
        if (fsm->tr[i].sid_from == fsm->current_sid && fsm->tr[i].e.evid == FSM_DEF_TR)
            return &fsm->tr[i];
    }
    return NULL;
}

static void make_default_tr(fsm_t* fsm)
{
    tr_t* tr;
    if (tr = check_default_tr(fsm))
        change_state(fsm, tr->sid_to);
}

static void change_state(fsm_t* fsm, int sid)
{
    state_t* curr = get_raw_state(fsm, fsm->current_sid);
    state_t* to = get_raw_state(fsm, sid);

    tr_t* tr;
    if (tr = is_tr_possible(fsm, curr, to))
    {
        if (fsm->states[fsm->current_sid].on_exit)
            fsm->states[fsm->current_sid].on_exit(get_raw_state(fsm, fsm->current_sid));

        fsm->current_sid = sid;

        if (fsm->states[fsm->current_sid].on_enter)
            fsm->states[fsm->current_sid].on_enter(get_raw_state(fsm, fsm->current_sid));
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

void fsm_go_state(fsm_t* fsm, int stid)
{
    int i;
    for (i=0; i<fsm->states_size; i++) {
        if (fsm->states[i].sid == stid)
            change_state(fsm, stid);
    }
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
            curr = get_raw_state(fsm, fsm->current_sid);
            if (curr->on_exit)
                curr->on_exit(curr);

            if (fsm->tr[i].h)
                fsm->tr[i].h();

            if (fsm->tr[i].sid_to != FSM_NO_TR) {
                fsm->current_sid = fsm->tr[i].sid_to;
                st = get_raw_state(fsm, fsm->current_sid);
                if (st->on_enter)
                    st->on_enter(st);
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

