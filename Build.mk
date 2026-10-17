include files.mk
include defs.mk
include config.mk

.PHONY: all lib

# don't delete sources or headers if they happen to be intermediates
.SECONDARY: $(srcs) $(hdrs)

all: $(outbin)

$(outbin): $(binobjs)
	$(call echo,"LINK","$@")
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

# mai: the embedding library, shared and static
lib: $(outlib) $(outarc)

$(outlib): $(libobjs)
	$(call echo,"LINK","$@")
	$(LINK.o) -shared $^ $(LOADLIBES) $(LDLIBS) -o $@

$(outarc): $(libobjs)
	$(call echo,"AR","$@")
	rm -f $@
	$(AR) rcs $@ $^

# objdeps from source
# all headers as prereqs, so it picks up correct paths
$(depdir)/%.d: $(srcdir)/%.c $(hdrs)
	$(call echo,"OBJDEPS","$<")
	$(call mkoutdir)
	$(call mkcdeps,$<,$(builddir)/$*.o,$@)
#	CFLAGS="${CFLAGS}" ./mkcdep.sh $< $(builddir)/$*.o > $@

# m4 expansion from in
% :: %.in
	$(call echo,"M4","$^")
	m4 -P -I$(srcdir)/ $^ > $@

# c and h from yacc
%.tab.c %.h: %.y
# bison puts outputs in cwd by default, even if inputs have paths.
# needless to say, we do not want this.
	$(call echo,"YACC","$<")
	bison --defines=$*.h --file-prefix=$* -Wno-other $^
#	bison -o $@ --report=states,itemset,lookahead,solved -Wno-other $^

# o from c
$(builddir)/%.o: $(srcdir)/%.c
	$(call echo,"CC","$<")
	$(call mkoutdir)
	$(COMPILE.c) $(OUTPUT_OPTION) $<

ifeq (,$(filter $(admin),$(MAKECMDGOALS)))
# one objdep file for each source file
-include $(srcs:$(srcdir)/%.c=$(depdir)/%.d)
endif
