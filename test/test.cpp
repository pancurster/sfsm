#include <CppUTest/TestHarness.h>
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestRegistry.h>
#include <CppUTestExt/MockSupport.h>
#include <CppUTestExt/MockSupportPlugin.h>
#include <cstdio>
#include "../src/sfsm.h"

void st1_enter(state_t* st);

enum {EV1_ID, EV2_ID, EV3_ID, EV4_ID, EV5_ID, EV6_ID, EV7_ID};
enum {ST1_ID, ST2_ID, ST3_ID, ST_MAX};

fsm_t fsm;

void reccall(const char* param) { mock().actualCall("reccall").withParameter("param", param); };

void handle_ev1(void* data) { reccall("Handling ev1"); }
void handle_ev2(void* data) { reccall("Handling ev2"); }
void handev1(void) { reccall("Handev1"); }
void hand2(void) { reccall("Hand2"); }
void hand3(void) { reccall("Hand3"); }
void hand6(void) { reccall("Hand6"); }
void hand7(void) { reccall("Hand7"); }
void st1_enter(state_t* st) { reccall("Entering st1"); }
void st2_enter(state_t* st) { reccall("Entering st2"); }
void st3_enter(state_t* st) { reccall("Entering st3"); }
void st1_exit(state_t* st) { reccall("Exit st1"); }
void st2_exit(state_t* st) { reccall("Exit st2"); }
void st3_exit(state_t* st) { reccall("Exit st3"); }
void st1_enter_def_transition_to_st2(state_t* st)
{
    reccall("Entering st1 going st2");
    //fsm_go_state(&fsm, ST2_ID);
}

event_t ev1 = {FSM_DEF_TR};
event_t ev2 = {EV2_ID};
event_t ev3 = {EV3_ID};
event_t ev4 = {EV4_ID};
event_t ev5 = {EV5_ID};
event_t ev6 = {EV6_ID};
event_t ev7 = {EV7_ID};
/* Row equal state id, column mean nothing or could equal max events in one of state */
tr_t tr[] = {
    /*sid_from, sid_to, ev, hand.| sidfrom, sidto, ev,  handler */
    {ST1_ID, ST1_ID, ev2, hand2},
    {ST1_ID, ST2_ID, ev1, handev1},
    {ST1_ID, FSM_NO_TR, ev3, hand3},
    /* line group states */
    {ST2_ID, ST1_ID, ev4, NULL},
    {ST2_ID, ST3_ID, ev5, NULL},

    {ST3_ID, ST2_ID, ev6, hand6},
    {ST3_ID, ST3_ID, ev7, hand7},
};

TEST_GROUP(sfsm_fixture)
{
    state_t st[ST_MAX];
    void setup()
    {
        st[ST1_ID].sid = ST1_ID;
        st[ST2_ID].sid = ST2_ID;
        st[ST3_ID].sid = ST3_ID;

        fsm.states = st;
        fsm.states_size = ST_MAX;
        fsm.defstid = ST1_ID;
        fsm.tr = tr;
        fsm.tr_size = sizeof(tr)/sizeof(tr[0]);
    }
    void teardown()
    {
    }
};

TEST(sfsm_fixture, test_start_and_default_transition_to_st1)
{
    st[ST1_ID].on_enter = st1_enter;
    st[ST2_ID].on_enter = NULL;
    mock().expectOneCall("reccall").withParameter("param", "Entering st1");
    fsm_start(&fsm);

    // on st2: no on_enter defined - no call
}

TEST(sfsm_fixture, test_on_enter_on_exit)
{
    st[ST1_ID].on_enter = st1_enter_def_transition_to_st2;
    st[ST1_ID].on_exit = st1_exit;
    st[ST2_ID].on_enter = st2_enter;
    st[ST2_ID].on_exit = NULL;

    mock().expectOneCall("reccall").withParameter("param", "Entering st1 going st2");
    mock().expectOneCall("reccall").withParameter("param", "Exit st1");
    mock().expectOneCall("reccall").withParameter("param", "Entering st2");

    fsm_start(&fsm);
}

TEST(sfsm_fixture, test_transition_on_event)
{
    st[ST2_ID].on_exit = st2_exit;
    st[ST3_ID].on_enter = st3_enter;

    mock().expectOneCall("reccall").withParameter("param", "Exit st2");
    mock().expectOneCall("reccall").withParameter("param", "Entering st3");

    fsm_start(&fsm);
    fsm_ev(&fsm, &ev5);
}

TEST(sfsm_fixture, test_transition_on_event_with_handler)
{
    st[ST2_ID].on_exit = st2_exit;
    st[ST3_ID].on_enter = st3_enter;

    mock().expectOneCall("reccall").withParameter("param", "Exit st2");
    mock().expectOneCall("reccall").withParameter("param", "Entering st3");
    mock().expectOneCall("reccall").withParameter("param", "Hand6");

    fsm_start(&fsm);
    fsm_ev(&fsm, &ev5);
    fsm_ev(&fsm, &ev6);
}

TEST(sfsm_fixture, test_self_transition)
{
    st[ST3_ID].on_enter = st3_enter;
    st[ST3_ID].on_exit = st3_exit;

    mock().expectOneCall("reccall").withParameter("param", "Entering st3");
    mock().expectOneCall("reccall").withParameter("param", "Exit st3");
    mock().expectOneCall("reccall").withParameter("param", "Hand7");
    mock().expectOneCall("reccall").withParameter("param", "Entering st3");

    fsm_start(&fsm);
    fsm_ev(&fsm, &ev5); // tr from 2 to 3
    fsm_ev(&fsm, &ev7); // tr from 3 to 3 - is reduced to call ev handler
}

int main(int ac, char* av[])
{
    MockSupportPlugin mockPlugin;
    TestRegistry::getCurrentRegistry()->installPlugin(&mockPlugin);
    return CommandLineTestRunner::RunAllTests(ac, av);
}

