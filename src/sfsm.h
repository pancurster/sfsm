#ifndef _SFSM_H_
#define _SFSM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h> /* size_t */

#define FSM_NO_DEF (-1)
#define FSM_NO_STATE (-1)
#define FSM_DEF_TR_ID (-1)

typedef struct fsm_t fsm_t;
typedef struct state_t state_t;
typedef struct event_t event_t;
typedef struct tr_t tr_t;

extern event_t FSM_DEF_TR;

struct event_t {
    int evid;
};


struct tr_t {
    int sid_from;               /* transition from state... */
    int sid_to;                 /* ...to state */
    event_t e;                  /* transition triger, event */
    void (*h)(void);            /* event handler */
};

struct fsm_t {
    void* data;                 /* fsm data */
    state_t* states;            /* states table */
    size_t states_size;         /* states tabel size */

    tr_t* tr;                   /* transitions table */
    size_t tr_size;             /* transitions table size */

    int defstid;                /* default state id */
    int current_sid;            /* current state id */
};

struct state_t {
    int sid;                    /* state id */
    void (*on_enter)(state_t* st);
    void (*on_exit)(state_t* st);

    void* data;                 /* state data */

    state_t* substate;
    size_t nsubstates;
};

void fsm_start(fsm_t* fsm);
void fsm_ev(fsm_t* fsm, event_t* ev);
void* fsm_get_state_data(fsm_t* fsm);

#ifdef __cplusplus
}
#endif
#endif//_SFSM_H_

