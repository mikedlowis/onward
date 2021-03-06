#include "onward.h"
#include "onward_sys.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

static value_t char_oneof(char ch, char* chs);

/** Version number of the implementation */
defconst("VERSION", VERSION, 0, 0u);

/** Number of bytes in a stack cell */
defconst("CELLSZ", CELLSZ, sizeof(value_t), &VERSION_word);

/** Number of bits that make up a stack cell */
defconst("BITCOUNT", BITCOUNT, SYS_BITCOUNT, &CELLSZ_word);

/** Bit mask to retrieve the "primitive" flag */
defconst("F_PRIMITIVE", F_PRIMITIVE, F_PRIMITIVE_MSK, &BITCOUNT_word);

/** Bit mask to retrieve the "hidden" flag */
defconst("F_HIDDEN", F_HIDDEN, F_HIDDEN_MSK, &F_PRIMITIVE_word);

/** Bit mask to retrieve the "immediate" flag */
defconst("F_IMMEDIATE", F_IMMEDIATE, F_IMMEDIATE_MSK, &F_HIDDEN_word);

/** Counter containing the address of the next word to execute */
defvar("pc", pc, 0, &F_IMMEDIATE_word);

/** The address of the base of the argument stack */
defvar("asb", asb, (value_t)Argument_Stack-1, &pc_word);

/** The size of the argument stack in bytes */
defvar("assz", assz, ARG_STACK_SZ, &asb_word);

/** The address of the top of the argument stack */
defvar("asp", asp, (value_t)Argument_Stack-1, &assz_word);

/** The address of the base of the return stack */
defvar("rsb", rsb, (value_t)Return_Stack-1, &asp_word);

/** The size of the return stack in bytes */
defvar("rssz", rssz, RET_STACK_SZ, &rsb_word);

/** The address of the top of the return stack */
defvar("rsp", rsp, (value_t)Return_Stack-1, &rssz_word);

/** Base of the user-defined word buffer */
defvar("hbase", hbase, (value_t)Word_Buffer, &rsp_word);

/** The address where the next word or instruction will be written */
defvar("here", here, (value_t)Word_Buffer, &hbase_word);

/** Size of the user-defined word buffer */
defvar("hsize", hsize, WORD_BUF_SZ, &here_word);

/** The last generated error code */
defvar("errcode", errcode, 0, &hsize_word);

/** Address of the most recently defined word */
defvar("latest", latest, (value_t)LATEST_BUILTIN, &errcode_word);

/** The current state of the interpreter */
defvar("state", state, 0, &latest_word);

/** Read a character from the default input source */
defcode("key", key, &state_word, 0u) {
    onward_aspush(fetch_char());
}

/** Read a character from the default input source */
defcode("emit", emit, &key, 0u) {
    emit_char((int)onward_aspop());
}

/** Drop the rest of the current line from the default input source */
defcode("\\", dropline, &emit, F_IMMEDIATE_MSK) {
    int curr;
    do {
        ((primitive_t)key.code)();
        curr = (int)onward_aspop();
    } while(((char)curr != '\n') && (curr != EOF));
}

/** Fetches the next word from the input string */
defcode("word", word, &dropline, 0u) {
    static char buffer[32u];
    char* str = buffer;
    int curr;
    /* Skip any whitespace */
    do {
        key_code();
        curr = (int)onward_aspop();
    } while (char_oneof((char)curr, " \t\r\n"));
    /* Copy characters into the buffer */
    while(((int)curr != EOF) && !char_oneof((char)curr, " \t\r\n")) {
        *str++ = (char)curr;
        key_code();
        curr = (int)onward_aspop();
    }
    /* Terminate the string */
    *str = '\0';
    /* Return the internal buffer */
    onward_aspush((value_t)buffer);
}

