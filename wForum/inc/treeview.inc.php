<?php

/*
 * ���λظ��ṹ��ں��� showTree(boardName, groupID, articles, displayFN, maxThread=51, startNum=0)
 *
 * @param boardName    ����Ӣ������
 * @param groupID      ������� groupID
 * @param articles     �� bbs_get_threads_from_gid() �����ƺ������ص� $articles ����
 * @param displayFN    ������ʾÿ�����ӵĻص�����
 *        displayFN(boardName,groupID,article,startNum,level,lastflag)
 *        @param boardName,groupID    ͬ��
 *        @param article              ��ƪ���¡���� == null ������ʾָʾ���и����ʡ�Ժ�
 *        @param startNum             ��ƪ��������������е����
 *        @param level                ����ڼ��㡣ԭ�� level = 0
 *        @param lastflag             $lastflag[$l] ��ʾ�� $l ���Ƿ��и���ظ�
 * @param maxThread    �����ʾ���ٸ����ӣ������������Ĭ�� 51��
 * @param startNum     ���������������Ŀ����maxThread����ƽ����ʾstartNum������maxThread�������Լ��������һ������
 *
 * ע�����������ȫ�ֳ��� SHOWREPLYTREE Ӱ�졣��� SHOWREPLYTREE ����Ϊ 0������Զ��ֻƽ����ʾ���лظ���
 * @author atppp
 */

class TreeNode {
	var $data;
	var $index;
	var $showed;
	var $first_child;
	var $last_child;
	var $next_sibling;
	
	function TreeNode($data, $index) {
		$this->data = &$data;
		$this->index = $index;
		$this->showed = false;
		$this->first_child = $this->last_child = $this->next_sibling = null;
	}
	
	function addChild(&$node) { /* here it's very important to assign by reference */
		if ($this->first_child == null) $this->first_child = &$node;
		if ($this->last_child != null) {
			$this->last_child->next_sibling = &$node;
		}
		$this->last_child = &$node;
	}
}

function showTree($boardName, $groupID, $articles, $displayFN, $maxthread = 51, $startNum = 0) {
	$threadNum = count($articles);
	$more = ($threadNum > $maxthread);
	$lastflag = array();
	if ($more || !SHOWREPLYTREE) {
		$start = $startNum - (int)($maxthread / 2);
		if ($start <= 0) $start = 1;
		$end = $start + $maxthread;
		if ($end > $threadNum) {
			$end = $threadNum;
			$start = $threadNum - $maxthread;
			if ($start <= 0) $start = 1;
		}

		$displayFN($boardName, $groupID, $articles[0], 0, 0, $lastflag);
		if ($threadNum > 1) {
			$lastflag[0] = true;
			if ($start > 1) $displayFN($boardName, $groupID, null, $start - 1, 1, $lastflag);
			for($i = $start; $i < $end; $i++) {
				$lastflag[0] = ($i != $threadNum - 1);
				$displayFN($boardName, $groupID, $articles[$i], $i, 1, $lastflag);
			}
			if ($lastflag[0]) { //�ٺ٣�����е���� :p
				$lastflag[0] = false;
				$displayFN($boardName, $groupID, null, $end, 1, $lastflag);
			}
		}
	} else {
		/* �����ظ����ṹ, optimized by roy */
		$treenodes = array();
		$treenodeIndexes = array();
		for($i = 0; $i < $threadNum; $i++) {
			$treenodeIndexes[$articles[$i]['ID']] = $i;
			$treenodes[$i] = new TreeNode($articles[$i],$i);
			if ( ($i > 0) && ( isset($treenodeIndexes[$articles[$i]['REID']]) ) ) {
				$treenodes[$treenodeIndexes[$articles[$i]['REID']]]->addChild($treenodes[$i]);
			}
		}

		showTreeRecursively($boardName, $groupID, $treenodes, 0, 0, $lastflag, $displayFN);
		for($i=0; $i < $threadNum; $i++) { // �����ĺ��ӣ�û�е�������
			if (!$treenodes[$i]->showed) {
				//$displayFN($boardName, $groupID, $treenodes[$i]->data, $i, 0, $lastflag);
				showTreeRecursively($boardName, $groupID, $treenodes, $i, 0, $lastflag, $displayFN);
			}
		}
	}
}

function showTreeRecursively($boardName, $groupID, &$treenodes, $index, $level, &$lastflag, $displayFN) {
	/*
	 * ������ʵ�и����⣬����ظ��ṹ�л��Ļ�������ݹ飬�����������ж�һ�� $treenodes[$index]->showed �ǲ��� false.
	 * ���� smthbbs ϵͳӦ�ò���������ֻ�������ʱ�����ˡ�
	 */
	$displayFN($boardName, $groupID, $treenodes[$index]->data, $index, $level, $lastflag);
	$treenodes[$index]->showed = true;
	$cur = &$treenodes[$index]->first_child;
	while($cur != null) {
		$temp = &$cur->next_sibling;
		$lastflag[$level] = ($temp != null);
		showTreeRecursively($boardName, $groupID, $treenodes, $cur->index, $level+1, $lastflag, $displayFN);
		$cur = &$temp;
	}
}

?>