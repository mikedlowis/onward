/**
  @file onward_sys.h
  @brief TODO: Describe this file
  $Revision$
  $HeadURL$
*/
#ifndef ONWARD_SYS_H
#define ONWARD_SYS_H

#include "onward.h"

#ifndef ARG_STACK_SZ
#define ARG_STACK_SZ (64 * sizeof(value_t))
#endif

#ifndef RET_STACK_SZ
#define RET_STACK_SZ (64 * sizeof(value_t))
#endif

#ifndef WORD_BUF_SZ
#define WORD_BUF_SZ (8192 / sizeof(value_t))
#endif

extern value_t Argument_Stack[ARG_STACK_SZ];

extern value_t Return_Stack[RET_STACK_SZ];

extern value_t Word_Buffer[WORD_BUF_SZ];

value_t fetch_char(void);

void emit_char(value_t val);

#endif /* ONWARD_SYS_H */
