
$(SRCDIR)/experiment/trial_1/searcher_auto.cc:\
	$(SRCDIR)/experiment/trial_1/gen_searcher.py\
	$(SRCDIR)/experiment/trial_1/functions.py\
	$(SRCDIR)/experiment/trial_1/gen_domain_decls.py
	@echo $@
	@python $(SRCDIR)/experiment/trial_1/functions.py searcher

CLEAN+=$(SRCDIR)/experiment/trial_1/searcher_auto.cc



$(BUILDDIR)/trial_1/searcher:\
	$(SRCDIR)/experiment/trial_1/searcher_auto.cc\
	$(DEPDIR)/experiment/trial_1/searcher_auto.d\
	$(OBJDIR)/experiment/trial_1/searcher_auto.o\
	$(BUILDDIR)/libs/app.a\
	$(BUILDDIR)/libs/search.a\
	$(BUILDDIR)/libs/util.a
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) -o $@\
		-Wl,--start-group\
		$(OBJDIR)/experiment/trial_1/searcher_auto.o\
		$(BUILDDIR)/libs/app.a\
		$(BUILDDIR)/libs/search.a\
		$(BUILDDIR)/libs/util.a\
		-Wl,--end-group
		



BINS+=$(BUILDDIR)/trial_1/searcher

-include $(DEPDIR)/experiment/trial_1/searcher_auto.d




TRIAL_1_FILES:=$(addprefix $(EXPDIR)/trial_1/, functions.py gen_problems.py gen_domain_decls.py gen_searcher.py trial_1.txt searcher)


$(EXPDIR)/trial_1/%: $(SRCDIR)/experiment/trial_1/%
	@echo $@
	@mkdir -p $(@D)
	@cp $< $@


$(EXPDIR)/trial_1/searcher: $(BUILDDIR)/trial_1/searcher
	@echo $@
	@mkdir -p $(@D)
	@cp $< $@

BINS+=$(TRIAL_1_FILES)
