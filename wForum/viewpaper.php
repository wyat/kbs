<?php
require("inc/funcs.php");
require("inc/board.inc.php");
require("inc/user.inc.php");
require("inc/conn.php");
require("inc/ubbcode.php");

global $boardArr;
global $boardID;
global $boardName;

preprocess();

setStat("����С�ֱ�");

html_init();
echo "<body>";

if (isErrFounded()) {
	html_error_quit() ;
} else {
	showpaper($boardID,$boardName);
}

show_footer(false);

CloseDatabase();

function preprocess(){
	global $boardID;
	global $boardName;
	global $currentuser;
	global $boardArr;
	global $loginok;
	global $id;


	if (!isset($_GET['boardname'])) {
		foundErr("δָ�����档");
		return false;
	}
	$boardName=$_GET['boardname'];
	$brdArr=array();
	$boardID= bbs_getboard($boardName,$brdArr);
	$boardArr=$brdArr;
	$boardName=$brdArr['NAME'];
	if ($boardID==0) {
		foundErr("ָ���İ��治���ڡ�");
		return false;
	}
	$usernum = $currentuser["index"];
	if (bbs_checkreadperm($usernum, $boardID) == 0) {
		foundErr("����Ȩ�Ķ����棡");
		return false;
	}
	if (!isset($_GET['id'])){
		foundErr("����Ĳ�����");
		return false;
	}
	$id=intval($_GET['id']);
	return true;
}

function showpaper($boardID,$boardName){
	global $conn;
	global $id;
	$rs=$conn->getRow("select * from smallpaper_tb where ID=".$id,DB_FETCHMODE_ASSOC);
	if (count($rs)==0) {
		foundErr("û���ҵ������Ϣ��");
		return false;
	}	else  {
		$conn->query("update smallpaper_tb set Hits=Hits+1 where ID=".$id);
?>
<table cellpadding=3 cellspacing=1 align=center class=TableBorder1>
<TBODY> 
<TR> 
<Th height=24><?php echo htmlspecialchars($rs["Title"],ENT_QUOTES); ?></Th>
</TR>
<TR> 
<TD class=TableBody1>
<p align=center><a href=dispuser.php?id=<?php echo $rs["Owner"]; ?> target=_blank><?php echo $rs["Owner"]; ?></a> ������ <?php  echo $rs["Addtime"]; ?></p>
    <blockquote>   
      <br>   
<?php     echo dvbcode($rs["Content"],1); ?>  
      <br>
<div align=right>���������<?php  echo $rs["Hits"]; ?></div>
    </blockquote>
</TD>
</TR>
<TR align=center> 
<TD height=24 class=TableBody2><a href=# onclick="window.close();">�� �رմ��� ��</a></TD>
</TR>
</TBODY>
</TABLE>
<?php 
  } 
  
  $rs=null;

} 

?>
