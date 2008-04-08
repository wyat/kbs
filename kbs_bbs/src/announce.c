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

int ann_quick_view = 0;
#define A_PAGESIZE      ((ann_quick_view)?((t_lines - 5)/2):(t_lines - 5))

#define ADDITEM         0
#define ADDGROUP        1
#define ADDMAIL         2
#define ADDGOPHER       3

int bmonly = 0;
int a_fmode = 1;
int a_fmode_show = 1;
void a_menu();
void a_report();                /*Haohmaru.99.12.06.版主精华区操作记录，作为考查工作的依据 */

extern int draw_content(char *fn, struct fileheader *fh);

int t_search_down();
int t_search_up();

static char *import_path[ANNPATH_NUM];  /*多丝路 */
static char *import_title[ANNPATH_NUM];
static int import_path_select = 0;
static time_t import_path_time = 0;

void a_prompt(bot, pmt, buf)    /* 精华区状态下 输入 */
int bot;
char *pmt, *buf;
{
    move(t_lines + bot, 0);
    clrtoeol();
    getdata(t_lines + bot, 0, pmt, buf, 39, DOECHO, NULL, true);
}

void a_prompt2(int bot, char *pmt, char *buf)
{                               /* 精华区状态下 输入 ,包含原来的内容 */
    move(t_lines + bot, 0);
    clrtoeol();
    getdata(t_lines + bot, 0, pmt, buf, 39, DOECHO, NULL, false);
}

void a_report(s)                /* Haohmaru.99.12.06 */
char *s;
{
    /*
     * disable it because of none using it , KCN,2002.07.31 
     */
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
        sprintf(buf,"%s %s %s\n",getCurrentUser()->userid, timestr, s) ;
        write(fd,buf,strlen(buf)) ;
        flock(fd,LOCK_UN) ;
        close(fd) ;
        return ;
    }
*/
}

typedef struct {
    bool save_mode;             /* in save mode,path need valid */
    bool show_path;
} a_select_path_arg;

static int a_select_path_onselect(struct _select_def *conf)
{
    a_select_path_arg *arg = (a_select_path_arg *) conf->arg;

    if (arg->save_mode) {       /* check valid path */
        if (import_title[conf->pos - 1][0] == 0 || import_path[conf->pos - 1][0] == 0) {
            bell();
            return SHOW_CONTINUE;
        }
    } else {                    /* confirm replace */
        if (import_title[conf->pos - 1][0] != 0 && import_path[conf->pos - 1][0] != 0) {
            char ans[STRLEN];

            a_prompt(-2, "要覆盖已有的丝路么？(Y/N) [N]", ans);
            if (toupper(ans[0]) != 'Y')
                return SHOW_REFRESH;
        }
    }
    return SHOW_SELECT;
}

static int a_select_path_show(struct _select_def *conf, int i)
{
    a_select_path_arg *arg = (a_select_path_arg *) conf->arg;

    if (import_title[i - 1][0] != 0)
        if (arg->show_path)
            prints(" %2d   %s", i, import_path[i - 1]);
        else
            prints(" %2d   %s", i, import_title[i - 1]);
    else
        prints(" %2d   \x1b[32m<尚未设定>\x1b[m", i);
    return SHOW_CONTINUE;
}

static int a_select_path_prekey(struct _select_def *conf, int *key)
{
    switch (*key) {
    case 'e':
    case 'q':
        *key = KEY_LEFT;
        break;
    case 'p':
    case 'k':
        *key = KEY_UP;
        break;
    case ' ':
    case 'N':
        *key = KEY_PGDN;
        break;
    case 'n':
    case 'j':
        *key = KEY_DOWN;
        break;
    }
    return SHOW_CONTINUE;
}

static int a_select_path_key(struct _select_def *conf, int key)
{
    a_select_path_arg *arg = (a_select_path_arg *) conf->arg;
    int oldmode;

    switch (key) {
    case 'h':
    case 'H':
        show_help("help/import_announcehelp");
        return SHOW_REFRESH;
    case 'R':
    case 'r':
        free_import_path(import_path,import_title,&import_path_time);
        load_import_path(import_path,import_title,&import_path_time,&import_path_select, getSession());
        return SHOW_DIRCHANGE;
    case 'a':
    case 'A':
        arg->show_path = !arg->show_path;
        return SHOW_DIRCHANGE;
    case 'T':
    case 't':
        if (import_path[conf->pos - 1][0] != 0) {
            char new_title[STRLEN];

            strncpy(new_title, import_title[conf->pos - 1], STRLEN);
            new_title[STRLEN - 1]=0;
	    getdata(t_lines - 2, 0, "新名称：", new_title, STRLEN - 1, DOECHO, NULL, false);
            if (new_title[0] != 0) {
                free(import_title[conf->pos - 1]);
                new_title[STRLEN - 1] = 0;
                import_title[conf->pos - 1] = (char *) malloc(strlen(new_title) + 1);
                strcpy(import_title[conf->pos - 1], new_title);
        		save_import_path(import_path,import_title,&import_path_time, getSession());
                return SHOW_DIRCHANGE;
            }
            return SHOW_REFRESH;
        }
        break;
    case 'D':
    case 'd':
        if (import_title[conf->pos - 1][0] != 0) {
            char ans[STRLEN];

            a_prompt(-2, "要删除这个丝路？(Y/N)[N]", ans);
            if (toupper(ans[0]) == 'Y') {
                free(import_title[conf->pos - 1]);
                import_title[conf->pos - 1] = (char *) malloc(1);
                import_title[conf->pos - 1][0] = 0;
        		save_import_path(import_path,import_title,&import_path_time, getSession());
                return SHOW_DIRCHANGE;
            }
            return SHOW_REFRESH;
        }
        break;
    case 'M':
    case 'm':
        {
            char ans[STRLEN];

            a_prompt(-2, "输入要移动到的位置：", ans);
            if ((ans[0] != 0) && isdigit(ans[0])) {
                int new_pos;

                new_pos = atoi(ans);
                if ((new_pos >= 1) && (new_pos <= ANNPATH_NUM) && (new_pos != conf->pos)) {
                    char *tmp;

                    tmp = import_title[conf->pos - 1];
                    import_title[conf->pos - 1] = import_title[new_pos - 1];
                    import_title[new_pos - 1] = tmp;
                    tmp = import_path[conf->pos - 1];
                    import_path[conf->pos - 1] = import_path[new_pos - 1];
                    import_path[new_pos - 1] = tmp;
        			save_import_path(import_path,import_title,&import_path_time, getSession());
                    conf->pos = new_pos;
                    return SHOW_DIRCHANGE;
                }
            }
            return SHOW_REFRESH;
        }
        break;
    case Ctrl('Z'):
        oldmode = uinfo.mode;
        r_lastmsg();
        modify_user_mode(oldmode);
        return SHOW_REFRESH;
    case 'l':		/* 原来是大L 改成小的了 统一起见 by pig2532 on 2005-12-1 */
        oldmode = uinfo.mode;
        show_allmsgs();
        modify_user_mode(oldmode);
        return SHOW_REFRESH;
    case 'w':		/* 原来是大W 改成小的了 统一起见 by pig2532 on 2005-12-1 */
        oldmode = uinfo.mode;
        if (!HAS_PERM(getCurrentUser(), PERM_PAGE))
            break;
        s_msg();
        modify_user_mode(oldmode);
        return SHOW_REFRESH;
    case 'u':
        oldmode = uinfo.mode;
        clear();
        modify_user_mode(QUERY);
        t_query(NULL);
        modify_user_mode(oldmode);
        clear();
        return SHOW_REFRESH;
	case 'U':		/* pig2532 2005.12.10 */
		board_query();
        return SHOW_REFRESH;
    }
    return SHOW_CONTINUE;
}

static int a_select_path_refresh(struct _select_def *conf)
{
    clear();
    docmdtitle("[丝路选择菜单]",
               "退出[\x1b[1;32m←\x1b[0;37m,\x1b[1;32me\x1b[0;37m] 进入[\x1b[1;32mEnter\x1b[0;37m] 选择[\x1b[1;32m↑\x1b[0;37m,\x1b[1;32m↓\x1b[0;37m] 切换[\x1b[1;32ma\x1b[0;37m] 改名[\x1b[1;32mT\x1b[0;37m] 删除[\x1b[1;32md\x1b[0;37m]\x1b[m 移动[\x1b[1;32mm\x1b[0;37m]帮助[\x1b[1;32mh\x1b[0;37m]\x1b[m");
    move(2, 0);
    prints("\033[0;1;37;44m %4s   %-64s", "编号", "丝路名");
    clrtoeol();
    update_endline();
    return SHOW_CONTINUE;
}

