/*
    Pirate Bulletin Board System
    Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU
    Eagles Bulletin Board System
    Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
                        Guy Vega, gtvega@seabass.st.usm.edu
                        Dominic Tynes, dbtynes@seabass.st.usm.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	only too int global: bmonly a_fmode
*/

/* 精华区 相关 函数 */

#include "bbs.h"

#define A_PAGESIZE      (t_lines - 5)

#define ADDITEM         0
#define ADDGROUP        1
#define ADDMAIL         2
#define ADDGOPHER       3

int bmonly = 0;
int a_fmode = 1;
char *email_domain();
void a_menu();
void a_report();                /*Haohmaru.99.12.06.版主精华区操作记录，作为考查工作的依据 */

extern void a_prompt();         /* added by netty */
int t_search_down();
int t_search_up();


void a_report(s)                /* Haohmaru.99.12.06 */
    char *s;
{
    int fd;

    /* disable it because of none using it , KCN,2002.07.31 */
    return;
/*	
    if((fd = open("a_trace",O_WRONLY|O_CREAT,0644)) != -1 ) {
        char buf[512] ;
        char timestr[24], *thetime;
        time_t dtime;
        time(&dtime);
        thetime = ctime(&dtime);
        strncpy(timestr, thetime, 20);
        timestr[20] = '\0';
        flock(fd,LOCK_EX) ;
        lseek(fd,0,SEEK_END) ;
        sprintf(buf,"%s %s %s\n",currentuser->userid, timestr, s) ;
        write(fd,buf,strlen(buf)) ;
        flock(fd,LOCK_UN) ;
        close(fd) ;
        return ;
    }
*/
}

int valid_fname(str)
    char *str;
{
    char ch;

    while ((ch = *str++) != '\0') {
        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || strchr("0123456789@[]-._", ch) != NULL) {
            ;
        } else {
            return 0;
        }
    }
    return 1;
}

void a_showmenu(pm)             /* 精华区 菜单 状态 */
    MENU *pm;
{
    struct stat st;
    struct tm *pt;
    char title[STRLEN * 2], kind[20];
    char fname[STRLEN];
    char ch;
    char buf[STRLEN], genbuf[STRLEN * 2];
    time_t mtime;
    int n;
    int chkmailflag = 0;

    clear();
    chkmailflag = chkmail();

    if (chkmailflag == 2) {     /*Haohmaru.99.4.4.对收信也加限制 */
        prints("[5m");
        sprintf(genbuf, "[您的信箱超过容量,不能再收信!]");
    } else if (chkmailflag) {
        prints("[5m");
        sprintf(genbuf, "[您有信件]");
    } else
        strncpy(genbuf, pm->mtitle, STRLEN * 2 - 1);
    sprintf(buf, "%*s", (80 - strlen(genbuf)) / 2, "");
    prints("[44m%s%s%s[m\n", buf, genbuf, buf);
    prints("            F 寄回自己的信箱┃↑↓ 移动┃→ <Enter> 读取┃←,q 离开[m\n");
    prints("[44m[37m 编号  %-45s 整  理           %8s [m", "[类别] 标    题", a_fmode == 2 ? "档案名称" : "编辑日期");
    prints("\n");
    if (pm->num == 0)
        prints("      << 目前没有文章 >>\n");
    for (n = pm->page; n < pm->page + 19 && n < pm->num; n++) {
        strncpy(title, pm->item[n]->title, STRLEN * 2 - 1);
        if (a_fmode) {
            sprintf(fname, "%s", pm->item[n]->fname);
            sprintf(genbuf, "%s/%s", pm->path, fname);
            if (a_fmode == 2) {
                ch = (dashf(genbuf) ? ' ' : (dashd(genbuf) ? '/' : ' '));
                fname[10] = '\0';
            } else {
                if (dashf(genbuf) || dashd(genbuf)) {
                    stat(genbuf, &st);
                    mtime = st.st_mtime;
                } else
                    mtime = time(0);

                pt = localtime(&mtime);
                sprintf(fname, "[1m%04d[m.[1m%02d[m.[1m%02d[m", pt->tm_year + 1900, pt->tm_mon + 1, pt->tm_mday);
                ch = ' ';
            }
            if (pm->item[n]->host != NULL) {
                strcpy(kind, "[[33m连线[m]");
            } else if (dashf(genbuf)) {
                strcpy(kind, "[[36m文件[m]");
            } else if (dashd(genbuf)) {
                strcpy(kind, "[目录]");
            } else {
                strcpy(kind, "[[32m错误[m]");
            }
            if (!strncmp(title, "[目录] ", 7) || !strncmp(title, "[文件] ", 7)
                || !strncmp(title, "[连线] ", 7))
                sprintf(genbuf, "%-s %-55.55s%-s%c", kind, title + 7, fname, ch);
            else
                sprintf(genbuf, "%-s %-55.55s%-s%c", kind, title, fname, ch);
            strncpy(title, genbuf, STRLEN * 2 - 1);
        }
        prints("  %3d  %s\n", n + 1, title);
    }
    clrtobot();
    move(t_lines - 1, 0);
    prints("%s", (pm->level & PERM_BOARDS) ?
           "[31m[44m[版  主]  [33m说明 h │ 离开 q,← │ 新增文章 a │ 新增目录 g │ 修改档案 e        [m" :
           "[31m[44m[功能键] [33m 说明 h │ 离开 q,← │ 移动游标 k,↑,j,↓ │ 读取资料 Rtn,→         [m");
}

void a_additem(pm, title, fname, host, port)    /* 产生ITEM object,并初始化 */
    MENU *pm;
    char *title, *fname, *host;
    int port;
{
    ITEM *newitem;

    if (pm->num < MAXITEMS) {
        newitem = (ITEM *) malloc(sizeof(ITEM));
        strncpy(newitem->title, title, sizeof(newitem->title) - 1);
        if (host != NULL) {
            newitem->host = (char *) malloc(sizeof(char) * (strlen(host) + 1));
            strcpy(newitem->host, host);
        } else
            newitem->host = host;
        newitem->port = port;
        strncpy(newitem->fname, fname, sizeof(newitem->fname) - 1);
        pm->item[(pm->num)++] = newitem;
    }
}

