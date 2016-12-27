

$(BUILDDIR)/experiment/tiles/searcher:\
	$(SRCDIR)/experiment/tiles/searcher_auto.cc\
	$(DEPDIR)/experiment/tiles/searcher_auto.d\
	$(OBJDIR)/experiment/tiles/searcher_auto.o\
	$(BUILDDIR)/libs/app.a\
	$(BUILDDIR)/libs/search.a\
	$(BUILDDIR)/libs/domain.a\
	$(BUILDDIR)/libs/util.a
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) -o $@\
		-Wl,--start-group\
		$(OBJDIR)/experiment/tiles/searcher_auto.o\
		$(BUILDDIR)/libs/app.a\
		$(BUILDDIR)/libs/search.a\
		$(BUILDDIR)/libs/domain.a\
		$(BUILDDIR)/libs/util.a\
		-Wl,--end-group
		

$(BUILDDIR)/experiment/tiles/%.py: $(SRCDIR)/experiment/tiles/%.py
	@echo $@
	cp $< $@

TILES_EXP_FILES:=$(BUILDDIR)/experiment/tiles/searcher $(BUILDDIR)/experiment/tiles/configuration.py $(BUILDDIR)/experiment/tiles/gen_problems.py
BINS+=$(TILES_EXP_FILES)

-include $(DEPDIR)/experiment/tiles/searcher_auto.d


.PHONY: tiles_src tiles_exp

tiles_src:
	python $(SRCDIR)/experiment/tiles/gen_searcher.py


tiles_exp: $(TILES_EXP_FILES)
