<?php

/**
 * Checking whether a board is set with some specific flags or not.
 * 
 * @param $board the board object to be checked
 * @param $flag the flags to check
 * @return TRUE  the board is set with the flags
 *         FALSE the board is not set with the flags
 * @author flyriver
 */
function bbs_check_board_flag($board,$flag)
{
	if ($board["FLAG"] & $flag)
		return TRUE;
	else
		return FALSE;
}

/**
 * Checking whether a board is an anonymous board or not.
 * 
 * @param $board the board object to be checked
 * @return TRUE  the board is an anonymous board
 *         FALSE the board is not an anonymous board
 * @author flyriver
 */
function bbs_is_anony_board($board)
{
	global $BOARD_FLAGS;
	return bbs_check_board_flag($board, $BOARD_FLAGS["ANONY"]);
}

/**
 * Checking whether a board is an outgo board or not.
 * 
 * @param $board the board object to be checked
 * @return TRUE  the board is an outgo board
 *         FALSE the board is not an outgo board
 * @author flyriver
 */
function bbs_is_outgo_board($board)
{
	global $BOARD_FLAGS;
	return bbs_check_board_flag($board, $BOARD_FLAGS["OUTGO"]);
}

/**
 * Checking whether a board is a junk board or not.
 * 
 * @param $board the board object to be checked
 * @return TRUE  the board is a junk board
 *         FALSE the board is not a junk board
 * @author flyriver
 */
function bbs_is_junk_board($board)
{
	global $BOARD_FLAGS;
	return bbs_check_board_flag($board, $BOARD_FLAGS["JUNK"]);
}

/**
 * Checking whether a board is an attachment board or not.
 * 
 * @param $board the board object to be checked
 * @return TRUE  the board is an attachment board
 *         FALSE the board is not an attachment board
 * @author flyriver
 */
function bbs_is_attach_board($board)
{
	global $BOARD_FLAGS;
	return bbs_check_board_flag($board, $BOARD_FLAGS["ATTACH"]);
}

/**
 * Checking whether a board is a readonly board or not.
 * 
 * @param $board the board object to be checked
 * @return TRUE  the board is a readnoly board
 *         FALSE the board is not a readonly board
 * @author flyriver
 */
function bbs_is_readonly_board($board)
{
	global $BOARD_FLAGS;
	return bbs_check_board_flag($board, $BOARD_FLAGS["READONLY"]);
}


/*
 * print out javascript for displaying articles.
 * See also board.php showBoardContents() how to use this javascript.
 */
function print_file_display_javascript($boardName) {
?>

	function Post(id, owner, posttime, flags) {
		this.ID = id;
		this.OWNER = owner;
		this.POSTTIME = posttime;
		this.FLAGS = flags;
	}
	
	function writepost(unused_id, html_title, threadNum, origin, lastreply, origin_exists) {
		/* note: when origin post does not exists, origin is actually the same as lastreply except
		 * groupID is different. See also www_generateOriginIndex() */
		//if (!origin_exists) return;
		document.write("<TR align=middle><TD class=TableBody2 width=32 height=27 align=\"center\">");
		if (article_is_zhiding(origin.FLAGS)) {
			document.write("<img src=\"pic/istop.gif\" alt=�̶�������>");
		} else if( threadNum > 10 ) {
			document.write("<img src=\"pic/blue/hotfolder.gif\" alt=�ظ�����10��>");
		} else if(article_is_noreply(origin.FLAGS)) {
			document.write("<img src=\"pic/blue/lockfolder.gif\" alt=����������>");
		} else if(article_is_digest(origin.FLAGS)) {
			document.write("<img src=\"pic/isbest.gif\" alt=������>");
		} else {
			document.write("<img src=\"pic/blue/folder.gif\" alt=��������>");
		}
		document.write("</TD><TD align=left class=TableBody1 width=* >");
		if (threadNum==0) {
			document.write('<img src="pic/nofollow.gif" id="followImg' + unused_id + '">');
		} else {
			document.write('<img loaded="no" src="pic/plus.gif" id="followImg' + origin.ID + '" style="cursor:hand;" onclick="loadThreadFollow(\'' + origin.ID + "','" + boardName + "')\" title=չ�������б�>");
		}
		if (origin_exists) {
			href_title = html_title + ' <br>���ߣ�' + origin.OWNER + '<br>������' + origin.POSTTIME;
		} else {
			href_title = "ԭ����ɾ��";
		}
		document.write('<a href="disparticle.php?boardName=' + boardName + '&ID=' + origin.ID + '" title="' + href_title + '">' + html_title + ' </a>');
		threadPages = Math.ceil((threadNum+1)/THREADSPERPAGE);
		if (threadPages>1) {
			document.write("<b>[<img src=\"pic/multipage.gif\"> ");
			for (t=1; (t<7) && (t<=threadPages); t++) {
				document.write("<a href=\"disparticle.php?boardName=" + boardName + "&ID=" + origin.ID + "&start=" + ((t-1)*THREADSPERPAGE) + "\">" + t + "</a> ");
			}
			if (threadPages>7) {
				if (threadPages>8) {
					document.write("...");
				}
				document.write("<a href=\"disparticle.php?boardName=" + boardName + "&ID=" + origin.ID + "&start=" + ((threadPages-1)*THREADSPERPAGE) + "\">" + threadPages + "</a> ");
			}
			document.write(" ]</b>");
		}
		if (article_is_unread(lastreply.FLAGS)) {
			 //���ظ�δ������� thread ��δ��
			document.write("<img src=\"pic/topnew2.gif\" alt=\"δ��\">");
		}
		document.write("</TD>");
		document.write('<TD class=TableBody2 width=80 align="center">');
		if (origin_exists) {
			document.write('<a href="dispuser.php?id=' + origin.OWNER + '" target=_blank>' + origin.OWNER + '</a>');
		} else {
			document.write('ԭ����ɾ��');
		}
		document.write('</TD>');
		document.write('<TD class=TableBody1 width=64 align="center">' + ((origin_exists?0:1)+threadNum) + '</TD>');
		document.write('<TD align=left class=TableBody2 width=200><nobr>&nbsp;<a href="disparticle.php?boardName=' + boardName + '&ID=' + origin.ID + '&start=' + threadNum + '">');
		document.write(lastreply.POSTTIME + '</a>&nbsp;<font color=#FF0000>|</font>&nbsp;<a href=dispuser.php?id=' + lastreply.OWNER + ' target=_blank>');
		document.write(lastreply.OWNER + '</nobr></a></TD></TR>');
		if (threadNum>0) {
			document.write('<tr style="display:none" id="follow' + origin.ID + '"><td colspan=5 id="followTd' + origin.ID + '" style="padding:0px"><div style="width:240px;margin-left:18px;border:1px solid black;background-color:lightyellow;color:black;padding:2px" onclick="loadThreadFollow(\'' + origin.ID + '\', \'' + boardName + '\')">���ڶ�ȡ���ڱ�����ĸ��������Ժ��</div></td></tr>');
		}
	}
<?php
}