int a_loadnames(pm)             /* 装入 .Names */
    MENU *pm;
{
    FILE *fn;
    ITEM litem;
    char buf[PATHLEN], *ptr;
    char hostname[STRLEN];

    pm->num = 0;
    sprintf(buf, "%s/.Names", pm->path);        /*.Names记录菜单信息 */
    if ((fn = fopen(buf, "r")) == NULL)
        return 0;
    hostname[0] = '\0';
    while (fgets(buf, sizeof(buf), fn) != NULL) {
        if ((ptr = strchr(buf, '\n')) != NULL)
            *ptr = '\0';
        if (strncmp(buf, "Name=", 5) == 0) {
            strncpy(litem.title, buf + 5, sizeof(litem.title));
        } else if (strncmp(buf, "Path=", 5) == 0) {
            if (strncmp(buf, "Path=~/", 7) == 0)
                strncpy(litem.fname, buf + 7, sizeof(litem.fname));
            else
                strncpy(litem.fname, buf + 5, sizeof(litem.fname));
            if ((!strstr(litem.title, "(BM: BMS)") || HAS_PERM(currentuser, PERM_BOARDS)) &&
                (!strstr(litem.title, "(BM: SYSOPS)") || HAS_PERM(currentuser, PERM_SYSOP)) && (!strstr(litem.title, "(BM: ZIXIAs)") || HAS_PERM(currentuser, PERM_SECANC))) {
                if (strstr(litem.fname, "!@#$%")) {     /*取 host & port */
                    char *ptr1, *ptr2, gtmp[STRLEN];

                    strncpy(gtmp, litem.fname, STRLEN - 1);
                    ptr1 = strtok(gtmp, "!#$%@");
                    strcpy(hostname, ptr1);
                    ptr2 = strtok(NULL, "@");
                    strncpy(litem.fname, ptr2, sizeof(litem.fname) - 1);
                    litem.port = atoi(strtok(NULL, "@"));
                }
                a_additem(pm, litem.title, litem.fname, (strlen(hostname) == 0) ?       /*产生ITEM */
                          NULL : hostname, litem.port);
            }
            hostname[0] = '\0';
        } else if (strncmp(buf, "# Title=", 8) == 0) {
            if (pm->mtitle[0] == '\0')
                strcpy(pm->mtitle, buf + 8);
        } else if (strncmp(buf, "Host=", 5) == 0) {
            strcpy(hostname, buf + 5);
        } else if (strncmp(buf, "Port=", 5) == 0) {
            litem.port = atoi(buf + 5);
        }
    }
    fclose(fn);
    return 1;
}

void a_savenames(pm)            /*保存当前MENU到 .Names */
    MENU *pm;
{
    FILE *fn;
    ITEM *item;
    char fpath[PATHLEN];
    int n;

    sprintf(fpath, "%s/.Names", pm->path);
    if ((fn = fopen(fpath, "w")) == NULL)
        return;
    fprintf(fn, "#\n");
    if (!strncmp(pm->mtitle, "[目录] ", 7) || !strncmp(pm->mtitle, "[文件] ", 7)
        || !strncmp(pm->mtitle, "[连线] ", 7)) {
        fprintf(fn, "# Title=%s\n", pm->mtitle + 7);
    } else {
        fprintf(fn, "# Title=%s\n", pm->mtitle);
    }
    fprintf(fn, "#\n");
    for (n = 0; n < pm->num; n++) {
        item = pm->item[n];
        if (!strncmp(item->title, "[目录] ", 7) || !strncmp(item->title, "[文件] ", 7)
            || !strncmp(item->title, "[连线] ", 7)) {
            fprintf(fn, "Name=%s\n", item->title + 7);
        } else
            fprintf(fn, "Name=%s\n", item->title);
        if (item->host != NULL) {
            fprintf(fn, "Host=%s\n", item->host);
            fprintf(fn, "Port=%d\n", item->port);
            fprintf(fn, "Type=1\n");
            fprintf(fn, "Path=%s\n", item->fname);
        } else
            fprintf(fn, "Path=~/%s\n", item->fname);
        fprintf(fn, "Numb=%d\n", n + 1);
        fprintf(fn, "#\n");
    }
    fclose(fn);
    chmod(fpath, 0644);
}

void a_prompt(bot, pmt, buf)    /* 精华区状态下 输入 */
    int bot;
    char *pmt, *buf;
{
    move(t_lines + bot, 0);
    clrtoeol();
    getdata(t_lines + bot, 0, pmt, buf, 39, DOECHO, NULL, true);
}

void a_prompt2(int bot,char* pmt,char* buf)    /* 精华区状态下 输入 ,包含原来的内容*/
{
    move(t_lines + bot, 0);
    clrtoeol();
    getdata(t_lines + bot, 0, pmt, buf, 39, DOECHO, NULL, false);
}

/* a_SeSave 用来删除存到暂存档时的文件头和尾 Life 1997.4.6 */
int a_SeSave(char *path, char *key, struct fileheader *fileinfo, int nomsg)
{

    char ans[STRLEN];
    FILE *inf, *outf;
    char qfile[STRLEN], filepath[STRLEN];
    char buf[256];

    sprintf(qfile, "boards/%s/%s", key, fileinfo->filename);
    sprintf(filepath, "tmp/se.%s", currentuser->userid);
    outf = fopen(filepath, "w");
    if (*qfile != '\0' && (inf = fopen(qfile, "r")) != NULL) {
        fgets(buf, 256, inf);
        fprintf(outf, "%s", buf);
        fprintf(outf, "\n");
        while (fgets(buf, 256, inf) != NULL)
            if (buf[0] == '\n')
                break;


        while (fgets(buf, 256, inf) != NULL) {
            if (strcmp(buf, "--\n") == 0)
                break;
            if (buf[250] != '\0')
                strcpy(buf + 250, "\n");
            fprintf(outf, "%s", buf);
        }
        fprintf(outf, "\n\n");
        fclose(inf);
    }
    fclose(outf);

    if (dashf(filepath)) {
        if (!nomsg) {
            sprintf(buf, "要附加在旧暂存档之後吗?(Y/N/C) [Y]: ");
            a_prompt(-1, buf, ans);
        }
        /*if( ans[0] == 'N' || ans[0] == 'n' ||nomsg) { */
        /* Leeward 98.04.16: fix bugs */
        if ((ans[0] == 'N' || ans[0] == 'n') && (!nomsg)) {
            /*
               sprintf( genbuf, "/bin/cp -r %s  tmp/bm.%s", filepath, currentuser->userid );
             */
            sprintf(buf, "tmp/bm.%s", currentuser->userid);
            f_cp(filepath, buf, 0);
        } else if (((ans[0] == 'C' || ans[0] == 'c')) && (!nomsg))
            return 1;
        else {
            sprintf(buf, "/bin/cat %s >> tmp/bm.%s", filepath, currentuser->userid);
            system(buf);
        }
    } else {
        /*
           sprintf( genbuf, "/bin/cp -r %s  tmp/bm.%s", filepath , currentuser->userid );
         */
        f_cp(filepath, buf, 0);
    }
    unlink(filepath);
    sprintf(buf, "将 %s 存入暂存档", filepath);
    a_report(buf);
    if (!nomsg) {
        sprintf(buf, " 已将该文章存入暂存档, 请按任何键以继续 << ");
        a_prompt(-1, buf, ans);
    }
    return 1;
}


