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

# commands
COMPILE = @echo CC $@; ${CC} ${CFLAGS} -c -o $@ $<
LINK    = @echo LD $@; ${LD} -o $@ $^ ${LDFLAGS}
ARCHIVE = @echo AR $@; ${AR} ${ARFLAGS} $@ $^
CLEAN   = @rm -f

#------------------------------------------------------------------------------
# Build Targets and Rules
#------------------------------------------------------------------------------
LIBNAME = onward
LIB     = lib${LIBNAME}.a
BIN     = ${LIBNAME}
DEPS    = ${OBJS:.o=.d}
OBJS    = source/onward.o source/main.o

# load user-specific settings
-include config.mk

all: options ${LIB} ${BIN}

options:
	@echo "Toolchain Configuration:"
	@echo "  CC       = ${CC}"
	@echo "  CFLAGS   = ${CFLAGS}"
	@echo "  LD       = ${LD}"
	@echo "  LDFLAGS  = ${LDFLAGS}"
	@echo "  AR       = ${AR}"
	@echo "  ARFLAGS  = ${ARFLAGS}"

${LIB}: ${OBJS}
	${ARCHIVE}

${BIN}: ${LIB}
	${LINK}

.c.o:
	${COMPILE}

clean:
	${CLEAN} ${LIB} ${BIN} ${OBJS} ${DEPS}
	${CLEAN} ${OBJS:.o=.gcno} ${OBJS:.o=.gcda}

# load dependency files
-include ${DEPS}

.PHONY: all options

