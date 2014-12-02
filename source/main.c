#include "onward_sys.h"
#include "syscall.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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
            printf("\t%s", (*code)->name);
            if ((*code == &lit) || (*code == &zbr) || (*code == &br))
                printf(" %zd", (intptr_t)*(++code));
            code++;
            puts("");
        }
        printf("\tret\n");
    }
}

defcode("syscall", syscall, &dumpw, 0u) {
    System_Calls[onward_aspop()]();
}

defvar("infile",  infile,  0u, &syscall);
defvar("outfile", outfile, 0u, &infile_word);
defvar("errfile", errfile, 0u, &outfile_word);

/*****************************************************************************/
bool Newline_Consumed = false;

value_t fetch_char(void)
{
    value_t ch = (value_t)fgetc((FILE*)infile);
    if ((char)ch == '\n')
        Newline_Consumed = true;
    return ch;
}

void emit_char(value_t val)
{
    fputc((int)val, (FILE*)outfile);
}

void print_stack(void) {
    value_t* base = (value_t*)asb;
    value_t* top  = (value_t*)asp;
    printf("( ");
    int i;
    if (top-5 >= base)
        printf("... ");
    for (i = 4; i >= 0; i--) {
        value_t* curr = top-i;
        if (curr > base)
            printf("%zd ", *curr);
    }
    puts(")");
    printf("errcode: %zd\n", errcode);
    puts(!errcode ? "OK." : "?");
}

void parse(FILE* file) {
    value_t old = infile;
    infile = (value_t)file;
    if (file == stdin)
        printf(":> ");
    while (!feof(file)) {
        interp_code();
        if ((file == stdin) && Newline_Consumed) {
            print_stack();
            printf(":> ");
            Newline_Consumed = false;
            errcode = 0;
        }
    }
    infile = old;
}

void parse_file(char* fname) {
    FILE* file = fopen(fname, "r");
    if (file) {
        parse(file);
        fclose(file);
    }
}

int main(int argc, char** argv) {
    int i;
    /* Initialize implementation specific words */
    latest  = (value_t)&errfile_word;
    infile  = (value_t)stdin;
    outfile = (value_t)stdout;
    errfile = (value_t)stderr;
    /* Load any dictionaries specified on the  command line */
    for (i = 1; i < argc; i++)
        parse_file(argv[i]);
    /* Start the REPL */
    parse(stdin);
    return 0;
}