/* added by netty to handle post saving into (0)Announce */
int a_Save(char *path, char *key, struct fileheader *fileinfo, int nomsg, char *direct, int ent)
{

    char board[STRLEN];
    char ans[STRLEN];
    char buf[256];

    sprintf(board, "tmp/bm.%s", currentuser->userid);
    if (dashf(board)) {
        if (!nomsg) {
            sprintf(buf, "要附加在旧暂存档之后吗?(Y/N/C) [Y]: ");
            a_prompt(-1, buf, ans);
        }
        /*if( ans[0] == 'N' || ans[0] == 'n' ||nomsg) { */
        /* Leeward 97.11.18: fix bugs */
        if ((ans[0] == 'N' || ans[0] == 'n') && (!nomsg)) {
            /*
               sprintf( genbuf, "/bin/cp -r boards/%s/%s  tmp/bm.%s", key , fileinfo->filename , currentuser->userid );
             */
            sprintf(genbuf, "boards/%s/%s", key, fileinfo->filename);
            sprintf(board, "tmp/bm.%s", currentuser->userid);
            f_cp(genbuf, board, 0);
        } else if (((ans[0] == 'C' || ans[0] == 'c')) && (!nomsg))
            return 1;
        else {
            sprintf(genbuf, "/bin/cat boards/%s/%s >> tmp/bm.%s", key, fileinfo->filename, currentuser->userid);
            system(genbuf);
            /*                   sprintf( genbuf, "/bin/cp -r boards/%s/%s  tmp/bm.%s", key , fileinfo->filename , currentuser->userid );
             */
        }
    } else {
        /*
           sprintf( genbuf, "/bin/cp -r boards/%s/%s  tmp/bm.%s", key , fileinfo->filename , currentuser->userid );
         */
        sprintf(buf, "boards/%s/%s", key, fileinfo->filename);
        sprintf(board, "tmp/bm.%s", currentuser->userid);
        f_cp(buf, board, 0);
    }
    sprintf(buf, "将 boards/%s/%s 存入暂存档", key, fileinfo->filename);
    change_post_flag(currBM, currentuser, digestmode, currboard, ent, fileinfo, direct, FILE_IMPORT_FLAG, 0);
    a_report(buf);
    if (!nomsg) {
        sprintf(buf, " 已将该文章存入暂存档, 请按任何键以继续 << ");
        a_prompt(-1, buf, ans);
    }
    return 1;
}

/* added by netty to handle post saving into (0)Announce */
int a_Import(path, key, fileinfo, nomsg, direct, ent)
    char *path, *key;
    struct fileheader *fileinfo;
    int nomsg;
    char *direct;               /* Leeward 98.04.15 */
    int ent;
{

    FILE *fn;
    char fname[STRLEN], bname[PATHLEN];
    char buf[PATHLEN], *ptr;
    int ch;
    MENU pm;
    char ans[STRLEN];
	char Importname[STRLEN];

    modify_user_mode(CSIE_ANNOUNCE);
    if (ann_get_path(key, buf, sizeof(buf)) == 0)
	{
		snprintf(Importname, sizeof(Importname), "%s/%s", path, buf);

		/* Leeward: 97.12.17: 对版主的多个窗口同步丝路 */
		sprintf(buf, "%s", Importname);
		ptr = strstr(buf, ".faq/");
		if (ptr) {
			if ((ptr = strchr(ptr + 5, '/')) != NULL)
				strcpy(ptr + 1, ".BMpath");
			else
				strcat(buf, "/.BMpath");
			fn = fopen(buf, "rt");
			if (fn) {
				fgets(netty_path, 256, fn);
				fclose(fn);
			}
		}

		if (netty_path[0] != '\0') {
			/* 直接加入到精华区内，不用确认 Life */
			pm.path = netty_path;
		}
		else {
			sprintf(buf, "将该文章放进 %s,确定吗?(Y/N) [N]: ", Importname);
			if (!nomsg)
				a_prompt(-1, buf, ans);
			if (ans[0] == 'Y' || ans[0] == 'y' || nomsg) {
				pm.path = Importname;
			} else {
				sprintf(buf, "你改变心意了?? ,请按任何键以结束 << ");
				a_prompt(-1, buf, ans);
				return 1;
			}
		}
		strcpy(pm.mtitle, "");
		a_loadnames(&pm);
		ann_get_postfilename(fname, fileinfo, &pm);
		sprintf(bname, "%s/%s", pm.path, fname);
		sprintf(buf, "%-38.38s %s ", fileinfo->title, currentuser->userid);
		a_additem(&pm, buf, fname, NULL, 0);
		a_savenames(&pm);
		/*
		   sprintf( buf, "/bin/cp -r boards/%s/%s %s", key , fileinfo->filename , bname );
		 */
		sprintf(buf, "boards/%s/%s", key, fileinfo->filename);
		f_cp(buf, bname, 0);

		/* Leeward 98.04.15 */
		sprintf(buf, "将 boards/%s/%s 收入目录 %s", key, fileinfo->filename, pm.path + 17);
		a_report(buf);
		sprintf(buf, " 收入精华区目录 %s, 请按 Enter 继续 << ", /*fileinfo->title, */ pm.path);
		if (!nomsg)
			a_prompt(-1, buf, ans);

		/* Leeward 98.04.15 add below FILE_IMPORTED */
		change_post_flag(currBM, currentuser, digestmode, currboard, ent, fileinfo, direct, FILE_IMPORT_FLAG, 0);
		bmlog(currentuser->userid, currboard, 12, 1);

		for (ch = 0; ch < pm.num; ch++)
			free(pm.item[ch]);
		return 1;
    }
    return 0;
}

int a_menusearch(path, key, level)
    char *path, *key;
    int level;
{
    FILE *fn;
    char bname[STRLEN], bpath[STRLEN];
    char buf[PATHLEN], *ptr;
    int len;
    struct stat st;
    struct boardheader fhdr;

    if (key == NULL) {
        key = bname;
        a_prompt(-1, "输入欲搜寻之讨论区名称: ", key);
    }

    setbpath(bpath, key);
    if ((*key == '\0') || (stat(bpath, &st) == -1))     /* 判断board是否存在 */
        return 0;
    if (!(st.st_mode & S_IFDIR))
        return 0;
    if (getboardnum(key, &fhdr) == 0)
        return 0;


    if (!((fhdr.level & PERM_POSTMASK) || HAS_PERM(currentuser, fhdr.level)
          || (fhdr.level & PERM_NOZAP))) {
        return 0;
    }


    len = strlen(key);
    sprintf(buf, "%s/.Search", path);
    if (len > 0 && (fn = fopen(buf, "r")) != NULL) {
        while (fgets(buf, sizeof(buf), fn) != NULL) {
            if (strncmp(buf, key, len) == 0 && buf[len] == ':' && (ptr = strtok(&buf[len + 1], " \t\n")) != NULL) {
                sprintf(bname, "%s/%s", path, ptr);
                fclose(fn);
                a_menu("", bname, level, 0);
                return 1;
            }
        }
        /*---	added by period	2000-09-21	---*/
        fclose(fn);
        /*---	---*/
    }
    return 0;
}

