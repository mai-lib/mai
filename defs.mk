
# find arg1s in sources, then substitute arg1s with arg2s
srcfilt=$(patsubst $(1),$(2),$(filter $(1),$(sources)))

# FIXME: play with noweave's -delay argument to see if it's responsible
# for the failure of @-at-bob

# unmangle the prereq noweb markup, from cc-mode-compat to actual noweb
# /*anything at bob -> nil at bob (this is technically a noweb bug, it fails on @ at bob)
# */ at eob -> nil at eob
# /*@ at bol -> @; remove following % if at eol; remove following blank line if any
# ==xxx*/ at bol -> <<xxx>>=
# [spaces]//xxx// at bol and eol -> [spaces]<<xxx>>
# $ doubled, semicolon command separator, because make
# call remarkup,file

define remarkup
sed -E -e \
    '1 s,^/\*.*$$,,; \
     $$ s,^\*/$$,,; \
     \,^/\*@, { s,^/\*,,; s, %$$,,; N; s,\n$$,,; P; D }; \
     s,^==(.*)\*/$$,<<\1>>=,; \
     s,^([ \t]*)//(.*)//$$,\1<<\2>>,' $(1)
endef

# make the parent dirs of the output, if required
define mkoutdir
@-mkdir -p $(@D)
endef

# print something informative
define echo
@echo $(1)'\t'$(2)
endef

# list the root chunks of a web, prepending each with srcdir
# call nwchunks,file,srcdir
define nwchunks
$(call remarkup,$(1)) | noroots | sed "s,<<,$(2),g; s,>>,,g;" | tr '\n' ' '
endef

# emit makerule (and append variable) to extract source files from a web to a srcdep fragment
# call mknwdeps,file,outfile
define mknwdeps
(files=`$(call nwchunks,$(1),$(srcdir)/)` && \
echo "$${files} : $(1)" && \
echo "sources += $${files}") > $(2)
endef

# emit makerule to build an object file from sources to an objdep fragment
# call mkcdeps,file,target,outfile
define mkcdeps
$(COMPILE.c) -MM -MG -MT $(2) $(CFLAGS) $(1) > $(3)
endef

