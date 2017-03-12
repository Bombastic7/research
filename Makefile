CXX:=g++-5
CC:=gcc-5

DEFS:=-DDEBUGLEVEL=2 -DNDEBUG=1
FLAGS:=-Wall -Werror -g -O2 $(DEFS)
CXXFLAGS:=$(FLAGS) -std=c++0x
LDFLAGS:=$(FLAGS) -std=c++0x


ROOTDIR=$(CURDIR)

SRCDIR:=src
OBJDIR:=build/obj
DEPDIR:=build/obj
BUILDDIR:=build



BINS:=
TESTBINS:=
CLEAN:=
all: everything

include src/app/Make.inc
include src/structs/Make.inc
include src/util/Make.inc
include src/search/Make.inc
include src/domain/Make.inc
include src/experiment/Make.inc
include src/test/Make.inc


everything: $(BINS) $(TESTBINS)
apps: $(BINS)
#tests: $(TESTBINS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cc
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) -c $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) -o $@ $<

$(DEPDIR)/%.d: $(SRCDIR)/%.cc
	@echo $@
	@mkdir -p $(@D)
	@./dep.sh $(CXX) $(@D) $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) $< > $@




.PHONY: clean

clean:
	rm -rf $(OBJDIR) $(DEPDIR) $(BUILDDIR)/libs $(CLEAN)
