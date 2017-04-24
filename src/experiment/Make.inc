


$(BUILDDIR)/experiment/test_all_doms:\
	$(OBJDIR)/experiment/main_test_all.o\
	$(DEPDIR)/experiment/main_test_all.d\
	$(BUILDDIR)/libs/app.a\
	$(BUILDDIR)/libs/util.a\
	$(BUILDDIR)/libs/domain.a
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) -o $@\
		-Wl,--start-group\
		$(OBJDIR)/experiment/main_test_all.o\
		$(BUILDDIR)/libs/app.a\
		$(BUILDDIR)/libs/util.a\
		$(BUILDDIR)/libs/domain.a\
		-Wl,--end-group


$(BUILDDIR)/experiment/bubbles:\
	$(OBJDIR)/experiment/main_bubbles.o\
	$(DEPDIR)/experiment/main_bubbles.d\
	$(BUILDDIR)/libs/util.a
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) -o $@\
		-Wl,--start-group\
		$(OBJDIR)/experiment/main_bubbles.o\
		$(BUILDDIR)/libs/util.a\
		-Wl,--end-group






-include $(DEPDIR)/experiment/main_test_all.d
-include $(DEPDIR)/experiment/main_bubbles.d

