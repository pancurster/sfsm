#include <CppUTest/TestHarness.h>
#include <CppUTest/CommandLineTestRunner.h>

TEST_GROUP(sfsm_fixture)
{
    void setup()
    {
    }
    void deardown()
    {
    }
};

TEST(sfsm_fixture, test1)
{
    CHECK(true == false);
}

int main(int ac, char* av[])
{
    return CommandLineTestRunner::RunAllTests(ac, av);
}