void a_forward(path, pitem, mode)
    char *path;
    ITEM *pitem;
    int mode;
{
    struct fileheader fhdr;
    char fname[PATHLEN], *mesg;

    sprintf(fname, "%s/%s", path, pitem->fname);
    if (dashf(fname)) {
        strncpy(fhdr.title, pitem->title, STRLEN);
        strncpy(fhdr.filename, pitem->fname, FILENAME_LEN);
        switch (doforward(path, &fhdr, mode)) {
        case 0:
            mesg = "文章转寄完成!\n";
            break;
        case -1:
            mesg = "system error!!.\n";
            break;
        case -2:
            mesg = "invalid address.\n";
            break;
        case -552:
            prints
                ("\n[1m[33m信件超长（本站限定信件长度上限为 %d 字节），取消转寄操作[m[m\n\n请告知收信人（也许就是您自己吧:PP）：\n\n*1* 使用 [1m[33mWWW[m[m 方式访问本站，随时可以保存任意长度的文章到自己的计算机；\n*2* 使用 [1m[33mpop3[m[m 方式从本站用户的信箱取信，没有任何长度限制。\n*3* 如果不熟悉本站的 WWW 或 pop3 服务，请阅读 [1m[33mAnnounce[m 版有关公告。\n",
                 MAXMAILSIZE);
            break;
        default:
            mesg = "取消转寄动作.\n";
        }
        prints(mesg);
    } else {
        move(t_lines - 1, 0);
        prints("无法转寄此项目.\n");
    }
    pressanykey();
}

void a_newitem(pm, mode)        /* 用户创建新的 ITEM */
    MENU *pm;
    int mode;
{
    char uident[STRLEN];
    char board[STRLEN], title[STRLEN];
    char fname[STRLEN], fpath[PATHLEN], fpath2[PATHLEN];
    char *mesg;
    FILE *pn;
    char ans[STRLEN];
    char buf[255];

    pm->page = 9999;
    switch (mode) {
    case ADDITEM:
        mesg = "请输入新文件之英名名称(可含数字)：";
        break;
    case ADDGROUP:
        mesg = "请输入新目录之英名名称(可含数字)：";
        break;
    case ADDMAIL:
        sprintf(board, "tmp/bm.%s", currentuser->userid);
        if (!dashf(board)) {
            sprintf(buf, "哎呀!! 请先至该版(讨论区)将文章存入暂存档! << ");
            a_prompt(-1, buf, ans);
            return;
        }
        mesg = "请输入文件之英文名称(可含数字)：";
        break;
    }
    a_prompt(-2, mesg, fname);
    if (*fname == '\0')
        return;
    sprintf(fpath, "%s/%s", pm->path, fname);
    if (!valid_fname(fname)) {
        sprintf(buf, "哎呀!! 名称只能包含英文及数字! << ");
        a_prompt(-1, buf, ans);
    } else if (dashf(fpath) || dashd(fpath)) {
        sprintf(buf, "哎呀!! 系统内已经有 %s 这个文件存在了! << ", fname);
        a_prompt(-1, buf, ans);
    } else {
        mesg = "请输入文件或目录之中文名称 <<  ";
        a_prompt(-1, mesg, title);
        if (*title == '\0')
            return;
        sprintf(buf, "创建新文件或目录 %s (标题: %s)", fpath + 17, title);
        a_report(buf);
        switch (mode) {
        case ADDITEM:
            /*vedit( fpath, 0 ); */
            if (-1 == vedit(fpath, 0))
                return;         /* Leeward 98.06.12 fixes bug */
            chmod(fpath, 0644);
            break;
        case ADDGROUP:
            mkdir(fpath, 0755);
            chmod(fpath, 0755);
            break;
        case ADDMAIL:
            /*
               sprintf( genbuf, "mv -f %s %s",board, fpath );
             */
            f_mv(board, fpath);
            break;
        }
        if (mode != ADDGROUP)
            sprintf(buf, "%-38.38s %s ", title, currentuser->userid);
        else {
            /*Add by SmallPig */
            if (HAS_PERM(currentuser, PERM_SYSOP || HAS_PERM(currentuser, PERM_ANNOUNCE))) {
                move(1, 0);
                clrtoeol();
                /*$$$$$$$$ Multi-BM Input, Modified By Excellent $$$$$$$ */
                getdata(1, 0, "版主: ", uident, STRLEN - 1, DOECHO, NULL, true);
                if (uident[0] != '\0')
                    sprintf(buf, "%-38.38s(BM: %s)", title, uident);
                else
                    sprintf(buf, "%-38.38s", title);
            } else
                sprintf(buf, "%-38.38s", title);
        }
        a_additem(pm, buf, fname, NULL, 0);
        a_savenames(pm);
        if (mode == ADDGROUP) {
            sprintf(fpath2, "%s/%s/.Names", pm->path, fname);
            if ((pn = fopen(fpath2, "w")) != NULL) {
                fprintf(pn, "#\n");
                fprintf(pn, "# Title=%s\n", buf);
                fprintf(pn, "#\n");
                fclose(pn);
            }
        }
        if(mode == ADDMAIL)
            bmlog(currentuser->userid, currboard, 12, 1);
        else
            bmlog(currentuser->userid, currboard, 13, 1);
    }
}

void a_moveitem(pm)             /*改变 ITEM 次序 */
    MENU *pm;
{
    ITEM *tmp;
    char newnum[STRLEN];
    int num, n, temp;

    sprintf(genbuf, "请输入第 %d 项的新次序: ", pm->now + 1);
    temp = pm->now + 1;
    a_prompt(-2, genbuf, newnum);
    num = (newnum[0] == '$') ? 9999 : atoi(newnum) - 1;
    if (num >= pm->num)
        num = pm->num - 1;
    else if (num < 0)
        return;
    tmp = pm->item[pm->now];
    if (num > pm->now) {
        for (n = pm->now; n < num; n++)
            pm->item[n] = pm->item[n + 1];
    } else {
        for (n = pm->now; n > num; n--)
            pm->item[n] = pm->item[n - 1];
    }
    pm->item[num] = tmp;
    pm->now = num;
    a_savenames(pm);
    sprintf(genbuf, "改变 %s 下第 %d 项的次序到第 %d 项", pm->path + 17, temp, pm->now + 1);
    bmlog(currentuser->userid, currboard, 13, 1);
    a_report(genbuf);
}

