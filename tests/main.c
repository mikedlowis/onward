#include "atf.h"
#include "onward.h"

static value_t Arg_Stack_Buf[32u];
static value_t Ret_Stack_Buf[32u];
static value_t Ram_Data_Buf[8192/sizeof(value_t)];

void state_reset(void) {
    /* Initialize the system */
    onward_init_t init_data = {
        Arg_Stack_Buf,
        32u,
        Ret_Stack_Buf,
        32u,
        Ram_Data_Buf,
        8192/sizeof(value_t),
        (word_t*)LATEST_BUILTIN
    };
    onward_init(&init_data);
    errno = 0;
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
