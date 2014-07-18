#include <CppUTest/TestHarness.h>
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestRegistry.h>
#include <CppUTestExt/MockSupport.h>
#include <CppUTestExt/MockSupportPlugin.h>
#include <cstdio>
#include "../src/sfsm.h"

class eventComparator : public MockNamedValueComparator
{
public:
    virtual bool isEqual(void* object1, void* object2)
    {
        event_t* ev1 = (event_t*)object1;
        event_t* ev2 = (event_t*)object2;

        return ev1->evid == ev2->evid
            && ev1->data == ev2->data;
    }

    virtual SimpleString valueToString(void* object)
    {
        return StringFrom(object);
    }
};

enum {EV1_ID, EV2_ID, EV3_ID, EV4_ID, EV5_ID, EV6_ID, EV7_ID, EV8_ID};
enum {ST1_ID, ST2_ID, ST3_ID, ST_MAX};

fsm_t fsm;

void reccall(const char* param) { mock().actualCall("reccall").withParameter("param", param); };
void rechand(event_t* ev) { mock().actualCall("rechand").withParameter("ev", ev); }

void hand1(event_t* ev) { reccall("Hand1"); }
void hand2(event_t* ev) { reccall("Hand2"); }
void hand3(event_t* ev) { reccall("Hand3"); }
void hand6(event_t* ev) { reccall("Hand6"); }
void hand7(event_t* ev) { reccall("Hand7"); }
void hand8(event_t* ev)
{ 
    reccall("Hand8");
    rechand(ev);
}
void st1_enter(state_t* st) { reccall("Entering st1"); }
void st2_enter(state_t* st) { reccall("Entering st2"); }
void st3_enter(state_t* st) { reccall("Entering st3"); }
void st1_exit(state_t* st) { reccall("Exit st1"); }
void st2_exit(state_t* st) { reccall("Exit st2"); }
void st3_exit(state_t* st) { reccall("Exit st3"); }

event_t ev1 = {EV1_ID};
event_t ev2 = {EV2_ID};
event_t ev3 = {EV3_ID};
event_t ev4 = {EV4_ID};
event_t ev5 = {EV5_ID};
event_t ev6 = {EV6_ID};
event_t ev7 = {EV7_ID};
event_t ev8 = {EV8_ID};
/* Row equal state id, column mean nothing or could equal max events in one of state */
tr_t tr[] = {
    /*sid_from, sid_to, ev, handler */
    {ST1_ID, ST2_ID, FSM_DEF_TR, NULL},
    /* line group states */
    {ST2_ID, FSM_NO_STATE, ev3, hand3},
    {ST2_ID, ST1_ID, ev4, NULL},
    {ST2_ID, ST3_ID, ev5, NULL},
    {ST2_ID, ST2_ID, ev8, hand8},

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
        fsm.tr = tr;
        fsm.tr_size = sizeof(tr)/sizeof(tr[0]);
        fsm.defstid = ST1_ID;
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
    st[ST1_ID].on_enter = st1_enter;
    st[ST1_ID].on_exit = st1_exit;
    st[ST2_ID].on_enter = st2_enter;
    st[ST2_ID].on_exit = NULL;

    mock().expectOneCall("reccall").withParameter("param", "Entering st1");
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

TEST(sfsm_fixture, test_transition_on_event_next_default_transition)
{
    st[ST1_ID].on_enter = st1_enter;
    st[ST1_ID].on_exit = st1_exit;
    st[ST2_ID].on_enter = st2_enter;
    st[ST2_ID].on_exit = st2_exit;

    // default transition
    mock().expectOneCall("reccall").withParameter("param", "Entering st1");
    mock().expectOneCall("reccall").withParameter("param", "Exit st1");
    mock().expectOneCall("reccall").withParameter("param", "Entering st2");

    fsm_start(&fsm);

    // rection on event with default transition st1->st2
    mock().expectOneCall("reccall").withParameter("param", "Exit st2");
    mock().expectOneCall("reccall").withParameter("param", "Entering st1");
    mock().expectOneCall("reccall").withParameter("param", "Exit st1");
    mock().expectOneCall("reccall").withParameter("param", "Entering st2");
    fsm_ev(&fsm, &ev4);
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

TEST(sfsm_fixture, test_event_handle_but_no_transition)
{
    fsm_start(&fsm);

    mock().expectOneCall("reccall").withParameter("param", "Hand3");
    fsm_ev(&fsm, &ev3);
}

TEST(sfsm_fixture, test_passing_data_in_event)
{
    fsm_start(&fsm);
    ev8.data = (void*)666;
    event_t evx = {EV8_ID};
    evx.data = (void*)666;

    mock().expectOneCall("reccall").withParameter("param", "Hand8");
    mock().expectOneCall("rechand").withParameterOfType("event_t*", "ev", &ev8);
    fsm_ev(&fsm, &ev8);
}

int main(int ac, char* av[])
{
    MockSupportPlugin mockPlugin;

    eventComparator evcmp;
    mockPlugin.installComparator("event_t*", evcmp);

    TestRegistry::getCurrentRegistry()->installPlugin(&mockPlugin);
    return CommandLineTestRunner::RunAllTests(ac, av);
}

