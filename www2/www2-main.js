/* TODO: obfuscate this file... */
var agt = navigator.userAgent.toLowerCase();
var gIE = ((agt.indexOf("msie") != -1) && (agt.indexOf("opera") == -1));
var gFx = (agt.indexOf("gecko") != -1);

if (!Array.prototype.push) {
	Array.prototype.push = function() {
		var s = this.length;
		for (var i = 0; i < arguments.length; i++)
			this[s + i] = arguments[i];
		return this.length;
	};
}

function htmlize(s) {
	s = s.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
	s = s.replace(/\x20\x20/g, " &nbsp;");
	return s;
}

function prints(s) {
	s = s.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
	s = s.replace(/\r[\[\d;]+[a-z]/gi, "");
	s = s.replace(/\x20\x20/g, " &nbsp;").replace(/\n /g, "<br/>&nbsp;");
	s = s.replace(/\n(: .*)/g, "<br/><span class=\"f006\">$1</span>").replace(/\n/g, "<br/>");
	document.write(s);
}
var attachURL = null;
function attach(name, len, pos) {
	var bImg = false;
	var o = name.lastIndexOf(".");
	var s = "";
	if (!attachURL) return;
	if (o != -1) {
		var ext = name.substring(o + 1).toLowerCase();
		bImg = (ext == "jpg" || ext == "jpeg" || ext == "gif"
			 || ext == "ico" || ext == "png"  || ext == "pcx"
			 || ext == "bmp");
	}
	if (bImg) {
		s += "<br /><img src=\"/images/files/img.gif\" border=\"0\" />此主题相关图片如下："
		  + name + "(" + len + " 字节)<br /><a href=\"" + attachURL + "&amp;ap=" 
		  + pos + "\" target=\"_blank\"><img src=\"" + attachURL + "&amp;ap=" 
		  + pos + "\" border=\"0\" title=\"按此在新窗口浏览图片\" onload=\"javascript:resizeImg(this)\" /></a> ";
	} else {
		s += "<br />附件: <a href=\"" + attachURL + "&amp;ap=" + pos + "\">"
		  + name + "</a> (" + len + " 字节)<br />";
	}
	document.write(s);
}


function getCookie(name, def){
	var cname = name + "="; 
	var dc = document.cookie; 
	if (dc.length > 0) { 
		var begin = dc.indexOf(cname); 
		if (begin != -1) { 
			begin += cname.length; 
			var end = dc.indexOf(";", begin);
			if (end == -1) end = dc.length;
			return unescape(dc.substring(begin, end));
		} 
	}
	return def;
}

function saveParaCookie(v, mask) {
	var ex = '';
	if (!isLogin()) {
		var expire = new Date();
		expire.setTime(expire.getTime() + 3600000 * 24 * 7);
		ex = ';expires=' + expire.toGMTString();
	}
	var cook = (readParaCookie() & ~mask) | (v & mask);
	document.cookie = 'WWWPARAMS=' + cook + ex + ';path=/';
}
function readParaCookie() {
	return parseInt(getCookie('WWWPARAMS', 0));
}
function isLogin() {
	return (getCookie("UTMPUSERID", "guest") != "guest");
}

function queryString(param, def) {
	 var url = location.search.toUpperCase() + "&";
	 param = param.toUpperCase();
	 if (!def) def="";
	 var pos = url.indexOf(param + "=");
	 if (pos == -1) return def;
	 pos += param.length + 1;
	 return location.search.substring(pos, url.indexOf("&", pos));
}


function getObj(n) {
	return document.getElementById(n);
}

var bbsconImg = new Array();
function resizeImg(obj) {
	bbsconImg[bbsconImg.length] = obj;
	obj.o_width = obj.width;
	var maxWidth = document.body.clientWidth - 40;
	if (obj.width > maxWidth) obj.width = maxWidth;
}
function adjustImg() {
	var maxWidth = document.body.clientWidth - 40;
	for (var i in bbsconImg) {
		obj = bbsconImg[i];
		o_width = obj.o_width;
		obj.width = (o_width > maxWidth) ? maxWidth : o_width;
	}
}
window.onresize = adjustImg;


function cancelEvent(ev) {
	if (gIE) {
		ev.cancelBubble = true;
		ev.returnValue = false;
	} else {
		ev.preventDefault();
		ev.stopPropagation();
	}
}

/* textarea onkeydown event handler. to submit form with ctrl+W(IE only) or ctrl+ENTER */
function textarea_okd(func, ev) {
	var evt = (ev) ? ev : ((window.event) ? event : null);
	if (evt == null) return true;
	var key = evt.keyCode ? evt.keyCode : evt.charCode;
	if ((key == 87 && evt.ctrlKey) || (key == 13 && evt.ctrlKey)) {
		cancelEvent(evt);
		if (typeof func == "function")  {
			func();
		} else {
			func.form.submit();
		}
		return false;
	}
	return true;
}

function setCursorPosition(oInput,oStart,oEnd) {
	oInput.focus();
	if( oInput.setSelectionRange ) {
		oInput.setSelectionRange(oStart,oEnd);
	} else if( oInput.createTextRange ) {
		var range = oInput.createTextRange();
		range.collapse(true);
		range.moveEnd('character',oEnd);
		range.moveStart('character',oStart);
		range.select();
	}
}

/* refresh message frame */
function alertmsg() {
	if (top.fmsg && !top.fmsg.document.getElementById("msgs")) top.fmsg.location.reload();
}


var bootFn = Array();
function addBootFn(fn) {
	bootFn[bootFn.length] = fn;
}

window.onload = function() {
	/* set focus */
	var f = getObj("sfocus");
	if (f) {
		f.focus();
		setCursorPosition(f, 0, 0);
	} else {
		f = getObj("sselect");
		if (f) {
			f.focus(); f.select();
		}
	}
	
	/*
	 * apply col class. this is a workaround for css2.1 - atppp
	 * see also: http://ln.hixie.ch/?start=1070385285&count=1
	 * TODO: consider td's span properties
	 *
	 * also: apply alternate style for rows.
	 *
	 * Don't use this with long table, as IE runs it extremely slowly.
	 */
	var m = document.getElementsByTagName("table");
	for(var i=0;i<m.length;i++) {
		var tab = m[i];
		if (tab.className.indexOf("adj") == -1) continue;
		var cols = tab.getElementsByTagName("col");
		if (cols.length == 0) continue;
		var rows = tab.getElementsByTagName("tr");
		for (var j=0;j<rows.length;j++) {
			var row = rows[j];
			row.className = (j%2)?"odd":"even";
			var td = row.getElementsByTagName("td");
			for (var k=0;k<td.length;k++) {
				if (cols[k] && cols[k].className) {
					if (td[k].className) {
						td[k].className += " " + cols[k].className;
					} else {
						td[k].className = cols[k].className;
					}
				}
			}
		}
	}
	
	/* this is a workaround for some weird behavior... ask atppp if you are interested. BUGID 7629 */
	if (gFx) {
		var i, ll, links = document.getElementsByTagName("link");
		for(i=0; i<links.length; i++) {
			ll = links[i];
			if((ll.getAttribute("rel") == "stylesheet")) {
				ll.disabled = true;
				ll.disabled = false;
			}
		}
	}
	
	for(i=0; i<bootFn.length; i++) {
		var fn = bootFn[i]; fn();
	}
};

/* check required fields when submitting form. required fields are denoted with classname "req" */
function chkreq(frm) {
	var f = frm.getElementsByTagName("input");
	var i;
	for(i=0;i<f.length;i++) {
		var fe = f[i];
		if (fe.className=="req") {
			if(fe.value==''){
				alert('有必填内容为空');
				fe.focus();
				return false;
			}
		}
	}
	return true;
}

var gTimer;
function footerStart() {
	var prefixZero = function(d) {
		if (d > 9) return d;
		else return "0" + d;
	};
	var MyTimer = function() {
		this.start = (new Date()).getTime();
		this.serverDiff = serverTime * 1000 - this.start;
		this.serverTime = serverTime * 1000;
		this.startTime = this.start - stayTime * 1000;
		this.lastStay = this.lastClock = "";
		this.refreshTime = 0;
	};
	MyTimer.prototype.refresh = function() {
		var now = (new Date()).getTime();
		this.serverTime = this.serverDiff + now;
		var Timer = new Date(this.serverTime);
		var str = Timer.getUTCFullYear() + "年" + prefixZero(Timer.getUTCMonth() + 1) + "月" 
				+ prefixZero(Timer.getUTCDate()) + "日" + prefixZero(Timer.getUTCHours())
				+ ":" + prefixZero(Timer.getUTCMinutes());
		if (this.lastClock != str) {
			this.lastClock = str;
			getObj("divTime").innerHTML = str;
		}
		var staySec = (now - this.startTime) / 60000;
		str = parseInt(staySec/60) + "小时" + parseInt(staySec % 60) + "分钟";
		if (this.lastStay != str) {
			this.lastStay = str;
			getObj("divStay").innerHTML = str;
		}
		if (this.refreshTime > 0 && now > this.refreshTime) {
			location.reload();
		} else {
			setTimeout("gTimer.refresh()", 1000);
		}
	};
	gTimer = new MyTimer();
	gTimer.refresh();
	
	if (hasMail) {
		if (typeof top.hasMsgBox == "undefined") {
			top.hasMsgBox = false;
		}
		if (!top.hasMsgBox) {
			top.hasMsgBox = true;
			if(confirm("您有新邮件，现在查收？")) {
				top.f3.location.href = "bbsmailbox.php?path=.DIR&title=%CA%D5%BC%FE%CF%E4";
			}
			top.hasMsgBox = false;
		}
	}
	gTimer.refreshTime = (new Date()).getTime() + 540000;
}

function getFindBox(board) { /* TODO: sfocus here might conflict with others */
	var s = 
	'<form action="bbsbfind.php" method="GET" class="medium"><input type="hidden" name="q" value="1"/>\
		<fieldset><legend>版内文章搜索 <span id="bbbHide"></span></legend>\
			<div class="inputs">\
				<label>版面名称:</label><input type="text" maxlength="24" size="24" name="board" value="' + board + '"/><br/>\
				<label>标题含有:</label><input type="text" maxlength="50" size="20" name="title" id="sfocus" /> AND<br/>\
				<label>　　　　 </label><input type="text" maxlength="50" size="20" name="title2"/><br/>\
				<label>标题不含:</label><input type="text" maxlength="50" size="20" name="title3"/><br/>\
				<label>作者帐号:</label><input type="text" maxlength="12" size="12" name="userid"/><br/>\
				<label>时间范围:</label><input type="text" maxlength="4"  size="4"  name="dt" value="7"/> 天以内<br/>\
				<input type="checkbox" name="mg" id="mg"><label for="mg" class="clickable">精华文章</label>\
				<input type="checkbox" name="ag" id="ag"><label for="ag" class="clickable">带附件文章</label>\
				<input type="checkbox" name="og" id="og"><label for="og" class="clickable">不含跟贴</label>\
			</div>\
		</fieldset>\
		<div class="oper"><input type="submit" value="递交查询结果"/></div>\
	</form>';
	return s;
}

function showFindBox(board) {
	return true; /* disable for now */
	var divID = "articleFinder";
	var div = getObj(divID);
	if (!div) {
		div = document.createElement("div");
		div.id = divID;
		div.innerHTML = getFindBox(board);
		div.className = "float";
		div.style.top = "5em";
		div.style.left = "20em";
		div.style.padding = "0.5em";
		div.style.zIndex = 2;
		div.style.backgroundColor = "lightyellow";
		div.style.display = "none";
		if (gIE) {
			div.style.filter = "alpha(opacity=0)";
		}
		document.body.appendChild(div);
		var o = getObj("bbbHide");
		o.innerHTML = "(隐藏)";
		o.className = "clickable";
		o.title = "隐藏搜索窗口";
		o.onclick = function() {
			div.style.display = "none";
		}
	}
	if (gIE) {
		div.filters[0].opacity  = 0;
	} else {
		div.style.opacity = 0;
	}
	div.style.display = "";
	getObj("sfocus").focus();
	
	/* TODO: general fade-in fade-out effect control */
	var opa = 0;
	var fn = function() {
		opa += 10;
		if (gIE) {
			div.filters[0].opacity = opa;
		} else {
			div.style.opacity = opa / 100.0;
		}
		if (opa < 90) {
			setTimeout(fn, 10);
		}
	};
	fn();
	return false;
}

function goAttachWindow(){
	var hWnd = window.open("bbsupload.php","_blank","width=600,height=300,scrollbars=yes");
	if ((document.window != null) && (!hWnd.opener))
	   hWnd.opener = document.window;
	hWnd.focus();
	return false;
}

function dosubmit() {
	document.postform.post.value='发表中，请稍候...';
	document.postform.post.disabled=true;
	document.postform.submit();
}


var hotBoard = '', hotMove = true, hotFn = null;
function setHots(h) {
	var hots = new Array();
	for(var i=0; i<h.length; i++) {
		if (h[i]) {
			hots.push('<a href="bbscon.php?board=' + hotBoard + '&id=' + h[i][0] + '">' + h[i][1] + '</a>' +
			'[<a href="bbstcon.php?board=' + hotBoard + '&gid=' + h[i][0] + '">同主题</a>](' + h[i][2] + ')');
		}
	}
	if (gIE) {
		var str = '';
		for(var i=0; i<hots.length; i++) {
			if (hots[i]) str+= hots[i] + '&nbsp;&nbsp;&nbsp;&nbsp;';
		}
		getObj('hotTopics').innerHTML = str;
	} else {
		var index = -1;
		hotFn = function() {
			var ii = '';
			if (hots.length) {
				var move = 1;
				if (!hotMove) {
					if (arguments.length) move = arguments[0];
					else return;
				}
				if (move > 0) {
					index++; if (index >= hots.length) index = 0;
				} else {
					index--; if (index < 0) index = hots.length - 1;
				}
				if (hots.length > 1) {
					ii += '<span class="clickable" onclick="hotFn(-1)" title="上一条">&lt;</span> '
						+ '<span class="clickable" onclick="hotFn(1)" title="下一条">&gt;</span> ';
				}
				ii += (index+1) + ": " + hots[index];
			}
			getObj('hotTopics').innerHTML = ii;
		};
		hotFn();
		setInterval("hotFn()", 5000);
	}
}

function hotTopic(board) { /* TODO: no table, use AJAX */
	/* clear: both is for stupid Firefox */
	var str = '<table cellspacing="0" cellpadding="5" border="0" width="100%" style="margin: 0.5em auto 0 auto;clear:both;"><tr>' +
			  '<td width="100" align="center">[<span class="red">热门话题</span>]</td><td>';
	if (gIE) {
		str += '<marquee onmouseover="this.stop()" onmouseout="this.start()"><span id="hotTopics">载入中...</span></marquee>';
	} else {
		str += '<span id="hotTopics" onmouseover="hotMove=false;" onmouseout="hotMove=true;">载入中...</span>';
	}
	str += '</td></tr></table>';
	document.write(str + '<iframe width=0 height=0 src="" frameborder="0" scrolling="no" id="hiddenframe" name="hiddenframe"></iframe>');
	addBootFn(function() {
		window.frames["hiddenframe"].document.location.href = "bbshot.php?board=" + board;
	});
	hotBoard = escape(board);
}



function checkFrame(isPHP) {
	if (arguments.length == 0) isPHP = 0;
	var msg = '<div id="fontSizer"';
	if (!(readParaCookie() & 8)) {
		msg += ' style="display:none"';
	}
	msg += '>'
	+ '<span class="clickable" onclick="sizer(1)" title="字体大大大...大点">+</span> '
	+ '<span class="clickable" onclick="sizer(-1)" title="字体小小小...小点">-</span> '
	+ '<span class="clickable" onclick="sizer(0)" title="字体变变变...变回去">R</span></div>';
	document.write(msg);
	if (top == self) { /* TODO: use better way */
		var url = document.location.toString();
		var uri = url.substr(7);
		if ((pos = uri.indexOf("/")) != -1) {
			url = uri.substr(pos);
		}
		var ex = '(<a href="frames.' + (isPHP?"php":"html") + '?mainurl=' + escape(url) + '">展开完整界面</a>)';
		var o = getObj("idExp");
		if (o) o.innerHTML = ex;
	}
}

var bfsI, bfsD = 4;
var bfsArr = [0,11,12,13,14,16,18,20];
var bfsSma = [0,100,100,92,85,85,85,85];
function sizer(flag) {
	if (flag == 1) {
		if (bfsI < 7) bfsI++;
	} else if (flag == -1) {
		if (bfsI > 1) bfsI--;
	} else bfsI = bfsD;
	saveParaCookie(bfsI, 7);
	document.body.style.fontSize = bfsArr[bfsI] + 'px';
	/* here's the trick/test part: change .smaller definition! */
	var i, t;
	if (!(t = document.styleSheets)) return;
	for(i = 0; i < t.length; i++) {
		if (t[i].title == "myStyle") {
			t = t[i];
			if (t.cssRules) t = t.cssRules;
			else if (t.rules) t = t.rules;
			else return;
			for (i = 0; i < t.length; i++) {
				if (t[i].selectorText == ".smaller") {
					t[i].style.fontSize = bfsSma[bfsI] + '%';
					break;
				}
			}
			return;
		}
	}
}

function writeCss() {
	var cssID,cssURL;
	cssID = (readParaCookie() & 0xF80) >> 7;
	cssURL = 'images/' + cssID + '/www2-default.css';
	document.write('<link rel="stylesheet" type="text/css" href="'+cssURL+'" />');
	bfsI = readParaCookie() & 7;
	if (bfsI <= 0 || bfsI >= bfsArr.length) bfsI = bfsD;
	var ret = '<style type="text/css" title="myStyle"><!--';
	ret += 'body{font-size:' + bfsArr[bfsI] + 'px;}';
	ret += '.smaller{font-size:' + bfsSma[bfsI] + '%;}';
	ret += '--></style>';
	document.write(ret);
}

function writeCssLeft() {
	var cssID,cssURL;
	cssID = (readParaCookie() & 0xF80) >> 7;
	cssURL = 'images/' + cssID + '/bbsleft.css';
	document.write('<link rel="stylesheet" type="text/css" href="'+cssURL+'" />');
}

function writeCssMainpage() {
	var cssID,cssURL;
	cssID = (readParaCookie() & 0xF80) >> 7;
	cssURL = 'images/' + cssID + '/mainpage.css';
	document.write('<link rel="stylesheet" type="text/css" href="'+cssURL+'" />');
}

function putImageCode(filename,otherparam)
{
	var cssID;
	cssID = (readParaCookie() & 0xF80) >> 7;
	return('<img src="images/'+cssID+'/'+filename+'" '+otherparam+'>');
}

function putImage(filename,otherparam)
{
	document.write(putImageCode(filename,otherparam));
}

var writeBM_str;

function writeBM_getStr(start) {
	var ret = '', maxbm = 100;
	for(var i = start; i < writeBM_str.length; i++) {
		if (i >= start + maxbm) {
			break;
		} else {
			var bm = writeBM_str[i];
			ret += ' <a href="bbsqry.php?userid=' + bm + '">' + bm + '</a>';
		}
	}
	if (start > 0) {
		ret += ' <a href="#" onclick="return writeBM_page(' + (start-1) + ')" title="版主前滚翻">&lt;&lt;</a>';
	} else if (writeBM_str.length > maxbm) {
		ret += ' <span class="gray">&lt;&lt;</span>';
	}
	if (start < writeBM_str.length - maxbm) {
		ret += ' <a href="#" onclick="return writeBM_page(' + (start+1) + ')" title="版主后滚翻">&gt;&gt;</a>';
	} else if (writeBM_str.length > maxbm) {
		ret += ' <span class="gray">&gt;&gt;</span>';
	}
	return ret;
}

function writeBM_page(start) {
	getObj("idBMs").innerHTML = writeBM_getStr(start);
	return false;
}

function writeBMs(bmstr) {
	if (typeof bmstr == "string") {
		document.write(' ' + bmstr);
	} else {
		writeBM_str = bmstr;
		document.write('<span id="idBMs">' + writeBM_getStr(0) + '</span>');
	}
}

var dir_modes = {"FIND": -2, "ANNOUNCE": -1, "NORMAL": 0, "DIGEST": 1, "MARK": 3, "ORIGIN": 6, "ZHIDING": 11};
function dir_name(ftype) {
	switch(ftype) {
		case -1: return "(精华区)";
		case 1: return "(文摘区)";
		case 3: return "(保留区)";
		case 6: return "(主题模式)";
		default: return "";
	}
}


function docWriter(board, start, man, ftype, page, total, apath, showHot) {
	this.monthStr = ["January","February","March","April","May","June","July","August","September","October","November","December"];
	this.board = escape(board);
	this.start = start;
	this.page = page;
	this.total = total;
	this.man = man;
	this.ftype = ftype;
	this.num = 0;
	this.baseurl = "bbsdoc.php?board=" + this.board;
	this.showHot = showHot;
	this.hotOpt = (readParaCookie() & 0x60) >> 5;
	if (this.man) this.baseurl += "&manage=1";
	if (this.ftype) this.baseurl += "&ftype=" + this.ftype;

	if (showHot && this.hotOpt == 1) hotTopic(this.board);

	var str = '<div class="doc"><div class="docTab">';
	if (!ftype && isLogin()) {
		url = 'bbspst.php?board=' + this.board;
		str += '<div class="post"><a href="' + url + '">' + putImageCode('postnew.gif','alt="发表话题" class="flimg" onclick="location.href=\'' + url + '\';"') + '</a></div>';
	}

	var mls = [[ftype || man, "普通模式", "bbsdoc.php?board=" + this.board],
			   [ftype != dir_modes["DIGEST"], "文摘区", "bbsdoc.php?board=" + this.board + "&ftype=" + dir_modes["DIGEST"]],
			   [ftype != dir_modes["MARK"], "保留区", "bbsdoc.php?board=" + this.board + "&ftype=" + dir_modes["MARK"]],
			   [ftype != dir_modes["ORIGIN"], "同主题", "bbsdoc.php?board=" + this.board + "&ftype=" + dir_modes["ORIGIN"]],
			   [apath && ftype != dir_modes["ANNOUNCE"], "精华区", "bbs0an.php?path=" + escape(apath)],
			   [ftype != dir_modes["FIND"], "查询", "bbsbfind.php?board=" + this.board]];
	for (var i = mls.length - 1; i >= 0; i--) {
		links = mls[i];
		if (links[0]) {
			str += '<a href="' + links[2] + '" class="smaller">' + links[1] + '</a>';
		} else {
			str += '<b class="smaller">' + links[1] + '</b>';
		}
	}
	str += '</div>';

	if (ftype >= 0) {
		if (man) {
			str += '<form name="manage" id="manage" method="post" action="bbsdoc.php?manage=1&board=' + this.board + '&page=' + page + '">';
		}
		str += '<table class="main wide">';
		str += '<col width="50"/><col width="50"/>';
		if (man) {
			str += '<col width="50"/>';
		}
		str += '<col width="85"/><col width="60"/><col width="*"/>';
		str += '<tbody><tr><th>序号</th><th>标记</th>';
		if (man) {
			str += '<th>管理</th>';
		}
		var links = '标题';
		if (readParaCookie() & 16) {
			links = '<div class="relative">标题<div class="pagerlink">';
			if (this.page > 1) {
				links += '<a href="' + this.baseurl + '&page=1" title="第一页">&lt;&lt;</a>';
				links += ' <a href="' + this.baseurl + '&page=' + (this.page - 1) + '" title="上一页">&lt;</a>';
			} else {
				links += '&lt;&lt; &lt;';
			}
			if (this.start <= this.total - 20) {
				links += ' <a href="' + this.baseurl + '&page=' + (this.page + 1) + '" title="下一页">&gt;</a>';
				links += ' <a href="' + this.baseurl + '" title="最后一页">&gt;&gt;</a>';
			} else {
				links += ' &gt; &gt;&gt;';
			}
			links += '</div></div>';
		}
		str += '<th>作者</th><th>日期</th><th>' + links + '</th></tr>';
	}
	document.write(str);
}
docWriter.prototype.o = function(id, gid, author, flag, time, title, size) {
	var str = '<tr class="' + (this.num%2?"even":"odd") + '">';
	if (flag === false) { /* 置顶 */
		str += '<td class="center red strong">提示</td><td class="center"><img src="images/istop.gif" alt="提示"/></td>';
		if (this.man) {
			str += '<td class="center"><input type="checkbox" name="ding' + this.num + '" value="' + id + '" /></td>';
		}
	} else {
		str += '<td class="center">' + (this.num + this.start) + '</td><td class="center">' + flag + '</td>';
		if (this.man) {
			str += '<td class="center"><input type="checkbox" name="art' + this.num + '" value="' + id + '" /></td>';
		}
	}
	str += '<td class="center"><a href="bbsqry.php?userid=' + author + '">' + author + '</a></td>';
	var fd = new Date(time * 1000);
	str += '<td><nobr>' + this.monthStr[fd.getMonth()].substr(0,3) + "&nbsp;"
	var dd = fd.getDate();
	str += ((dd < 10) ? ' ' : '') + dd + '</nobr></td>';
	str += '<td><b>';

	title = htmlize(title);
	if (title.substr(0,4)!="Re: ") title = "● " + title;

	switch (this.ftype) {
		case dir_modes["ORIGIN"]:
			str += '<a href="bbstcon.php?board=' + this.board + '&gid=' + gid + '">' + title + '</a>';
			break;
		case dir_modes["NORMAL"]:
			str += '<a href="bbscon.php?board=' + this.board + '&id=' + id;
			if (flag === false) str += "&ftype=" + dir_modes["ZHIDING"]
			str += '">' + title + '</a>';
			if (size >= 1000) {
				str += '<span class="red">(' + (Math.floor(size / 100) / 10.0) + 'k)</span>';
			} else {
				str += '<span class="normal">(' + size + ')</span>';
			}
			break;
		default:
			str += '<a href="bbscon.php?board=' + this.board + '&id=' + id + '&ftype=' + this.ftype + '&num=' + (this.start + this.num) + '">' + title + '</a>';
			break;
	}
	str += '</b></td></tr>';
	document.write(str);
	this.num++;
};
function mansubmit(flag) {
	document.manage.act.value = flag;
	document.manage.submit();
}
docWriter.prototype.t = function() {
	var ret = '';
	ret += '</tbody></table>';
	if (this.man) {
		var bbsman_modes = {"DEL": 1, "MARK": 2, "DIGEST": 3, "NOREPLY": 4,	"ZHIDING": 5};
		ret += '<div class="oper">';
		ret += '<input type="hidden" name="act" value=""/>';
		ret += '<input type="button" value="删除" onclick="mansubmit(' + bbsman_modes['DEL'] + ');"/>';
		ret += '<input type="button" value="切换M" onclick="mansubmit(' + bbsman_modes['MARK'] + ');"/>';
		ret += '<input type="button" value="切换G" onclick="mansubmit(' + bbsman_modes['DIGEST'] + ');"/>';
		ret += '<input type="button" value="切换不可Re" onclick="mansubmit(' + bbsman_modes['NOREPLY'] + ');"/>';
		ret += '<input type="button" value="切换置顶" onclick="mansubmit(' + bbsman_modes['ZHIDING'] + ');"/>';
		ret += '</div></form>';
	}
	ret += '<form action="bbsdoc.php" method="get" class="docPager smaller">';
	ret += '<input type="hidden" name="board" value="' + this.board + '"/>';
	if (this.man) {
		ret += '<input type="hidden" name="manage" value="1"/>';
	}
	if (!this.ftype) {
		if (isLogin()) {
			url = 'bbspst.php?board=' + this.board;
			ret += '<a href="' + url + '" class="flimg">' + putImageCode('postnew.gif','alt="发表话题" class="flimg" onclick="location.href=\'' + url + '\';"') + '</a>';
		}
	} else {
		ret += '<input type="hidden" name="ftype" value="' + this.ftype + '"/>';
	}

	if (!this.man) {
		ret += '[<a href="javascript:location.reload()">刷新</a>]';
	}
	if (this.page > 1) {
		ret += ' [<a href="' + this.baseurl + '&page=1">第一页</a>]';
		ret += ' [<a href="' + this.baseurl + '&page=' + (this.page - 1) + '">上一页</a>]';
	} else {
		ret += ' [第一页] [上一页]';
	}
	if (this.start <= this.total - 20) {
		ret += ' [<a href="' + this.baseurl + '&page=' + (this.page + 1) + '">下一页</a>]';
		ret += ' [<a href="' + this.baseurl + '">最后一页</a>]';
	} else {
		ret += ' [下一页] [最后一页]';
	}
	ret += ' <input type="submit" value="跳转到"/> ';
	ret += '第 <input type="text" name="start" size="3" onmouseover="this.focus()" onfocus="this.select()" /> 篇/共 ' + this.total + ' 篇';
	ret += '</form>';

	ret += '</div>'; //class="doc"
	document.write(ret);
	
	if (this.showHot && this.hotOpt == 0) hotTopic(this.board);
};


function conWriter(ftype, board, bid, id, gid, reid, file, favtxt, num) {
	this.board = escape(board);
	this.ftype = ftype;
	this.bid = bid;
	this.id = id;
	this.gid = gid;
	this.reid = reid;
	this.file = file;
	this.favtxt = favtxt;
	this.num = num;
	this.baseurl = "bbscon.php?bid=" + bid + "&id=" + id;
}
conWriter.prototype.h = function() {
	if (!isLogin() && this.ftype) return;
	var ret = '<div class="conPager smaller right">';
	if (isLogin()) {
		var url = 'bbspst.php?board=' + this.board + '&reid=' + this.id ;
		ret += '<a href="' + url + '">' + putImageCode('reply.gif','alt="回复帖子" class="flimg" onclick="location.href=\'' + url + '\';"') + '</a>';
		url = 'bbspst.php?board=' + this.board;
		ret += '<a href="' + url + '" class="flimg">' + putImageCode('postnew.gif','alt="发表话题" class="flimg" onclick="location.href=\'' + url + '\';"') + '</a>';
	}
	if (this.ftype == 0) {
		ret += '[<a href="' + this.baseurl + '&p=p">上一篇</a>] ';
		ret += '[<a href="' + this.baseurl + '&p=n">下一篇</a>] ';
		ret += '[<a href="' + this.baseurl + '&p=tp">同主题上篇</a>] ';
		ret += '[<a href="' + this.baseurl + '&p=tn">同主题下篇</a>]';
	} else {
		ret += '<span style="color:#CCCCCC">[上一篇] [下一篇] [同主题上篇] [同主题下篇]</span>';
	}
	ret += '</div>';
	document.write(ret);
};
conWriter.prototype.t = function() {
	var PAGE_SIZE = 20;
	var url = "bbsdoc.php?board=" + this.board;
	var zd = false;
	if (this.ftype != dir_modes["ZHIDING"]) {
		url += "&page=" + Math.floor((this.num + PAGE_SIZE - 1) / PAGE_SIZE);
		if (this.ftype) url += "&ftype=" + this.ftype;
	} else zd = true;
	var ao = (!this.ftype || zd);
	var ret = '<div class="oper smaller">';
	if (ao) {
		var qry = '?board=' + this.board + '&id=' + this.id;
		ret += '[<a href="bbstcon.php?board=' + this.board + '&gid=' + this.gid + '">同主题展开</a>] ';
		ret += '[<a href="bbscon.php?board=' + this.board + '&id=' + this.gid + '">同主题第一篇</a>] ';
		ret += '[<a href="bbstcon.php?board=' + this.board + '&gid=' + this.gid + '&start=' + this.id + '">从此处展开</a>] ';
		ret += '[<a href="bbscon.php?board=' + this.board + '&id=' + this.reid + '" title="跳转到本文所回复的文章">溯源</a>] ';
	}
	ret += '[<a href="' + url + '">返回版面' + dir_name(this.ftype) + '</a>] ';
	ret += '[<a href="javascript:history.go(-1)">快速返回</a>]<br/>';
	if (ao && isLogin()) {
		ret += '[<a href="bbsfwd.php' + qry + '">转寄</a>] ';
		ret += '[<a href="bbsccc.php' + qry + '">转贴</a>] ';
		if (!zd) ret += '[<a href="bbscommend.php' + qry + '">推荐</a>] ';
		ret += '[' + this.favtxt + '] ';
		ret += '[<a href="bbspstmail.php' + qry + '">回信</a>] ';
		ret += '[<a href="bbsedit.php' + qry + '">修改</a>] ';
		if (!zd) {
			ret += '[<a href="bbsedittitle.php' + qry + '">修改标题</a>] ';
			ret += '[<a onclick="return confirm(\'你真的要删除本文吗?\')" href="bbsdel.php?board=' + this.board + '&file=' + this.file + '">删除</a>] ';
		}
	}
	ret += '</div>';
	document.write(ret);
};


function tconWriter(board, id, owner, num) {
	this.board = escape(board);
	this.id = id;
	this.owner = owner;
	this.num = num;
}
tconWriter.prototype.o = function() {
	var ret = '<br/>';
	ret += '<div class="conPager smaller left">';
	ret += '[<a href="bbscon.php?board=' + this.board + '&id=' + this.id + '">本篇全文</a>] ';
	if (isLogin()) {
		ret += '[<a href="bbspst.php?board=' + this.board + '&reid=' + this.id + '">回复文章</a>] ';
		ret += '[<a href="bbspstmail.php?board=' + this.board + '&id=' + this.id + '">回信给作者</a>] ';
	}
	ret += '[本篇作者：<a href="bbsqry.php?userid=' + this.owner + '">' + this.owner + '</a>] ';
	ret += '[<a href="bbsdoc.php?board=' + this.board + '">进入讨论区</a>] ';
	ret += '[<a href="#top">返回顶部</a>]';
	ret += '<div class="tnum">' + this.num + '</div>';
	ret += '</div>';
	document.write(ret);
};


function tconHeader(board, gid, start, tpage, pno) {
	this.board = escape(board);
	this.gid = gid;
	this.start = start;
	this.tpage = tpage;
	this.pno = pno;
}
tconHeader.prototype.h = function() {
	var ret = '<div class="tnav smaller">【分页： ';
	for(var j = 1; j < this.tpage + 1; j ++ ) {
		if (this.pno != j) {
			ret += '<a href="bbstcon.php?board=' + this.board + '&gid=' + this.gid + '&start=' + this.start + '&pno=' + j + '"><u>' + j + '</u></a> ';
		} else {
			ret += j + ' ';
		}
	}
	ret += '】</div>';
	document.write(ret);
};



function tabWriter(num, tabC, caption, header) {
	/* header[i][0]: title, header[i][1]: width, header[i][2]: class */
	this.header = header;
	if (typeof header == "string") {
		switch (header) {
			case 'boards':
				break;
		}
	}
	this.num = num;
	this.row = 0;
	var ret = '<table class="' + tabC + '">';
	if (caption) ret += '<caption>' + caption + '</caption>';
	for(var i = 0; i < header.length; i++) {
		ret += '<col';
		if (header[i][1]) ret += ' width="' + header[i][1] + '"';
		ret += '/>';
	}	
	ret += '<tr>';
	for(var i = 0; i < header.length; i++) {
		ret += '<th>' + header[i][0] + '</th>';
	}
	ret += '</tr><tbody>';
	document.write(ret);
}
tabWriter.prototype.pr = function(col, content) {
	var ret = '<td';
	var c = this.header[col];
	if (c) {
		if (c[2]) ret += ' class="' + c[2] + '"';
	}
	ret += '>' + content + '</td>';
	return ret;
};
tabWriter.prototype.r = function() {
	var ret = '<tr class="' + (((this.row++)%2)?'even':'odd') + '">';
	var i = 0; j = 0;
	if (this.num) {
		ret += this.pr(0, this.row);
		j++;
	}
	for(var i = 0; i < arguments.length; i++,j++) {
		ret += this.pr(j, arguments[i]);
	}
	ret += '</tr>';
	document.write(ret);
};
tabWriter.prototype.t = function() {
	document.write("</tbody></table>");
};