void a_copypaste(pm, paste)
    MENU *pm;
    int paste;
{
    /* KCN 2002.03.22,下面变量的static应该可以去掉 */
    char title[STRLEN], filename[STRLEN], fpath[PATHLEN];
    ITEM *item;
    char ans[STRLEN], newpath[PATHLEN];
    FILE *fn;                   /* Leeward 98.02.19 */

    move(t_lines - 1, 0);
    if (!paste) {
        item = pm->item[pm->now];
        strncpy(title, item->title, STRLEN);
        strncpy(filename, item->fname, FILENAME_LEN);
        sprintf(genbuf, "%s/%s", pm->path, filename);
        strncpy(fpath, genbuf, PATHLEN);
        prints("拷贝标识完成。注意：粘贴文章後才能用 d 命令将文章删除! -- 请按任意键继续 << ");
        pressanykey();

        /* Leeward: 98.02.19: 对版主的多个窗口同步 C/P 操作 */
        sprintf(genbuf, "home/%c/%s/.CP", toupper(currentuser->userid[0]), currentuser->userid);
        fn = fopen(genbuf, "wt");
        if (fn) {
            fputs(title, fn);
            fputs("\n", fn);
            fputs(filename, fn);
            fputs("\n", fn);
            fputs(fpath, fn);
            fputs("\n", fn);
            fclose(fn);
        } else {
            prints("File open ERROR -- please report SYSOP");
            pressanykey();
        }
    } else {
        /* Leeward: 98.02.19: 对版主的多个窗口同步 C/P 操作 */
        sprintf(genbuf, "home/%c/%s/.CP", toupper(currentuser->userid[0]), currentuser->userid);
        fn = fopen(genbuf, "rt");
        if (fn) {
            fgets(title, STRLEN, fn);
            if ('\n' == title[strlen(title) - 1])
                title[strlen(title) - 1] = 0;
            fgets(filename, STRLEN, fn);
            if ('\n' == filename[strlen(filename) - 1])
                filename[strlen(filename) - 1] = 0;
            fgets(fpath, /*STRLEN*/ PATHLEN, fn);       /* Leeward 98.04.15 */
            if ('\n' == fpath[strlen(fpath) - 1])
                fpath[strlen(fpath) - 1] = 0;
            fclose(fn);
        }

        sprintf(newpath, "%s/%s", pm->path, filename);
        if (*title == '\0' || *filename == '\0') {
            prints("请先使用 copy 命令再使用 paste 命令. ");
            pressanykey();
        } else if (dashf(newpath) || dashd(newpath)) {
            prints("%s %s 已经存在. ", (dashd(newpath) ? "目录" : "文件"), filename);
            pressanykey();
        } else if (strstr(newpath, fpath) != NULL) {
            prints("无法将一个目录搬进自己的子目录中, 会造成死循环. ");
            pressanykey();
        } else {
            /* modified by cityhunter to simplify annouce c/p */
            sprintf(genbuf, "您确定要粘贴%s %s 吗? (C/L/N)C为复制方式 L为链接方式 [N]: ", (dashd(fpath) ? "目录" : "文件"), filename);
            a_prompt(-2, genbuf, ans);
            if (ans[0] == 'C' || ans[0] == 'c') {
                char buf[256];

                if (dashd(fpath)) {     /* 是目录 */
                    sprintf(genbuf, "/bin/cp -rp %s %s", fpath, newpath);
                    system(genbuf);
                } else {        /* 是文件 
                                   sprintf( genbuf, "/bin/cp -p %s %s", fpath, newpath ); */
                    f_cp(fpath, newpath, 0);
                }
                a_additem(pm, title, filename, NULL, 0);
                a_savenames(pm);
                sprintf(buf, "复制精华区文件或目录: %s", genbuf);
                a_report(buf);
            } else if (ans[0] == 'L' || ans[0] == 'l') {
                char buf[256];

                if (dashd(fpath)) {     /* 是目录 */
                    sprintf(genbuf, "/bin/cp -rp %s %s", fpath, newpath);
                    system(genbuf);
                } else {        /* 是文件 
                                   sprintf( genbuf, "/bin/ln %s %s", fpath, newpath ); */
                    f_ln(fpath, newpath);
                }
                a_additem(pm, title, filename, NULL, 0);
                a_savenames(pm);
                sprintf(buf, "复制精华区文件或目录: %s", genbuf);
                a_report(buf);
            }
        }
        /*            sprintf( genbuf, "您确定要粘贴%s %s 吗? (Y/N) [N]: ", (dashd(fpath) ? "目录" : "文件"), filename);
           a_prompt( -2, genbuf, ans );
           if( ans[0] == 'Y' || ans[0] == 'y' ) {
           if (dashd(fpath))
           { 
           sprintf( genbuf, "/bin/cp -rp %s %s", fpath, newpath );
           }
           else
           { 
           sprintf( genbuf, "使用链接方式(L)还是复制方式(C)？前者能大大节省磁盘空间 (L/C) [L]: ", filename ); 
           a_prompt( -2, genbuf, ans );
           if( ans[0] == 'C' || ans[0] == 'c' ) 
           sprintf( genbuf, "/bin/cp -p %s %s", fpath, newpath );
           else
           sprintf( genbuf, "/bin/ln %s %s", fpath, newpath );
           }
           system( genbuf );
           a_additem( pm, title, filename  ,NULL,0);
           a_savenames( pm );
           sprintf(genbuf,"复制精华区文件或目录: %s",genbuf);
           a_report(genbuf);
           }
           }
         */
    }
    pm->page = 9999;
}

void a_delete(pm)
    MENU *pm;
{
    ITEM *item;
    char fpath[PATHLEN];
    char ans[STRLEN];
    int n;

    item = pm->item[pm->now];
    move(t_lines - 2, 0);
    prints("%5d  %-50s\n", pm->now + 1, item->title);
    sprintf(fpath, "%s/%s", pm->path, item->fname);
    if (dashf(fpath)) {
        a_prompt(-1, "删除此文件, 确定吗?(Y/N) [N]：", ans);
        if (ans[0] != 'Y' && ans[0] != 'y')
            return;
        unlink(fpath);
    } else if (dashd(fpath)) {
        a_prompt(-1, "删除整个子目录, 别开玩笑哦, 确定吗?(Y/N) [N]: ", ans);
        if (ans[0] != 'Y' && ans[0] != 'y')
            return;
        /*
           sprintf( genbuf, "/bin/rm -rf %s", fpath ); */
        f_rm(fpath);
    }
    free(item);
    (pm->num)--;
    for (n = pm->now; n < pm->num; n++)
        pm->item[n] = pm->item[n + 1];
    a_savenames(pm);
    sprintf(genbuf, "删除文件或目录: %s", fpath + 17);
    bmlog(currentuser->userid, currboard, 13, 1);
    a_report(genbuf);
}