/** Parses a string as a number literal */
defcode("num", num, &word, 0u) {
    char* word = (char*)onward_aspop();
    char* start = word;
    value_t success = 0;
    value_t value = 0;
    int sign = 1;
    int base = 10;
    char c;
    /* Detect the sign of the number */
    if (*start == '-') {
        sign = -1;
        start++;
    }

    /* Detect the base of the number to parse */
    if (*start == '0') {
        start++;
        switch (*(start++)) {
            case 'b': base = 2;  break;
            case 'o': base = 8;  break;
            case 'd': base = 10; break;
            case 'x': base = 16; break;
            case '\0':
                success = 1;
                value = 0;
            default:  base = -1; break;
        }
    }

    /* Parse the number */
    if (base > 1) {
        for (c = *start++; c != '\0'; c= *start++) {
            /* Get the digit value */
            if ((c >= '0') && (c <= '9'))
                c -= '0';
            else if (((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F')))
                c -= (c >= 'A' && c <= 'Z') ? 'A' - 10 : 'a' - 10;
            else
                break;
            /* Bail if the digit value is too high */
            if (c >= base) break;
            /* Update the accumulated value */
            value = (value * base) + c;
            success = 1;
        }

        /* Convert to the required sign */
        value *= sign;
    }

    /* Push the results back on the stack */
    success = (success && (*(start-1) == '\0'));
    if (success)
        onward_aspush(value);
    else
        onward_aspush((value_t)word);
    onward_aspush(success);
}

/** Push the number pointed to by the program counter onto the argument stack */
defcode("lit", lit, &num, 0u) {
    onward_aspush( onward_pcfetch() );
}

/** Lookup a string in the dictionary */
defcode("find", find, &lit, 0u) {
    const word_t* curr = (const word_t*)latest;
    char* name = (char*)onward_aspop();
    while(curr) {
        if (0 == strcmp(curr->name,name))
            break;
        curr = curr->link;
    }
    onward_aspush((value_t)curr);
}

/** Execute a word */
defcode("exec", exec, &find, 0u) {
    /* Load up the word to be executed, saving off the current state */
    value_t start = rsp;
    word_t* to_exec[] = { (word_t*)onward_aspop(), 0u };
    onward_rspush(pc);
    pc = (value_t)to_exec;
    /* Loop through the instructions of the word until completion */
    do {
        word_t* current = (word_t*)( onward_pcfetch() );
        /* If the current instruction is null then "return" */
        if (0u == current) {
            pc = (value_t)onward_rspop();
        /* if the instruction is a primitive then execute the c function */
        } else if (current->flags & F_PRIMITIVE_MSK) {
            ((primitive_t)current->code)();
        /* else "call" the word by pushing the current context on the stack
         * and loading the instruction register */
        } else {
            onward_rspush(pc);
            pc = (value_t)current->code;
        }
    } while(pc && rsp != start);
}

/** Create a new word definition with default attributes */
defcode("create", create, &exec, 0u) {
    /* Pop the arguments into temporary variables */
    char* name = (char*)onward_aspop();
    /* Copy the name to a more permanent location */
    size_t str_size = strlen(name) + 1;
    size_t new_size = str_size + ((sizeof(value_t) - (str_size % sizeof(value_t))) % sizeof(value_t));
    name = memcpy((void*)here, name, str_size);
    here += new_size;
    /* Start populating the word definition */
    ((word_t*)here)->link  = (word_t*)latest;
    ((word_t*)here)->flags = F_HIDDEN;
    ((word_t*)here)->name  = name;
    ((word_t*)here)->code  = (value_t*)(((word_t*)here) + 1);
    /* Update latest and here variables and initialize the code array */
    latest  = here;
    here   += sizeof(word_t);
    *((value_t*)here) = 0u;
}

/** Append a word to the latest word definition */
defcode(",", comma, &create, 0u) {
    *((value_t*)here)  = onward_aspop();
    here              += sizeof(value_t);
    *((value_t*)here)  = 0u;
}

/** Set the interpreter mode to "interpret" */
defcode("[", lbrack, &comma, F_IMMEDIATE_MSK) {
    state = 0;
}

/** Set the interpreter mode to "compile" */
defcode("]", rbrack, &lbrack, 0u) {
    state = 1;
}

/** Start a new word definition */
defword(":", colon, &rbrack, 0u) {
    W(word), W(create), W(rbrack), 0u
};

/** Start a new word definition */
defcode(";", semicolon, &colon, F_IMMEDIATE_MSK) {
    ((word_t*)latest)->flags &= ~F_HIDDEN;
    here += sizeof(value_t);
    state = 0;
}

/** Retrieve the next word to execute and put it on the stack */
defcode("'", tick, &semicolon, 0u) {
    onward_aspush(onward_pcfetch());
}

/** Branch unconditionally to the offset specified by the next instruction */
defcode("br", br, &tick, 0u) {
    pc += *((value_t*)pc);
}

/** Branch to the offset specified by the next instruction if the top item on
 * the stack is 0 */
defcode("0br", zbr, &br, 0u) {
    if (!onward_aspop())
        pc += *((value_t*)pc);
    else
        pc += sizeof(intptr_t);
}

/** Take the input string, tokenize it, and execute or compile each word */
defcode("interp", interp, &zbr, 0u) {
    /* Grab the next word of input */
    word_code();
    /* if we actually got anything */
    if (strlen((char*)onward_aspeek(0)) > 0) {
        /* Try to parse it as a number */
        num_code();
        /* If it's a number */
        if (onward_aspop()) {
            /* If we're compiling, then append the number to the word */
            if (state == 1) {
                onward_aspush((intptr_t)&lit);
                comma_code();
                comma_code();
            }
        /* otherwise, look it up */
        } else {
            char* name = (char*)onward_aspeek(0);
            /* Lookup the word in the dictionary */
            find_code();
            /* If we found a definition execute it */
            if (onward_aspeek(0)) {
                /* If we are in immediate more or the word is immediate */
                if((state == 0) || (((word_t*)onward_aspeek(0))->flags & F_IMMEDIATE))
                {
                    exec_code();
                }
                /* Otherwise, compile it! */
                else
                {
                    comma_code();
                }
            /* Report an error */
            } else {
                errcode = ERR_UNKNOWN_WORD;
                (void)onward_aspop();
                printf("Unknown word: %s\n", name);
            }
        }
    /* Otherwise, discard it */
    } else {
        (void)onward_aspop();
    }
}

/* Memory Access Words
 *****************************************************************************/
/** Fetch the value at the given address and place it on the stack */
defcode("@", fetch, &interp, 0u) {
    onward_aspush( *((value_t*)onward_aspop()) );
}

/** Store the top item on the stack at the address represented by the second
 * item on the stack */
defcode("!", store, &fetch, 0u) {
    value_t  val  = onward_aspop();
    value_t* addr = (value_t*)onward_aspop();
    *(addr) = val;
}

/** Add the given amount to the value at the given location */
defcode("+!", add_store, &store, 0u) {
    value_t  val  = onward_aspop();
    value_t* addr = (value_t*)onward_aspop();
    *(addr) += val;
}

/** Subtract the given ammount from the value at the given location */
defcode("-!", sub_store, &add_store, 0u) {
    value_t  val  = onward_aspop();
    value_t* addr = (value_t*)onward_aspop();
    *(addr) -= val;
}

/** Fetch a byte from the given location */
defcode("b@", byte_fetch, &sub_store, 0u) {
    onward_aspush( (value_t)*((char*)onward_aspop()) );
}

/** Store a byte in an address at the given location */
defcode("b!", byte_store, &byte_fetch, 0u) {
    char val   = (char)onward_aspop();
    char* addr = (char*)onward_aspop();
    *(addr) = val;
}

/** Copy a block of memory to a new location */
defcode("bmove", block_copy, &byte_store, 0u) {
    size_t length = (size_t)onward_aspop();
    void*  dest   = (void*)onward_aspop();
    void*  source = (void*)onward_aspop();
    memmove(dest, source, length);
}

/* Common Stack Manipulation Words
 *****************************************************************************/
/* Discards the top item on the stack */
defcode("drop", drop, &block_copy, 0u) {
    (void)onward_aspop();
}

/* Swaps the order of the top two items on the stack */
defcode("swap", swap, &drop, 0u) {
    value_t temp1 = onward_aspop();
    value_t temp2 = onward_aspop();
    onward_aspush(temp1);
    onward_aspush(temp2);
}

/* Duplicates the top item of the stack */
defcode("dup", _dup, &swap, 0u) {
    onward_aspush(onward_aspeek(0));
}

/* Duplicates the first item on the stack if the item is non-zero */
defcode("?dup", dup_if, &_dup, 0u) {
    if (onward_aspeek(0)) onward_aspush(onward_aspeek(0));
}

/* Duplicate the second item on the stack */
defcode("over", over, &dup_if, 0u) {
    onward_aspush(onward_aspeek(-1));
}

/* Rotate the top three items such that the second item becomes the first and
 * the first item becomes the third */
defcode("rot", rot, &over, 0u) {
    value_t temp1 = onward_aspop();
    value_t temp2 = onward_aspop();
    value_t temp3 = onward_aspop();
    onward_aspush(temp1);
    onward_aspush(temp3);
    onward_aspush(temp2);
}

/* Rotate the top three items such that the third item becomes the first and
 * the first item becomes the second */
defcode("-rot", nrot, &rot, 0u) {
    value_t temp1 = onward_aspop();
    value_t temp2 = onward_aspop();
    value_t temp3 = onward_aspop();
    onward_aspush(temp2);
    onward_aspush(temp1);
    onward_aspush(temp3);
}

/* Arithmetic Words
 *****************************************************************************/
/** Add the top two items on the stack */
defcode("+", add, &nrot, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval + rval);
}

/** Subtract the top two items on the stack */
defcode("-", sub, &add, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval - rval);
}

/** Multiply the top two items on the stack */
defcode("*", mul, &sub, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval * rval);
}

