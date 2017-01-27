



$(BUILDDIR)/test:\
	$(OBJDIR)/test/main.o\
	$(DEPDIR)/test/main.d\
	$(BUILDDIR)/libs/app.a\
	$(BUILDDIR)/libs/search.a\
	$(BUILDDIR)/libs/util.a\
	$(BUILDDIR)/libs/domain.a
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) -o $@\
		-Wl,--start-group\
		$(OBJDIR)/test/main.o\
		$(BUILDDIR)/libs/app.a\
		$(BUILDDIR)/libs/search.a\
		$(BUILDDIR)/libs/util.a\
		$(BUILDDIR)/libs/domain.a\
		-Wl,--end-group
		


.PHONY: tests
tests: $(BUILDDIR)/test



-include $(DEPDIR)/test/main.d


