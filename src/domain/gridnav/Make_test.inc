
$(BUILDDIR)/domain/gridnav/test_gridnav:\
	$(DEPDIR)/domain/gridnav/test_cell_array.d\
	$(OBJDIR)/domain/gridnav/test_cell_array.o
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) -o $@\
		$(OBJDIR)/domain/gridnav/test_cell_array.o


$(EXPDIR)/trial_1/test_gridnav: $(BUILDDIR)/domain/gridnav/test_gridnav
	cp $< $@



-include $(DEPDIR)/domain/gridnav/test_cell_array.d


