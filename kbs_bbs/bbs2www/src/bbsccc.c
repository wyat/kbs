/*
 * $Id$
 */  
#include "bbslib.h"
char quote_title[120], quote_board[120];
char quote_file[120], quote_user[120];
char save_title[STRLEN];
char save_filename[4096];


/* Add by SmallPig */ 
/* 把quote_file复制到filepath (转贴或自动发信)*/ 
void getcross2(char *filepath, char *board, struct userec *user) 
{
	FILE * inf, *of;
	char buf[256];
	char owner[256];
	int count;

	time_t now;
	int normal_file;
	int header_count;

	now = time(0);
	inf = fopen(quote_file, "r");
	of = fopen(filepath, "w");
	if(inf == NULL || of == NULL)
	{
		if(NULL != inf)
			fclose(inf);
		if(NULL != of)
			fclose(of);
		report("Cross Post error");
		return;
	}
	normal_file = 1;
	write_header2(of, board, save_title, user->userid, user->username, 0);
	// 从文章头中取出原作者ID
	if(fgets(buf, 256, inf) != NULL)
	{
		for(count = 8; buf[count] != ' '; count++)
			owner[count - 8] = buf[count];
	}
	owner[count - 8] = '\0';
	fprintf(of, "【 以下文字转载自 %s 讨论区 】\n", quote_board);
	if(id_invalid(owner))
		normal_file = 0;
	if(normal_file)
	{
		for(header_count = 0; header_count < 3; header_count++)
		{
			if(fgets(buf, 256, inf) == NULL)
				break;		   /*Clear Post header */
		}
		if((header_count != 2) || (buf[0] != '\n'))
			normal_file = 0;
	}
	if(normal_file)
		fprintf(of, "【 原文由 %s 所发表 】\n", owner);
	else
		fseek(inf, 0, SEEK_SET);
	while(fgets(buf, 256, inf) != NULL)
	{
		if((strstr(buf, "【 以下文字转载自 ") && strstr(buf, "讨论区 】"))  ||(strstr(buf, "【 原文由") && strstr(buf, "所发表 】")))
			continue;		   /* 避免引用重复 */
		else
			fprintf(of, "%s", buf);
	}
	fclose(inf);
	fclose(of);
	*quote_file = '\0';
}


/* Add by SmallPig */ 
int post_cross2(int local_save, char *board) 
{
	struct fileheader postfile;
	char filepath[STRLEN], fname[STRLEN];
	char buf[256], buf4[STRLEN], whopost[IDLEN];
	int fp, i;

	time_t now;

	if(!haspostperm(currentuser, board))
	{
		printf("no post perm.<br>\n");
		return -1;
	}
	memset(&postfile, 0, sizeof(postfile));

	if(!strstr(quote_title, "(转载)"))
		sprintf(buf4, "%s (转载)", quote_title);
	else
		strcpy(buf4, quote_title);

	/* 还有一个破 save_title 的问题 */ 
	strncpy(save_title, buf4, STRLEN);
	setbfile(fname, board, "");
	GET_POSTFILENAME(postfile.filename, fname);
	strcpy(whopost, getcurruserid());
	strncpy(postfile.owner, whopost, OWNER_LEN);
	postfile.owner[OWNER_LEN-1]=0;
	setbfile(filepath, board, postfile.filename);
	getcross2(filepath, board, getcurrusr()); /*根据fname完成 文件复制 */
	strncpy(postfile.title, save_title, ARTICLE_TITLE_LEN - 1);
	postfile.title[ARTICLE_TITLE_LEN - 1] = '\0';
	if(local_save == 1)	  /* local save */
	{
		postfile.innflag[1] = 'L';
		postfile.innflag[0] = 'L';
	}
	else
	{
		struct userec *user = getcurrusr();

		postfile.innflag[1] = 'S';
		postfile.innflag[0] = 'S';
		outgo_post2(&postfile, board, user->userid, user->username, save_title);
	}
	after_post(currentuser, &postfile, board, NULL, 1);

	return 1;
}


