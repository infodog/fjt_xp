 mod_fjt.la: mod_fjt.slo config_utils.slo license.slo convert.slo exportapi.slo baseutil.slo stack.slo ifconfigip.slo memstream.slo convert_utils.slo htmlparser.slo notfoundurls.slo
	$(SH_LINK) -rpath $(libexecdir) -module -avoid-version  mod_fjt.lo config_utils.lo license.lo convert.lo exportapi.lo baseutil.lo stack.lo ifconfigip.lo memstream.lo convert_utils.lo htmlparser.lo notfoundurls.lo
DISTCLEAN_TARGETS = modules.mk
shared =  mod_fjt.la
export MOD_CFLAGS = -std=c99  -I$(dist)/openssl-1.1.1b/include -Wno-error=declaration-after-statement
export MOD_LDFLAGS = -L$(dist)/openssl-1.1.1b/lib -lssl -lcrypto -lpthread
