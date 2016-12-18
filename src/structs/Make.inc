



$(BUILDDIR)/tests/unit_structs:\
	$(OBJDIR)/structs/main_test.o\
	$(DEPDIR)/structs/main_test.d\
	$(OBJDIR)/structs/test_fast_binheap.o\
	$(DEPDIR)/structs/test_fast_binheap.d\
	$(OBJDIR)/structs/test_fast_vector.o\
	$(DEPDIR)/structs/test_fast_vector.d\
	$(OBJDIR)/structs/test_fast_hashtable.o\
	$(DEPDIR)/structs/test_fast_hashtable.d\
	$(OBJDIR)/structs/test_mem_pool.o\
	$(DEPDIR)/structs/test_mem_pool.d
	@echo $@
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -static $(LDFLAGS) -I$(SRCDIR) -o $@\
		$(OBJDIR)/structs/main_test.o\
		$(OBJDIR)/structs/test_fast_binheap.o\
		$(OBJDIR)/structs/test_fast_vector.o\
		$(OBJDIR)/structs/test_fast_hashtable.o\
		$(OBJDIR)/structs/test_mem_pool.o
		
TESTBINS+=$(BUILDDIR)/tests/unit_structs

-include $(DEPDIR)/structs/main_test.d
-include $(DEPDIR)/structs/test_fast_binheap.d
-include $(DEPDIR)/structs/test_fast_vector.d
-include $(DEPDIR)/structs/test_fast_hashtable.d
-include $(DEPDIR)/structs/test_mem_pool.d