void a_newname(pm)
    MENU *pm;
{
    ITEM *item;
    char fname[STRLEN];
    char fpath[PATHLEN];
    char *mesg;

    item = pm->item[pm->now];
    a_prompt(-2, "新文件名: ", fname);
    if (*fname == '\0')
        return;
    sprintf(fpath, "%s/%s", pm->path, fname);
    if (!valid_fname(fname)) {
        mesg = "不合法文件名称.";
    } else if (dashf(fpath) || dashd(fpath)) {
        mesg = "系统中已有此文件存在了.";
    } else {
        sprintf(genbuf, "%s/%s", pm->path, item->fname);
        if (f_mv(genbuf, fpath) == 0) {
            char r_buf[256];

            strcpy(item->fname, fname);
            sprintf(r_buf, "更改文件名: %s -> %s", genbuf + 17, fpath + 17);
            a_report(r_buf);
            a_savenames(pm);
            return;
        }
        mesg = "文件名更改失败 !!";
    }
    prints(mesg);
    pressanykey();
}

void a_manager(pm, ch)
    MENU *pm;
    int ch;
{
    char uident[STRLEN];
    ITEM *item;
    char fpath[PATHLEN], changed_T[STRLEN], ans[STRLEN];

    if (pm->num > 0) {
        item = pm->item[pm->now];
        sprintf(fpath, "%s/%s", pm->path, item->fname);
    }
    switch (ch) {
    case 'a':
        a_newitem(pm, ADDITEM);
        break;
    case 'g':
        a_newitem(pm, ADDGROUP);
        break;
    case 'i':
        a_newitem(pm, ADDMAIL);
        break;
        /*case 'G':  a_newitem( pm, ADDGOPHER );    break; */
    case 'p':
        a_copypaste(pm, 1);
        break;
    case 'f':
        pm->page = 9999;
        sprintf(genbuf, "路径为 %s, 要设为当前路径吗?(Y/N) [Y]: ", pm->path);
        a_prompt(-1, genbuf, ans);
        /*if( ans[0] == 'Y' || ans[0] == 'y' ) { */
        if (ans[0] != 'N' && ans[0] != 'n') {
            strcpy(netty_path, pm->path);
            /*sprintf( genbuf, "已将该路径设为当前路径, 请按任何键以继续 <<" );
               a_prompt( -1, genbuf, ans );Leeward 98.04.15 */
            {                   /* Leeward: 97.12.17: 对版主的多个窗口同步丝路 */
                FILE *sl;
                char *ptr;

                /* by zixia: 用相对路经 sprintf(genbuf, "%s/%s", BBSHOME,netty_path); */
                sprintf(genbuf, "%s", netty_path);
                ptr = strstr(genbuf, ".faq/");
                if (ptr) {
                    if ((ptr = strchr(ptr + 5, '/')) != NULL)
                        strcpy(ptr + 1, ".BMpath");
                    else
                        strcat(genbuf, "/.BMpath");
                    sl = fopen(genbuf, "wt");
                    if (sl) {
                        fputs(netty_path, sl);
                        fclose(sl);
                    }
                }
            }                   /* End if  # Leeward */
        }
        break;
    }
    if (pm->num > 0)
        switch (ch) {
        case 's':
            if (++a_fmode >= 3)
                a_fmode = 1;
            pm->page = 9999;
            break;
        case 'm':
            a_moveitem(pm);
            pm->page = 9999;
            break;
        case 'd':
            a_delete(pm);
            pm->page = 9999;
            break;
        case 'V':
        case 'v':
            if (HAS_PERM(currentuser, PERM_SYSOP)) {
                if (ch == 'v')
                    sprintf(fpath, "%s/.Names", pm->path);
                else
                    sprintf(fpath, "0Announce/.Search");        /*.Search控制各版对应的精华区 */

                if (dashf(fpath)) {
                    modify_user_mode(EDITANN);
                    vedit(fpath, 0);
                    modify_user_mode(CSIE_ANNOUNCE);
                }
                pm->page = 9999;
            }
            break;
        case 't':
	    strncpy(changed_T,item->title,39);
	    changed_T[39]=0;
	    {
                char*p;
                p=changed_T+strlen(changed_T)-1;
                for (;p>=changed_T;p--) {
		    if (*p==' ') *p=0;
		    else break;
		};
            }
            a_prompt2(-2, "新标题: ", changed_T);
            /* modified by netty to properly handle title change,add bm by SmallPig */
            if (*changed_T) {
                if (dashf(fpath)) {
                    sprintf(genbuf, "%-38.38s %s ", changed_T, currentuser->userid);
                    strcpy(item->title, genbuf);
                    sprintf(genbuf, "改变文件 %s 的标题", fpath + 17);
                    a_report(genbuf);
                } else if (dashd(fpath)) {
                    if (HAS_PERM(currentuser, PERM_SYSOP || HAS_PERM(currentuser, PERM_ANNOUNCE))) {
                        move(1, 0);
                        clrtoeol();
                        /*usercomplete("版主: ",uident) ; */
                        /*$$$$$$$$ Multi-BM Input, Modified By Excellent $$$$$$$ */
                        getdata(1, 0, "版主: ", uident, STRLEN - 1, DOECHO, NULL, true);
                        if (uident[0] != '\0')
                            sprintf(genbuf, "%-38.38s(BM: %s)", changed_T, uident);
                        else
                            sprintf(genbuf, "%-38.38s", changed_T);
                    } else
                        sprintf(genbuf, "%-38.38s", changed_T);

                    strcpy(item->title, genbuf);
                    sprintf(genbuf, "改变目录 %s 的标题", fpath + 17);
                    a_report(genbuf);
                }
                a_savenames(pm);
            }
            pm->page = 9999;
            break;
        case 'e':
            if (dashf(fpath)) {
                modify_user_mode(EDITANN);
                vedit(fpath, 0);
                modify_user_mode(CSIE_ANNOUNCE);
                sprintf(genbuf, "修改文章 %s 的内容", pm->path + 17);
                a_report(genbuf);
            }
            pm->page = 9999;
            break;
        case 'n':
            a_newname(pm);
            pm->page = 9999;
            break;
        case 'c':
            a_copypaste(pm, 0);
            break;
/*  do not support thread read in announce: COMMAN 2002.7
        case '=':  t_search_down();     break;
        case '+':  t_search_up();       break;
*/
        }
}

