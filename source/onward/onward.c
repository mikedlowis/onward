#include "onward.h"
#include <string.h>

static value_t char_oneof(char ch, char* chs);
static void abort_on_error(value_t cond, value_t errcode);

/** Version number of the implementation */
defconst("VERSION", VERSION, 0, 0u);

/** Number of bytes in a stack cell */
defconst("CELLSZ", CELLSZ, sizeof(value_t), &VERSION_word);

/** Number of bits that make up a stack cell */
defconst("BITCOUNT", BITCOUNT, SYS_BITCOUNT, &VERSION_word);

/** Bit mask to retrieve the "primitive" flag */
defconst("F_PRIMITIVE", F_PRIMITIVE, F_PRIMITIVE_MSK, &BITCOUNT_word);

/** Bit mask to retrieve the "hidden" flag */
defconst("F_HIDDEN", F_HIDDEN, F_HIDDEN_MSK, &F_PRIMITIVE_word);

/** Bit mask to retrieve the "immediate" flag */
defconst("F_IMMEDIATE", F_IMMEDIATE, F_IMMEDIATE_MSK, &F_HIDDEN_word);

/** Counter containing the address of the next word to execute */
defvar("pc", pc, 0, &F_IMMEDIATE_word);

/** The address of the base of the argument stack */
defvar("asb", asb, 0, &pc_word);

/** The size of the argument stack in bytes */
defvar("assz", assz, 0, &asb_word);

/** The address of the top of the argument stack */
defvar("asp", asp, 0, &assz_word);

/** The address of the base of the return stack */
defvar("rsb", rsb, 0, &asp_word);

/** The size of the return stack in bytes */
defvar("rssz", rssz, 0, &rsb_word);

/** The address of the top of the return stack */
defvar("rsp", rsp, 0, &rssz_word);

/** The address of the current input string */
defvar("input", input, 0, &rsp_word);

/** The last generated error code */
defvar("errno", errno, 0, &input_word);

/** Address of the most recently defined word */
defvar("latest", latest, 0, &errno_word);

/** The current state of the interpreter */
defvar("state", state, 0, &latest_word);

/** The address where the next word or instruction will be written */
defvar("here", here, 0, &state_word);

/** Fetches the next word from the input string */
defcode("word", word, &here_word, 0u) {
    static char buffer[32u];
    char* str = buffer;
    /* Skip any whitespace */
    while(char_oneof(*((char*)input), " \t\r\n"))
        input++;
    /* Copy characters into the buffer */
    while((*((char*)input) != '\0') && !char_oneof(*((char*)input), " \t\r\n"))
        *(str++) = *((char*)input++);
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

defcode("abort", _abort, &find, 0u) {
    asp = asb;
    rsp = rsb;
    pc  = 0u;
}

/** Execute a word */
defcode("exec", exec, &_abort, 0u) {
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
    size_t new_size = str_size + (sizeof(value_t) - (str_size % sizeof(value_t)));
    name = memcpy((void*)here, name, str_size);
    here += new_size;
    /* Start populating the word definition */
    ((word_t*)here)->link  = (word_t*)latest;
    ((word_t*)here)->flags = F_HIDDEN;
    ((word_t*)here)->name  = name;
    ((word_t*)here)->code  = (value_t*)(((word_t*)here) + 1);
    /* Update latest and here variables and initialize the code array */
    latest = here;
    here   = (value_t)(((word_t*)here) + 1);
    *((value_t*)here) = 0u;
}

/** Append a word to the latest word definition */
defcode(",", comma, &create, 0u) {
    *((value_t*)here++) = onward_aspop();
    *((value_t*)here)   = 0u;
}

/** Set the interpreter mode to "interpret" */
defcode("[", lbrack, &comma, 0u) {
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
    state = 0;
}

/** Retrieve the next word to execute and put it on the stack */
defcode("'", tick, &semicolon, 0u) {
    onward_aspush(onward_pcfetch());
}

/** Branch unconditionally to the offset specified by the next instruction */
defcode("br", br, &tick, 0u) {
    pc += (*((value_t*)pc) * sizeof(value_t));
}

/** Branch to the offset specified by the next instruction if the top item on
 * the stack is 0 */
defcode("0br", zbr, &br, 0u) {
    if (!onward_aspop())
        pc += (*((value_t*)pc) * sizeof(value_t));
}

/** Take the input string, tokenize it, and execute or compile each word */
defcode("interp", interp, &zbr, 0u) {
    /* Grab the next word of input */
    word_code();
    /* if we actually got anything */
    if (strlen((char*)onward_aspeek(0)) > 0) {
        /* Try to parse it as a number */
        num_code();
        /* If it's not a number */
        if (!onward_aspop()) {
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
            } else {
                errno = ERR_UNKNOWN_WORD;
                (void)onward_aspop();
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
    *((value_t*)onward_aspop()) = onward_aspop();
}

defcode("+!", add_store, &store, 0u) {
    *((value_t*)onward_aspop()) += onward_aspop();
}

defcode("-!", sub_store, &add_store, 0u) {
    *((value_t*)onward_aspop()) -= onward_aspop();
}

defcode("b@", byte_fetch, &sub_store, 0u) {
    onward_aspush( (value_t)*((char*)onward_aspop()) );
}

defcode("b!", byte_store, &byte_fetch, 0u) {
    *((char*)onward_aspop()) = (char)onward_aspop();
}

defcode("bmove", block_copy, &byte_store, 0u) {
    size_t length = (size_t)onward_aspop();
    void*  dest   = (void*)onward_aspop();
    void*  source = (void*)onward_aspop();
    memcpy(dest, source, length);
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
defcode("dup", dup, &swap, 0u) {
    onward_aspush(onward_aspeek(0));
}

/* Duplicates the first item on the stack if the item is non-zero */
defcode("?dup", dup_if, &dup, 0u) {
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
defcode("+", add, &nrot, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval + rval);
}

defcode("-", sub, &add, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval - rval);
}

defcode("*", mul, &sub, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval * rval);
}

defcode("/", divide, &mul, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval / rval);
}

defcode("%", mod, &divide, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval % rval);
}

