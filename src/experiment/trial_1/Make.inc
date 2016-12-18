
$(SRCDIR)/experiment/trial_1/searcher.cc: $(SRCDIR)/experiment/trial_1/gen_searcher.py $(SRCDIR)/experiment/trial_1/alg_dom_defs.py
	@echo $@
	@python $(SRCDIR)/experiment/trial_1/gen_searcher.py > $(SRCDIR)/experiment/trial_1/searcher.cc

CLEAN+=$(SRCDIR)/experiment/trial_1/searcher.cc



$(BUILDDIR)/trial_1/searcher:\
	$(SRCDIR)/experiment/trial_1/searcher.cc\
	$(DEPDIR)/experiment/trial_1/searcher.d\
	$(OBJDIR)/experiment/trial_1/searcher.o\
	$(DEPDIR)/experiment/trial_1/main_searcher.d\
	$(OBJDIR)/experiment/trial_1/main_searcher.o\
	$(BUILDDIR)/libs/app.a\
	$(BUILDDIR)/libs/search.a\
	$(BUILDDIR)/libs/util.a
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) -o $@\
		-Wl,--start-group\
		$(OBJDIR)/experiment/trial_1/searcher.o\
		$(OBJDIR)/experiment/trial_1/main_searcher.o\
		$(BUILDDIR)/libs/app.a\
		$(BUILDDIR)/libs/search.a\
		$(BUILDDIR)/libs/util.a\
		-Wl,--end-group
		



BINS+=$(BUILDDIR)/trial_1/searcher

-include $(DEPDIR)/experiment/trial_1/searcher.d




TRIAL_1_FILES:=$(addprefix $(EXPDIR)/trial_1/, gen_problems.py gen_runs.py trial_1.txt searcher alg_dom_defs.py exec_runs.py)


$(EXPDIR)/trial_1/%: $(SRCDIR)/experiment/trial_1/%
	@echo $@
	@mkdir -p $(@D)
	@cp $< $@


$(EXPDIR)/trial_1/searcher: $(BUILDDIR)/trial_1/searcher
	@echo $@
	@mkdir -p $(@D)
	@cp $< $@

BINS+=$(TRIAL_1_FILES)
