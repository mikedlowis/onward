#------------------------------------------------------------------------------
# Build Configuration
#------------------------------------------------------------------------------
# Update these variables according to your requirements.

# tools
CC = c99
LD = ${CC}
AR = ar

# flags
INCS      = -Isource/ -Itests/
CPPFLAGS  = -D_XOPEN_SOURCE=700
CFLAGS   += ${INCS} ${CPPFLAGS}
LDFLAGS  += ${LIBS}
ARFLAGS   = rcs

#------------------------------------------------------------------------------
# Build Targets and Rules
#------------------------------------------------------------------------------
SRCS = source/onward.c source/main.c
OBJS = ${SRCS:.c=.o}
LIB  = libonward.a
BIN  = onward
TEST_SRCS = tests/atf.c tests/main.c tests/test_interpreter.c tests/test_vars.c
TEST_OBJS = ${TEST_SRCS:.c=.o}
TEST_BIN  = testonward

all: options ${BIN} ${TEST_BIN}

options:
	@echo "Toolchain Configuration:"
	@echo "  CC       = ${CC}"
	@echo "  CFLAGS   = ${CFLAGS}"
	@echo "  LD       = ${LD}"
	@echo "  LDFLAGS  = ${LDFLAGS}"
	@echo "  AR       = ${AR}"
	@echo "  ARFLAGS  = ${ARFLAGS}"
	@echo "  ARFLAGS  = ${MAKEDEPEND}"

${LIB}: ${OBJS}
	@echo AR $@
	@${AR} ${ARFLAGS} $@ ${OBJS}

${BIN}: ${LIB}
	@echo LD $@
	@${LD} -o $@ ${LIB} ${LDFLAGS}


${TEST_BIN}: ${TEST_OBJS} ${LIB}
	@echo LD $@
	@${LD} -o $@ ${TEST_OBJS} ${LIB} ${LDFLAGS}
	-./$@

.c.o:
	@echo CC $<
	@${CC} ${CFLAGS} -c -o $@ $<

clean:
	@rm -f ${LIB} ${BIN} ${TEST_BIN} ${OBJS} ${TEST_OBJS}

.PHONY: all options

