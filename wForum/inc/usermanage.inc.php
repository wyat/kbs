<?php
function showUserManageMenu(){
?>
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1>
<tr>
<th width=14% height=25 id=TableTitleLink><a href=usermanagemenu.php>控制面板首页</a></th>
<th width=14%  id=TableTitleLink><a href=modifyuserdata.php>基本资料修改</a></th>
<th width=14%  id=TableTitleLink><a href=changepasswd.php>用户密码修改</a></th>
<th width=14%  id=TableTitleLink><a href=userparam.php>用户自定义参数</a></th>
<th width=14%  id=TableTitleLink><a href=usermailbox.php>用户信件服务</a></th>
<th width=14%  id=TableTitleLink><a href=friendlist.php>编辑好友列表</a></th>
<th width=14%  id=TableTitleLink><a href=modifyfavboards.php>收藏版面管理</a></th>
</tr>
</table>
<?php
}

function showmailBoxes() {
?>
<TABLE cellpadding=6 cellspacing=1 align=center class=TableBorder1><TBODY><TR>
<TD align=center class=TableBody1><a href="usermailbox.php?boxname=inbox"><img src=pic/m_inbox.gif border=0 alt=收件箱></a> &nbsp; <a href="usermailbox.php?boxname=sendbox"><img src=pic/m_outbox.gif border=0 alt=发件箱></a> &nbsp; <a href="usermailbox.php?boxname=deleted"><img src=pic/m_recycle.gif border=0 alt=废件箱></a>&nbsp; <a href="friendlist.php"><img src=pic/m_address.gif border=0 alt=地址簿></a>&nbsp;<a href="sendmail.php"><img src=pic/m_write.gif border=0 alt=发送消息></a></td></tr></TBODY></TABLE>
<?php
}

function getMailBoxName($name){
	if ($name=='inbox') {
		return "收件箱";
	}
	if ($name=='sendbox') {
		return "收件箱";
	}
	if ($name=='deleted') {
		return "收件箱";
	}
	return "未知邮箱";
}
?>
