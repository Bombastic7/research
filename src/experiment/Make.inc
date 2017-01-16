

$(BUILDDIR)/experiment/searcher:\
	$(SRCDIR)/experiment/searcher_auto.cc\
	$(DEPDIR)/experiment/searcher_auto.d\
	$(OBJDIR)/experiment/searcher_auto.o\
	$(BUILDDIR)/libs/app.a\
	$(BUILDDIR)/libs/search.a\
	$(BUILDDIR)/libs/domain.a\
	$(BUILDDIR)/libs/util.a
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) -o $@\
		-Wl,--start-group\
		$(OBJDIR)/experiment/searcher_auto.o\
		$(BUILDDIR)/libs/app.a\
		$(BUILDDIR)/libs/search.a\
		$(BUILDDIR)/libs/domain.a\
		$(BUILDDIR)/libs/util.a\
		-Wl,--end-group
		

$(BUILDDIR)/experiment/%.py: $(SRCDIR)/experiment/%.py
	@echo $@
	cp $< $@

EXPERIMENT_FILES:=\
	$(BUILDDIR)/experiment/searcher\
	$(BUILDDIR)/experiment/configuration.py\
	$(BUILDDIR)/experiment/gen_tiles_problems.py\
	$(BUILDDIR)/experiment/gen_pancake_problems.py\
	$(BUILDDIR)/experiment/gen_gridnav_problems.py\
	$(BUILDDIR)/experiment/gen_searcher.py\
	$(BUILDDIR)/experiment/plot_problem_bars.py\
	$(BUILDDIR)/experiment/ugsa_analyse.py

BINS+=$(EXPERIMENT_FILES)

-include $(DEPDIR)/experiment/searcher_auto.d


.PHONY: exp_src exp_bins

exp_src:
	python $(SRCDIR)/experiment/configuration.py gencode $(SRCDIR)/experiment/searcher_auto.cc


exp_bin: $(EXPERIMENT_FILES)
