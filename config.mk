# XXX until we stop using i686/get valgrind fixed/ship omg
# (it chokes on avx-on-ia32)
#ifeq "$(shell hostname)" "charon"
#CTUNE := -march=core2 -mtune=generic
#else
# mai: CTUNE may be overridden, e.g. `make CTUNE=` for a portable binary
# (as CI does); the default tunes for the build machine as the thesis did.
CTUNE ?= -march=native -mtune=native
#endif

# we don't use fp exceptions or errno, so don't ask for them. also, vectorize.
CMATH := -fno-signaling-nans -fno-trapping-math -fno-math-errno
# mai: -mfpmath=sse is only meaningful (and accepted) on x86
ifneq (,$(filter x86_64 i%86 amd64,$(shell uname -m)))
CMATH := $(CMATH) -mfpmath=sse
endif
# we don't consider zero signed, and associativity is specified at the interpreter level.
CMATH := $(CMATH) -fno-signed-zeros -fassociative-math

# -DDBG_SUPPRESS
ifdef DEBUG
CDEBUG := -Og -g3 -ggdb -UNDEBUG -fstrict-aliasing
endif
override CFLAGS := -std=c11 -fno-common $(CTUNE) $(CMATH) \
	-I$(srcdir) -O3 -DNDEBUG $(CDEBUG) -Wall -Wstrict-aliasing -Winline \
	-D_GNU_SOURCE $(CFLAGS)
#  -minline-all-stringops rubs valgrind the wrong way
override LDFLAGS := -ggdb $(LDFLAGS)
override LDLIBS := -lm -lreadline $(LDLIBS)
