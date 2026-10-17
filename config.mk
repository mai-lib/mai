# XXX until we stop using i686/get valgrind fixed/ship omg
# (it chokes on avx-on-ia32)
#ifeq "$(shell hostname)" "charon"
#CTUNE := -march=core2 -mtune=generic
#else
CTUNE := -march=native -mtune=native
#endif

# we don't use fp exceptions or errno, so don't ask for them. also, vectorize.
CMATH := -fno-signaling-nans -fno-trapping-math -fno-math-errno -mfpmath=sse
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
