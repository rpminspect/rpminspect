topdir := $(dir $(lastword $(MAKEFILE_LIST)))/..

srpm:
	( cd $(topdir) ; ./autogen.sh )
	( cd $(topdir) ; ./configure )
	$(MAKE) -C $(topdir) rpminspect.spec
	$(topdir)/.copr/adjust-release.sh $(topdir)/rpminspect.spec
	$(MAKE) -C $(topdir) srpm
	if [ ! -d $(outdir) ]; then mkdir -p $(outdir) ; fi
	mv $(topdir)/*.src.rpm $(outdir)
