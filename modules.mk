mod_fjt.la: mod_fjt.slo config_utils.lo license.lo
	$(SH_LINK) -rpath $(libexecdir) -module -avoid-version  mod_fjt.lo config_utils.lo license.lo
DISTCLEAN_TARGETS = modules.mk
shared =  mod_fjt.la
