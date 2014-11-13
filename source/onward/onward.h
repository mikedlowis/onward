/**
  @file onward.h
  @brief TODO: Describe this file
*/
#ifndef ONWARD_H
#define ONWARD_H

#include <stdint.h>
#if defined(BITS_16)
    typedef int16_t value_t;
#elif defined(BITS_32)
    typedef int32_t value_t;
#elif defined(BITS_64)
    typedef int64_t value_t;
#else
    typedef intptr_t value_t;
#endif

/** This structure represents a word definition */
typedef struct word_t {
    /** Pointer to the next most recently defined word in the dictionary. */
    struct word_t const* link;
    /** A collection of flags describing attributes of the word. */
    value_t flags;
    /** Pointer to the null terminated string that holds the name of the word. */
    char const* name;
    /** A pointer to the list of instructions that make up this word. For words
     * defined in C this will be 0u (NULL). */
    value_t* code;
} word_t;

/** Type definition for the C function associated with primitive words */
typedef void (*primitive_t)(void);

#define deccode(c_name)       \
    void c_name##_code(void); \
    const word_t c_name       \

/** Define a built-in word that executes native code */
#define defcode(name_str, c_name, prev, flags) \
    extern void c_name##_code(void);           \
    extern const char c_name##_str[];          \
    const word_t c_name = {                    \
        prev,                                  \
        F_PRIMITIVE_MSK | flags,               \
        c_name##_str,                          \
        (value_t*)&c_name##_code,              \
    };                                         \
    const char c_name##_str[] = name_str;      \
    void c_name##_code(void)

#define decword(c_name) \
    const word_t c_name \

/** Define a built-in word that is defined by references to other words. */
#define defword(name_str, c_name, prev, flags) \
    extern const value_t c_name##_code[];      \
    extern const char c_name##_str[];          \
    const word_t c_name = {                    \
        prev,                                  \
        flags,                                 \
        c_name##_str,                          \
        (value_t*)c_name##_code                \
    };                                         \
    const char c_name##_str[] = name_str;      \
    const value_t c_name##_code[] =

#define decvar(c_name)         \
    value_t c_name;            \
    const word_t c_name##_word \

/** Define a built-in word representing a variable with the provided value */
#define defvar(name_str, c_name, initial, prev)  \
    value_t c_name = initial;                    \
    defcode(name_str, c_name##_word, prev, 0u) { \
        onward_aspush((value_t)&c_name);         \
    }

#define decconst(c_name)       \
    const value_t c_name;      \
    const word_t c_name##_word \

/** Define a built-in word representing a constant with the provided value */
#define defconst(name_str, c_name, value, prev)  \
    const value_t c_name = value;                \
    defcode(name_str, c_name##_word, prev, 0u) { \
        onward_aspush(c_name);                   \
    }

/** The number of bits that make up a stack cell */
#define SYS_BITCOUNT ((value_t)(sizeof(value_t) * 8u))

/** Bit mask to retrieve the "primitive" flag */
#define F_PRIMITIVE_MSK ((value_t)((value_t)1u << (SYS_BITCOUNT-1u)))

/** Bit mask to retrieve the "hidden" flag */
#define F_HIDDEN_MSK ((value_t)((value_t)1u << (SYS_BITCOUNT-2u)))

/** Bit mask to retrieve the "immediate" flag */
#define F_IMMEDIATE_MSK ((value_t)((value_t)1u << (SYS_BITCOUNT-3u)))

/** Macro to get use the word pointer in a defined word */
#define W(name) ((value_t)&name)

decconst(VERSION);
decconst(CELLSZ);
decconst(BITCOUNT);
decconst(F_PRIMITIVE);
decconst(F_HIDDEN);
decconst(F_IMMEDIATE);
decvar(pc);
decvar(asb);
decvar(asp);
decvar(rsb);
decvar(rsp);
decvar(input);
decvar(errno);
decvar(latest);
decvar(state);
decvar(here);
deccode(word);
deccode(num);
deccode(lit);
deccode(find);
deccode(exec);
deccode(create);
deccode(comma);
deccode(lbrack);
deccode(rbrack);
decword(colon);
deccode(semicolon);
deccode(tick);
deccode(br);
deccode(zbr);
deccode(interp);
deccode(fetch);
deccode(store);
deccode(add_store);
deccode(sub_store);
deccode(byte_fetch);
deccode(byte_store);
deccode(block_copy);
deccode(drop);
deccode(swap);
deccode(dup);
deccode(dup_if);
deccode(over);
deccode(rot);
deccode(nrot);
deccode(add);
deccode(sub);
deccode(mul);
deccode(divide);
deccode(mod);
deccode(eq);
deccode(ne);
deccode(lt);
deccode(gt);
deccode(lte);
deccode(gte);
deccode(band);
deccode(bor);
deccode(bxor);
deccode(bnot);

value_t onward_pcfetch(void);
void onward_aspush(value_t val);
value_t onward_aspeek(value_t val);
value_t onward_aspop(void);
void onward_rspush(value_t val);
value_t onward_rspop(void);
void onward_init(value_t* arg_stack, value_t* ret_stack, value_t* ram_buf, word_t* latest_word);

#endif /* ONWARD_H */
