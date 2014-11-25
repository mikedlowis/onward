// Unit Test Framework Includes
#include "atf.h"

// File To Test
#include "onward.h"

void state_reset(void);

static intptr_t get_const(const word_t* word) {
    ((primitive_t)word->code)();
   return onward_aspop();
}

static bool is_var(intptr_t* data, const word_t* word) {
    ((primitive_t)word->code)();
    return ((intptr_t)data == onward_aspop());
}

//-----------------------------------------------------------------------------
// Begin Unit Tests
//-----------------------------------------------------------------------------
TEST_SUITE(Constants_And_Variables) {
    TEST(Verify_constants_are_the_expected_values)
    {
        state_reset();
        CHECK(0               == get_const(&VERSION_word));
        CHECK(CELLSZ          == get_const(&CELLSZ_word));
        CHECK(SYS_BITCOUNT    == get_const(&BITCOUNT_word));
        CHECK(F_PRIMITIVE_MSK == get_const(&F_PRIMITIVE_word));
        CHECK(F_HIDDEN_MSK    == get_const(&F_HIDDEN_word));
        CHECK(F_IMMEDIATE_MSK == get_const(&F_IMMEDIATE_word));
    }

    TEST(Verify_variables_behave_as_expected)
    {
        state_reset();
        CHECK(is_var(&pc, &pc_word));
        CHECK(is_var(&asb, &asb_word));
        CHECK(is_var(&asp, &asp_word));
        CHECK(is_var(&rsb, &rsb_word));
        CHECK(is_var(&rsp, &rsp_word));
        CHECK(is_var(&input, &input_word));
        CHECK(is_var(&errno, &errno_word));
        CHECK(is_var(&latest, &latest_word));
        CHECK(is_var(&state, &state_word));
        CHECK(is_var(&here, &here_word));
    }
}
