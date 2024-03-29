


$(ROOTDIR)/experiment/tiles15/run_tiles15:\
	$(OBJDIR)/test/main.o\
	$(DEPDIR)/test/main.d\
	$(BUILDDIR)/libs/app.a\
	$(BUILDDIR)/libs/util.a\
	$(BUILDDIR)/libs/domain.a
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) -o $@\
		-Wl,--start-group\
		$(OBJDIR)/test/main.o\
		$(BUILDDIR)/libs/app.a\
		$(BUILDDIR)/libs/util.a\
		$(BUILDDIR)/libs/domain.a\
		-Wl,--end-group

		
$(BUILDDIR)/test2:\
	$(OBJDIR)/test/test2.o\
	$(DEPDIR)/test/test2.d\
	$(BUILDDIR)/libs/app.a\
	$(BUILDDIR)/libs/util.a\
	$(BUILDDIR)/libs/domain.a
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) -o $@\
		-Wl,--start-group\
		$(OBJDIR)/test/test2.o\
		$(BUILDDIR)/libs/app.a\
		$(BUILDDIR)/libs/util.a\
		$(BUILDDIR)/libs/domain.a\
		-Wl,--end-group


		
$(BUILDDIR)/test3:\
	$(OBJDIR)/test/test3.o\
	$(DEPDIR)/test/test3.d\
	$(BUILDDIR)/libs/app.a\
	$(BUILDDIR)/libs/util.a\
	$(BUILDDIR)/libs/domain.a
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) -o $@\
		-Wl,--start-group\
		$(OBJDIR)/test/test3.o\
		$(BUILDDIR)/libs/app.a\
		$(BUILDDIR)/libs/util.a\
		$(BUILDDIR)/libs/domain.a\
		-Wl,--end-group

		
$(BUILDDIR)/gridnav_dom_test:\
	$(OBJDIR)/test/gridnav_dom_test.o\
	$(DEPDIR)/test/gridnav_dom_test.d\
	$(BUILDDIR)/libs/app.a\
	$(BUILDDIR)/libs/util.a\
	$(BUILDDIR)/libs/domain.a
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) -o $@\
		-Wl,--start-group\
		$(OBJDIR)/test/gridnav_dom_test.o\
		$(BUILDDIR)/libs/app.a\
		$(BUILDDIR)/libs/util.a\
		$(BUILDDIR)/libs/domain.a\
		-Wl,--end-group
	
.PHONY: tests
tests: $(ROOTDIR)/experiment/tiles15/run_tiles15



-include $(DEPDIR)/test/main.d
-include $(DEPDIR)/test/test2.d
-include $(DEPDIR)/test/test3.d
-include $(DEPDIR)/test/gridnav_dom_test.d
