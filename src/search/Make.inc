

SEARCH_LIB_OBJS :=

include src/search/hastar/generic/Make.inc
include src/search/ugsa/v2/Make.inc
include src/search/ugsa/v2_bf/Make.inc

$(BUILDDIR)/libs/search.a: $(SEARCH_LIB_OBJS)
	@echo $@
	@mkdir -p $(@D)
	@ar rcs $@ $(SEARCH_LIB_OBJS)
		
BINS+=$(BUILDDIR)/libs/search.a

