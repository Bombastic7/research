

$(BUILDDIR)/experiment/gridnav/searcher:\
	$(SRCDIR)/experiment/gridnav/searcher_auto.cc\
	$(DEPDIR)/experiment/gridnav/searcher_auto.d\
	$(OBJDIR)/experiment/gridnav/searcher_auto.o\
	$(BUILDDIR)/libs/app.a\
	$(BUILDDIR)/libs/search.a\
	$(BUILDDIR)/libs/domain.a\
	$(BUILDDIR)/libs/util.a
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) -o $@\
		-Wl,--start-group\
		$(OBJDIR)/experiment/gridnav/searcher_auto.o\
		$(BUILDDIR)/libs/app.a\
		$(BUILDDIR)/libs/search.a\
		$(BUILDDIR)/libs/domain.a\
		$(BUILDDIR)/libs/util.a\
		-Wl,--end-group
		

$(BUILDDIR)/experiment/gridnav/%.py: $(SRCDIR)/experiment/gridnav/%.py
	@echo $@
	cp $< $@


BINS+=$(BUILDDIR)/experiment/gridnav/searcher $(BUILDDIR)/experiment/gridnav/configuration.py $(BUILDDIR)/experiment/gridnav/gen_problems.py

-include $(DEPDIR)/experiment/gridnav/searcher_auto.d


.PHONY: gridnav_src

gridnav_src:
	python $(SRCDIR)/experiment/gridnav/gen_searcher.py
