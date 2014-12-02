#include "atf.h"
#include "onward.h"

char* input = "";
value_t Word_Buffer[1024 / sizeof(value_t)];

value_t fetch_char(void)
{
    return (value_t)*input++;
}

void emit_char(value_t val)
{
    (void)val;
}

void state_reset(void) {
    /* Initialize the system */
    asp = asb;
    rsp = rsb;
    errcode = 0;
    state = 0;
    here = (value_t)Word_Buffer;
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    /* Run the tests and report the results */
    RUN_EXTERN_TEST_SUITE(Constants_And_Variables);
    RUN_EXTERN_TEST_SUITE(Interpreter);
    return PRINT_TEST_RESULTS();
}
