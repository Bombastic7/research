

$(BUILDDIR)/experiment/pancake/searcher:\
	$(SRCDIR)/experiment/pancake/searcher_auto.cc\
	$(DEPDIR)/experiment/pancake/searcher_auto.d\
	$(OBJDIR)/experiment/pancake/searcher_auto.o\
	$(BUILDDIR)/libs/app.a\
	$(BUILDDIR)/libs/search.a\
	$(BUILDDIR)/libs/domain.a\
	$(BUILDDIR)/libs/util.a
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) -o $@\
		-Wl,--start-group\
		$(OBJDIR)/experiment/pancake/searcher_auto.o\
		$(BUILDDIR)/libs/app.a\
		$(BUILDDIR)/libs/search.a\
		$(BUILDDIR)/libs/domain.a\
		$(BUILDDIR)/libs/util.a\
		-Wl,--end-group
		

$(BUILDDIR)/experiment/pancake/%.py: $(SRCDIR)/experiment/pancake/%.py
	@echo $@
	cp $< $@


BINS+=$(BUILDDIR)/experiment/pancake/searcher $(BUILDDIR)/experiment/pancake/configuration.py

-include $(DEPDIR)/experiment/pancake/searcher_auto.d


.PHONY: pancake_src

pancake_src:
	python $(SRCDIR)/experiment/pancake/gen_searcher.py
