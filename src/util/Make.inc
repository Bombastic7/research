
$(BUILDDIR)/libs/util.a:\
	$(DEPDIR)/util/time.d\
	$(OBJDIR)/util/time.o\
	$(DEPDIR)/util/debug.d\
	$(OBJDIR)/util/debug.o\
	$(DEPDIR)/util/hash.d\
	$(OBJDIR)/util/hash.o\
	$(DEPDIR)/util/impl/LambertW/LambertW.d\
	$(OBJDIR)/util/impl/LambertW/LambertW.o\
	$(DEPDIR)/util/impl/LambertW/FukushimaLambertW.d\
	$(OBJDIR)/util/impl/LambertW/FukushimaLambertW.o
	@echo $@
	@mkdir -p $(@D)
	@ar rcs $@\
		$(OBJDIR)/util/time.o\
		$(OBJDIR)/util/debug.o\
		$(OBJDIR)/util/hash.o\
		$(OBJDIR)/util/impl/LambertW/LambertW.o\
		$(OBJDIR)/util/impl/LambertW/FukushimaLambertW.o

		
BINS+=$(BUILDDIR)/libs/util.a

-include $(DEPDIR)/util/time.d
-include $(DEPDIR)/util/debug.d
-include $(DEPDIR)/util/hash.d
-include $(DEPDIR)/util/impl/LambertW/LambertW.d
-include $(DEPDIR)/util/impl/LambertW/FukushimaLambertW.d


$(BUILDDIR)/tests/util:\
	$(DEPDIR)/util/main_testing.d\
	$(OBJDIR)/util/main_testing.o
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -I$(SRCDIR) -o $@\
		$(OBJDIR)/util/main_testing.o

BINS+=$(BUILDDIR)/tests/util

-include $(DEPDIR)/util/main_testing.d
