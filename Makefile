include files.mk
include defs.mk
export sources

# problem: .e must be made before .d
# problem: if .d is made before srcs are extracted, headers won't be resolved
# correctly (ie "global.h" instead of "src/global.h src/util.h src/array.h
# ...") in prereqs
# solution: split makefile in two. first makes .e, re-executes,
# extracts sources; then calls the second, which makes .d &c.

# see also http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/

DOC ?= final
# XXX could remove this, at the cost of lifting the includes and build target above all
.DEFAULT_GOAL := build

.PHONY: all doc build check $(admin)
.SECONDARY: $(out).bbl

all: build doc

doc: $(out).pdf

vers.tex.inc:
	./vers.sh > "$@"

# ---------------
# tempy temp temp
# FIXME not working correctly
# %.bbl:  %.bib | %.aux
# 	$(call echo,"BIB","$|")
# 	bibtex -terse $| >/dev/null

# %.aux:  %.tex
# 	$(call echo,"AUX","$<")
# 	-pdflatex -interaction nonstopmode -file-line-error $< >/dev/null

# pdf from tex
# FIXME die if pdflatex not found
# 'Rerun' -> run again
# 'Warning: There were undefined citations.' -> run bibtex and run again
#%.pdf:	%.tex %.bbl
# $(call echo,"PDF","$<")
# -pdflatex -interaction nonstopmode -file-line-error $< >/dev/null
# 	pdflatex -interaction nonstopmode -file-line-error "\def\target{$(doctarget)}\input $<"
#	while pdflatex -interaction nonstopmode -file-line-error $< | \
#		grep -Fq 'Rerun'; do echo "..."; done
# ---------------
# output pdf from texes
$(out).pdf: $(out).tex $(texs) $(bib).bib config.tex.inc vers.tex.inc
	$(call echo,"PDF","$<")
	while { \
	    pdflatex -interaction nonstopmode -file-line-error \
	             "\def\target{\target$(DOC)}\input $<" >/dev/null; \
	        sed -ne '/^Package natbib Warning: Citation/q2;# \
	                 /^Package rerunfilecheck Warning: File/q1;# \
	                 /^LaTeX Warning: Label(s) may/q1;' < $(out).log; \
	    res=$$?; [ "$$res" != "0" ]; }; do \
	    if [ "$$res" = "2" ]; then \
	        echo "(bib)"; \
	        bibtex -terse $(out).aux >/dev/null; \
	        grep '^Warning' $(bib).blg; \
	    fi; \
	    echo "..."; \
	done; \
# ---------------


# doc from noweb
%.tex: %.nw
# | cpif $@ if needed
	$(call echo,"WEAVE","$<")
	$(call remarkup,$<) | noweave -latex -delay -filter ./esc-und.sh > $@

# code from noweb
$(srcdir)/%:
# make sure some dep rule has given us a prereq; can probably be
# dispensed with when debugged
	@[ ! -z "$^" ] || { echo "no prereq for this target!"; exit 1; }
	$(call echo,"TANGLE","$* <- $<")
	$(call mkoutdir)
# tangle the chunk named by the stem (ie without src/) from the
# prereq to the target file
	$(call remarkup,$<) | notangle -R$* | cpif $@

# sourcedeps from noweb
$(depdir)/%.e: %.nw
	$(call echo,"SRCDEPS","$<")
	$(call mkoutdir)
	$(call mknwdeps,$<,$@)


# -L'#line %L "%F"%N'

ifeq (,$(filter $(admin),$(MAKECMDGOALS)))
# initialise source list; accumulated by sourcedeps
sources :=
# one sourcedep file for each noweb file
-include $(srcwebs:%.nw=$(depdir)/%.e)
endif

# extract sources and run build
build: $(sources)
	$(MAKE) -f Build.mk

# mai: run the smoke tests in tests/
check: build
	./tests/run.sh $(builddir)/main

# bootstrap sources from a designated file
#bootsrcs := (shell $(call nwchunks,$(boot)))

# extract the makefiles and scripts
#bootstrap: $(bootsrcs)

#$(bootsrcs): $(boot)
# preserve tabs, since there are makefiles
#	@echo "BOOTSTRAP $<"
#	notangle -R$@ -t8 $< | cpif $@

# leave the sources so rebuild works without noweb
clean:
	$(call echo,"CLEAN")
	-rm -rf $(outdoc) $(out).log $(out).toc writeup.tar.gz \
	        $(texs:.tex=.aux) $(out).out $(out).bbl *.blg *.tex \
	        $(builddir) vers.tex.inc

distclean: clean
	$(call echo,"DISTCLEAN")
# do not delete the bootstrap outputs, needless to say
	-rm -rf $(outdoc) $(out).log $(out).toc writeup.tar.gz \
	        $(texs:.tex=.aux) $(out).out $(out).bbl *.blg *.tex \
	        $(srcdir) $(builddir) $(depdir) vers.tex.inc

# make a tarball
archive:
	tar zcf writeup.tar.gz $(wildcard *.nw) Makefile *.mk README writeup.bib \
	        config.tex.inc esc-und.sh vers.sh $(depdir) $(srcdir)

# silent unless V=1
$(V).SILENT:
