<?php
	/*
	**manage personal corp.
	**@id: windinsn Nov 19,2003	
	*/
	/*
	**	对收藏夹的剪切、复制操作需要 session 支持 windinsn nov 25,2003
	*/
	require("pcfuncs.php");
						
	if ($loginok != 1)
		html_nologin();
	elseif(!strcmp($currentuser["userid"],"guest"))
	{
		html_init("gb2312");
		html_error_quit("guest 没有Blog!");
	}
	else
	{
		$link = pc_db_connect();
		$pc = pc_load_infor($link,$_GET["userid"]);
		
		if(!$pc)
		{
			pc_db_close($link);
			html_error_quit("对不起，您要查看的Blog不存在");
		}
		
		if(!pc_is_admin($currentuser,$pc))
		{
			pc_db_close($link);
			html_error_quit("对不起，您要查看的Blog不存在");
		}
		
		if($pc["EDITOR"] != 1)
			$pcconfig["EDITORALERT"] = NULL;
			
		$act = $_GET["act"]?$_GET["act"]:$_POST["act"];
		
		if($act == "post" && !$_POST["subject"] && $pc["EDITOR"] != 0)
			pc_html_init("gb2312",stripslashes($pc["NAME"]),"","","",$pc["EDITOR"]);
		elseif($act == "edit" && !$_POST["subject"] && $pc["EDITOR"] != 0)
			pc_html_init("gb2312",stripslashes($pc["NAME"]),"","","",1);
		elseif($act != "favcut" && $act != "favcopy" && $act != "favpaste")
			pc_html_init("gb2312",stripslashes($pc["NAME"]));
		else
			;//nth :p
		
		if($act == "cut" || $act == "copy")
		{
			$access = intval($_POST["access"]);
			if(stristr($_POST["target"],'T'))
			{
				$target = intval(substr($_POST["target"],1,strlen($_POST["target"])-1));
				$in_section = 1;
				if(!pc_load_topic($link,$pc["UID"],$target,$topicname))
					$target = 0; //如果参数错误就移入未分类
			}
			else
			{
				$target = intval($_POST["target"]);
				$in_section = 0;
				if($target < 0 || $target > 4 )
					$target = 2;//如果参数错误先移入私人区
			}
			
			
			if(!$in_section && 3 == $target ) //跨区  移入收藏区
			{
				$rootpid = pc_fav_rootpid($link,$pc["UID"]);
				if(!$rootpid)
				{
					html_error_quit("收藏夹根目录错误!");
					exit();
				}
			}
			else
				$rootpid = 0;
			
			if($in_section)
			{
				if($act == "cut")
					$query = "UPDATE nodes SET created = created , `tid` = '".$target."' , `changed` = NOW( ) , `pid` = '0' WHERE `uid` = '".$pc["UID"]."' AND `type` = 0 AND ( `nid` = '0' ";
				else
					$query = "SELECT * FROM nodes WHERE `uid` = '".$pc["UID"]."' AND `type` = 0 AND ( `nid` = '0' ";
			}
			else
			{
				if($act == "cut" && $target == 3)
					$query = "UPDATE nodes SET created = created , `access` = '".$target."' , `changed` = '".date("YmdHis")."' , `pid` = '".$rootpid."', `tid` = 0 WHERE `uid` = '".$pc["UID"]."' AND ( `nid` = '0' ";
				elseif($act == "cut")
					$query = "UPDATE nodes SET created = created , `access` = '".$target."' , `changed` = '".date("YmdHis")."' , `pid` = '0' , `tid` = 0 WHERE `uid` = '".$pc["UID"]."' AND `type` = 0  AND ( `nid` = '0' ";
				else
					$query = "SELECT * FROM nodes WHERE `uid` = '".$pc["UID"]."' AND `type` = 0 AND ( `nid` = '0' ";
			}
				
			$j = 0;
			for($i = 1 ;$i < $pc["NLIM"]+1 ; $i ++)
			{
				if($_POST["art".$i])
				{
					$query .= " OR `nid` = '".(int)($_POST["art".$i])."' ";
					$j ++;
				}
			}
			$query .= " ) ";
			
			if($act == "cut")
			$query .= " AND nodetype = 0 ";
			//nodetype != 0的是公有blog的log文件
			
			if($in_section)
			{
				if("cut" == $act)
				{
					mysql_query($query,$link);
				}
				else
				{
					$result = mysql_query($query,$link);
					$num_rows = mysql_num_rows($result);
					$j = $num_rows;
					if(pc_used_space($link,$pc["UID"],$access)+$num_rows > $pc["NLIM"])
					{
						html_error_quit("目标区域文章数超过上限 (".$pc["NLIM"]." 篇)!");
						exit();
					}
					for($i = 0;$i < $num_rows ; $i ++)
					{
						/*	目前复制文章的时候评论不同步复制	*/
						$rows = mysql_fetch_array($result);
						$query = "INSERT INTO `nodes` ( `pid` , `tid` , `type` , `source` , `hostname` , `changed` , `created` , `uid` , `comment` , `commentcount` , `subject` , `body` , `access` , `visitcount` ,`htmltag`)  ".
							" VALUES ('0','".$target."' , '0', '".addslashes($rows[source])."', '".addslashes($rows[hostname])."','NOW( )' , '".$rows[created]."', '".$pc["UID"]."', '".$rows[comment]."', '0', '".addslashes($rows[subject])."', '".addslashes($rows[body])."', '".$access."', '0','".$rows[htmltag]."');";
						mysql_query($query,$link);
					}
					if($access == 0)
						pc_update_record($link,$pc["UID"]," + ".$j);
				}
			}
			else
			{
				if($act == "cut")
				{
					if(pc_used_space($link,$pc["UID"],$target)+$j > $pc["NLIM"])
					{
						html_error_quit("目标区域文章数超过上限 (".$pc["NLIM"]." 篇)!");
						exit();
					}
					else
					{
						mysql_query($query,$link);
					}
				}
				else
				{
					$result = mysql_query($query,$link);
					$num_rows = mysql_num_rows($result);
					$j = $num_rows;
					
					if(pc_used_space($link,$pc["UID"],$target)+$num_rows > $pc["NLIM"])
					{
						html_error_quit("目标区域文章数超过上限 (".$pc["NLIM"]." 篇)!");
						exit();
					}
					for($i = 0;$i < $num_rows ; $i ++)
					{
						/*	目前复制文章的时候评论不同步复制	*/
						$rows = mysql_fetch_array($result);
						$query = "INSERT INTO `nodes` ( `pid` , `tid` , `type` , `source` , `hostname` , `changed` , `created` , `uid` , `comment` , `commentcount` , `subject` , `body` , `access` , `visitcount` ,`htmltag`)  ".
							" VALUES ('".$rootpid."','0' , '0', '".addslashes($rows[source])."', '".addslashes($rows[hostname])."',NOW( ) , '".$rows[created]."', '".$pc["UID"]."', '".$rows[comment]."', '0', '".addslashes($rows[subject])."', '".addslashes($rows[body])."', '".$target."', '0','".$rows[htmltag]."');";
						mysql_query($query,$link);
					}
				}	
				if($access == 0 && $act == "cut")
					pc_update_record($link,$pc["UID"]," - ".$j);
				if($target == 0)
					pc_update_record($link,$pc["UID"]," + ".$j);
			}
			$log_action = "CUT/COPY NODE";
			pc_return("pcdoc.php?userid=".$pc["USER"]."&tag=".$access."&tid=".intval($_GET["tid"]));
		}
		elseif($act == "post")
		{
			if($_POST["subject"])
			{
				if($pc["EDITOR"]==2)//use ubb
					$blogbody = pc_ubb_parse($_POST["blogbody"]);
				else
					$blogbody = $_POST["blogbody"];
				
				$ret = pc_add_node($link,$pc,$_GET["pid"],$_POST["tid"],$_POST["emote"],$_POST["comment"],$_GET["tag"],$_POST["htmltag"],$_POST["trackback"],$_POST["subject"],$blogbody,0,$_POST["autodetecttbps"],$_POST["trackbackurl"],$_POST["trackbackname"]);
				$error_alert = "";
				switch($ret)
				{
					case -1:
						html_error_quit("缺少日志主题");
						exit();
						break;
					case -2:
						html_error_quit("目录不存在");
						exit();
						break;
					case -3:
						html_error_quit("该目录的日志数已达上限");
						exit();
						break;
					case -4:
						html_error_quit("分类不存在");
						exit();
						break;
					case -5:
						html_error_quit("由于系统原因日志添加失败,请联系管理员");
						exit();
						break;
					case -6:
						$error_alert = "由于系统错误,引用通告发送失败!";
						break;
					case -7:
						$error_alert = "TrackBack Ping URL 错误,引用通告发送失败!";
						break;
					case -8:
						$error_alert = "对方服务器无响应,引用通告发送失败!";
						break;
					default:
				}
				
				if($error_alert)
					echo "<script language=\"javascript\">alert('".$error_alert."');</script>";
				$log_action = "ADD NODE: ".$_POST["subject"];
				pc_return("pcdoc.php?userid=".$pc["USER"]."&tag=".intval($_GET["tag"])."&tid=".intval($_POST["tid"])."&pid=".intval($_GET["pid"]));
			}
			else
			{
				$tid = intval($_GET["tid"]);
				$pid = intval($_GET["pid"]);
				$tag = intval($_GET["tag"]);
				if($tag < 0 || $tag > 4)
					$tag =2 ;
				
				if($tid)
				{
					if(!pc_load_topic($link,$pc["UID"],$tid,$topicname,$tag))
					{
						html_error_quit("所指定的分类不存在，请重试!");
						exit();
					}
				}
				if($pid)
				{
					if(!pc_load_directory($link,$pc["UID"],$pid))
					{
						html_error_quit("所指定的分类不存在，请重试!");
						exit();
					}
				}
				
				if($pc["TMPSAVE"]) //启用发文暂存档
				{
?>
<iframe name="tmpsave" id="tmpsave" src="pctmpsave.php" frameborder="0" width="0" height="0" scrolling="no"></iframe>
<script language="javascript">
var init = 0;
function pc_client_tmpsave_import() 
{
	if (init == 0) 
		init = 1;
	else 
	{
		document.postform.action = "pctmpsave.php?userid=<?php echo $pc["USER"]; ?>&<?php echo "tag=".$tag."&pid=".$pid; ?>";
		document.postform.target = "tmpsave";
		document.postform.submit();
		document.postform.action = "pcmanage.php?userid=<?php echo $pc["USER"]; ?>&act=post&<?php echo "tag=".$tag."&pid=".$pid; ?>";
		document.postform.target = "_self;
	}
	setTimeout('pc_client_tmpsave_import()',<?php echo $pcconfig["TMPSAVETIME"]; ?>);	
}
pc_client_tmpsave_import();
</script>
<?php					
				}
				
?>
<br><center>
<form name="postform" id="postform" target="_self" action="pcmanage.php?userid=<?php echo $pc["USER"]; ?>&act=post&<?php echo "tag=".$tag."&pid=".$pid; ?>" method="post" onsubmit="if(this.subject.value==''){alert('请输入文章主题!');return false;}">
<input type="hidden" name="tmpsave" id="tmpsave" value="0">
<table cellspacing="0" cellpadding="5" border="0" width="90%" class="t1">
<tr>
	<td class="t2">发表文章</td>
</tr>
<tr>
	<td class="t8">主题
	<input type="text" size="100" maxlength="200" name="subject" class="f1">
	</td>
</tr>
<tr>
	<td class="t5">
	评论
	<input type="radio" name="comment" value="1" checked class="f1">允许
	<input type="radio" name="comment" value="0" class="f1">不允许
	</td>
</tr>
<tr>
	<td class="t8">
	Blog
	<select name="tid" class="f1">
<?php
		$blogs = pc_blog_menu($link,$pc,$tag);
		for($i = 0 ; $i < count($blogs) ; $i ++)
		{
			if($blogs[$i]["TID"] == $tid )
				echo "<option value=\"".$blogs[$i]["TID"]."\" selected>".html_format($blogs[$i]["NAME"])."</option>";
			else
				echo "<option value=\"".$blogs[$i]["TID"]."\">".html_format($blogs[$i]["NAME"])."</option>";
		}
?>
	</select>
	</td>
</tr>
<tr>
	<td class="t13">心情符号</td>
</tr>
<tr>
	<td class="t5"><?php @require("emote.html"); ?></td>
</tr>
<tr>
	<td class="t11">内容
	<input type="checkbox" name="htmltag" value=1 <?php if($pc["EDITOR"] != 0) echo "checked"; ?>>使用HTML标记
	</td>
</tr>
<tr>
	<td class="t8">
<?php
	if($pc["EDITOR"]!=2)// not use ubb
	{
?>	
	<textarea name="blogbody" class="f1" cols="120" rows="30" id="blogbody"  onkeydown='if(event.keyCode==87 && event.ctrlKey) {document.postform.submit(); return false;}'  onkeypress='if(event.keyCode==10) return document.postform.submit()' wrap="physical"><?php echo $pcconfig["EDITORALERT"].$_POST["blogbody"]; ?></textarea>
<?php
	}
	else
		pc_ubb_content();
?>
	</td>
</tr>
<?php
	if($tag == 0)
	{
?>
<tr>
	<td class="t8">
	引用通告
	</td>
</tr>
<tr>
	<td class="t8">
	<input type="checkbox" name="autodetecttbps" value="1">自动发掘引用通告
	(什么是自动发掘引用通告?)<br />
	文章链接: <input type="text" size="80" maxlength="255" name="trackbackname" class="f1" value="<?php echo htmlspecialchars($_GET[tbArtAddr]); ?>"><br />
	Trackback Ping URL: <input type="text" size="80" maxlength="255" name="trackbackurl" value="<?php echo htmlspecialchars($_GET[tbTBP]); ?>" class="f1">
	(必须以"http://"开头)
	</td>
</tr>
<tr>
	<td class="t5">
	<input type="checkbox" name="trackback" value="1" checked>允许引用
	</td>
</tr>
<?php
	}
?>
<tr>
	<td class="t2">
		<input type="button" name="ins" value="插入HTML" class="b1" onclick="return insertHTML();" />
		<input type="button" name="hil" value="高亮" class="b1" onclick="return highlight();" />
		<input type="submit" value="发表本文" class="b1">
		<input type="button" value="返回上页" onclick="history.go(-1)" class="b1">
	</td>
</tr>
</table>
</form></center>
<?php				
			}
		}
		elseif($act == "edit")
		{
			$nid = (int)($_GET["nid"]);
			$query = "SELECT `nodetype` , `subject` , `body` ,`comment`,`type`,`tid`,`access`,`htmltag`,`trackback`,`pid` FROM nodes WHERE `nid` = '".$nid."' AND `uid` = '".$pc["UID"]."' LIMIT 0 , 1 ;";
			$result = mysql_query($query,$link);
			$rows = mysql_fetch_array($result);
			mysql_free_result($result);
			if(!$rows)
			{
				html_error_quit("文章不存在!");
				exit();
			}
			if($rows[nodetype] != 0)
			{
				html_error_quit("该文不可编辑!");
				exit();
			}
			
			if($_POST["subject"])
			{
				if($_POST["comment"]==1)
					$c = 0;
				else
					$c = 1;
				$useHtmlTag = ($_POST["htmltag"]==1)?1:0;
				$trackback = ($_POST["trackback"]==1)?1:0;
				$emote = (int)($_POST["emote"]);
				$query = "UPDATE nodes SET `subject` = '".addslashes($_POST["subject"])."' , `body` = '".addslashes(html_editorstr_format($_POST["blogbody"]))."' , `changed` = '".date("YmdHis")."' , `comment` = '".$c."' , `tid` = '".(int)($_POST["tid"])."' , `emote` = '".$emote."' , `htmltag` = '".$useHtmlTag."' , `trackback` = '".$trackback."' WHERE `nid` = '".$nid."' AND nodetype = 0;";
				mysql_query($query,$link);
				pc_update_record($link,$pc["UID"]);
				if($rows[subject]==$_POST["subject"])
					$log_action = "EDIT NODE: ".$rows[subject];
				else
				{
					$log_action = "EDIT NODE: ".$_POST["subject"];
					$log_content = "OLD SUBJECT: ".$rows[subject]."\nNEW SUBJECT: ".$_POST["subject"];
				}
				if($rows[type]==1)
					pc_return("pcdoc.php?userid=".$pc["USER"]."&tag=3&pid=".$rows[pid]);
				else
					pc_return("pccon.php?id=".$pc["UID"]."&nid=".$nid);
			}
			else
			{
?>
<br><center>			
<form name="postform" id="postform" action="pcmanage.php?userid=<?php echo $pc["USER"]; ?>&act=edit&nid=<?php echo $nid; ?>" method="post" onsubmit="if(this.subject.value==''){alert('请输入文章主题!');return false;}">
<table cellspacing="0" cellpadding="5" border="0" width="90%" class="t1">
<?php
		if($rows[type]==1)
		{
?>
<tr>
	<td class="t2">修改目录</td>
</tr>
<tr>
	<td class="t8">
	主题
	<input type="text" size="100" class="f1" maxlength="200" name="subject" value="<?php echo htmlspecialchars(stripslashes($rows[subject])); ?>">
	</td>
</tr>
<tr>
	<td class="t2">
		<input type="submit" value="修改目录" class="b1">
		<input type="button" value="返回上页" class="b1" onclick="history.go(-1)">
	</td>
</tr>
<?php
		}
		else
		{
?>
<tr>
	<td class="t2">修改文章</td>
</tr>
<tr>
	<td class="t8">主题
	<input type="text" size="100" class="f1" name="subject" value="<?php echo htmlspecialchars($rows[subject]); ?>">
	</td>
</tr>
<tr>
	<td class="t5">
	评论
	<input type="radio" name="comment" class="f1" value="0" <?php if($rows[comment]!=0) echo "checked"; ?>>允许
	<input type="radio" name="comment" class="f1" value="1" <?php if($rows[comment]==0) echo "checked"; ?>>不允许
	</td>
</tr>
<tr>
	<td class="t8">
	Blog
	<select name="tid" class="f1">
<?php
		$blogs = pc_blog_menu($link,$pc,$rows[access]);
		for($i = 0 ; $i < count($blogs) ; $i ++)
		{
			if($blogs[$i]["TID"] == $rows[tid])
				echo "<option value=\"".$blogs[$i]["TID"]."\" selected>".html_format($blogs[$i]["NAME"])."</option>";
			else
				echo "<option value=\"".$blogs[$i]["TID"]."\" >".html_format($blogs[$i]["NAME"])."</option>";
		}
?>
	</select>
	</td>
</tr>
<tr>
	<td class="t13">心情符号</td>
</tr>
<tr>
	<td class="t5"><?php @require("emote.html"); ?></td>
</tr>
<tr>
	<td class="t11">内容
	<input type="checkbox" name="htmltag" value=1 <?php if(strstr($rows[body],$pcconfig["NOWRAPSTR"]) || $rows[htmltag] == 1) echo "checked"; ?> >使用HTML标记
	</td>
</tr>
<tr>
	<td class="t8">
	<textarea name="blogbody" class="f1" cols="120" rows="30" id="blogbody"  onkeydown='if(event.keyCode==87 && event.ctrlKey) {document.postform.submit(); return false;}'  onkeypress='if(event.keyCode==10) return document.postform.submit()' wrap="physical"><?php echo $pcconfig["EDITORALERT"]; ?><?php echo htmlspecialchars($rows[body]); ?></textarea>
	</td>
</tr>
<tr>
	<td class="t5">
	允许引用
	<input type="checkbox" name="trackback" value="1" <?php if($rows[trackback]==1) echo "checked"; ?>>
	</td>
</tr>
<tr>
	<td class="t2">
		<input type="button" name="ins" value="插入HTML" class="b1" onclick="return insertHTML();" />
		<input type="button" name="hil" value="高亮" class="b1" onclick="return highlight();" />
		<input type="submit" value="修改本文" class="b1">
		<input type="button" value="返回上页" onclick="history.go(-1)" class="b1">
	</td>
</tr>
<?php
		}
?>
</table>
</form></center>
<?php				
			}
		}
		elseif($act == "del")
		{
			$nid = (int)($_GET["nid"]);	
			$query = "SELECT `tid`,`pid`,`access`,`type`,`nodetype`,`subject` FROM nodes WHERE `uid` = '".$pc["UID"]."' AND `nid` = '".$nid."' ;";
			$result = mysql_query($query,$link);
			$rows = mysql_fetch_array($result);
			mysql_free_result($result);
			if(!$rows)
			{
				html_error_quit("文章不存在!");
				exit();
			}
			if($rows[nodetype]!=0)
			{
				html_error_quit("该文不能删除!");
				exit();
			}
			
			if($rows[access] == 4)
			{
				//彻底删除	
				$query = "DELETE FROM nodes WHERE `nid` = '".$nid."' ";
				mysql_query($query,$link);
				$query = "DELETE FROM comments WHERE `nid` = '".$nid."' ";
				mysql_query($query,$link);
				$query = "DELETE FROM trackback WHERE `nid` = '".$nid."' ";
				mysql_query($query,$link);
				$log_action = "DEL NODE: ".$rows[subject];
			}
			else
			{
				if($rows[type] == 1)
				{
					$query = "SELECT `nid` FROM nodes WHERE `pid` = '".$nid."' LIMIT 0, 1 ;";
					$result = mysql_query($query);
					if($rows0 = mysql_fetch_array($result))
					{
						mysql_free_result($result);
						html_error_quit("请先删除该目录下的文章!");
						exit();
					}
					mysql_free_result($result);
					$query = "DELETE FROM nodes WHERE `nid` = '".$nid."' ;";
					mysql_query($query,$link);
					$log_action = "DEL DIR: ".$rows[subject];
				}
				else
				{
					$query = "UPDATE nodes SET `access` = '4' , `changed` = '".date("YmdHis")."' , `tid` = '0' WHERE `nid` = '".$nid."' ;";
					mysql_query($query,$link);
					$log_action = "DEL TO JUNK: ".$rows[subject];
					if($rows[access] == 0)
						pc_update_record($link,$pc["UID"]," - 1");
				}
			}
			pc_update_record($link,$pc["UID"]);
			pc_return("pcdoc.php?userid=".$pc["USER"]."&tag=".$rows[access]."&tid=".$rows[tid]."&pid=".$rows[pid]);	
		}
		elseif($act == "clear")
		{
			$query = "SELECT `nid` FROM nodes WHERE `uid` = '".$pc["UID"]."' AND `access` = '4' ;";	
			$result = mysql_query($query,$link);
			$query = "DELETE FROM comments WHERE `nid` = '0' ";
			$query_tb = "DELETE FROM trackback WHERE `nid` = '0' ";
			while($rows = mysql_fetch_array($result))
			{
				$query.= "  OR `nid` = '".$rows[nid]."' ";	
				$query_tb.= "  OR `nid` = '".$rows[nid]."' ";	
			}
			mysql_query($query,$link);
			mysql_query($query_tb,$link);
			$query = "DELETE FROM nodes WHERE `uid` = '".$pc["UID"]."' AND `access` = '4' ;";
			mysql_query($query,$link);
			$log_action = "EMPTY JUNK";
			pc_update_record($link,$pc["UID"]);
			pc_return($_GET["ret"]);		
		}
		elseif($act == "tedit")
		{
			$tid = pc_load_topic($link,$pc["UID"],intval($_GET["tid"]),$topicname);
			if(!$tid)
			{
				html_error_quit("Blog不存在!");
				exit();
			}
			if($_POST["topicname"])
			{
				pc_edit_topics($link,$tid,$_POST["topicname"]);
				$log_action = "UPDATE TOPIC: ".$_POST["topicname"];
				pc_update_record($link,$pc["UID"]);
				pc_return("pcdoc.php?userid=".$pc["USER"]."&tag=6");		
			}
			else
			{
				$sec = array("公开区","好友区","私人区");
?>
<br>
<center>
<form action="pcmanage.php?userid=<?php echo $pc["USER"]; ?>&act=tedit&tid=<?php echo $tid; ?>" method="post" onsubmit="if(this.topicname.value==''){alert('请输入Blog名称!');return false;}">
<table cellspacing="0" cellpadding="5" border="0" width="90%" class="t1">
<tr>
	<td class="t2">修改Blog</td>
</tr>
<tr>
	<td class="t8">
	Blog名
	<input type="text" class="f1" name="topicname" value="<?php echo htmlspecialchars(stripslashes($topicname)); ?>">
	</td>
</tr>
<tr>
	<td class="t2">
	<input type="submit" value="修改Blog" class="b1">
	<input type="button" value="返回上页" class="b1" onclick="history.go(-1)">
	</td>
</tr>
</table>
</form></center>
<?php
			}
		}
		elseif($act == "tdel")
		{
			$tid = pc_load_topic($link,$pc["UID"],intval($_GET["tid"]),$topicname);
			if(!$tid)
			{
				html_error_quit("Blog不存在!");
				exit();
			}
			$ret = pc_del_topics($link,$tid);
			if($ret==-1)
			{
				html_error_quit("请先删除该分类的所有文章!");
				exit();
			}
			if($ret!=0)
			{
				html_error_quit("删除失败,请联系管理员!");
				exit();
			}
			pc_update_record($link,$pc["UID"]);
			$log_action = "DEL TOPIC: ".$topicname;
			pc_return("pcdoc.php?userid=".$pc["USER"]."&tag=6");				
		}
		elseif($act == "tadd" && $_POST["topicname"])
		{
			if(!pc_add_topic($link,$pc,$_POST["access"],$_POST["topicname"]))
			{
				html_error_quit("分类添加失败");
				exit();
			}
			$log_action = "ADD TOPIC: ".$_POST["topicname"];
			pc_return("pcdoc.php?userid=".$pc["USER"]."&tag=6");	
		}
		elseif($act == "sedit" && $_POST["pcname"])
		{
			$favmode = (int)($_POST["pcfavmode"]);
			if($favmode != 1 && $favmode != 2)
				$favmode = 0;
			$tmpsave = ($_POST["pctmpsave"]==0)?0:1;
			$query = "UPDATE `users` SET `createtime` = `createtime` , `corpusname` = '".addslashes(undo_html_format($_POST["pcname"]))."',`description` = '".addslashes(undo_html_format($_POST["pcdesc"]))."',`theme` = '".addslashes(undo_html_format($_POST["pcthem"]))."' , `backimage` = '".addslashes(undo_html_format($_POST["pcbkimg"]))."' , `logoimage` = '".addslashes(undo_html_format($_POST["pclogo"]))."' , `htmleditor` = '".(int)($_POST["htmleditor"])."', `style` = '".(int)($_POST["template"])."' , `indexnodechars` = '".(int)($_POST["indexnodechars"])."' , `indexnodes` = '".(int)($_POST["indexnodes"])."' , `favmode` = '".$favmode."' , `useremail` = '".addslashes(trim($_POST["pcuseremail"]))."' , `userinfor` = '".addslashes(trim($_POST["userinfor"]))."' , `defaulttopic` = '".addslashes(trim($_POST["pcdefaulttopic"]))."' , `tempsave` = '".$tmpsave."'  WHERE `uid` = '".$pc["UID"]."';";	
			mysql_query($query,$link);
			
			$log_action = "UPDATE SETTINGS";
			pc_return("pcdoc.php?userid=".$pc["USER"]."&tag=7");	
			
		}
		elseif($act == "adddir" && $_POST["dir"])
		{
			$ret = pc_add_favdir($link,$pc,$_POST["pid"],$_POST["dir"]);
			switch($ret)
			{
				case -1:
					html_error_quit("缺少Blog信息!");
					exit();
				case -2:
					html_error_quit("缺少父目录ID!");
					exit();
				case -3:
					html_error_quit("缺少目录名!");
					exit();
				case -4:
					html_error_quit("该目录下目录数已达上限!");
					exit();
				case -5:
					html_error_quit("系统错误,请联系管理员!");
					exit();
				default:	
			}
			pc_update_record($link,$pc["UID"]);
			$log_action = "ADD DIR: ".$_POST["dir"];
			pc_return("pcdoc.php?userid=".$pc["USER"]."&tag=3&pid=".intval($_POST["pid"]));
		}
		elseif($act == "favcut" || $act == "favcopy")
		{
			//目前不支持目录的剪切和复制
			$query = "SELECT `nid`,`type`,`pid`,`subject`,`tid` FROM nodes WHERE `nid` = '".(int)($_GET["nid"])."' AND `uid` = '".$pc["UID"]."' AND `access` = 3  AND `type` = 0 LIMIT 0 , 1;";
			$result = mysql_query($query,$link);
			$rows = mysql_fetch_array($result);
			if(!$rows)
			{
				pc_html_init("gb2312",stripslashes($pc["NAME"]));
				html_error_quit("文章不存在!");
				exit();
			}
			mysql_free_result($result);
			setcookie("BLOGFAVACTION",$act);
			setcookie("BLOGFAVNID",$rows[nid]);
			
			pc_html_init("gb2312",stripslashes($pc["NAME"]));
?>
<script language="javascript">
alert("已将 <?php echo htmlspecialchars($rows[subject]); ?> 放入剪切板!");
</script>
<?php			pc_return("pcdoc.php?userid=".$pc["USER"]."&tag=3&pid=".$rows[pid]);
			
		}
		elseif($act == "favpaste")
		{
			if(!$_COOKIE["BLOGFAVACTION"])
			{
				pc_html_init("gb2312",stripslashes($pc["NAME"]));
				html_error_quit("您的剪贴板是空的，请先剪切或者复制一个文件!");
				exit();
			}
			$pid = intval($_GET["pid"]);
			if(!pc_load_directory($link,$pc["UID"],$pid))
			{
				pc_html_init("gb2312",stripslashes($pc["NAME"]));
				html_error_quit("目标文件夹不存在!");
				exit();
			}
			
			if(pc_file_num($link,$pc["UID"],$pid)+1 > $pc["NLIM"])
			{
				pc_html_init("gb2312",stripslashes($pc["NAME"]));
				html_error_quit("目标文件夹中的文件数已达上限 ".$pc["NLIM"]. " 个!");
				exit();
			}
			
			if(intval($_COOKIE["BLOGFAVNID"]))
			{
				if($_COOKIE["BLOGFAVACTION"] == "favcut")
				{
					$query = "UPDATE nodes SET `pid` = '".$pid."' WHERE `nid` = '".intval($_COOKIE["BLOGFAVNID"])."';";
				}
				elseif($_COOKIE["BLOGFAVACTION"] == "favcopy")
				{
					$query = "SELECT * FROM nodes WHERE `nid` = '".intval($_COOKIE["BLOGFAVNID"])."' LIMIT 0 , 1 ;";
					$result = mysql_query($query,$link);
					$rows = mysql_fetch_array($result);
					mysql_free_result($result);
					$query = "INSERT INTO `nodes` ( `nid` , `pid` , `type` , `source` , `hostname` , `changed` , `created` , `uid` , `comment` , `commentcount` , `subject` , `body` , `access` , `visitcount` , `tid` , `emote` ,`htmltag`) ".
						"VALUES ('', '".$pid."', '0', '".addslashes($rows[source])."', '".addslashes($rows[hostname])."', NOW( ) , '".addslashes($rows[created])."', '".$pc["UID"]."', '".intval($rows[comment])."', '".intval($rows[commentcount])."', '".addslashes($rows[subject])."', '".addslashes($rows[body])."', '3', '".intval($rows[visitcount])."', '".intval($rows[tid])."', '".intval($rows[emote])."','".intval($rows[htmltag])."');";
				}
				mysql_query($query,$link);
			}
			setcookie("BLOGFAVACTION");
			setcookie("BLOGFAVNID");
			
			pc_html_init("gb2312",stripslashes($pc["NAME"]));
			pc_update_record($link,$pc["UID"]);
			$log_action = "CUT/COPY FAV";
			pc_return("pcdoc.php?userid=".$pc["USER"]."&tag=3&pid=".$pid);	
		}
	
		if(pc_is_groupwork($pc))
			pc_group_logs($link,$pc,$log_action,$log_content);
		
		html_normal_quit();
	}
	
?>