/* Boolean Logic Words
 *****************************************************************************/
defcode("=", eq, &mod, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval == rval);
}

defcode("<>", ne, &eq, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval != rval);
}

defcode("<", lt, &ne, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval < rval);
}

defcode(">", gt, &lt, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval > rval);
}

defcode("<=", lte, &gt, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval <= rval);
}

defcode(">=", gte, &lte, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval >= rval);
}

/* Bitwise Operation Words
 *****************************************************************************/
defcode("&", band, &gte, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval & rval);
}

defcode("|", bor, &band, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval | rval);
}

defcode("^", bxor, &bor, 0u) {
    value_t rval = onward_aspop();
    value_t lval = onward_aspop();
    onward_aspush(lval ^ rval);
}

defcode("~", bnot, &bxor, 0u) {
    onward_aspush(~onward_aspop());
}

/* Helper C Functions
 *****************************************************************************/

void onward_init(onward_init_t* init_data) {
    asb    = (value_t)(init_data->arg_stack - 1);
    asp    = (value_t)(init_data->arg_stack - 1);
    assz   = (value_t)(init_data->arg_stack_sz * sizeof(value_t));
    rsb    = (value_t)(init_data->ret_stack - 1);
    rsp    = (value_t)(init_data->ret_stack - 1);
    rssz   = (value_t)(init_data->ret_stack_sz * sizeof(value_t));
    here   = (value_t)(init_data->word_buf);
    latest = (value_t)(init_data->latest);
}

value_t onward_pcfetch(void) {
    value_t* reg = (value_t*)pc;
    value_t  val = *reg++;
    pc = (value_t)reg;
    return val;
}

void onward_aspush(value_t val) {
    #ifndef UNSAFE_MODE
    if ((asp-asb) <= assz) {
    #endif
        asp += sizeof(value_t);
        *((value_t*)asp) = val;
    #ifndef UNSAFE_MODE
    } else {
        abort_on_error(1, ERR_ARG_STACK_OVRFLW);
    }
    #endif
}

value_t onward_aspeek(value_t val) {
    return *((value_t*)(asp + (val * sizeof(value_t))));
}

value_t onward_aspop(void) {
    value_t val = *((value_t*)asp);
    asp -= sizeof(value_t);
    abort_on_error(asp < asb, ERR_ARG_STACK_UNDRFLW);
    return val;
}

void onward_rspush(value_t val) {
    #ifndef UNSAFE_MODE
    if ((rsp-rsb) <= rssz) {
    #endif
        rsp += sizeof(value_t);
        *((value_t*)rsp) = val;
    #ifndef UNSAFE_MODE
    } else {
        abort_on_error(1, ERR_RET_STACK_OVRFLW);
    }
    #endif
}

value_t onward_rspop(void) {
    value_t val = *((value_t*)rsp);
    rsp -= sizeof(value_t);
    abort_on_error(rsp < rsb, ERR_RET_STACK_UNDRFLW);
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

static void abort_on_error(value_t cond, value_t errcode) {
    #ifndef UNSAFE_MODE
    if(cond) {
        errno = errcode;
        _abort_code();
    }
    #endif
}
