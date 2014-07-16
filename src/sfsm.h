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
    int sid_from;
    int sid_to;
    event_t e;
    void (*h)(void);
};

struct fsm_t {
    void* data;                 /* fsm data */
    state_t* states;            /* states table */
    size_t states_size;         /* states tabel size */

    tr_t* tr;                /* permited transitions - handle states id */
    int tr_size;

    int defstid;                /* default state id */

    int current_sid;               /* present state */
    int* id2state;              /* maps states id to state index in table */
};

struct state_t {
    int sid;
    void (*on_enter)(state_t* st);
    void (*on_exit)(state_t* st);

    void* data;                 /* state data */

    state_t* substate;
    size_t nsubstates;
};

void fsm_init(fsm_t* fsm, state_t* statetab, int nstates);
void fsm_run(fsm_t* fsm);
void fsm_go_default(fsm_t* fsm);
void* fsm_get_state_data(fsm_t* fsm);
void fsm_ev(fsm_t* fsm, event_t* ev);

void fsm_start(fsm_t* fsm);

#ifdef __cplusplus
}
#endif
#endif//_SFSM_H_

