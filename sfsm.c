#include "sfsm.h"

void fsm_run(fsm_t* fsm)
{
    fsm->p_state = 0;
    fsm->state[fsm->p_state].on_enter(fsm);
}

