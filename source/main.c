#include "onward.h"
#include <stdio.h>

#define STACK_SZ (64u)
value_t Arg_Stack_Buf[STACK_SZ];
value_t Ret_Stack_Buf[STACK_SZ];
char     Input_Line[1024];
value_t Ram_Data_Buf[8192/sizeof(value_t)];

defcode("dumpw", dumpw, LATEST_BUILTIN, 0u) {
    word_t* word = (word_t*)onward_aspop();
    printf("name:\t'%s'\n", word->name);
    printf("flags:\t%#lx\n", word->flags);
    printf("link:\t%p\n", word->link);
    /* Print the word's instructions */
    if (word->flags & F_PRIMITIVE_MSK) {
        printf("code:\t%p\n", word->code);
    } else {
        printf("code:");
        word_t** code = (word_t**)word->code;
        while(*code) {
            printf("\t%s\n", (*code)->name);
            code++;
        }
        printf("\tret\n");
    }
}

void print_stack(void) {
    value_t* base = (value_t*)asb;
    value_t* top  = (value_t*)asp;
    int i;
    printf("( ");
    if (top-5 >= base)
        printf("... ");
    for (i = 4; i >= 0; i--) {
        value_t* curr = top-i;
        if (curr > base)
            printf("%zd ", *curr);
    }
    puts(")");
    puts(!errno ? "OK." : "?");
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    onward_init_t init_data = {
        Arg_Stack_Buf,
        STACK_SZ,
        Ret_Stack_Buf,
        STACK_SZ,
        Ram_Data_Buf,
        8192/sizeof(value_t),
        (word_t*)&dumpw
    };
    onward_init(&init_data);

    printf(":> ");
    while(0 != (input = (value_t)fgets(Input_Line, 1024u, stdin))) {
        errno = 0;
        while (*((char*)input) != '\0')
            interp_code();
        print_stack();
        printf(state ? ".. " : ":> ");
    }

    return 0;
}