void a_menu(maintitle, path, lastlevel, lastbmonly)
    char *maintitle, *path;
    int lastlevel, lastbmonly;
{
    MENU me;
    char fname[PATHLEN], tmp[STRLEN];
    int ch;
    char *bmstr;
    char buf[STRLEN];
    int bmonly;
    int number = 0;

    modify_user_mode(CSIE_ANNOUNCE);
    me.path = path;
    strcpy(me.mtitle, maintitle);
    me.level = lastlevel;
    bmonly = lastbmonly;
    a_loadnames(&me);           /* Load .Names */

    strcpy(buf, me.mtitle);
    bmstr = strstr(buf, "(BM:");
    if (bmstr != NULL) {
        if (chk_currBM(bmstr + 4, currentuser) || HAS_PERM(currentuser, PERM_SYSOP))
            me.level |= PERM_BOARDS;
        else if (bmonly == 1 && !(me.level & PERM_BOARDS))
            return;
    }

    if (strstr(me.mtitle, "(BM: BMS)") || strstr(me.mtitle, "(BM: SECRET)") || strstr(me.mtitle, "(BM: SYSOPS)"))
        bmonly = 1;

    strcpy(buf, me.mtitle);
    bmstr = strstr(buf, "(BM:");

    me.page = 9999;
    me.now = 0;
    while (1) {
        if (me.now >= me.num && me.num > 0) {
            me.now = me.num - 1;
        } else if (me.now < 0) {
            me.now = 0;
        }
        if (me.now < me.page || me.now >= me.page + A_PAGESIZE) {
            me.page = me.now - (me.now % A_PAGESIZE);
            a_showmenu(&me);
        }
        move(3 + me.now - me.page, 0);
        prints("->");
        ch = igetkey();
        move(3 + me.now - me.page, 0);
        prints("  ");
        if (ch == 'Q' || ch == 'q' || ch == KEY_LEFT || ch == EOF)
            break;
      EXPRESS:                 /* Leeward 98.09.13 */
        switch (ch) {
        case KEY_REFRESH:
            a_showmenu(&me);
            break;
        case Ctrl('Z'):
            r_lastmsg();        /* Leeward 98.07.30 support msgX */
            break;
        case KEY_UP:
        case 'K':
        case 'k':
            if (--me.now < 0)
                me.now = me.num - 1;
            break;
        case KEY_DOWN:
        case 'J':
        case 'j':
            if (++me.now >= me.num)
                me.now = 0;
            break;
        case KEY_PGUP:
        case Ctrl('B'):
            if (me.now >= A_PAGESIZE)
                me.now -= A_PAGESIZE;
            else if (me.now > 0)
                me.now = 0;
            else
                me.now = me.num - 1;
            break;
        case KEY_PGDN:
        case Ctrl('F'):
        case ' ':
            if (me.now < me.num - A_PAGESIZE)
                me.now += A_PAGESIZE;
            else if (me.now < me.num - 1)
                me.now = me.num - 1;
            else
                me.now = 0;
            break;
        case Ctrl('C'):
        case Ctrl('P'):
            if (!HAS_PERM(currentuser, PERM_POST))
                break;
            if (!me.item[me.now])
                break;
            sprintf(fname, "%s/%s", path, me.item[me.now]->fname);
            if (!dashf(fname))
                break;
            if (me.now < me.num) {
                char bname[30];

                clear();
		move(1,0);
                if (get_a_boardname(bname, "请输入要转贴的讨论区名称: ")) {
                    move(1, 0);
                    clrtoeol();
                    strcpy(tmp, currboard);
                    strcpy(currboard, bname);
                    if (deny_me(currentuser->userid, currboard)) {
                        prints("对不起，你在 %s 版被停止发表文章的权力", bname);
                        pressreturn();
                        strcpy(currboard, tmp);
                        me.page = 9999;
                        break;
                    }
                    if (!haspostperm(currentuser, currboard)) {
                        move(1, 0);
                        prints("您尚无权限在 %s 发表文章.\n", currboard);
                        prints("如果您尚未注册，请在个人工具箱内详细注册身份\n");
                        prints("未通过身份注册认证的用户，没有发表文章的权限。\n");
                        prints("谢谢合作！ :-) \n");
                        pressreturn();
                        strcpy(currboard, tmp);
                        me.page = 9999;
                        break;
                    }
                    if (check_readonly(currboard)) {
                        strcpy(currboard, tmp);
                        me.page = 9999;
                        break;
                    }
                    strcpy(currboard, tmp);
                    sprintf(tmp, "你确定要转贴到 %s 版吗", bname);
                    if (askyn(tmp, 0) == 1) {
                        post_file(currentuser, "", fname, bname, me.item[me.now]->title, 0, 2);
                        move(2, 0);
                        sprintf(tmp, "[1m已经帮你转贴至 %s 版了[m", bname);
                        prints(tmp);
                        refresh();
                        sleep(1);
                    }
                }
                me.page = 9999;
            }
            show_message(NULL);
            break;
        case 'h':
            show_help("help/announcereadhelp");
            me.page = 9999;
            break;
        case '\n':
        case '\r':
            if (number > 0) {
                me.now = number - 1;
                number = 0;
                continue;
            }
        case 'R':
        case 'r':
        case KEY_RIGHT:
            if (me.now < me.num) {
                if (me.item[me.now]->host != NULL) {
                    /* gopher(me.item[ me.now ]->host,me.item[ me.now ]->fname,
                       me.item[ me.now ]->port,me.item[ me.now ]->title); */
                    me.page = 9999;
                    break;
                } else
                    sprintf(fname, "%s/%s", path, me.item[me.now]->fname);
                if (dashf(fname)) {
                    /*ansimore( fname, true ); */
                    /* Leeward 98.09.13 新添功能∶
                       ，用上／下箭头直接跳转到前／后一项 */
                    ansimore_withzmodem(fname, false, me.item[me.now]->title);
                    prints("[1m[44m[31m[阅读精华区资料]  [33m结束 Q,← │ 上一项资料 U,↑│ 下一项资料 <Enter>,<Space>,↓ [m");
                    switch (ch = igetkey()) {
                    case KEY_DOWN:
                    case ' ':
                    case '\n':
                        if (++me.now >= me.num)
                            me.now = 0;
                        ch = KEY_RIGHT;
                        goto EXPRESS;
                    case KEY_UP:
                        if (--me.now < 0)
                            me.now = me.num - 1;
                        ch = KEY_RIGHT;
                        goto EXPRESS;
                    case Ctrl('Y'):
                        zsend_file(fname, me.item[me.now]->title);
                        break;
                    case Ctrl('Z'):
                    case 'h':
                        goto EXPRESS;
                    default:
                        break;
                    }
                } else if (dashd(fname)) {
                    a_menu(me.item[me.now]->title, fname, me.level, bmonly);
                }
                me.page = 9999;
            }
            break;
        case '/':
            if (a_menusearch(path, NULL, me.level))
                me.page = 9999;
            break;
        case 'F':
        case 'U':
            if (me.now < me.num && HAS_PERM(currentuser, PERM_BASIC)) {
                a_forward(path, me.item[me.now], ch == 'U');
                me.page = 9999;
            }
            break;
        case 'o':
            t_friends();
            me.page = 9999;
            break;              /*Haohmaru 98.09.22 */
        case 'u':
            clear();
            modify_user_mode(QUERY);
            t_query(NULL);
            modify_user_mode(CSIE_ANNOUNCE);
            me.page = 9999;
            break;              /*Haohmaru.99.11.29 */
        case '!':
            Goodbye();
            me.page = 9999;
            break;              /*Haohmaru 98.09.24 */
            /*
               case 'Z':
               if( me.now < me.num && HAS_PERM(currentuser, PERM_BASIC ) ) {
               sprintf( fname, "%s/%s", path, me.item[ me.now ]->fname );
               a_download( fname );
               me.page = 9999;
               }
               break;
             */
        case Ctrl('Y'):
            if (me.now < me.num) {
                if (me.item[me.now]->host != NULL) {
                    /* gopher(me.item[ me.now ]->host,me.item[ me.now ]->fname,
                       me.item[ me.now ]->port,me.item[ me.now ]->title); */
                    me.page = 9999;
                    break;
                } else
                    sprintf(fname, "%s/%s", path, me.item[me.now]->fname);
                if (dashf(fname)) {
                    zsend_file(fname, me.item[me.now]->title);
                    me.page = 9999;
                }
            }
            break;
        }
        if (ch >= '0' && ch <= '9') {
            number = number * 10 + (ch - '0');
            ch = '\0';
        } else {
            number = 0;
        }
        if (me.level & PERM_BOARDS)
            a_manager(&me, ch);
    }
    for (ch = 0; ch < me.num; ch++)
        free(me.item[ch]);
}

