
$(BUILDDIR)/libs/util.a:\
	$(DEPDIR)/util/time.d\
	$(OBJDIR)/util/time.o\
	$(DEPDIR)/util/hash.d\
	$(OBJDIR)/util/hash.o
	@echo $@
	@mkdir -p $(@D)
	@ar rcs $@\
		$(OBJDIR)/util/time.o\
		$(OBJDIR)/util/hash.o

		
BINS+=$(BUILDDIR)/libs/util.a

-include $(DEPDIR)/util/time.d
-include $(DEPDIR)/util/hash.d


$(BUILDDIR)/tests/util:\
	$(DEPDIR)/util/main_testing.d\
	$(OBJDIR)/util/main_testing.o
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) -o $@\
		$(OBJDIR)/util/main_testing.o

BINS+=$(BUILDDIR)/tests/util

-include $(DEPDIR)/util/main_testing.d
