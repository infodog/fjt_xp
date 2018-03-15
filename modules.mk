mod_fjt.la: mod_fjt.slo test.lo
	$(SH_LINK) -rpath $(libexecdir) -module -avoid-version  mod_fjt.lo 
DISTCLEAN_TARGETS = modules.mk
shared =  mod_fjt.la
