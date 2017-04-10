


$(BUILDDIR)/experiment/run_tiles15:\
	$(OBJDIR)/experiment/main_tiles.o\
	$(DEPDIR)/experiment/main_tiles.d\
	$(BUILDDIR)/libs/app.a\
	$(BUILDDIR)/libs/util.a\
	$(BUILDDIR)/libs/domain.a
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) -o $@\
		-Wl,--start-group\
		$(OBJDIR)/experiment/main_tiles.d\
		$(BUILDDIR)/libs/app.a\
		$(BUILDDIR)/libs/util.a\
		$(BUILDDIR)/libs/domain.a\
		-Wl,--end-group


.PHONY: exp_tiles
exp_tiles: $(BUILDDIR)/experiment/run_tiles15



-include $(DEPDIR)/experiment/main_tiles.d