/* select a announce path
     return the index in import_path
     		1 base,0=error
     author: KCN
     */
static int a_select_path(bool save_mode)
{
    int i;
    struct _select_def pathlist_conf;
    POINT *pts;
    a_select_path_arg arg;

    clear();
    load_import_path(import_path,import_title,&import_path_time,&import_path_select, getSession());
    arg.save_mode = save_mode;
    arg.show_path = false;
    pts = (POINT *) malloc(sizeof(POINT) * ANNPATH_NUM);
    for (i = 0; i < 20; i++) {
        pts[i].x = 2;
        pts[i].y = i + 3;
    }
    bzero(&pathlist_conf, sizeof(struct _select_def));
    pathlist_conf.item_count = ANNPATH_NUM;
    pathlist_conf.item_per_page = 20;
    /*
     * 加上 LF_VSCROLL 才能用 LEFT 键退出 
     */
    pathlist_conf.flag = LF_NUMSEL | LF_VSCROLL | LF_BELL | LF_LOOP | LF_MULTIPAGE;
    pathlist_conf.prompt = "◆";
    pathlist_conf.item_pos = pts;
    pathlist_conf.arg = &arg;
    pathlist_conf.title_pos.x = 0;
    pathlist_conf.title_pos.y = 0;
    pathlist_conf.pos = import_path_select;
    pathlist_conf.page_pos = 1; /* initialize page to the first one */

    pathlist_conf.on_select = a_select_path_onselect;
    pathlist_conf.show_data = a_select_path_show;
    pathlist_conf.pre_key_command = a_select_path_prekey;
    pathlist_conf.key_command = a_select_path_key;
    pathlist_conf.show_title = a_select_path_refresh;
    pathlist_conf.get_data = NULL;

    i = list_select_loop(&pathlist_conf);
    free(pts);
    if (i == SHOW_SELECT)
        return pathlist_conf.pos;
    else
        return 0;
}

void a_showmenu(pm)             /* 精华区 菜单 状态 */
MENU *pm;
{
    struct stat st;
    struct tm *pt;
    char title[MAXPATH], kind[32];
    char fname[STRLEN];
    char ch;
    char buf[MAXPATH], genbuf[MAXPATH];
    time_t mtime;
    int n;
    int chkmailflag = 0;

    clear();
    chkmailflag = chkmail();

    if (chkmailflag == 2) {     /*Haohmaru.99.4.4.对收信也加限制 */
        prints("\033[5m");
        sprintf(genbuf, "[您的信箱超过容量,不能再收信!]");
    } else if (chkmailflag) {
        prints("\033[5m");
        sprintf(genbuf, "[您有信件]");
    } else
        strncpy(genbuf, pm->mtitle, MAXPATH);
    if (strlen(genbuf) <= 80)
        sprintf(buf, "%*s", (int)( (80 - strlen(genbuf)) / 2 ), "");
    else
        strcpy(buf, "");
    prints("\033[44m%s%s%s\033[m\n", buf, genbuf, buf);
    prints("            F 寄回自己的信箱┃↑↓ 移动┃→ <Enter> 读取┃←,q 离开\033[m\n");
#ifdef ANN_COUNT
    prints("\033[44m\033[37m 编号  %-20s\033[32m本目录已被浏览\033[33m%9d\033[32m次\033[37m 整  理           %8s \033[m", "[类别] 标    题", pm->count, a_fmode_show == 2 ? "档案名称" : "编辑日期");
#else
    prints("\033[44m\033[37m 编号  %-45s 整  理           %8s \033[m", "[类别] 标    题", a_fmode_show == 2 ? "档案名称" : "编辑日期");
#endif
    prints("\n");
    if (pm->num == 0)
        prints("      << 目前没有文章 >>\n");
    for (n = pm->page; n < pm->page + BBS_PAGESIZE && n < pm->num; n++) {

        /* etnlegend, 2006.04.29, 天哪这原来都是什么写法... */

        snprintf(title,2*STRLEN,"%s",M_ITEM(pm,n)->title);
        snprintf(fname,STRLEN,"%s",M_ITEM(pm,n)->fname);
        snprintf(genbuf,MAXPATH,"%s/%s",pm->path,fname);

        if(lstat(genbuf,&st)==-1||!(S_ISDIR(st.st_mode)||S_ISREG(st.st_mode)||S_ISLNK(st.st_mode))){
            st.st_mode=0;
            st.st_mtime=time(NULL);
        }

        if(a_fmode_show==2){
            ch=(S_ISDIR(st.st_mode)?'/':' ');
            fname[10]=0;
        }
        else{
            mtime=st.st_mtime;
            pt=localtime(&mtime);
            sprintf(fname,"\033[1;37m%04d.%02d.%02d\033[m",(pt->tm_year+1900),(pt->tm_mon+1),pt->tm_mday);
            ch=' ';
        }

        if(!(M_ITEM(pm,n)->host)){
            switch(st.st_mode&S_IFMT){
                case S_IFDIR:
                    snprintf(kind,32,"%s","[\033[0;37m目录\033[m]");
                    break;
                case S_IFREG:
                    snprintf(kind,32,"%s","[\033[0;36m文件\033[m]");
                    break;
                case S_IFLNK:
                    if(stat(genbuf,&st)==-1||!(S_ISDIR(st.st_mode)||S_ISREG(st.st_mode)))
                        snprintf(kind,32,"%s","[\033[0;32m错误\033[m]");
                    else if(S_ISDIR(st.st_mode))
                        snprintf(kind,32,"%s","[\033[0;33m目录\033[m]");
                    else
                        snprintf(kind,32,"%s","[\033[0;33m文件\033[m]");
                    break;
                default:
                    snprintf(kind,32,"%s","[\033[0;32m错误\033[m]");
                    break;
            }
        }
        else
            snprintf(kind,32,"%s","[\033[0;33m连线\033[m]");

        if (!strncmp(title,"[目录] ",7)||!strncmp(title,"[文件] ",7)||!strncmp(title,"[连线] ",7))
            sprintf(genbuf,"%-s %-55.55s%-s%c",kind,&title[7],fname,ch);
        else
            sprintf(genbuf,"%-s %-55.55s%-s%c",kind,title,fname,ch);

        snprintf(title,2*STRLEN,"%s",genbuf);
        sprintf(genbuf,"  %3d %c%s\n",(n+1),(!(M_ITEM(pm,n)->attachpos)?' ':'@'),title);
        prints("%s",genbuf);

        /* --END--, etnlegend, 2006.04.29, 天哪这都是什么写法... */

    }
    clrtobot();
    move(t_lines - 1, 0);
    prints("%s", (pm->level & PERM_BOARDS) ?
           "\033[31m\033[44m[版  主]  \033[33m说明 h │ 离开 q,← │ 新增文章 a │ 新增目录 g │ 修改档案 e        \033[m" :
           "\033[31m\033[44m[功能键] \033[33m 说明 h │ 离开 q,← │ 移动游标 k,↑,j,↓ │ 读取资料 Rtn,→         \033[m");
}

int a_chkbmfrmpath(path)
char *path;
{
	int objectbid , pathnum;
	char *pathslice , *pathdelim;
    const struct boardheader* objectboard;
	char *savept;
	
#ifdef FB2KPC
	if(!strncmp(FB2KPC,path,strlen(FB2KPC))){
		if(fb2kpc_is_owner(path))
			return 1;
		else
			return 0;
	}
#endif
	pathnum = 0;
	pathdelim = "/";
	strtok_r( path , pathdelim , &savept);
	while( pathnum < 3 )
	{
	    pathslice = strtok_r( NULL , pathdelim , &savept);
	    pathnum ++ ;
	}
	objectbid = getbnum_safe(pathslice, getSession(), 1);
	if (!objectbid) return 0; //不可见版面 ? - atppp 20051118
	objectboard = getboard(objectbid);
	if (chk_currBM(objectboard->BM, getCurrentUser()))
        	return 1;
        else
        	return 0;
}