function showBoardStaticsTop($boardArr){
?>
<TABLE cellpadding=3 cellspacing=1 class=TableBorder1 align=center>
<TR><Th height=25 width=100% align=left id=TableTitleLink style="font-weight:normal">
���浱ǰ����<b><?php echo $boardArr['CURRENTUSERS'];?></b>�����ߡ���������<?php echo bbs_get_today_article_num($boardArr['NAME'] ); ?>��
[<a href="favboard.php?bname=<?php echo $boardArr["NAME"]; ?>" title="�ղر����浽�ղؼж���Ŀ¼">�ղر���</a>]</Th></TR></td></tr></TABLE>
<BR>
<table cellpadding=2 cellspacing=0 border=0 width=97% align=center valign=middle><tr><td align=center width=2> </td>
<td align=left style="height:27" valign="center"><table cellpadding=0 cellspacing=0 border=0 ><tr>
<td width="110"><a href=postarticle.php?board=<?php echo $boardArr['NAME']; ?>><div class="buttonClass1" border=0 alt=������></div></a></td>
<td width="110"><a href=# onclick="alert('���������ڿ����У�')"><div class="buttonClass2" border=0 alt=������ͶƱ></div></a></td>
<td width="110"><a href=smallpaper.php?board=<?php echo $boardArr['NAME']; ?>><div class="buttonClass3" border=0 alt=����С�ֱ�></div></a></td>
</tr></table></td>
<td align=right><img src=pic/team2.gif align=absmiddle>
<?php 
	$bms=split(' ',$boardArr['BM']);
	foreach($bms as $bm) {
?>
<a href="dispuser.php?id=<?php echo $bm; ?>" target=_blank title=����鿴�ð�������><?php echo $bm; ?></a>
<?php
	}
?>
</td></tr></table>
<?php
}


