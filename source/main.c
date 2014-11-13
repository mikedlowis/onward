#include "onward.h"
#include <stdio.h>

//extern value_t input;
//extern value_t state;
//extern value_t errno;
//extern value_t asb;
//extern value_t asp;
//extern value_t rsb;
//extern value_t rsp;

#define STACK_SZ (64u)

value_t Arg_Stack_Buf[STACK_SZ];
value_t Ret_Stack_Buf[STACK_SZ];
char     Input_Line[1024];
value_t Ram_Data_Buf[8192/sizeof(value_t)];

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

    onward_init(Arg_Stack_Buf, Ret_Stack_Buf, Ram_Data_Buf, (word_t*)&bnot);

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

