

DOMAINOBJS:=


-include $(SRCDIR)/domain/gridnav/Make.inc


$(BUILDDIR)/libs/domain.a: $(DOMAINOBJS) $(DOMAINOBJS:.o=.d)
	@echo $@
	@mkdir -p $(@D)
	@ar rcs $@\
		$(DOMAINOBJS)

		
BINS+=$(BUILDDIR)/libs/domain.a

