function kbsrcHost(host, userid) {
	this.host = host;
	this.lastSync = false;
	this.rc = new Object();
	this.dirty = new Object();
	this.userid = userid;
}
kbsrcHost.prototype = {
	/* TODO: ������ߵ��������Ҫ��� cookie */
	isUnread: function(bid, id) {
		var lst = this.rc[bid];
		if (!lst) return false;
		for(var n=0; n<kbsrc.BRCMaxItem; n++) {
			if (lst[n] == 0) {
				if (n == 0) return true;
				return false;
			}
			if (id > lst[n]) {
				return true;
			} else if (id == lst[n]) {
				return false;
			}
		}
		return false;
	},
	addRead: function(bid, id) {
		kbsrc.debugOut(bid + "," + id + " read.", this);
		var lst = this.rc[bid];
		var n;
		if (!lst) return;
		for(n=0; n<kbsrc.BRCMaxItem && lst[n]; n++) {
			if (id == lst[n]) {
				return;
			} else if (id > lst[n]) {
				for (var i=kbsrc.BRCMaxItem-1; i>n; i--) {
					lst[i] = lst[i-1];
				}
				lst[n] = id;
				this.dirty[bid] = true;
				return;
			}
		}
		if (n==0) {
			lst[0] = id;
			lst[1] = 1;
			lst[2] = 0;
			this.dirty[bid] = true;
		}
	},	
	getSyncString: function() {
		var bid, j, str = "";
		for(bid in this.dirty) {
			if (this.dirty[bid]) {
				var lst = this.rc[bid];
				str += kbsrc.toHex(bid, 4);
				for (j=0; j<kbsrc.BRCMaxItem; j++) if (lst[j] == 0) break;
				str += kbsrc.toHex(j, 4);
				for (j=0; j<kbsrc.BRCMaxItem; j++) {
					if (lst[j] == 0) break;
					str += kbsrc.toHex(lst[j], 8);
				}
			}
		}
		return str;
	},
	trySync: function() {
		var str = this.getSyncString();
		if (str) {
			this.sync(function() {
				//alert("RC Saved");
			});
		}
	},
	sync: function(callback) {
		var req = new XMLHttpRequest();
		req.oHost = this;
		req.callback = callback;
		req.onload = function(event) {
			var self = event.target;
	    	var i=0,j,rc = self.responseText;
	    	if (rc.length == 0) return;
	    	try {
	    		while(i<rc.length) {
	    			var bid = parseInt(rc.substr(i, 4), 16);
	    			var n = parseInt(rc.substr(i+4, 4), 16);
	    			self.oHost.rc[bid] = new Array();
	    			self.oHost.dirty[bid] = false;
	    			for (j=0; j<kbsrc.BRCMaxItem; j++) self.oHost.rc[bid][j] = 0;
	    			for (j=0; j<n; j++) {
	    				self.oHost.rc[bid][j] = parseInt(rc.substr(i+8+j*8, 8), 16);
	    			}
	    			i += 8 + n * 8;
	    		}
	    		kbsrc.debugOut("sync OK.", self.oHost);
	    	} catch(e) {
	    		kbsrc.debugOut("sync error.", self.oHost);
	    	}
	    	self.oHost.lastSync = (new Date()).getTime();
	    	if (self.callback) self.callback();
		};
		/* TODO: use relative path */
		req.open("POST", "http://" + this.host + "/kbsrc.php", callback ? false : true);
		req.send(this.getSyncString());
	}
};

function KBSRC() {}
KBSRC.prototype = {
	BRCMaxItem: 50,
	hosts: false,
	timer : function() {
		var host, now = (new Date()).getTime();
		for (host in this.hosts) {
			var oHost = this.hosts[host];
			if (!oHost) continue;
			if (now - oHost.lastSync > 600000) {
				oHost.lastSync = now;
				oHost.sync();
			}
		}
	},
	init : function() {
		var browser = gBrowser;
		browser.addEventListener("DOMContentLoaded", kbsrcPageLoadedHandler, true);
		
		var hw = new kbsrcHTTPHeaderWatcher();
		var observerService = Components.classes["@mozilla.org/observer-service;1"].getService(Components.interfaces.nsIObserverService);
		observerService.addObserver(hw, "http-on-examine-response", false);
	    
		this.hosts = new Object();
	    
	    var self = this;
		setInterval(function() {
			self.timer.call(self);
		}, 1000);
		
		kbsrc.debugOut("Loaded OK.");
	},
	hexD: "0123456789ABCDEF",
	toHex: function(num, digits) {
		var ret = "";
		while(digits>0) {
			ret = this.hexD.substr(num & 15, 1) + ret;
			num >>= 4;
			digits--;
		}
		return ret;
	},
	debugOut: function(str, oHost) {
		var now = new Date();
		var d = now.getHours() + ":" + now.getMinutes() + ":" + now.getSeconds();
		var d1 = "";
		if (oHost) d1 = oHost.host + "(" + oHost.userid + ") ";
		dump("KBSRC: [" + d + "] " + d1 + str + "\n");
	}
}
	
var kbsrcPageLoadedHandler = function(event) {
	const doc = event.originalTarget;
	if(!(doc instanceof HTMLDocument)) return;
	
	if(doc._kbsrc_haveChecked) return;
	doc._kbsrc_haveChecked = true;
	
	const protocol = doc.location.protocol;
	if(!/^(?:https|http)\:$/.test(protocol)) return;
	
	const host = doc.location.host;
	const oHost = kbsrc.hosts[host];
	if (!oHost) return;

	var metas = doc.getElementsByTagName("meta");
	for(var i = 0; i < metas.length; i++) {
		if (metas[i].name == "kbsrc.doc") {
			var bid = metas[i].content;
			var tds = doc.getElementsByTagName("td");
			for (var j=0; j<tds.length; j++) {
				var td = tds[j];
				if (td.id.substr(0, 5) != "kbsrc") continue;
				var thisid = td.id.substr(5);
				if (oHost.isUnread(bid, thisid)) td.innerHTML += "*";
			}
			break;
		} else if (metas[i].name == "kbsrc.con") {
			var ids = metas[i].content.split(",");
			var bid = ids[0];
			var thisid = ids[1];
			if (ids[2]) {
				if (ids[2] == 'f') { //clear all
					
				}
			} else {
				oHost.addRead(bid, thisid);
			}
		} else if (metas[i].name == "kbsrc.menu") {
			var f = doc.getElementById("logoutlink");
			if (f) f.addEventListener("click", function() { oHost.trySync(); }, false);
		}
	}
};

function kbsrcHTTPHeaderWatcher() {}
kbsrcHTTPHeaderWatcher.prototype = {
	observe: function(aSubject, aTopic, aData) {
		if (aTopic == 'http-on-examine-response') {
			aSubject.QueryInterface(Components.interfaces.nsIHttpChannel);
			this.onExamineResponse(aSubject);
		}
	},
	onExamineResponse : function (oHttp) {
		var value = false;
		try {
			value = oHttp.getResponseHeader("Set-KBSRC");
		} catch(e) {}
		if (value) {
			var host = oHttp.URI.host;
			if (value == "/") {
				kbsrc.hosts[host] = false;
			} else {
				kbsrc.hosts[host] = new kbsrcHost(host, value);
			}
		}
	}
};

var kbsrc = new KBSRC();
window.addEventListener("load", function() { kbsrc.init.call(kbsrc) }, false); 