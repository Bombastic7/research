

$(BUILDDIR)/launcher:\
	$(DEPDIR)/app/launcher.d\
	$(OBJDIR)/app/launcher.o\
	$(BUILDDIR)/libs/app.a\
	$(BUILDDIR)/libs/search.a\
	$(BUILDDIR)/libs/util.a
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) -o $@\
		$(OBJDIR)/app/launcher.o\
		-Wl,--start-group\
		$(BUILDDIR)/libs/app.a\
		$(BUILDDIR)/libs/search.a\
		$(BUILDDIR)/libs/util.a\
		-Wl,--end-group
		

#BINS+=$(BUILDDIR)/launcher

#-include $(DEPDIR)/app/launcher.d




$(BUILDDIR)/dom_utils:\
	$(DEPDIR)/app/dom_utils.d\
	$(OBJDIR)/app/dom_utils.o\
	$(BUILDDIR)/libs/app.a\
	$(BUILDDIR)/libs/search.a\
	$(BUILDDIR)/libs/util.a
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) -o $@\
		$(OBJDIR)/app/dom_utils.o\
		-Wl,--start-group\
		$(BUILDDIR)/libs/app.a\
		$(BUILDDIR)/libs/search.a\
		$(BUILDDIR)/libs/util.a\
		-Wl,--end-group
		

#BINS+=$(BUILDDIR)/dom_utils

#-include $(DEPDIR)/app/dom_utils.d



$(BUILDDIR)/libs/app.a:\
	$(DEPDIR)/app/platform/resource.d\
	$(OBJDIR)/app/platform/resource.o
	@echo $@
	@mkdir -p $(@D)
	@ar rcs $@\
		$(OBJDIR)/app/platform/resource.o


BINS+=$(BUILDDIR)/libs/app.a

-include $(DEPDIR)/app/platform/resource.d
