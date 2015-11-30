#include <onward.h>
#include <onward_sys.h>

/* System Calls
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>

static void syscall_open(void)
{
    intptr_t modenum = onward_aspop();
    char* fname = (char*)onward_aspop();
    char* mode;
    switch (modenum) {
        case 0: mode = "r";   break;
        case 1: mode = "w";   break;
        case 2: mode = "a";   break;
        case 3: mode = "r+";  break;
        case 4: mode = "w+";  break;
        case 5: mode = "a+";  break;
        default: mode = NULL; break;
    }
    onward_aspush(mode ? (intptr_t)fopen(fname, mode) : 0);
}

static void syscall_close(void)
{
    onward_aspush(fclose((FILE*)onward_aspop()));
}

static void syscall_read(void)
{
    size_t nbytes = (size_t)onward_aspop();
    FILE* fhndl   = (FILE*)onward_aspop();
    void* dest    = (void*)onward_aspop();
    onward_aspush(nbytes != fread(dest, 1u, nbytes, fhndl));
}

static void syscall_write(void)
{
    size_t nbytes = (size_t)onward_aspop();
    void* src     = (void*)onward_aspop();
    FILE* fhndl   = (FILE*)onward_aspop();
    onward_aspush(nbytes != fwrite(src, 1u, nbytes, fhndl));
}

static void syscall_seek(void)
{
    intptr_t nbytes = onward_aspop();
    intptr_t origin = onward_aspop();
    FILE* fhndl     = (FILE*)onward_aspop();
    origin = (origin == 0) ? SEEK_CUR : (origin < 0) ? SEEK_SET : SEEK_END;
    onward_aspush(fseek(fhndl, nbytes, origin));
}

static void syscall_alloc(void)
{
    onward_aspush((intptr_t)malloc((size_t)onward_aspop()));
}

static void syscall_free(void)
{
    free((void*)onward_aspop());
}

typedef void (*syscall_fn_t)(void);

static syscall_fn_t System_Calls[7] = {
    /* File Operations */
    &syscall_open,
    &syscall_close,
    &syscall_read,
    &syscall_write,
    &syscall_seek,

    /* Memory Operations */
    &syscall_alloc,
    &syscall_free,
};

/* Standalone Interpreter
 *****************************************************************************/
#include <stdbool.h>

static bool Newline_Consumed = false;
value_t Argument_Stack[ARG_STACK_SZ];
value_t Return_Stack[RET_STACK_SZ];
value_t Word_Buffer[WORD_BUF_SZ];

defvar("infile",  infile,  0u, LATEST_BUILTIN);
defvar("outfile", outfile, 0u, &infile_word);
defvar("errfile", errfile, 0u, &outfile_word);
defcode("syscall", syscall, &errfile_word, 0u) {
    System_Calls[onward_aspop()]();
}

defcode("dumpw", dumpw, &syscall, 0u) {
    word_t* word = (word_t*)onward_aspop();
    printf("name:\t'%s'\n", word->name);
    printf("flags:\t%#zx\n", word->flags);
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
            printf("%#zx ", *curr);
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
        errcode = 0;
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
    latest  = (value_t)&dumpw;
    infile  = (value_t)stdin;
    outfile = (value_t)stdout;
    errfile = (value_t)stderr;
    /* Load any dictionaries specified on the  command line */
    for (i = 1; i < argc; i++)
        parse_file(argv[i]);
    printf("Memory Usage: %zd / %zd\n", here - (value_t)Word_Buffer, sizeof(Word_Buffer));
    /* Start the REPL */
    parse(stdin);
    return 0;
}
