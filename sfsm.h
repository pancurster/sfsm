#ifndef _SFSM_H_
#define _SFSM_H_

#endif//_SFSM_H_

typedef struct fsm_t_ fsm_t;
typedef struct state_t_ state_t;

struct fsm_t_ {
    void* data;                 /* fsm data */
    state_t* state;             /* states table */
    int nstates;                /* numer of states in states tabel */

    int p_state;                /* present state */
};

struct state_t_ {
    void (*on_enter)(fsm_t* this);
    void (*on_exit)(fsm_t* this);

    int default_transition_state_id;
    void* data;                 /* state data */
};

void fsm_run(fsm_t* fsm);

