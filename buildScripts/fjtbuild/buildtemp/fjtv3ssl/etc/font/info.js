function IsPluginInstalled() {
	var plugName = "";
	var verMajor = parseInt(navigator.appVersion);
	var verMinor = parseFloat(navigator.appVersion);
	
	if (navigator.platform == "Win32" || navigator.platform == "Windows") {
		var agent = "" + navigator.userAgent;		
		
		if (navigator.appName=="Netscape") {
			plugName = 'Infoscape Plugin';				
		}
	} else {
		return true;
	}
	
	if (navigator.plugins[plugName] == null && navigator.plugins.refresh) {
		navigator.plugins.refresh(false);
	}
		
	return (navigator.plugins[plugName] != null);
}

function CheckPlugin(up) {
	if (!IsPluginInstalled()) {
		window.open(up + "info.htm","InstallEot", "location=0,toolbar=0,directories=0,status=0,alwaysRaised=1,width=360,height=236,top=400,left=400");
	}
}

