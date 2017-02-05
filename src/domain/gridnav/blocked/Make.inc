

$(BUILDDIR)/experiment/gridnav/test_connected:\
	$(DEPDIR)/domain/gridnav/blocked/solvable_main.d\
	$(OBJDIR)/domain/gridnav/blocked/solvable_main.o\
	$(BUILDDIR)/libs/app.a\
	$(BUILDDIR)/libs/search.a\
	$(BUILDDIR)/libs/domain.a\
	$(BUILDDIR)/libs/util.a
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) -o $@\
		-Wl,--start-group\
		$(OBJDIR)/domain/gridnav/blocked/solvable_main.o\
		$(BUILDDIR)/libs/app.a\
		$(BUILDDIR)/libs/search.a\
		$(BUILDDIR)/libs/domain.a\
		$(BUILDDIR)/libs/util.a\
		-Wl,--end-group
		


BINS+=$(BUILDDIR)/experiment/gridnav/test_connected

-include $(DEPDIR)/domain/gridnav/blocked/solvable_main.d