/* added by netty to handle post saving into (0)Announce */
int a_Import(path, key, fileinfo, nomsg, direct, ent)
char *path, *key;
struct fileheader *fileinfo;
int nomsg; /* fancyrabbit Oct 12 2007, nomsg == true 从不出现提示改为(不出现提示 && 不计 bmlog) */
char *direct;                   /* Leeward 98.04.15 */
int ent;
{

    char fname[STRLEN], bname[PATHLEN];
    char buf[PATHLEN];
    MENU pm;
    char ans[STRLEN];
    char importpath[MAXPATH];
    int ret;
    char newpath[MAXPATH];

    ret = 0;
    modify_user_mode(CSIE_ANNOUNCE);
    if (ann_get_path(key, buf, sizeof(buf)) == 0) {
        int i;

        bzero(&pm, sizeof(pm));
        if ((path == NULL) || (path[0] == 0)) {
            i = a_select_path(true);
            if (i == 0)
                return 1;
            import_path_select = i;
            i--;
            if (import_path[i][0] != '\0') {
                pm.path = import_path[i];
                if (path) {
                    strncpy(path, import_path[i], MAXPATH);
                }
            } else {
                strncpy(importpath, buf, MAXPATH);
                importpath[MAXPATH - 1]=0;
                pm.path = importpath;
            }
        } else
            pm.path = path;
        
        strncpy(newpath,pm.path, MAXPATH);
        newpath[MAXPATH - 1] = '\0';
        if (!HAS_PERM(getCurrentUser(), PERM_SYSOP) ) 
        {
                if(!a_chkbmfrmpath(newpath))
                {
                	sprintf(buf, " a.o... 你已经不能再收录到这里了... ... ");
                	a_prompt(-1, buf, ans);
                	return 3;
				}
		}
         /*
         * if (!nomsg) {
         * sprintf(buf, "将该文章放进 %s,确定吗?(Y/N) [N]: ", pm.path);
         * a_prompt(-1, buf, ans);
         * if (ans[0] != 'Y' && ans[0] != 'y')
         * return 2;
         * }
         */

#ifdef FB2KPC
		if(!strncmp(FB2KPC,newpath,strlen(FB2KPC)))
			ret = 2;
#endif
        a_loadnames(&pm, getSession());
        ann_get_postfilename(fname, fileinfo, &pm);
        sprintf(bname, "%s/%s", pm.path, fname);
        sprintf(buf, "%-38.38s %s ", fileinfo->title, getCurrentUser()->userid);
        a_additem(&pm, buf, fname, NULL, 0, fileinfo->attachment);
        if (a_savenames(&pm) == 0) {
            sprintf(buf, "boards/%s/%s", key, fileinfo->filename);
            f_cp(buf, bname, 0);

            /*
             * Leeward 98.04.15 add below FILE_IMPORTED 
             */
			if(ret==0 && !nomsg) /* b4 操作不再重复计算 bmlog, 注意不要误伤, fancyrabbit Oct 12 2007 */
            	bmlog(getCurrentUser()->userid, currboard->filename, 12, 1);
        } else {
            if (!nomsg) {
                sprintf(buf, " 收入精华区失败，可能有其他版主在处理同一目录，按 Enter 继续 ");
                a_prompt(-1, buf, ans);
            }
            ret = 3;
        }
        a_freenames(&pm);
        return ret;
    }
    return 1;
}

int a_menusearch(path, key, level)
char *path, *key;
int level;
{
    char bname[STRLEN], bpath[STRLEN];
    struct stat st;
    const struct boardheader *fhdr;
    int num;

    if (key == NULL) {
        key = bname;
        a_prompt(-1, "输入欲搜寻之讨论区名称: ", key);
    }

    if ((num = getbid(key, &fhdr)) == 0)
        return 0;
    setbpath(bpath, fhdr -> filename);
    if ((*key == '\0') || (stat(bpath, &st) == -1))     /* 判断board是否存在 */
        return 0;
    if (!S_ISDIR(st.st_mode))
        return 0;


    if (check_read_perm(getCurrentUser(), fhdr) == 0)
        return 0;

    sprintf(bname,"0Announce/groups/%s",fhdr->ann_path);
    a_menu("", bname, (HAS_PERM(getCurrentUser(), PERM_ANNOUNCE) || HAS_PERM(getCurrentUser(), PERM_SYSOP) || HAS_PERM(getCurrentUser(), PERM_OBOARDS)) ? PERM_BOARDS : 0, 0, NULL);
    return 1;
}