/** Divide the top two items on the stack */
defcode("/", divide, &mul, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval / rval);
}

/** Modulo the top two items on the stack */
defcode("%", mod, &divide, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval % rval);
}

/* Boolean Logic Words
 *****************************************************************************/
/** Test if the top two items on the stack are equal */
defcode("=", eq, &mod, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval == rval);
}

/** Test if the top two items on the stack are not equal */
defcode("<>", ne, &eq, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval != rval);
}

/** Test if the second item is less than the first item */
defcode("<", lt, &ne, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval < rval);
}

/** Test if the second item is greater than the first item */
defcode(">", gt, &lt, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval > rval);
}

/** Test if the second item is less than or equal to the first item */
defcode("<=", lte, &gt, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval <= rval);
}

/** Test if the second item is greater than or equal to the first item */
defcode(">=", gte, &lte, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval >= rval);
}

/* Bitwise Operation Words
 *****************************************************************************/
/** Bitwise AND the top two items */
defcode("&", band, &gte, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval & rval);
}

/** Bitwise OR the top two items */
defcode("|", bor, &band, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval | rval);
}

/** Bitwise XOR the top two items */
defcode("^", bxor, &bor, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval ^ rval);
}

/** Bitwise NOT the top two items */
defcode("~", bnot, &bxor, 0u) {
    onward_aspush(~onward_aspop());
}

