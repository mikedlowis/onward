// Unit Test Framework Includes
#include "atf.h"
#include <string.h>

// File To Test
#include "onward.h"

void state_reset(void);

//-----------------------------------------------------------------------------
// Begin Unit Tests
//-----------------------------------------------------------------------------
TEST_SUITE(Interpreter) {
    //-------------------------------------------------------------------------
    // Testing: word
    //-------------------------------------------------------------------------
    TEST(Verify_word_reads_the_next_word_and_places_it_on_the_stack)
    {
        state_reset();
        input = (intptr_t)" foo ";
        ((primitive_t)word.code)();
        char* result = (char*)onward_aspop();
        CHECK(0 == strcmp(result, "foo"));
    }

    //-------------------------------------------------------------------------
    // Testing: \
    //-------------------------------------------------------------------------
    TEST(Verify_dropline_drops_the_rest_of_the_given_input)
    {
        state_reset();
        input = (intptr_t)" foo ";
        ((primitive_t)dropline.code)();
        CHECK(0 == strcmp((char*)input, ""));
    }

    //-------------------------------------------------------------------------
    // Testing: num
    //-------------------------------------------------------------------------
    TEST(Verify_num_parses_a_decimal_zero)
    {
        state_reset();
        onward_aspush((intptr_t)"0");
        ((primitive_t)num.code)();
        CHECK(1 == onward_aspop());
        CHECK(0 == onward_aspop());
    }

    TEST(Verify_num_parses_a_negative_decimal_number)
    {
        state_reset();
        onward_aspush((intptr_t)"-42");
        ((primitive_t)num.code)();
        CHECK(1 == onward_aspop());
        CHECK(-42 == onward_aspop());
    }

    TEST(Verify_num_parses_a_positive_decimal_number)
    {
        state_reset();
        onward_aspush((intptr_t)"42");
        ((primitive_t)num.code)();
        CHECK(1 == onward_aspop());
        CHECK(42 == onward_aspop());
    }

    TEST(Verify_num_parses_a_binary_radix_zero)
    {
        state_reset();
        onward_aspush((intptr_t)"0b0");
        ((primitive_t)num.code)();
        CHECK(1 == onward_aspop());
        CHECK(0 == onward_aspop());
    }

    TEST(Verify_num_parses_a_negative_binary_radix_number)
    {
        state_reset();
        onward_aspush((intptr_t)"-0b101010");
        ((primitive_t)num.code)();
        CHECK(1 == onward_aspop());
        CHECK(-42 == onward_aspop());
    }

    TEST(Verify_num_parses_a_positive_binary_radix_number)
    {
        state_reset();
        onward_aspush((intptr_t)"0b101010");
        ((primitive_t)num.code)();
        CHECK(1 == onward_aspop());
        CHECK(42 == onward_aspop());
    }

    TEST(Verify_num_parses_a_octal_radix_zero)
    {
        state_reset();
        onward_aspush((intptr_t)"0o0");
        ((primitive_t)num.code)();
        CHECK(1 == onward_aspop());
        CHECK(0 == onward_aspop());
    }

    TEST(Verify_num_parses_a_negative_octal_radix_number)
    {
        state_reset();
        onward_aspush((intptr_t)"-0o52");
        ((primitive_t)num.code)();
        CHECK(1 == onward_aspop());
        CHECK(-42 == onward_aspop());
    }

    TEST(Verify_num_parses_a_positive_octal_radix_number)
    {
        state_reset();
        onward_aspush((intptr_t)"0o52");
        ((primitive_t)num.code)();
        CHECK(1 == onward_aspop());
        CHECK(42 == onward_aspop());
    }

    TEST(Verify_num_parses_a_decimal_radix_zero)
    {
        state_reset();
        onward_aspush((intptr_t)"0d0");
        ((primitive_t)num.code)();
        CHECK(1 == onward_aspop());
        CHECK(0 == onward_aspop());
    }

    TEST(Verify_num_parses_a_negative_decimal_radix_number)
    {
        state_reset();
        onward_aspush((intptr_t)"-0d42");
        ((primitive_t)num.code)();
        CHECK(1 == onward_aspop());
        CHECK(-42 == onward_aspop());
    }

    TEST(Verify_num_parses_a_positive_decimal_radix_number)
    {
        state_reset();
        onward_aspush((intptr_t)"0d42");
        ((primitive_t)num.code)();
        CHECK(1 == onward_aspop());
        CHECK(42 == onward_aspop());
    }

    TEST(Verify_num_parses_a_hexadecimal_radix_zero)
    {
        state_reset();
        onward_aspush((intptr_t)"0x0");
        ((primitive_t)num.code)();
        CHECK(1 == onward_aspop());
        CHECK(0 == onward_aspop());
    }

    TEST(Verify_num_parses_a_negative_hexadecimal_radix_number)
    {
        state_reset();
        onward_aspush((intptr_t)"-0x2A");
        ((primitive_t)num.code)();
        CHECK(1 == onward_aspop());
        CHECK(-42 == onward_aspop());
    }

    TEST(Verify_num_parses_a_positive_hexadecimal_radix_number)
    {
        state_reset();
        onward_aspush((intptr_t)"0x2a");
        ((primitive_t)num.code)();
        CHECK(1 == onward_aspop());
        CHECK(42 == onward_aspop());
    }

    TEST(Verify_num_fails_to_parse_a_string_containing_chars_of_the_wrong_base)
    {
        state_reset();
        onward_aspush((intptr_t)"0b2");
        ((primitive_t)num.code)();
        CHECK(0 == onward_aspop());
        CHECK(0 == strcmp("0b2", (char*)onward_aspop()));
    }

    TEST(Verify_num_fails_to_parse_a_string_containing_chars_of_the_wrong_base)
    {
        state_reset();
        onward_aspush((intptr_t)"0b02");
        ((primitive_t)num.code)();
        CHECK(0 == onward_aspop());
        CHECK(0 == strcmp("0b02", (char*)onward_aspop()));
    }

    TEST(Verify_num_fails_to_parse_a_string_containing_chars_of_the_wrong_base)
    {
        state_reset();
        onward_aspush((intptr_t)"0b20");
        ((primitive_t)num.code)();
        CHECK(0 == onward_aspop());
        CHECK(0 == strcmp("0b20", (char*)onward_aspop()));
    }

    TEST(Verify_num_fails_to_parse_a_string_containing_chars_of_the_wrong_base)
    {
        state_reset();
        onward_aspush((intptr_t)"0b0Z");
        ((primitive_t)num.code)();
        CHECK(0 == onward_aspop());
        CHECK(0 == strcmp("0b0Z", (char*)onward_aspop()));
    }

    //-------------------------------------------------------------------------
    // Testing: lit
    //-------------------------------------------------------------------------
    TEST(Verify_lit_grabs_the_next_instruction_and_pushes_it_on_the_stack_as_a_literal)
    {
        state_reset();
        intptr_t code[] = { 42 };
        pc = (intptr_t)code;
        ((primitive_t)lit.code)();
        CHECK(42 == onward_aspop());
    }

    //-------------------------------------------------------------------------
    // Testing: find
    //-------------------------------------------------------------------------
    TEST(Verify_find_pushes_the_word_address_on_the_stack_if_found)
    {
        state_reset();
        onward_aspush((intptr_t)"+");
        ((primitive_t)find.code)();
        CHECK((intptr_t)&add == onward_aspop());
    }

    TEST(Verify_find_pushes_null_on_the_stack_if_not_found)
    {
        state_reset();
        onward_aspush((intptr_t)"foo");
        ((primitive_t)find.code)();
        CHECK((intptr_t)NULL == onward_aspop());
    }

    //-------------------------------------------------------------------------
    // Testing: exec
    //-------------------------------------------------------------------------
    TEST(Verify_exec_should_execute_the_word_on_the_top_of_the_stack)
    {
        state_reset();
        onward_aspush(1);
        onward_aspush(1);
        onward_aspush((intptr_t)&add);
        ((primitive_t)exec.code)();
        CHECK(2 == onward_aspop());
        CHECK(asb == asp);
    }

    //-------------------------------------------------------------------------
    // Testing: create
    //-------------------------------------------------------------------------
    TEST(Verify_create_a_new_word_definition)
    {
        state_reset();
        word_t* old_word = (word_t*)latest;
        onward_aspush((intptr_t)"foo");
        ((primitive_t)create.code)();
        word_t* new_word = (word_t*)latest;
        CHECK(F_HIDDEN_MSK == new_word->flags);
        CHECK((intptr_t*)here == new_word->code);
        CHECK(old_word == new_word->link);
        CHECK(0 == *(intptr_t*)here);
    }

    //-------------------------------------------------------------------------
    // Testing: ,
    //-------------------------------------------------------------------------
    TEST(Verify_comma_appends_a_word_to_the_latest_word)
    {
        state_reset();
        intptr_t code[] = { 0, 0 };
        here = (intptr_t)code;
        onward_aspush((intptr_t)&add);
        ((primitive_t)comma.code)();
        CHECK((intptr_t)&add == code[0]);
        CHECK(here == (intptr_t)&code[1]);
    }

    //-------------------------------------------------------------------------
    // Testing: [
    //-------------------------------------------------------------------------
    TEST(Verify_lbrack_switches_to_immediate_mode)
    {
        state_reset();
        state = 1;
        ((primitive_t)lbrack.code)();
        CHECK(0 == state);
    }

    //-------------------------------------------------------------------------
    // Testing: ]
    //-------------------------------------------------------------------------
    TEST(Verify_rbrack_switches_to_compile_mode)
    {
        state_reset();
        state = 0;
        ((primitive_t)rbrack.code)();
        CHECK(1 == state);
    }

    //-------------------------------------------------------------------------
    // Testing: :
    //-------------------------------------------------------------------------
    TEST(Verify_colon_creates_a_new_word_and_switches_to_compile_mode)
    {
        state_reset();
        input = (intptr_t)" foo ";
        onward_aspush((intptr_t)&colon);
        ((primitive_t)exec.code)();
        word_t* old_word = (word_t*)latest;
        onward_aspush((intptr_t)"foo");
        ((primitive_t)create.code)();
        word_t* new_word = (word_t*)latest;
        CHECK(F_HIDDEN_MSK == new_word->flags);
        CHECK((intptr_t*)here == new_word->code);
        CHECK(old_word == new_word->link);
        CHECK(0 == *(intptr_t*)here);
        CHECK(1 == state);
    }

    //-------------------------------------------------------------------------
    // Testing: '
    //-------------------------------------------------------------------------
    TEST(Verify_tick_fetches_next_instruction_and_places_it_on_the_stack)
    {
        state_reset();
        intptr_t code[] = { (intptr_t)&add, 0 };
        pc = (intptr_t)code;
        ((primitive_t)tick.code)();
        CHECK((intptr_t)&add == onward_aspop());
    }

    //-------------------------------------------------------------------------
    // Testing: br
    //-------------------------------------------------------------------------
    TEST(Verify_br_adds_the_offset_to_the_program_counter_uncoditionally)
    {
        state_reset();
        intptr_t code[] = { sizeof(intptr_t)*2, 0, 0 };
        pc = (intptr_t)code;
        ((primitive_t)br.code)();
        CHECK((intptr_t)&code[2] == pc);
    }

    //-------------------------------------------------------------------------
    // Testing: 0br
    //-------------------------------------------------------------------------
    TEST(Verify_br0_adds_the_offset_to_the_program_counter_if_the_top_of_the_stack_is_zero)
    {
        state_reset();
        intptr_t code[] = { sizeof(intptr_t)*2, 0, 0 };
        pc = (intptr_t)code;
        onward_aspush(0);
        ((primitive_t)zbr.code)();
        CHECK((intptr_t)&code[2] == pc);
    }

    TEST(Verify_br0_should_skip_past_the_branch_offset_if_top_of_the_stack_is_non_zero)
    {
        state_reset();
        intptr_t code[] = { sizeof(intptr_t)*2, 0, 0 };
        pc = (intptr_t)code;
        onward_aspush(1);
        ((primitive_t)zbr.code)();
        CHECK((intptr_t)&code[1] == pc);
    }

    //-------------------------------------------------------------------------
    // Testing: interp
    //-------------------------------------------------------------------------
}
