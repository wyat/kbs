<?php
/*
**  为水木清华blog申请定制的申请表
**  满足条件的申请表将被提交至BBSBLOGBOARD等候处理
**  @windinsn Mar 12 , 2004
*/
require("pcfuncs.php");
define(BBSBLOGBOARD , "SMTH_blog"); //blog版面
define(BBSBLOGMINREGTIME , 6); //最短注册时间

if ($loginok != 1)
	html_nologin();
elseif(!strcmp($currentuser["userid"],"guest"))
{
	html_init("gb2312");
	html_error_quit("请登录后再进行Blog申请!");
	exit();
}
else
{
	$link = pc_db_connect();
	if( pc_load_infor($link,$currentuser["userid"]) )
	{
		pc_db_close($link);
		html_init("gb2312");
		html_error_quit("对不起，您已经拥有Blog了");
		exit();	
	}
	pc_db_close($link);
	
	if( time() - $currentuser["firstlogin"] < intval( BBSBLOGMINREGTIME * 2592000 ) )
	{
		html_init("gb2312");
		html_error_quit("对不起，您的注册时间尚不足".BBSBLOGMINREGTIME."个月");
		exit();	
	}
	if( !$_POST["appname"] || !$_POST["appself"] || !$_POST["appdirect"] )
	{
		html_init("gb2312");
		html_error_quit("对不起，请详细填写Blog申请表");
		exit();	
	}
	
	$apptitle = "[申请] ".$currentuser["userid"]." 申请建立水木BLOG";
	$appbody  = "(1) BLOG名称：".$_POST["appname"]."\n\n\n".
		    "(2) 申请人 ID 及简要自我介绍\n".
		    "    ID：".$currentuser["userid"]."\n".
		    "    注册时间：".date("Y年m月d日",$currentuser["firstlogin"])."\n\n".
		    "        ".$_POST["appself"]."\n\n\n".
		    "(3) 经营方向：(您对您个人Blog的初步规划)\n        ".$_POST["appdirect"]."\n\n";
	
	$ret = bbs_postarticle(BBSBLOGBOARD, preg_replace("/\\\(['|\"|\\\])/","$1",$apptitle), preg_replace("/\\\(['|\"|\\\])/","$1",$appbody), 0 , 0 , 0 , 0);
	switch ($ret) {
			case -1:
				html_error_quit("错误的讨论区名称!");
				break;
			case -2: 
				html_error_quit("本版为二级目录版!");
				break;
			case -3: 
				html_error_quit("标题为空!");
				break;
			case -4: 
				html_error_quit("此讨论区是唯读的, 或是您尚无权限在此发表文章!");
				break;		
			case -5:	
				html_error_quit("很抱歉, 你被版务人员停止了本版的post权利!");
				break;	
			case -6:
				html_error_quit("两次发文间隔过密,请休息几秒再试!");	
				break;
			case -7: 
				html_error_quit("无法读取索引文件! 请通知站务人员, 谢谢! ");
				break;
			case -8:
				html_error_quit("本文不可回复!");
				break;
			case -9:
				html_error_quit("系统内部错误, 请迅速通知站务人员, 谢谢!");
				break;
		}
	pc_html_init("gb2312","Blog申请");
?>
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1>
<tr align=center><th width="100%">申请提交成功！</td>
</tr><tr><td width="100%" class=TableBody1>
您的BLOG申请已经提交成功，管理员会在两天内处理您的申请。<br/><br/>
本页面将在3秒后自动切换自Blog论坛<meta HTTP-EQUIV=REFRESH CONTENT='3; URL=/bbsdoc.php?board=<?php echo BBSBLOGBOARD; ?>' >，<b>您可以选择以下操作：</b><br><ul>
<li><a href="/mainpage.php">返回首页</a></li>
<li><a href="/pc/pcmain.php">返回Blog首页</a></li>
<li><a href="/bbsdoc.php?board=<?php echo BBSBLOGBOARD; ?>">返回Blog论坛</a></li>
</ul></td></tr></table>
<?php	
	html_normal_quit();
}
	
?>