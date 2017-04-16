


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
		$(OBJDIR)/experiment/main_tiles.o\
		$(BUILDDIR)/libs/app.a\
		$(BUILDDIR)/libs/util.a\
		$(BUILDDIR)/libs/domain.a\
		-Wl,--end-group


$(BUILDDIR)/experiment/gen_gridnav:\
	$(OBJDIR)/experiment/main_gen_gridnav.o\
	$(DEPDIR)/experiment/main_gen_gridnav.d\
	$(BUILDDIR)/libs/app.a\
	$(BUILDDIR)/libs/util.a\
	$(BUILDDIR)/libs/domain.a
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) -o $@\
		-Wl,--start-group\
		$(OBJDIR)/experiment/main_gen_gridnav.o\
		$(BUILDDIR)/libs/app.a\
		$(BUILDDIR)/libs/util.a\
		$(BUILDDIR)/libs/domain.a\
		-Wl,--end-group



$(BUILDDIR)/experiment/ulvl/run_tiles15:\
	$(OBJDIR)/experiment/ulvl/main.o\
	$(DEPDIR)/experiment/ulvl/main.d\
	$(BUILDDIR)/libs/app.a\
	$(BUILDDIR)/libs/util.a\
	$(BUILDDIR)/libs/domain.a
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) -o $@\
		-Wl,--start-group\
		$(OBJDIR)/experiment/ulvl/main.o\
		$(BUILDDIR)/libs/app.a\
		$(BUILDDIR)/libs/util.a\
		$(BUILDDIR)/libs/domain.a\
		-Wl,--end-group
		

.PHONY: exp_tiles
exp_tiles: $(BUILDDIR)/experiment/run_tiles15



-include $(DEPDIR)/experiment/main_tiles.d
-include $(DEPDIR)/experiment/main_gen_gridnav.d
-include $(DEPDIR)/experiment/ulvl/main.d
