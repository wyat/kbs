<?php
define("ANNOUNCENUMBER",5);

define("ARTICLESPERPAGE",30); //Ŀ¼�б���ÿҳ��ʾ��������

define("THREADSPERPAGE",10); //�����Ķ�ʱÿҳ��ʾ��������

$SiteName="ˮľ�廪";

$SiteURL="http://www.smth.org/wForum/";

$HTMLTitle="BBS ˮľ�廪վ";

$HTMLCharset="GB2312";

$DEFAULTStyle="defaultstyle";

$Banner="bar/bar.jpg";

$BannerURL="http://http://www.smth.org/";

define ("MAINTITLE","<OBJECT classid=clsid:D27CDB6E-AE6D-11cf-96B8-444553540000 codebase=http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=5,0,0,0 height=84 width=600> <PARAM NAME='MOVIE' VALUE='bar/smth2.swf' /><EMBED SRC=\"bar/smth2.swf\" height=84 width=600></EMBED></OBJECT>");

define("ATTACHMAXSIZE","2097152");
define("ATTACHMAXCOUNT","3");
$section_nums = array("0", "1", "2", "3", "4", "5", "6", "7", "8", "9","A","B","C");
$section_names = array(
    array("BBS ϵͳ", "[վ��]"),
    array("�廪��ѧ", "[��У]"),
    array("ѧ����ѧ", "[ѧ��/����]"),
    array("��������", "[����/����]"),
    array("�Ļ�����", "[�Ļ�/����]"),
    array("�����Ϣ", "[���/��Ϣ]"),
    array("��Ϸ���", "[��Ϸ/����]"),
    array("��������", "[�˶�/����]"),
    array("֪�Ը���", "[̸��/����]"),
    array("������Ϣ", "[����/��Ϣ]"),
    array("��������", "[����/����]"),
    array("����ϵͳ", "[ϵͳ/�ں�]"),
    array("���Լ���", "[ר���]")
);
$sectionCount=count($section_names);

require "default.php";
?>