int linkto(char *path, char *fname, char *title)
{
    MENU pm;

    pm.path = path;

    strcpy(pm.mtitle, title);
    a_loadnames(&pm);
    a_additem(&pm, title, fname, NULL, 0);
    a_savenames(&pm);
    return 0;
}

/*
 * 参数说明:
 *     group    groups[]中的某项
 *     bname    版的英文名
 *     title    版精华区的中文名
 *     gname    与 group 对应的中文名
 */
int add_grp(char group[STRLEN], char bname[STRLEN], char title[STRLEN], char gname[STRLEN])
        /* 精华区 加 目录 */
{
    FILE *fn;
    char buf[PATHLEN];
    char searchname[STRLEN];
    char gpath[STRLEN * 2];
    char bpath[STRLEN * 2];

    sprintf(buf, "0Announce/.Search");
    sprintf(searchname, "%s: groups/%s/%s", bname, group, bname);
    sprintf(gpath, "0Announce/groups/%s", group);
    sprintf(bpath, "%s/%s", gpath, bname);
    if (!dashd("0Announce")) {
        mkdir("0Announce", 0755);
        chmod("0Announce", 0755);
        if ((fn = fopen("0Announce/.Names", "w")) == NULL)
            return -1;
        fprintf(fn, "#\n");
        fprintf(fn, "# Title=%s 精华区公布栏\n", BBS_FULL_NAME);
        fprintf(fn, "#\n");
        fclose(fn);
    }
    if (!dashd("0Announce/groups")) {
        mkdir("0Announce/groups", 0755);
        chmod("0Announce/groups", 0755);

        linkto("0Announce", "groups", "讨论区精华");
    }
    if (!dashd(gpath)) {
        mkdir(gpath, 0755);
        chmod(gpath, 0755);
        linkto("0Announce/groups", group, gname);
    }
    if (!dashd(bpath)) {
        mkdir(bpath, 0755);
        chmod(bpath, 0755);
        linkto(gpath, bname, title);
        sprintf(buf, "%s/.Names", bpath);
        if ((fn = fopen(buf, "w")) == NULL) {
            return -1;
        }
        fprintf(fn, "#\n");
        fprintf(fn, "# Title=%s\n", title);
        fprintf(fn, "#\n");
        fclose(fn);
    }
    return 0;
}

int del_grp(bname, title)
    char bname[STRLEN], title[STRLEN];
{
    char buf2[STRLEN];
    char gpath[STRLEN * 2];
    char bpath[STRLEN * 2];
    char check[30];
	char *ptr;
    int i, n;
    MENU pm;

	/* 获取该版在精华区中的路径 */
	if (ann_get_path(bname, gpath, sizeof(gpath)) < 0)
		return 0;
	snprintf(bpath, sizeof(bpath), "0Announce/%s", gpath);
	strcpy(gpath, bpath);
	/* 获取该版对应 group 的路径 */
	if ((ptr = strrchr(gpath, '/')) == NULL)
		return 0;
	if (strncmp(bname, ptr+1, strlen(bname)) != 0)
		return 0;
	*ptr = '\0';

    f_rm(bpath);

    pm.path = gpath;
    a_loadnames(&pm);
    for (i = 0; i < pm.num; i++) {
        strcpy(buf2, pm.item[i]->title);
        strcpy(check, strtok(pm.item[i]->fname, "/~\n\b"));
        if (strstr(buf2, title) && !strcmp(check, bname)) {
            free(pm.item[i]);
            (pm.num)--;
            for (n = i; n < pm.num; n++)
                pm.item[n] = pm.item[n + 1];
            a_savenames(&pm);
            break;
        }
    }
    return 0;  /* FIXME: return value */
}

int edit_grp(char bname[STRLEN], char title[STRLEN], char newtitle[100])
{
    char buf2[STRLEN];
    char gpath[STRLEN * 2];
    char bpath[STRLEN * 2];
	char *ptr;
    int i;
    MENU pm;

	/* 获取该版在精华区中的路径 */
	if (ann_get_path(bname, gpath, sizeof(gpath)) < 0)
		return 0;
	snprintf(bpath, sizeof(bpath), "0Announce/%s", gpath);
	strcpy(gpath, bpath);
	/* 获取该版对应 group 的路径 */
	if ((ptr = strrchr(gpath, '/')) == NULL)
		return 0;
	if (strncmp(bname, ptr+1, strlen(bname)) != 0)
		return 0;
	*ptr = '\0';

    pm.path = gpath;
    a_loadnames(&pm);
    for (i = 0; i < pm.num; i++) {
        strcpy(buf2, pm.item[i]->title);
        if (strstr(buf2, title) && strstr(pm.item[i]->fname, bname)) {
            strcpy(pm.item[i]->title, newtitle);
            break;
        }
    }
    a_savenames(&pm);
    pm.path = bpath;
    a_loadnames(&pm);
    strcpy(pm.mtitle, newtitle);
    a_savenames(&pm);

    return 0;  /* FIXME: return value */
}

void Announce()
{
    sprintf(genbuf, "%s 精华区公布栏", BBS_FULL_NAME);
    a_menu(genbuf, "0Announce", HAS_PERM(currentuser, PERM_ANNOUNCE) ? PERM_BOARDS : 0, 0);
    clear();
}
