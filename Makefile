#------------------------------------------------------------------------------
# Build Configuration
#------------------------------------------------------------------------------
# Update these variables according to your requirements.

# version
VERSION = 0.0.1

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
# Build-Specific Macros
#------------------------------------------------------------------------------
# Main build settings
LIBNAME = onward
LIB     = lib${LIBNAME}.a
BIN     = ${LIBNAME}
DEPS    = ${OBJS:.o=.d}
OBJS    = source/onward.o source/main.o

# Distribution dir and tarball settings
DISTDIR   = ${LIBNAME}-${VERSION}
DISTTAR   = ${DISTDIR}.tar
DISTGZ    = ${DISTTAR}.gz
DISTFILES = config.mk LICENSE.md Makefile README.md source tests

# load user-specific settings
-include config.mk

#------------------------------------------------------------------------------
# Phony Targets
#------------------------------------------------------------------------------
.PHONY: all options dist

all: options ${LIB} ${BIN}

options:
	@echo "Toolchain Configuration:"
	@echo "  CC       = ${CC}"
	@echo "  CFLAGS   = ${CFLAGS}"
	@echo "  LD       = ${LD}"
	@echo "  LDFLAGS  = ${LDFLAGS}"
	@echo "  AR       = ${AR}"
	@echo "  ARFLAGS  = ${ARFLAGS}"

dist: clean
	@echo DIST ${DISTGZ}
	@mkdir -p ${DISTDIR}
	@cp -R ${DISTFILES} ${DISTDIR}
	@tar -cf ${DISTTAR} ${DISTDIR}
	@gzip ${DISTTAR}
	@rm -rf ${DISTDIR}

clean:
	${CLEAN} ${LIB} ${BIN} ${OBJS} ${DEPS}
	${CLEAN} ${OBJS:.o=.gcno} ${OBJS:.o=.gcda}
	${CLEAN} ${DEPS} ${TEST_DEPS}
	${CLEAN} ${DISTTAR} ${DISTGZ}

#------------------------------------------------------------------------------
# Target-Specific Rules
#------------------------------------------------------------------------------
.c.o:
	${COMPILE}

${LIB}: ${OBJS}
	${ARCHIVE}

${BIN}: ${LIB}
	${LINK}

# load dependency files
-include ${DEPS}