void a_forward(char *path, ITEM *pitem)
{
    struct fileheader fhdr;
    char fname[PATHLEN], *mesg=NULL;

    bzero(&fhdr,sizeof(struct fileheader));/* clear,or have attachment. binxun */
    sprintf(fname, "%s/%s", path, pitem->fname);
    if (dashf(fname)) {
        strnzhcpy(fhdr.title, pitem->title, ARTICLE_TITLE_LEN);
        strncpy(fhdr.filename, pitem->fname, FILENAME_LEN - 1);
		fhdr.filename[FILENAME_LEN - 1] = '\0';
        switch (doforward(path, &fhdr)) {
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
                ("\n\033[1m\033[33m信件超长（本站限定信件长度上限为 %d 字节），取消转寄操作\033[m\033[m\n\n请告知收信人（也许就是您自己吧:PP）：\n\n*1* 使用 \033[1m\033[33mWWW\033[m\033[m 方式访问本站，随时可以保存任意长度的文章到自己的计算机；\n*2* 使用 \033[1m\033[33mpop3\033[m\033[m 方式从本站用户的信箱取信，没有任何长度限制。\n*3* 如果不熟悉本站的 WWW 或 pop3 服务，请阅读 \033[1m\033[33mAnnounce\033[m 版有关公告。\n",
                 MAXMAILSIZE);
            break;
        default:
            mesg = "取消转寄动作.\n";
        }
	if (mesg)
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
    char *mesg=NULL;
    FILE *pn;
    char ans[STRLEN];
    char buf[255];
    long attachpos=0;
#ifdef ANN_AUTONAME
	char head;
#endif

    pm->page = 9999;
#ifdef ANN_AUTONAME
	head='X';
#endif
    switch (mode) {
    case ADDITEM:
#ifdef ANN_AUTONAME
		head='A';
#else
        mesg = "请输入新文件之英文名称(可含数字)：";
#endif
        break;
    case ADDGROUP:
#ifdef ANN_AUTONAME
		head='D';
#else
        mesg = "请输入新目录之英文名称(可含数字)：";
#endif
        break;
    case ADDMAIL:
        sprintf(board, "tmp/bm.%s", getCurrentUser()->userid);
        if (!dashf(board)) {
            sprintf(buf, "哎呀!! 请先至该版(讨论区)将文章存入暂存档! << ");
            a_prompt(-1, buf, ans);
            return;
        }
        mesg = "请输入文件之英文名称(可含数字)：";
        break;
    default:
	return;
    }
#ifdef ANN_AUTONAME
	sprintf(fname,"%c%X",head,time(0)+rand());
#else
    a_prompt(-2, mesg, fname);
#endif
    if (*fname == '\0')
        return;
    sprintf(fpath, "%s/%s", pm->path, fname);
#ifdef ANN_AUTONAME
	if (0){
#else
    if (!valid_fname(fname)) {
#endif
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
            modify_user_mode(EDITANN);
            if (-1 == vedit(fpath, 0, NULL,NULL, 0)){
				modify_user_mode(CSIE_ANNOUNCE);
                return;         /* Leeward 98.06.12 fixes bug */
			}
            modify_user_mode(CSIE_ANNOUNCE);
            chmod(fpath, 0644);
            break;
        case ADDGROUP:
            mkdir(fpath, 0755);
            chmod(fpath, 0755);
            break;
        case ADDMAIL:
            /*
             * sprintf( genbuf, "mv -f %s %s",board, fpath );
             */
            {
                f_mv(board, fpath);
                sprintf(fpath2, "tmp/bm.%s.attach", getCurrentUser()->userid);
                attachpos = a_append_attachment(fpath, fpath2);
                my_unlink(fpath2);
            }
            break;
        }
        if (mode != ADDGROUP)
            sprintf(buf, "%-38.38s %s ", title, getCurrentUser()->userid);
        else {
            move(1, 0);
            clrtoeol();
            getdata(1, 0, "版主: ", uident, STRLEN - 1, DOECHO, NULL, true);
            if (uident[0] != '\0')
                sprintf(buf, "%-38.38s(BM: %s)", title, uident);
            else
                sprintf(buf, "%-38.38s", title);
        }
        a_additem(pm, buf, fname, NULL, 0, attachpos);
        if(a_savenames(pm)){
            a_loadnames(pm,getSession());
            a_additem(pm,buf,fname,NULL,0,attachpos);
            if(a_savenames(pm)){
                sprintf(buf," 整理精华区失败，可能有其他版主在处理同一目录，按 Enter 继续 ");
                a_prompt(-1,buf,ans);
                a_loadnames(pm,getSession());
                return;
            }
        }
        if(mode==ADDGROUP){
            sprintf(fpath2,"%s/%s/.Names",pm->path,fname);
            if((pn=fopen(fpath2,"w"))){
                fprintf(pn,"#\n#Title=%s\n#\n",buf);
                fclose(pn);
            }
        }
        bmlog(getCurrentUser()->userid,currboard->filename,(mode==ADDMAIL?12:13),1);
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

    if(num<0||num==pm->now)
        return;
    if (num >= pm->num)
        num = pm->num - 1;

    tmp=M_ITEM(pm,pm->now);
    if(num>pm->now){
        for(n=pm->now;n<num;n++)
            M_ITEM(pm,n)=M_ITEM(pm,n+1);
    }
    else{
        for(n=pm->now;n>num;n--)
            M_ITEM(pm,n)=M_ITEM(pm,n-1);
    }
    M_ITEM(pm,num)=tmp;
    pm->now = num;

    if (a_savenames(pm) == 0) {
        sprintf(genbuf, "改变 %s 下第 %d 项的次序到第 %d 项", pm->path + 17, temp, pm->now + 1);
        bmlog(getCurrentUser()->userid, currboard->filename, 13, 1);
        a_report(genbuf);
    } else {
        char buf[80], ans[40];

        sprintf(buf, " 整理精华区失败，可能有其他版主在处理同一目录，按 Enter 继续 ");
        a_prompt(-1, buf, ans);
        a_loadnames(pm, getSession());
    }
}

/* etnlegend, 2006.04.29, [精华区] 剪切/复制/粘贴操作 */
void a_copypaste(MENU *pm,int mode){
    MENU menu;
    ITEM *item;
    FILE *fp;
    struct stat st;
    char title[STRLEN],filename[STRLEN],path[PATHLEN],newpath[PATHLEN],ans[4],*p;
    int copy,ret,i;
    long ap;
    size_t len;
    enum {PASTE_ERROR,PASTE_CUT,PASTE_COPY} type;
#define ACP_ANY_RETURN(msg) do{prints("\033[1;37m%s\033[0;33m<Any>\033[m",(msg));igetkey();pm->page=9999;return;}while(0)
    move(t_lines-1,0);clrtoeol();
    sethomefile(genbuf,getCurrentUser()->userid,".CP");
    if(!(fp=fopen(genbuf,(!(mode==0||mode==1)?"r":"w"))))
        ACP_ANY_RETURN("使用粘贴命令前应先使用剪切或复制命令...");
    if(mode==0||mode==1){
        item=M_ITEM(pm,pm->now);
        snprintf(title,STRLEN,"%s",item->title);
        snprintf(filename,STRLEN,"%s",item->fname);
        snprintf(path,PATHLEN,"%s/%s",pm->path,filename);
        fprintf(fp,"%d\n%s\n%s\n%s\n%ld\n",mode,title,filename,path,item->attachpos);
        fclose(fp);
        ACP_ANY_RETURN((!mode?"复制标识完成, 但在粘贴后方可删除源文件或目录!":"剪切标识完成, 但在粘贴后方可删除源文件或目录!"));
    }
    do{
#define ACP_FGETS(len) if(!fgets(genbuf,((len)+1),fp)||!genbuf[0]||genbuf[0]=='\r'||genbuf[0]=='\n'){type=PASTE_ERROR;continue;}
#define ACP_DUMPS(dst,src,len) do{snprintf((dst),(len),"%s",(src));if((p=strpbrk((dst),"\r\n"))){*p=0;}}while(0)
        ap=0;
        ACP_FGETS(2);
        if(!isdigit(genbuf[0])){
            type=PASTE_ERROR;
            continue;
        }
        switch(atoi(genbuf)){
            case 0:
                type=PASTE_COPY;
                break;
            case 1:
                type=PASTE_CUT;
                break;
            default:
                type=PASTE_ERROR;
                continue;
        }
        ACP_FGETS(STRLEN);
        ACP_DUMPS(title,genbuf,STRLEN);
        ACP_FGETS(STRLEN);
        ACP_DUMPS(filename,genbuf,STRLEN);
        ACP_FGETS(PATHLEN);
        ACP_DUMPS(path,genbuf,PATHLEN);
        ACP_FGETS(21);
        if(!isdigit(genbuf[0])){
            type=PASTE_ERROR;
            continue;
        }
        ap=atol(genbuf);
#undef ACP_FGETS
#undef ACP_DUMPS
    }
    while(0);
    fclose(fp);
    if(type==PASTE_ERROR)
        ACP_ANY_RETURN("使用粘贴命令前应先使用剪切或复制命令...");
    snprintf(newpath,PATHLEN,"%s/%s",pm->path,filename);
    if(!((access(newpath,F_OK)==-1)&&(errno==ENOENT)))
        ACP_ANY_RETURN("当前路径下已存在同名文件或目录...");
    if(!strncmp(path,newpath,(len=strlen(path)))&&newpath[len]=='/')
        ACP_ANY_RETURN("当前操作剪切或复制目录至其子目录中, 将导致死循环...");
    if(stat(path,&st)==-1||!(S_ISDIR(st.st_mode)||S_ISREG(st.st_mode)))
        ACP_ANY_RETURN("源文件或目录不存在...");
    if(type==PASTE_COPY)
        snprintf(genbuf,STRLEN,"复制方式粘贴%s %.30s, 确认? (C-复制/L-链接/N) [N]: ",S_ISDIR(st.st_mode)?"目录":"文件",filename);
    else
        snprintf(genbuf,STRLEN,"剪切方式粘贴%s %.38s, 确认? (Y/N) [N]: ",S_ISDIR(st.st_mode)?"目录":"文件",filename);
    getdata(t_lines-1,0,genbuf,ans,2,DOECHO,NULL,true);
    move(t_lines-1,0);clrtoeol();
    ans[0]=toupper(ans[0]);copy=0;
    if(ans[0]=='Y'||(type==PASTE_COPY&&ans[0]=='C')){
        a_additem(pm,title,filename,NULL,0,ap);
        if(a_savenames(pm)){
            a_loadnames(pm,getSession());
            a_additem(pm,title,filename,NULL,0,ap);
            if(a_savenames(pm)){
                a_loadnames(pm,getSession());
                ACP_ANY_RETURN("整理精华区失败...");
            }
        }
        if(!(type==PASTE_CUT&&!rename(path,newpath))){
            if(S_ISDIR(st.st_mode)){
                sprintf(genbuf,"/bin/cp -pr %s %s",path,newpath);
                if(system(genbuf))
                    ACP_ANY_RETURN("复制目录过程中发生错误...");
            }
            else{
                if(f_cp(path,newpath,0))
                    ACP_ANY_RETURN("复制文件过程中发生错误...");
            }
            copy=1;
        }
    }
    else if(type==PASTE_COPY&&ans[0]=='L'){
        a_additem(pm,title,filename,NULL,0,ap);
        if(a_savenames(pm)){
            a_loadnames(pm,getSession());
            a_additem(pm,title,filename,NULL,0,ap);
            if(a_savenames(pm)){
                a_loadnames(pm,getSession());
                ACP_ANY_RETURN("整理精华区失败...");
            }
        }
        if(path[0]!='/'){
            if(!getcwd(genbuf,PATHLEN))
                ACP_ANY_RETURN("发生致命错误...");
            p=&genbuf[strlen(genbuf)];
            snprintf(p,PATHLEN,"/%s",path);
        }
        else
            snprintf(genbuf,PATHLEN,"%s",path);
        if(S_ISDIR(st.st_mode)){
            if(symlink(genbuf,newpath)==-1)
                ACP_ANY_RETURN("链接目录过程中发生错误...");
        }
        else{
            if(link(path,newpath)==-1&&symlink(genbuf,newpath)==-1)
                ACP_ANY_RETURN("链接文件过程中发生错误...");
        }
    }
    else{
        pm->page=9999;
        return;
    }
    bmlog(getCurrentUser()->userid,currboard->filename,13,1);
    if(type==PASTE_CUT){
        sethomefile(genbuf,getCurrentUser()->userid,".CP");
        unlink(genbuf);
        if(copy)
            S_ISDIR(st.st_mode)?f_rm(path):unlink(path);
        if((p=strrchr(path,'/')))
            *p=0;
        memset(&menu,0,sizeof(MENU));
        menu.path=path;
        a_loadnames(&menu,getSession());
        for(i=0;i<menu.num;i++)
            if(!strcmp(M_ITEM(&menu,i)->fname,filename))
                a_delitem(&menu,i--);
        ret=a_savenames(&menu);
        a_freenames(&menu);
        if(ret)
            ACP_ANY_RETURN("剪切文件过程中发生错误...");
    }
    pm->page=9999;
#undef ACP_ANY_RETURN
    return;
}

/* etnlegend, 2006.04.29, [精华区] 删除操作 */
void a_delete(MENU *pm){
    struct stat st;
    char path[PATHLEN],ans[4];
    sprintf(genbuf,"%5d  %s",(pm->now+1),M_ITEM(pm,pm->now)->title);
    move(t_lines-2,0);clrtobot();
    prints("\033[1;37m%s\033[m",genbuf);
    snprintf(path,PATHLEN,"%s/%s",pm->path,M_ITEM(pm,pm->now)->fname);
    if(!lstat(path,&st)&&(S_ISDIR(st.st_mode)||S_ISREG(st.st_mode)||S_ISLNK(st.st_mode))){
        sprintf(genbuf,"\033[1;37m确认删除该%s? (Y/N) [N]: \033[m",
            (S_ISLNK(st.st_mode)?"链接":(S_ISDIR(st.st_mode)?"目录":"文件")));
        getdata(t_lines-1,0,genbuf,ans,2,DOECHO,NULL,true);
        if(toupper(ans[0])!='Y'){
            pm->page=9999;
            return;
        }
        S_ISLNK(st.st_mode)?unlink(path):(S_ISDIR(st.st_mode)?my_f_rm(path):my_unlink(path));
    }
    a_delitem(pm,pm->now);
    if(a_savenames(pm)){
        a_loadnames(pm,getSession());
        move(t_lines-1,0);
        prints("\033[1;37m%s\033[0;33m<Any>\033[m","整理精华区失败...");
        igetkey();
    }
    else
        bmlog(getCurrentUser()->userid,currboard->filename,13,1);
    pm->page=9999;
    return;
}

void a_newname(pm)
MENU *pm;
{
    char fname[STRLEN];
    char fpath[PATHLEN];
    char *mesg;

    a_prompt(-2, "新文件名: ", fname);
    if (*fname == '\0')
        return;
    sprintf(fpath, "%s/%s", pm->path, fname);
    if (!valid_fname(fname)) {
        mesg = "不合法文件名称.";
    } else if (dashf(fpath) || dashd(fpath)) {
        mesg = "系统中已有此文件存在了.";
    } else {
        sprintf(genbuf, "%s/%s", pm->path, M_ITEM(pm,pm->now)->fname);
        strcpy(M_ITEM(pm,pm->now)->fname, fname);
        if (a_savenames(pm) == 0) {
            if (f_mv(genbuf, fpath) == 0) {
                char r_buf[256];

                sprintf(r_buf, "更改文件名: %s -> %s", genbuf + 17, fpath + 17);
                a_report(r_buf);
                return;
            }
        } else {
            char buf[80], ans[40];

            sprintf(buf, "整理精华区失败，可能有其他版主在处理同一目录，按 Enter 继续 ");
            a_prompt(-1, buf, ans);
            a_loadnames(pm, getSession());
        }
        mesg = "文件名更改失败 !!";
    }
    prints(mesg);
    pressanykey();
}

/* add by stiger */
/* 寻找丢失条目 */
int a_repair(MENU *pm)
{
	DIR *dirp;
	struct dirent *direntp;
	int i,changed;
	char title[84];

	changed=0;
	
	dirp=opendir(pm->path);
	if(dirp==NULL) return -1;

	while( (direntp=readdir(dirp)) != NULL){
		if(direntp->d_name[0]=='.') continue;
#ifdef ANN_COUNT
		if(!strcmp(direntp->d_name,"counter.person")) continue;
#endif
		for( i=0; i < pm->num; i++ ){
			if(strcmp(M_ITEM(pm,i)->fname, direntp->d_name)==0){
				i=-1;
				break;
			}
		}
		if(i!=-1){
			sprintf(title, "%-38.38s(BM: SYSOPS)", direntp->d_name);
			a_additem(pm, title, direntp->d_name, NULL, 0, 0);
			changed++;
		}
	}
	closedir(dirp);

	if(changed>0){
		if(a_savenames(pm) != 0)
			changed = 0 - changed;
	}
	return changed;
}
/* add end */

#ifdef ANN_CTRLK
static int a_control_user(char *fpath)
{
    char buf[PATHLEN+20];
    int count;
    char ans[20];
    char uident[STRLEN];

    sprintf(buf, "%s/.allow", fpath);

    while (1) {
        clear();
        prints("设定目录访问名单\n");
        count = listfilecontent(buf);
        if (count)
            getdata(1, 0, "(A)增加 (D)删除or (E)离开[E]", ans, 7, DOECHO, NULL, true);
        else
            getdata(1, 0, "(A)增加 or (E)离开 [E]: ", ans, 7, DOECHO, NULL, true);
        if (*ans == 'A' || *ans == 'a') {
            move(1, 0);
            usercomplete("增加访问允许成员: ", uident);
            if (*uident != '\0') {
                if(seek_in_file(buf,uident)){
					move(2,0);
					prints("%s 已经在名单中\n",uident);
					continue;
				}
				addtofile(buf,uident);
				continue;
            }
        } else if ((*ans == 'D' || *ans == 'd') && count) {
            move(1, 0);
            namecomplete("删除访问允许成员: ", uident);
            if (uident[0] != '\0') {
                if (seek_in_file(buf, uident)) {
                    del_from_file(buf, uident);
                }
                continue;
            }
        } else
            break;
    }
	if(count<=0){
		clear();
		move(1,0);
		if(askyn("允许访问者名单空,你要取消访问设置,让所有用户可以访问吗?",0) == 1){
			unlink(buf);
		}
	}

    return 1;
}
#endif

static int admin_utils_announce(MENU *menu,ITEM *item,void *varg){
#define AU_LIBRARY  "admin/libadmin_utils.so"
#define AU_FUNCTION "process_key_announce"
    typedef int (*FUNC_ADMIN)(MENU*,ITEM*,void*);
    FUNC_ADMIN function;
    void *handle;
#ifdef SECONDSITE
    if(!HAS_PERM(getCurrentUser(),PERM_BOARDS)&&!(getCurrentUser()->title))
#else
    if(!HAS_PERM(getCurrentUser(),PERM_SYSOP))
#endif
        return -1;
    if(!(function=(FUNC_ADMIN)dl_function(AU_LIBRARY,AU_FUNCTION,&handle)))
        return -1;
    (*function)(menu,item,varg);
    dlclose(handle);
    return 0;
#undef AU_LIBRARY
#undef AU_FUNCTION
}

void a_manager(MENU *pm,int ch)
{
    char uident[STRLEN];
    ITEM *item=NULL;
    char fpath[PATHLEN], changed_T[STRLEN], ans[STRLEN];

    if (pm->num > 0) {
        item = M_ITEM(pm,pm->now);
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
        /*
         * case 'G':  a_newitem( pm, ADDGOPHER );    break; 
         */
    case 'p':
        a_copypaste(pm, 2);
        break;
    case 'f':{
            int i;

            pm->page = 9999;
            i = a_select_path(false);
            if (i == 0)
                break;

            import_path_select = i;
            i--;
            free(import_path[i]);
            import_path[i] = malloc(strlen(pm->path) + 1);
            strcpy(import_path[i], pm->path);
            free(import_title[i]);
            strncpy(ans, pm->mtitle, STRLEN);
            ans[STRLEN-1]=0;
            move(t_lines - 2, 0);
            clrtoeol();
            getdata(t_lines - 2, 0, "设定丝路名:", ans, STRLEN - 1, DOECHO, NULL, false);
            import_title[i] = malloc(strlen(ans) + 1);
            strcpy(import_title[i], ans);
        	save_import_path(import_path,import_title,&import_path_time, getSession());
        }
        break;
#ifdef ANN_CTRLK
	case Ctrl('K'):
		a_control_user(pm->path);
        pm->page = 9999;
		break;
#endif
	/* add by stiger,20030502 */
	/* 寻找丢失条目 */
	case 'z':
		if(HAS_PERM(getCurrentUser(), PERM_SYSOP)){
			int i;

			i=a_repair(pm);

			if(i>=0){
				sprintf(genbuf,"发现 %d 个丢失条目,请按Enter继续...",i);
			}else{
				sprintf(genbuf,"发现 %d 个丢失条目,更新索引失败,请按Enter继续...",0-i);
			}
			a_prompt(-1,genbuf,ans);
			pm->page = 9999;
		}
		break;
	/* add end */
    }
    if (pm->num > 0)
        switch (ch) {
        case 's':
            if (++a_fmode >= 3)
                a_fmode = 1;
			a_fmode_show = a_fmode;
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
            if (HAS_PERM(getCurrentUser(), PERM_SYSOP))
            {
                sprintf(fpath, "%s/.Names", pm->path);

                if (dashf(fpath)) {
                    modify_user_mode(EDITANN);
                    vedit(fpath, 0, NULL,NULL, 0);
                    modify_user_mode(CSIE_ANNOUNCE);
                }
                pm->page = 9999;
            }
            break;
        case 't':
	    if (item) {
            strncpy(changed_T, item->title, 39);
            changed_T[38] = 0;
            uident[0] = '\0';
            if(strlen(item->title) > 44)
            {
                char *ptr, *tempuid;
                if ((ptr = strchr(item->title + 38, '(')) != NULL) {
                    *ptr = '\0';
                    tempuid = ptr + 1;
                    if (strncmp(tempuid, "BM: ", 4) == 0)
                        tempuid += 4;
                    snprintf(uident, STRLEN, "%s", tempuid);
                    if ((ptr = strchr(uident, ')')) != NULL)
                        *ptr = '\0';
                }
            }
            {
                char *p;

                p = changed_T + strlen(changed_T) - 1;
                for (; p >= changed_T; p--) {
                    if (*p == ' ')
                        *p = 0;
                    else
                        break;
                };
            }
            a_prompt2(-2, "新标题: ", changed_T);
            /*
             * modified by netty to properly handle title change,add bm by SmallPig 
             */
            if (*changed_T) {
                if (dashf(fpath)) {
                    sprintf(genbuf, "%-38.38s %s ", changed_T, getCurrentUser()->userid);
                    strcpy(item->title, genbuf);
                    sprintf(genbuf, "改变文件 %s 的标题", fpath + 17);
                } else if (dashd(fpath)) {
                    move(1, 0);
                    clrtoeol();
                    getdata(1, 0, "版主: ", uident, IDLEN + 1, DOECHO, NULL, false);
                    if (uident[0] != '\0')
                        sprintf(genbuf, "%-38.38s(BM: %s)", changed_T, uident);
                    else
                        sprintf(genbuf, "%-38.38s", changed_T);
                    strcpy(item->title, genbuf);
                    sprintf(genbuf, "改变目录 %s 的标题", fpath + 17);
                }
                if (a_savenames(pm) != 0) {
                    sprintf(genbuf, "整理精华区失败，可能有其他版主在处理同一目录，按 Enter 继续 ");
                    a_prompt(-1, genbuf, ans);
                    a_loadnames(pm, getSession());
                }
                else
                    a_report(genbuf);
            }
            pm->page = 9999;
	    }
            break;

        case '`':
            if(!HAS_PERM(getCurrentUser(),PERM_SYSOP)||!HAS_PERM(getCurrentUser(),PERM_ANNOUNCE)||!item)
                break;
            strnzhcpy(genbuf,item->title,39);
            do{
                char *p,*q;
                for(q=NULL,p=&genbuf[0];*p;p++)
                    if(*p!=' ')
                        q=p;
                if(q)
                    *++q=0;
                else
                    strcpy(genbuf,"<无标题>");
            }
            while(0);
            strcpy(item->title,genbuf);
            move(t_lines-1,0);
            clrtoeol();
            if(a_savenames(pm)){
                prints("\033[1;31;47m\t%s\033[K\033[m","操作过程中发生错误, 按回车键继续...");
                WAIT_RETURN;
                a_loadnames(pm,getSession());
            }
            else{
                prints("\033[1;34;47m\t%s\033[K\033[m","操作[清空整理者]成功, 按回车键继续...");
                WAIT_RETURN;
            }
            pm->page=9999;
            break;
        case '~':
            if(!HAS_PERM(getCurrentUser(),PERM_SYSOP)||!HAS_PERM(getCurrentUser(),PERM_ANNOUNCE)||!item)
                break;
            strnzhcpy(genbuf,item->title,39);
            sprintf(item->title,"%-38.38s(BM: SYSOPS)",genbuf);
            move(t_lines-1,0);
            clrtoeol();
            if(a_savenames(pm)){
                prints("\033[1;31;47m\t%s\033[K\033[m","操作过程中发生错误, 按回车键继续...");
                WAIT_RETURN;
                a_loadnames(pm,getSession());
            }
            else{
                prints("\033[1;34;47m\t%s\033[K\033[m","操作[设置整理者为 (BM: SYSOPS)]成功, 按回车键继续...");
                WAIT_RETURN;
            }
            pm->page=9999;
            break;

        case 'S':
            if(!item)
                break;
            do{
                char ans[4];
                enum ANN_SORT_MODE mode;
                getdata(t_lines-1,0,"\033[1;33m[排序] \033[1;37m按文件名升/降序\033[1;32m{n/N} "
                    "\033[1;37m按标题升/降序\033[1;32m{t/T} \033[1;37m按整理字段升/降序\033[1;32m{b/B} "
                    "\033[1;37m[]: \033[m",ans,2,DOECHO,NULL,true);
                move(t_lines-1,0);
                clrtoeol();
                switch(ans[0]){
                    case 'n':
                        mode=ANN_SORT_BY_FILENAME;
                        break;
                    case 'N':
                        mode=ANN_SORT_BY_FILENAME_R;
                        break;
                    case 't':
                        mode=ANN_SORT_BY_TITLE;
                        break;
                    case 'T':
                        mode=ANN_SORT_BY_TITLE_R;
                        break;
                    case 'b':
                        mode=ANN_SORT_BY_BM;
                        break;
                    case 'B':
                        mode=ANN_SORT_BY_BM_R;
                        break;
                    default:
                        prints("\033[1;34;47m\t%s\033[K\033[m","取消排序操作, 按回车键继续...");
                        WAIT_RETURN;
                        continue;
                }
                switch(a_sort_items(pm,mode,getSession())){
                    case 0:
                        prints("\033[1;34;47m\t%s\033[K\033[m","排序操作成功, 按回车键继续...");
                        WAIT_RETURN;
                        break;
                    case -1:
                    case -3:
                        prints("\033[1;31;47m\t%s\033[K\033[m","排序操作中发生错误[加载索引失败], 按回车键继续...");
                        WAIT_RETURN;
                        break;
                    case -2:
                        prints("\033[1;31;47m\t%s\033[K\033[m","排序操作中发生错误[保存索引失败], 按回车键继续...");
                        WAIT_RETURN;
                        break;
                    case -4:
                    default:
                        prints("\033[1;31;47m\t%s\033[K\033[m","排序操作中发生错误[未知错误], 按回车键继续...");
                        WAIT_RETURN;
                        break;
                }
            }
            while(0);
            pm->page=9999;
            break;

        case 'e':
	    if (item) {
            if (dashf(fpath)) {
                long attachpos=item->attachpos;
                modify_user_mode(EDITANN);
                vedit(fpath, 0, NULL,&attachpos, 0);
                if (item->attachpos!=attachpos) {
                    item->attachpos=attachpos;
                    if (a_savenames(pm) != 0) {
                        int i;
                        ITEM saveitem;
                        saveitem=*item;
                        /* retry */
                        a_loadnames(pm, getSession());
                        for (i=0;i<pm->num;i++) {
                            if (!strcmp(M_ITEM(pm,i)->fname,saveitem.fname))
                                M_ITEM(pm,i)->attachpos=attachpos;
                        }
                        if(a_savenames(pm)){
                            a_loadnames(pm,getSession());
                            move(t_lines-1,0);
                            clrtoeol();
                            prints("\033[1;31;47m%s\033[K\033[m","文章附件大小发生变化, 写入索引文件失败, 按回车键继续...");
                            WAIT_RETURN;
                            pm->page=9999;
                            break;
                        }
                    }
                }
                modify_user_mode(CSIE_ANNOUNCE);
                sprintf(genbuf, "修改文章 %s 的内容", pm->path + 17);
                a_report(genbuf);
            }
            pm->page = 9999;
            }
            break;
        case 'n':
            a_newname(pm);
            pm->page = 9999;
            break;
        case 'c':
            a_copypaste(pm, 0);
            break;
        case 'x':              //added by bad 03-2-10
            a_copypaste(pm, 1);
            break;

        case Ctrl('S'):
            if(!admin_utils_announce(pm,item,NULL))
                pm->page=9999;
            break;

/*  do not support thread read in announce: COMMAN 2002.7
        case '=':  t_search_down();     break;
        case '+':  t_search_up();       break;
*/
        }
}

static void ann_get_current_url(char* buf,int buf_len,char *ext, int len,void* arg, int domainflag)
{
	char board[STRLEN], path[MAXPATH], phpname[20];
    MENU *m=(MENU *)arg;
	MENU *tmp;
	int bid;
	char bap[PATHLEN];
	const struct boardheader *fh;
	int sz;

 	/* "bbs0an.php" or "bbsanc.php", by pig2532 */
 	snprintf(path, MAXPATH, "%s/%s", m->path, M_ITEM(m,m->now)->fname);
 	if(dashd(path))
 	{
 	    strcpy(phpname, "bbs0an.php");
#if 0 // orz pig2532 started on 20061121 
 	    /* 如果bbs0an.php支持数字串方式的精华区路径，则下面一段可删除 */
 	    snprintf(buf, buf_len, "http://%s/bbs0an.php?path=%s",
 	        get_my_webdomain(domainflag), path+10);
 	    return;
 	    /* 以上 */
#endif
 	}
 	else
 	    strcpy(phpname, "bbsanc.php");
 
	board[0]='\0';

    ann_get_board(m->path, board, sizeof(board));
	if(board[0] =='\0' || (bid=getbnum_safe(board,getSession(), 1))==0){
        snprintf(buf,buf_len-9,"http://%s/%s?path=%s/%s",
          get_my_webdomain(domainflag), phpname, m->path+10, M_ITEM(m,m->now)->fname);
	  return;
	}

	strcpy(buf, "error\n");

    if ((fh = getboard(bid)) == NULL)
        return;

    sprintf(bap,"0Announce/groups/%s",fh->ann_path);
	sz = strlen(bap);

	for(tmp=m; tmp; tmp = (MENU *)(tmp->father)){
		if( !strncmp(bap, tmp->path, sz) && (tmp->path[sz]=='\0' || tmp->path[sz+1]=='\0') )
			break;
	}

	if(tmp==NULL) return;

    snprintf(buf,buf_len-9,"http://%s/%s?p=%d",
        get_my_webdomain(domainflag), phpname, bid);

	for(; tmp; tmp=(MENU *)(tmp->nowmenu)){
		if(strlen(buf) < buf_len-9){
			sprintf(bap, "-%d", tmp->now+1);
			strcat(buf, bap);
		}
	}

	return;

}

/* Show file info in announce, pig2532 */
void ann_showinfo(MENU *m)
{
    char url[STRLEN];
    
    ann_get_current_url(url, STRLEN, NULL, 0, m, 0);
    
    clear();
    move(3, 0);
    prints("精华区文件链接地址：");
    move(4, 1);
    prints("%s", url);
    pressanykey();
}

void ann_attach_link_num(char* buf,int buf_len,char *ext, int len,long attachpos,void* arg)
{
    char bap[PATHLEN];
    
    if(attachpos != -1)
    {
        ann_get_current_url(buf, buf_len, ext, len, arg, 1);
        sprintf(bap, "&ap=%ld", attachpos);
        strcat(buf, bap);
    } else {
        ann_get_current_url(buf, buf_len, ext, len, arg, 0);
    }
}

void ann_attach_link(char* buf,int buf_len,long attachpos,void* arg)
{
    char *fname=(char *)arg;
    /*
     *if (normal_board(currboard->filename)) {
     * @todo: generate temp sid
     */
      snprintf(buf,buf_len-9,"http://%s/bbsanc.php?path=%s&ap=%ld",
        get_my_webdomain(1),fname+10,attachpos);
}

#ifdef FB2KPC
int AddPCorpus(void){
	FILE *fn;
	char    personalpath[PATHLEN], title[200];
	struct userec *lookupuser;

        if (!check_systempasswd()) {
                return 1;
        }
        clear();
        prints("创建个人文集");

		move(1,0);
		usercomplete( "请输入使用者代号: ",title);
		if(title[0]=='\0')
                return 1;
		if(!getuser(title, &lookupuser))
				return 1;

	sprintf(personalpath,FB2KPC "/%c/%s", toupper(lookupuser->userid[0]),lookupuser->userid);
        if (dashd(personalpath)) {
			move(10,0);
			prints("该用户的个人文集目录已存在\n");
			pressanykey();
		return 1;
	}
	
	move(4,0);
	if(askyn("确定要为该用户创建一个个人文集吗?",1)==0){
		return 1;
	}

    mkdir(personalpath, 0755);
    chmod(personalpath, 0755);

	move(7,0);
	prints("[直接按 ENTER 键, 则标题缺省为: [32m%s 的个人文集[m]",lookupuser->userid);
	getdata(6, 0, "请输入个人文集之标题: ", title, 39, DOECHO, NULL, true);
	if(title[0] == '\0')
		sprintf(title,"%s 的个人文集",lookupuser->userid);
	sprintf(personalpath, "%s/.Names", personalpath);
        if ((fn = fopen(personalpath, "w")) == NULL) {
              return -1;
        }
        fprintf(fn, "#\n");
        fprintf(fn, "# Title=%s\n", title);
        fprintf(fn, "#\n");
        fclose(fn);

	move(15,0);
	prints("已经创建个人文集, 请按任意键继续...");
	pressanykey();
	return 0;
}
#endif

void a_menu(maintitle, path, lastlevel, lastbmonly, father)
char *maintitle, *path;
int lastlevel, lastbmonly;
MENU *father;
{
    MENU me;
    char fname[PATHLEN];
#ifdef ANN_SHOW_WELCOME
	char welcome[PATHLEN+20];
#endif
    int ch;
    char *bmstr;
    char buf[STRLEN];
    int bmonly;
    int oldmode;
    int number = 0;
#ifdef NEW_HELP
    int oldhelpmode = helpmode;
#endif

    bzero(&me, sizeof(me));
    modify_user_mode(CSIE_ANNOUNCE);
#ifdef NEW_HELP
	helpmode = HELP_ANNOUNCE;
#endif
    me.path = path;
#ifdef ANN_SHOW_WELCOME
	strcpy(welcome,path);
	strcat(welcome,"/welcome");
	if(dashf(welcome))
		show_help(welcome);
#endif
    strcpy(me.mtitle, maintitle);
    me.level = lastlevel;
    bmonly = lastbmonly;
	if(father){
		father->nowmenu = &me;
		me.father = father;
	}
    a_loadnames(&me, getSession());           /* Load .Names */

    strcpy(buf, me.mtitle);
#ifdef FB2KPC
	if(!strncmp(FB2KPC,me.path,strlen(FB2KPC))){
		if(fb2kpc_is_owner(me.path))
			me.level |= PERM_BOARDS;
		else if(bmonly==1 && !(me.level & PERM_BOARDS))
			return;
	}else{
#endif
    bmstr = strstr(buf, "(BM:");
    if (bmstr != NULL) {
        if (chk_currBM(bmstr + 4, getCurrentUser()) || HAS_PERM(getCurrentUser(), PERM_SYSOP))
            me.level |= PERM_BOARDS;
        else if (bmonly == 1 && !(me.level & PERM_BOARDS))
            return;
    }
#ifdef FB2KPC
	}
#endif

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

			if( ! (me.level & PERM_BOARDS) )
				a_fmode_show=1;

            a_showmenu(&me);
        }
        if (ann_quick_view && (me.now < me.num)) {
            if (M_ITEM(&me,me.now)->host == NULL) {
                snprintf(fname, sizeof(fname), "%s/%s", path, M_ITEM(&me,me.now)->fname);
                if (dashf(fname)) {
                    draw_content(fname, NULL);
                    outs("\x1b[m");
                } else {
                    int i; char buf[128];
                    move(BBS_PAGESIZE / 2+3, 0);
                    sprintf(buf, "\x1b[44m%-80.80s\033[m\n", "这是一个目录");
                    prints(buf);
                    for(i=BBS_PAGESIZE / 2+4;i<t_lines-1;i++) {
                        move(i,0);
                        clrtoeol();
                    }
                }
            }
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
        case ',':
            ann_quick_view = !ann_quick_view;
            if (!ann_quick_view) me.page = 9999;
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
		case KEY_HOME:
			me.now = 0;
			break;
		case KEY_END:
			me.now = me.num - 1;
			break;
        case Ctrl('Q'):    /* pig2532: show file info */
            if(!me.num)
                break;
            ann_showinfo(&me);
            me.page = 9999;
            break;
        case Ctrl('C'):
        case Ctrl('P'):
            if(me.now >= me.num)
                break;
            if(!HAS_PERM(getCurrentUser(),PERM_POST)||!M_ITEM(&me,me.now))
                break;
            sprintf(fname,"%s/%s",path,M_ITEM(&me,me.now)->fname);
            if(!dashf(fname))
                break;
            if(me.now<me.num){
                do{
                    const struct boardheader *bh;
                    char bname[32],ans[4];
                    int ret;
                    clear();
                    move(1,0);
                    if(!get_a_boardname(bname,"请输入要转贴的讨论区名称: ")||!(bh=getbcache(bname)))
                        break;
                    move(2,0);
                    clrtobot();
                    if(!haspostperm(getCurrentUser(),bname)){
                        ans[0]=(HAS_PERM(getCurrentUser(),PERM_LOGINOK)?'1':'0');
                        ans[1]=(('0'+'1')-ans[0]);
                        sprintf(genbuf,"\n\n    您目前无法在该讨论区发表文章!\n\n    导致上述问题的原因可能是\033[%c;33m版面的发文权限制\033[m或者\033[%c;33m您尚未通过注册\033[m,\n    尚未通过注册的用户可在\033[%c;33m个人工具箱\033[m内填写注册资料以完成注册:)\n\n    按回车键继>续...\033[0;33m<Enter>\033[m",ans[0],ans[1],ans[1]);
                        prints("%s",genbuf);
                        WAIT_RETURN;
                        break;
                    }
                    if(checkreadonly(bname)){
                        prints("\n\n    %s\033[0;33m<Enter>\033[m","\033[1;33m目的版面目前为\033[1;31m只读\033[1;33m模式, 取消转载操作...\033[m");
                        WAIT_RETURN;
                        break;
                    }
#ifdef NEWSMTH
                    if(!check_score_level(getCurrentUser(),bh)){
                        prints("\n\n    \033[1;33m%s\033[0;33m<Enter>\033[m","您的积分不符合目的讨论区的设定, 暂时无法向目的讨论区转载文章...");
                        WAIT_RETURN;
                        break;
                    }
#endif /* NEWSMTH */
                    if(!HAS_PERM(getCurrentUser(),PERM_SYSOP)&&deny_me(getCurrentUser()->userid,bname)){
                        prints("\n\n    \033[1;33m%s\033[0;33m<Enter>\033[m","您已被管理人员取消在目的版面的发文权限...");
                        WAIT_RETURN;
                        break;
                    }
                    sprintf(genbuf,"确认转载至 %s 版 %s(L)站内发表 (A)取消操作 [A]: ",bh->filename,
                        (!(bh->flag&BOARD_OUTFLAG)?"":"(S)转信发表 "));
                    clrtoeol();
                    getdata(1,0,genbuf,ans,2,DOECHO,NULL,true);
                    switch(ans[0]){
                        case 'S':
                        case 's':
                            ans[0]=(!(bh->flag&BOARD_OUTFLAG)?'L':'S');
                            break;
                        case 'L':
                        case 'l':
                            ans[0]='L';
                            break;
                        default:
                            ans[0]=0;
                            break;
                    }
                    move(3,0);
                    if(!ans[0]){
                        prints("\033[1;33m%s\033[0;33m<Enter>\033[m","取消转载操作...");
                        WAIT_RETURN;
                        break;
                    }
                    ret = post_cross(getCurrentUser(),bh,"",M_ITEM(&me, me.now)->title,fname,0,false,ans[0],5,getSession());
                    switch (ret)
                    {
                        case 0 :
                            prints("\033[1;32m%s\033[0;33m<Enter>\033[m","转载完成!");
                            break;
                        case -1:
                            prints("\033[1;33m%s\033[0;33m<Enter>\033[m", "转载过程中发生错误 ...");
                            break;
                        case -2:
                            prints("\n\n        很抱歉，本文可能含有不当内容，需经审核方可发表。\n\n"
                                    "        根据《帐号管理办法》，被系统过滤的文章视同公开发表。请耐心等待\n"
                                    "    站务人员的审核，不要多次尝试发表此文章。\n\n"
                                    "        如有疑问，请致信 SYSOP 咨询。");
                            move(t_lines - 1, 0);
                            prints("%s", "                              \x1b[33m请按 ◆\x1b[36mEnter\x1b[33m◆ 继续\x1b[m");
                            break;
                    }
                    WAIT_RETURN;
                }
                while(0);
                update_endline();
                me.page=9999;
            }
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
                if (M_ITEM(&me,me.now)->host != NULL) {
                    /*
                     * gopher(me.item[ me.now ]->host,me.item[ me.now ]->fname,
                     * me.item[ me.now ]->port,me.item[ me.now ]->title); 
                     */
                    me.page = 9999;
                    break;
                } else
                    snprintf(fname, sizeof(fname), "%s/%s", path, M_ITEM(&me,me.now)->fname);
                if (dashf(fname)) {
                    /*
                     * ansimore( fname, true ); 
                     */
                    /*
                     * Leeward 98.09.13 新添功能∶
                     * ，用上／下箭头直接跳转到前／后一项 
                     */
					//register_attach_link(ann_attach_link, fname);
					register_attach_link(ann_attach_link_num, &me);
                    ansimore_withzmodem(fname, false, M_ITEM(&me,me.now)->title);
					register_attach_link(NULL,NULL);
                    move(t_lines - 1, 0);
                    prints("\033[1m\033[44m\033[31m[阅读精华区资料]  \033[33m结束 Q,← │ 上一项资料 U,↑│ 下一项资料 <Enter>,<Space>,↓ \033[m");
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
                        zsend_file(fname, M_ITEM(&me,me.now)->title);
                        break;
                    case Ctrl('Z'):
                    case 'h':
                        goto EXPRESS;
                    default:
                        break;
                    }
                } else if (dashd(fname)) {
                    a_menu(M_ITEM(&me,me.now)->title, fname, me.level, bmonly, &me);
					me.nowmenu = NULL;
                    a_loadnames(&me, getSession());   /* added by bad 03-2-10 */
                }
                me.page = 9999;
            }
            break;
        case '/':
            if (a_menusearch(path, NULL, me.level))
                me.page = 9999;
            break;
        case 'F':
            if (me.now < me.num && HAS_PERM(getCurrentUser(), PERM_BASIC) && HAS_PERM(getCurrentUser(), PERM_LOGINOK)) {
                a_forward(path, M_ITEM(&me,me.now));
                me.page = 9999;
            }
            break;
        case 'o':
            if (HAS_PERM(getCurrentUser(), PERM_BASIC))
                t_friends();
            me.page = 9999;
            break;              /*Haohmaru 98.09.22 */
        case 'v':
            i_read_mail();
            modify_user_mode(CSIE_ANNOUNCE);
            me.page = 9999;
            break;
        case 'u':
            clear();
            modify_user_mode(QUERY);
            t_query(NULL);
            modify_user_mode(CSIE_ANNOUNCE);
            me.page = 9999;
            break;              /*Haohmaru.99.11.29 */
		case 'U':		/* pig2532 2005.12.10 */
			board_query();
			me.page = 9999;
			break;
        case '!':
            Goodbye();
            me.page = 9999;
            break;              /*Haohmaru 98.09.24 */
            /*
             * case 'Z':
             * if( me.now < me.num && HAS_PERM(getCurrentUser(), PERM_BASIC ) ) {
             * sprintf( fname, "%s/%s", path, me.item[ me.now ]->fname );
             * a_download( fname );
             * me.page = 9999;
             * }
             * break;
             */
        case Ctrl('Y'):
            if (me.now < me.num) {
                if (M_ITEM(&me,me.now)->host != NULL) {
                    /*
                     * gopher(me.item[ me.now ]->host,me.item[ me.now ]->fname,
                     * me.item[ me.now ]->port,me.item[ me.now ]->title); 
                     */
                    me.page = 9999;
                    break;
                } else
                    sprintf(fname, "%s/%s", path, M_ITEM(&me,me.now)->fname);
                if (dashf(fname)) {
                    zsend_file(fname, M_ITEM(&me,me.now)->title);
                    me.page = 9999;
                }
            }
            break;
          case 'l':		/* by pig2532 on 2005.12.01 */
               oldmode = uinfo.mode;
               show_allmsgs();
      	       modify_user_mode(oldmode);
               me.page = 9999;
      	       break;
          case 'w':                  /* by pig2532 on 2005.11.30 */
          	oldmode = uinfo.mode;
          	if (!HAS_PERM(getCurrentUser(), PERM_PAGE))
          	    break;
            s_msg();
          	modify_user_mode(oldmode);
            me.page = 9999;
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
#ifdef ANN_GUESTBOOK
		else if(ch=='a' && strstr(me.mtitle,"<guestbook>"))
			a_newitem(&me, ADDITEM);
#endif
    }
    a_freenames(&me);
#ifdef NEW_HELP
	helpmode = oldhelpmode;
#endif
}

int Announce(void){
    sprintf(genbuf,"%s 精华区公布栏",BBS_FULL_NAME);
    a_menu(genbuf,"0Announce",(HAS_PERM(getCurrentUser(),PERM_ANNOUNCE)?PERM_BOARDS:0),0,NULL);
    clear();
    return 0;
}

int set_import_path(char* path)
{
    int i;
    i = a_select_path(true);
    if (i == 0)
        return 1;
    import_path_select = i;
    i--;
    if (import_path[i][0] != '\0') {
        strncpy(path, import_path[i], MAXPATH);
        return 0;
    }
     return 2;
}