function showBroadcast($boardID,$boardName,$is_ann=false){
	global $conn;
?>
<tr><td class=TableBody1 colspan=5 height=20>
	<table width=100% ><tr><td valign=middle height=20 width=50><a href=allpaper.php?board=<?php echo $boardName; ?> title=����鿴����̳����С�ֱ�><b>�㲥</b></a>��</td><td width=*> <marquee scrolldelay=150 scrollamount=4 onmouseout="if (document.all!=null){this.start()}" onmouseover="if (document.all!=null){this.stop()}">
<?php
	$sth = $conn->query("SELECT ID,Owner,Title FROM smallpaper_tb where Addtime>=subdate(Now(),interval 1 day) and boardID=" . $boardID . " ORDER BY Addtime desc limit 5");
	while($rs = $sth->fetchRow(DB_FETCHMODE_ASSOC)) {
		print "����<font color=#ff0000>".$rs['Owner']."</font>˵��<a href=javascript:openScript('viewpaper.php?id=".$rs['ID']."&boardname=".$boardName."',500,400)>".htmlspecialchars($rs['Title'],ENT_QUOTES)."</a>";
  } 
  unset($rs);
  $sth->free();
?>
	</marquee>
	<td align=right width=240>
<?php
	if ($is_ann) {
?>
	<a href="board.php?name=<?php echo $boardName; ?>" title=�鿴��������><font color=blue><B>����</B></font></a> | 
<?php
	} else {
		$ann_path = bbs_getannpath($boardName);
		if ($ann_path != FALSE) {
	    	if (!strncmp($ann_path,"0Announce/",10))
			$ann_path=substr($ann_path,9);
?>
	<a href="elite.php?path=<?php echo urlencode($ann_path); ?>" title=�鿴���澫����><font color=#FF0000><B>����</B></font></a> | 
<?php
		}
	}
?>	
    <a href=# onclick="alert('���������ڿ����У�')" title=�鿴����������ϸ���>����</a>
	| <a href=# onclick="alert('���������ڿ����У�')" title=�鿴�����¼�>�¼�</a>
	| <a href=# onclick="alert('���������ڿ����У�')" title=�鿴�����û���Ȩ��>Ȩ��</a>
    | <a href=# onclick="alert('���������ڿ����У�')">����</a></td></tr></table>
</td></tr>
<?php
}

function board_head_var($boardDesc,$boardName,$secNum)
{
  GLOBAL $SiteName;
  GLOBAL $SiteURL;
  GLOBAL $stats;
  global $section_names;
  if ($URL=='') {
	  $URL=$_SERVER['PHP_SELF'];
  }
?>
<table cellspacing=1 cellpadding=3 align=center class=TableBorder2>
<tr><td>
<img src="pic/forum_nav.gif"> <a href="index.php"><?php   echo $SiteName; ?></a> �� 
<a href="section.php?sec=<?php echo $secNum; ?>"><?php echo $section_names[intval($secNum)][0] ; ?></a> �� <a href="board.php?name=<?php echo $boardName; ?>"><?php echo $boardDesc; ?></a> �� <?php echo $stats; ?> 
</td></tr>
</table>
<br>
<?php 
} 
function boardJump(){ /* ����ռ�˺ܶ�����ʱ������紫�䣬��cache����js�Ż��ǿ��ܵģ�����ҪС��Ȩ�����⡣ - atppp */
	global $section_names;
	global $sectionCount;
	global $section_nums;
	global $yank;
?>
<div align=right><select onchange="if(this.options[this.selectedIndex].value!=''){location=this.options[this.selectedIndex].value;}">
<option selected>��ת��̳��...</option>
<?php
	for ($i=0;$i<$sectionCount;$i++){
		echo "<option value=\"section.php?sec=".$i."\">��".$section_names[$i][0]."</option>";
		$boards = bbs_getboards($section_nums[$i], 0, $yank);
		if ($boards != FALSE) {
			$brd_desc = $boards["DESC"]; // ��������
			$brd_name = $boards["NAME"];
			$rows = sizeof($brd_desc);
			for ($t = 0; $t < $rows; $t++)	{
				echo "<option value=\"board.php?name=".$brd_name[$t]."\">&nbsp;&nbsp;��".$brd_desc[$t]."</option>";
			}
		}
	}
?>
</select></div>
<?php
}
function boardSearchAndJump($boardName, $boardID){

?>
<table border=0 cellpadding=0 cellspacing=3 width=97% align=center>
<tr>
<FORM METHOD=GET ACTION="queryresult.php">
<input type="hidden" name="boardName" value="<?php echo $boardName; ?>">
<td width=50% valign=middle nowrap height=40>����������<input type=text name=title>&nbsp;<input type=submit name=submit value=����></td>
</FORM>
<td valign=middle nowrap width=50% > 
<?php
	boardJump();
?>
</td></tr></table><BR>
<?php
}

function showBoardSampleIcons(){
	global $SiteName;
?>
<table cellspacing=1 cellpadding=3 width=100% class=TableBorder1 align=center><tr><th width=80% align=left>��-=> <?php echo $SiteName; ?>ͼ��</th><th noWrap width=20% align=right>����ʱ���Ϊ - <?php echo SERVERTIMEZONE; ?> &nbsp;</th></tr><tr><td colspan=2 class=TableBody1><table cellspacing=4 cellpadding=0 width=92% border=0 align=center><tr><td><img src=pic/blue/folder.gif> ���ŵ�����</td><td><img src=pic/blue/hotfolder.gif> �ظ�����10��</td><td><img src=pic/blue/lockfolder.gif> ����������</td><td><img src=pic/istop.gif> �̶������� </td><td><img src=pic/ztop.gif> �̶ܹ������� </td><td> <img src=pic/isbest.gif> �������� </td><td> <img src=pic/closedb.gif> ͶƱ���� </td></tr></table></td></tr></table>
<?php
}
?>