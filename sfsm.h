#ifndef _SFSM_H_
#define _SFSM_H_

#define FSM_NO_DEF (-1)

typedef struct fsm_t_ fsm_t;
typedef struct state_t_ state_t;

struct fsm_t_ {
    void* data;                 /* fsm data */
    state_t* state;             /* states table */
    int nstates;                /* numer of states in states tabel */

    int in_state;                /* present state */
    int* id2state;              /* maps states id to state index in table */
};

struct state_t_ {
    int id;
    void (*on_enter)(fsm_t* this);
    void (*on_exit)(fsm_t* this);

    int default_transition_state_id;
    void* data;                 /* state data */
};

void fsm_init(fsm_t* fsm, state_t* statetab, int nstates);
void fsm_run(fsm_t* fsm);
void fsm_go_state(fsm_t* fsm, int st_id);
void fsm_go_default(fsm_t* fsm);
void* fsm_get_state_data(fsm_t* fsm);

#endif//_SFSM_H_

