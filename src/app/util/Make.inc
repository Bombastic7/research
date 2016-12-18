$(BUILDDIR)/check_json:\
	$(OBJDIR)/app/util/json_parse_check.o\
	$(DEPDIR)/app/util/json_parse_check.d
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -static $(LDFLAGS) -I$(SRCDIR) -o $@\
		 $(OBJDIR)/app/util/json_parse_check.o

BINS+=$(BUILDDIR)/check_json

-include $(DEPDIR)/app/util/json_parse_check.d
