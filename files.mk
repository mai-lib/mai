
# administrative targets
# (i.e. do not require deps)
admin:=clean archive

# directories
srcdir := src
builddir := build
depdir := deps

# root noweb
root := writeup.nw
# bootstrap noweb
boot := build.nw
# all (other) nowebs
allwebs := $(wildcard *.nw)
srcwebs := $(filter-out $(root) $(boot),$(wildcard *.nw))
# bibtex
bib := writeup


# outputs
outbin := $(builddir)/main
outdoc := $(root:%.nw=%.pdf)
out := $(root:%.nw=%)

# all (other) tex files
texs := $(srcwebs:%.nw=%.tex)
# all .c source files
srcs = $(filter %.c,$(sources))
srcs += $(call srcfilt,%.c.in,%.c)
srcs += $(call srcfilt,%.y,%.tab.c)
# all .h files
hdrs = $(filter %.h,$(sources))
hdrs += $(call srcfilt,%.h.in,%.h)
hdrs += $(call srcfilt,%.y,%.h)
# all object files
objs = $(srcs:$(srcdir)/%.c=$(builddir)/%.o)