/* ent		1-based
 * board    source board
 * board2   dest board
*/ 
int do_cross(int ent, struct fileheader *fileinfo, char *direct, char *board, char *board2, int local_save) 
{
	char bname[STRLEN];
	char dbname[STRLEN];

	if(!HAS_PERM(currentuser, PERM_POST))	 /* 判断是否有POST权 */
	{
		return 0;
	}
	if((fileinfo->accessed[0] & FILE_FORWARDED) && !HAS_PERM(currentuser, PERM_SYSOP))
	{
		http_fatal("本文章已经转贴过一次，无法再次转贴");
	}
	if(fileinfo->attachment)
	{
		http_fatal("带附件文章暂时不支持转贴");
	}
	setbfile(quote_file, board, fileinfo->filename);
	strcpy(quote_title, fileinfo->title);
	if(!strcmp(board2, board))
	{
		http_fatal("\n\n                          本版的文章不需要转贴到本版!");
	}
	if(deny_me(currentuser->userid, board2) && !HAS_PERM(currentuser, PERM_SYSOP))	 /* 版主禁止POST 检查 */
	{
		http_fatal("\n\n                很抱歉，你在该版被其版主停止了 POST 的权力...\n");
	}
	else if(true == checkreadonly(board2))	   /* Leeward 98.03.28 */
	{
		http_fatal("\n\n                只读看版不能转入文章\n");
	}
	strcpy(quote_board, board);
	if(post_cross2(local_save, board2) == -1)	  /* 转贴 */
	{
		http_fatal("转贴文章失败.\n");
	}
	fileinfo->accessed[0] |= FILE_FORWARDED;   /*added by alex, 96.10.3 */
	substitute_record(direct, fileinfo, sizeof(*fileinfo), ent);
	return 0;
}

int do_ccc(int ent, struct fileheader *x, char *dir, char *board, char *board2, int local) 
{
	FILE * fp;
	char path[200];

	sprintf(path, "boards/%s/%s", board, x->filename);
	fp = fopen(path, "r");
	if(fp == 0)
		http_fatal("文件内容已丢失, 无法转载");
	fclose(fp);
	do_cross(ent, x, dir, board, board2, local);
	printf("'%s' 已转贴到 %s 版.<br>\n", nohtml(x->title), board2);
	printf("[<a href=\"javascript:history.go(-2)\">返回</a>]");
}

int main() 
{
	struct fileheader f;
	char board[80], dir[80], file[80], target[80];
	FILE * fp;
	int found = 0;
	int num = 0;
	int local;
	struct boardheader *src_bp;
	struct boardheader *dst_bp;

	init_all();
	strsncpy(board, getparm("board"), 30);
	strsncpy(file, getparm("file"), 30);
	strsncpy(target, getparm("target"), 30);
	local = atoi(getparm("outgo")) ? 0 : 1;
	if(!loginok)
		http_fatal("匆匆过客不能进行本项操作");
	src_bp = getbcache(board);
	if (src_bp == NULL)
		http_fatal("错误的讨论区");
	strcpy(board, src_bp->filename);
	if(!check_read_perm(currentuser, src_bp))
		http_fatal("错误的讨论区");
	sprintf(dir, "boards/%s/.DIR", board);
	fp = fopen(dir, "r");
	if(fp == 0)
		http_fatal("错误的参数");
	while(1)
	{
		if(fread(&f, sizeof(struct fileheader), 1, fp) <= 0)
			break;
		if(!strcmp(f.filename, file))
		{
			found = 1;
			break;
		}
		num++;
	}
	fclose(fp);
	if(found == 0)
		http_fatal("错误的参数");
	printf("<center>%s -- 转载文章 [使用者: %s]<hr color=\"green\">\n", BBSNAME, currentuser->userid);
	if(target[0])
	{
		dst_bp = getbcache(target);
		if (dst_bp == NULL)
			http_fatal("错误的讨论区");
		strcpy(target, dst_bp->filename);
		if(!haspostperm(currentuser, target))
			http_fatal("错误的讨论区名称或你没有在该版发文的权限");
		return do_ccc(num + 1, &f, dir, board, target, local);
	}
	printf("<table><tr><td>\n");
	printf("<font color=\"red\">转贴发文注意事项:<br>\n");
	printf("本站规定同样内容的文章严禁在 5 个或 5 个以上讨论区内重复发表。<br>\n");
	printf("违者将被封禁在本站发文的权利.<br><br></font>\n");
	printf("文章标题: %s<br>\n", nohtml(f.title));
	printf("文章作者: %s<br>\n", f.owner);
	printf("原讨论区: %s<br>\n", board);
	printf("<form action=\"bbsccc\" method=\"post\">\n");
	printf("<input type=\"hidden\" name=\"board\" value=\"%s\">", board);
	printf("<input type=\"hidden\" name=\"file\" value=\"%s\">", file);
	printf("转载到 <input name=\"target\" size=\"30\" maxlength=\"30\"> 讨论区.<br>\n ");
	printf("<input type=\"checkbox\" name=\"outgo\" value=\"1\" CHECKED>转信<br>\n");
	printf("<input type=\"submit\" value=\"确定\"></form></td></tr></table>");
	http_quit();
	return 0;
}