/* Helper C Functions
 *****************************************************************************/
value_t onward_pcfetch(void) {
    value_t* reg = (value_t*)pc;
    value_t  val = *reg++;
    pc = (value_t)reg;
    return val;
}

void onward_aspush(value_t val) {
    asp += sizeof(value_t);
    assert(asp <= (asb + ARG_STACK_SZ));
    *((value_t*)asp) = val;
}

value_t onward_aspeek(value_t val) {
    uintptr_t location = asp + (val * sizeof(value_t));
    assert(location > asb);
    return *((value_t*)(location));
}

value_t onward_aspop(void) {
    value_t val = *((value_t*)asp);
    asp -= sizeof(value_t);
    assert(asp >= asb);
    return val;
}

void onward_rspush(value_t val) {
    rsp += sizeof(value_t);
    assert(rsp <= (rsb + RET_STACK_SZ));
    *((value_t*)rsp) = val;
}

value_t onward_rspop(void) {
    value_t val = *((value_t*)rsp);
    rsp -= sizeof(value_t);
    assert(rsp >= rsb);
    return val;
}

static value_t char_oneof(char ch, char* chs) {
    value_t ret = 0;
    while(*chs != '\0') {
        if (ch == *chs) {
            ret = 1;
            break;
        }
        chs++;
    }
    return ret;
}

