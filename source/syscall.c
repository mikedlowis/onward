#include "syscall.h"
#include "onward.h"
#include <stdio.h>
#include <stdlib.h>

void syscall_open(void)
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

void syscall_close(void)
{
    onward_aspush(fclose((FILE*)onward_aspop()));
}

void syscall_read(void)
{
    size_t nbytes = (size_t)onward_aspop();
    FILE* fhndl   = (FILE*)onward_aspop();
    void* dest    = (void*)onward_aspop();
    onward_aspush(nbytes != fread(dest, 1u, nbytes, fhndl));
}

void syscall_write(void)
{
    size_t nbytes = (size_t)onward_aspop();
    void* src     = (void*)onward_aspop();
    FILE* fhndl   = (FILE*)onward_aspop();
    onward_aspush(nbytes != fwrite(src, 1u, nbytes, fhndl));
}

void syscall_seek(void)
{
    intptr_t nbytes = onward_aspop();
    intptr_t origin = onward_aspop();
    FILE* fhndl     = (FILE*)onward_aspop();
    origin = (origin == 0) ? SEEK_CUR : (origin < 0) ? SEEK_SET : SEEK_END;
    onward_aspush(fseek(fhndl, nbytes, origin));
}

void syscall_alloc(void)
{
    onward_aspush((intptr_t)malloc((size_t)onward_aspop()));
}

void syscall_free(void)
{
    free((void*)onward_aspop());
}

syscall_fn_t System_Calls[7] = {
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

