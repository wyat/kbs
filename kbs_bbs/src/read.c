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
*/

#include "bbs.h"
/* COMMAN : use mmap to speed up searching */
#include <unistd.h>
#include <sys/mman.h>

struct fileheader SR_fptr;
int SR_BMDELFLAG = false;
int B_to_b = false;


/*---	Modified by period	2000-11-12	---*
char *pnt;
 *---	current code memory leak ---*/
extern char MsgDesUid[14];
extern unsigned int tmpuser;
struct keeploc {
    char *key;
    int top_line;
    int crs_line;
    struct keeploc *next;
};
inline static void PUTCURS(struct keeploc *locmem)
{
    move(3 + locmem->crs_line - locmem->top_line, 0);
    prints(">");
}

inline static void RMVCURS(struct keeploc *locmem)
{
    move(3 + locmem->crs_line - locmem->top_line, 0);
    prints(" ");
}

static int search_articles(struct keeploc *locmem, char *query, int offset, int aflag);
static int search_author(struct keeploc *locmem, int offset, char *powner);
static int search_post(struct keeploc *locmem, int offset);
static int search_title(struct keeploc *locmem, int offset);
static int i_read_key(int cmdmode, struct one_key *rcmdlist, struct keeploc *locmem, int ch, int ssize, char* pnt,char *ding_direct, char *direct);
static int cursor_pos(struct keeploc *locmem, int val, int from_top);
static int search_thread(struct keeploc *locmem, int offset, char *title);
static int search_threadid(struct keeploc *locmem, int offset, int groupid, int mode);


/*struct fileheader *files = NULL;*/
int screen_len;
int last_line;
//only for declare
static int digestmode;
static char currdirect[PATHLEN];


int search_file(char *filename)
{
    char p_name[256];
    int i = 0;
    size_t size;
    struct fileheader *rptr, *rptr1;

    if (uinfo.mode != RMAIL)
        setbdir(digestmode, p_name, currboard->filename);
    else
        setmailfile(p_name, currentuser->userid, DOT_DIR);
    BBS_TRY {
        if (safe_mmapfile(p_name, O_RDONLY, PROT_READ, MAP_SHARED, (void *) &rptr, (size_t *) & size, NULL) == 0)
            BBS_RETURN(-1);
        for (i = 0, rptr1 = rptr; i < (int) (size / sizeof(struct fileheader)); i++, rptr1++)
            if (!strcmp(filename, rptr1->filename)) {
                end_mmapfile((void *) rptr, size, -1);
                BBS_RETURN(i);
            }
    }
    BBS_CATCH {
    }
    BBS_END end_mmapfile((void *) rptr, size, -1);
    return -1;
}

struct keeploc *getkeep(char *s, int def_topline, int def_cursline)
{
    static struct keeploc *keeplist = NULL;
    struct keeploc *p;

    for (p = keeplist; p != NULL; p = p->next) {
        if (!strcmp(s, p->key)) {
            if (p->crs_line < 1)
                p->crs_line = 1;        /* DAMMIT! - rrr */
            return p;
        }
    }
    p = (struct keeploc *) malloc(sizeof(*p));
    p->key = (char *) malloc(strlen(s) + 1);
    strcpy(p->key, s);
    p->top_line = def_topline;
    p->crs_line = def_cursline; /* this should be safe */
    p->next = keeplist;
    keeplist = p;
    return p;
}


void fixkeep(char *s, int first, int last)
{
    struct keeploc *k;

    k = getkeep(s, 1, 1);
    if (k->crs_line >= first) {
        k->crs_line = (first == 1 ? 1 : first - 1);
        k->top_line = (first < 11 ? 1 : first - 10);
    }
}


void modify_locmem(struct keeploc *locmem, int total)
{
    if (locmem->top_line > total) {
        locmem->crs_line = total;
        locmem->top_line = total - t_lines / 2;
        if (locmem->top_line < 1)
            locmem->top_line = 1;
    } else if (locmem->crs_line > total) {
        locmem->crs_line = total;
    }
}


int move_cursor_line(struct keeploc *locmem, int mode)
{
    int top, crs;
    int reload = 0;

    top = locmem->top_line;
    crs = locmem->crs_line;
    if (mode == READ_PREV) {
        if (crs <= top) {
            top -= screen_len - 1;
            if (top < 1)
                top = 1;
            reload = 1;
        }
        crs--;
        if (crs < 1) {
            crs = 1;
            reload = -1;
        }
    } else if (mode == READ_NEXT) {
        if (crs + 1 >= top + screen_len) {
            top += screen_len - 1;
            reload = 1;
        }
        crs++;
        if (crs > last_line) {
            crs = last_line;
            reload = -1;
        }
    }
    locmem->top_line = top;
    locmem->crs_line = crs;
    return reload;
}

static void draw_title(void (*dotitle) ())
{
    move(0, 0);
    (*dotitle) ();
}

static int cursor_pos(struct keeploc *locmem, int val, int from_top)
{
    if (val > last_line) {
        val = DEFINE(currentuser, DEF_CIRCLE) ? 1 : last_line;
    }
    if (val <= 0) {
        val = DEFINE(currentuser, DEF_CIRCLE) ? last_line : 1;
    }
    if (val >= locmem->top_line && val < locmem->top_line + screen_len - 1) {
        RMVCURS(locmem);
        locmem->crs_line = val;
        PUTCURS(locmem);
        return 0;
    }
    locmem->top_line = val - from_top;
    if (locmem->top_line <= 0)
        locmem->top_line = 1;
    locmem->crs_line = val;
    return 1;
}

/*---	Modified by period	2000-11-12	---*
void
draw_entry( doentry, locmem, num ,ssize)
char            *(*doentry)();
struct keeploc  *locmem;
int             num,ssize;
---*/
void draw_entry(READ_FUNC doentry, struct keeploc *locmem, int num, int ssize, char *pnt)
{
    char *str;
    int base, i;
    char foroutbuf[512];

    base = locmem->top_line;
    for (i = 0; i < num; i++) {
        move(i+3, 0);
        str = (*doentry) (foroutbuf, base + i, &pnt[i * ssize]);
	/*
        if (!check_stuffmode())
	*/
            prints("%s", str);
/*
        else
            showstuff(str);
	    */
        clrtoeol();
    }
    for(i=num+3; i<t_lines-1; i++) {
        move(i, 0);
        clrtoeol();
    }
    move(t_lines - 1, 0);
    update_endline();
}

int i_read(int cmdmode, char *direct, void (*dotitle) (), READ_FUNC doentry, struct one_key *rcmdlist, int ssize)
{
    char lbuf[11], lastfile[256];
    int num, entries, recbase;
    struct keeploc *locmem;

    int lbc, mode, ch;
    char *pnt;

    /* add by stiger*/
    char ding_direct[PATHLEN];
    /* add end */

    /* add by stiger */
    if ((cmdmode != RMAIL && cmdmode != GMENU)&&
        (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
  	  	sprintf(ding_direct,"boards/%s/%s",currboard->filename,DING_DIR);
	else
		ding_direct[0]='\0';
    /* add end */

    /*---	Moved from top of file	period	2000-11-12	---*/
    strcpy(currdirect,direct);

    /*---	HERE:	---*/
    screen_len = t_lines - 4;
    if (TDEFINE(TDEF_SPLITSCREEN)&&cmdmode!=GMENU)
        screen_len = screen_len/2-1;
    modify_user_mode(cmdmode);
    pnt = calloc(t_lines, ssize);
    draw_title(dotitle);
    last_line = get_num_records(direct, ssize);
    /* add by stiger */
    if ((cmdmode != RMAIL && cmdmode != GMENU)&&
        (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
	    last_line += get_num_records(ding_direct,ssize);
    /* add end */
    if (last_line == 0) {
        if (cmdmode == RMAIL) {
            prints("没有任何信件...");
            pressreturn();
            clear();
        }

        else if (cmdmode == GMENU) {
            getdata(t_lines - 1, 0, "没有任何好友 (A)新增好友 (Q)离开？[Q] ", genbuf, 4, DOECHO, NULL, true);
            if (genbuf[0] == 'a' || genbuf[0] == 'A')
                friend_add(0, NULL, 0);
        }

        else {
	    if (digestmode!=DIR_MODE_NORMAL) 
		    digestmode=DIR_MODE_NORMAL;
	    else {
            getdata(t_lines - 1, 0, "新版刚成立 (P)发表文章 (Q)离开？[Q] ", genbuf, 4, DOECHO, NULL, true);
            if (genbuf[0] == 'p' || genbuf[0] == 'P')
                do_post();
	    }
        }
        free(pnt);
        pnt = NULL;
        return;
    }
    num = last_line - screen_len + 2;
    locmem = getkeep(currdirect, num < 1 ? 1 : num, last_line);
    modify_locmem(locmem, last_line);

    if(locmem->crs_line-locmem->top_line>=screen_len) /*added by bad 2002.9.2*/
        locmem->crs_line = locmem->top_line;
    
    recbase = locmem->top_line;
    /* add by stiger */
    if (cmdmode != RMAIL && cmdmode != GMENU
         && (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
	    entries = read_get_records(currdirect, ding_direct,pnt, ssize, recbase, screen_len);
    else
    /* add end */
    entries = get_records(currdirect, pnt, ssize, recbase, screen_len);

    /*---	Modofied by period	2000-11-12	---*
    draw_entry( doentry, locmem, entries ,ssize);
     *---			---*/
    draw_entry(doentry, locmem, entries, ssize, pnt);
    if (TDEFINE(TDEF_SPLITSCREEN)&&cmdmode!=GMENU){
        char buf[256], *t;
        struct fileheader* h;
        strcpy(buf, currdirect);
        if ((t = strrchr(buf, '/')) != NULL)
            *t = '\0';
        h = &pnt[(locmem->crs_line - locmem->top_line) * ssize];
        sprintf(genbuf, "%s/%s", buf, h->filename);
        strcpy(lastfile, genbuf);
        draw_content(genbuf,h);
        update_endline();
    }
    PUTCURS(locmem);
    lbc = 0;
    mode = DONOTHING;
    while ((ch = igetkey()) != EOF) {
#ifndef NINE_BUILD
    	if ((ch==KEY_TIMEOUT)&&(TDEFINE(TDEF_SPLITSCREEN)&&cmdmode!=GMENU)) {
            char buf[256], *t;
            struct fileheader* h;
            strcpy(buf, currdirect);
            if ((t = strrchr(buf, '/')) != NULL)
                *t = '\0';
            h = &pnt[(locmem->crs_line - locmem->top_line) * ssize];
            sprintf(genbuf, "%s/%s", buf, h->filename);
            if (strcmp(genbuf,lastfile)) {
            	draw_content(genbuf,h);
            update_endline();
            	strcpy(lastfile, genbuf);
            }
            move(0, 0);
            (*dotitle) ();
            PUTCURS(locmem);
	    continue;
    	} else 
#endif
    	if (ch == KEY_REFRESH) {
            mode = FULLUPDATE;

            /*
             * } else if( ch >= '0' && ch <= '9' ) {
             * if( lbc < 9 )
             * talkreply();
             * lbuf[ lbc++ ] = ch;
                                                                                                                                   *//*---	Modified by period	2000-09-11	---*/
        } else if ((ch >= '0' && ch <= '9')
                   || ((Ctrl('H') == ch || '\177' == ch) && (lbc > 0))) {
            if (Ctrl('H') == ch || '\177' == ch)
                lbuf[lbc--] = 0;

            else if (lbc < 9)
                lbuf[lbc++] = ch;
            lbuf[lbc] = 0;
            if (!lbc)
                update_endline();

            else if (DEFINE(currentuser, DEF_ENDLINE)) {
                extern time_t login_start_time;
                int allstay;
                char pntbuf[256], nullbuf[2] = " ";

                allstay = (time(0) - login_start_time) / 60;
                snprintf(pntbuf, 256, "\033[33;44m转到∶[\033[36m%9.9s\033[33m]" "  呼叫器[好友:%3s∶一般:%3s] 使用者[\033[36m%.12s\033[33m]%s停留[%3d:%2d]\033[m", lbuf, (!(uinfo.pager & FRIEND_PAGER)) ? "NO " : "YES", (uinfo.pager & ALL_PAGER) ? "YES" : "NO ", currentuser->userid,      /*13-strlen(currentuser->userid)
                                                                                                                                                                                                                                                                                                 * TODO:这个地方有问题，他想对齐，但是代码不对
                                                                                                                                                                                                                                                                                                 * , */ nullbuf,
                         (allstay / 60) % 1000, allstay % 60);
                move(t_lines - 1, 0);
                prints(pntbuf);
                clrtoeol();
            }

            /*---		---*/
        } else if (lbc > 0 && (ch == '\n' || ch == '\r')) {

            /*---	2000-09-11	---*/
            update_endline();

            /*---	---*/
            lbuf[lbc] = '\0';
            lbc = atoi(lbuf);
            if (cursor_pos(locmem, lbc, screen_len/2))
                mode = PARTUPDATE;
            lbc = 0;
        } else {

            /*---	2000-09-11	---*/
            if (lbc)
                update_endline();

            /*---	---*/
            lbc = 0;

            /*---	Modified by period	2000-11-12	---*
                   mode = i_read_key( rcmdlist, locmem, ch ,ssize);
             *---		---*/
            mode = i_read_key(cmdmode, rcmdlist, locmem, ch, ssize, pnt,ding_direct,currdirect);
            while (mode == READ_NEXT || mode == READ_PREV) {
                int reload;

                reload = move_cursor_line(locmem, mode);
                if (reload == -1) {
                    mode = FULLUPDATE;
                    break;
                } else if (reload) {
                    recbase = locmem->top_line;
    /* add by stiger */
    if (cmdmode != RMAIL && cmdmode != GMENU
         && (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
	    entries = read_get_records(currdirect,ding_direct, pnt, ssize, recbase, screen_len);
    else
    /* add end */
                    entries = get_records(currdirect, pnt, ssize, recbase, screen_len);
                    if (entries <= 0) {
                        last_line = -1;
                        break;
                    }
                }
                num = locmem->crs_line - locmem->top_line;

                /*---	Modified by period	2000-11-12	---*
                              mode = i_read_key( rcmdlist, locmem, ch ,ssize);
                 *---		---*/
                mode = i_read_key(cmdmode, rcmdlist, locmem, ch, ssize, pnt,ding_direct,currdirect);
            }
            modify_user_mode(cmdmode);
        }
        if ((mode == DOQUIT)||(mode == CHANGEMODE))
            break;
        if (mode == GOTO_NEXT) {
            cursor_pos(locmem, locmem->crs_line + 1, 1);
            mode = PARTUPDATE;
        }
        if (mode == NEWSCREEN) {
            last_line = get_num_records(currdirect, ssize);
    /* add by stiger */
    if (cmdmode != RMAIL && cmdmode != GMENU
         && (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
	    last_line += get_num_records(ding_direct,ssize);
    /* add end */
            num = last_line - screen_len + 2;
            locmem = getkeep(currdirect, num < 1 ? 1 : num, last_line);
            modify_locmem(locmem, last_line);
            if(locmem->crs_line-locmem->top_line>=screen_len-1) /*added by bad 2002.9.2*/
                locmem->crs_line = locmem->top_line;
    
            recbase = locmem->top_line;
    /* add by stiger */
    if (cmdmode != RMAIL && cmdmode != GMENU
         && (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
	    entries = read_get_records(currdirect,ding_direct, pnt, ssize, recbase, screen_len);
    else
    /* add end */
            entries = get_records(currdirect, pnt, ssize, recbase, screen_len);

            mode = FULLUPDATE;
        }
        switch (mode) {
        case NEWDIRECT:
        case DIRCHANGED:
    if ((cmdmode != RMAIL && cmdmode != GMENU)&&
        (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
    	sprintf(ding_direct,"boards/%s/%s",currboard->filename,DING_DIR);
	else
		ding_direct[0]='\0';
            recbase = -1;
            last_line = get_num_records(currdirect, ssize);
    /* add by stiger */
    if (cmdmode != RMAIL && cmdmode != GMENU
         && (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
	    last_line += get_num_records(ding_direct,ssize);
    /* add end */
            if (last_line == 0 && digestmode > 0) {
                if (digestmode == 7 || digestmode == 8)
                    unlink(currdirect);
                digestmode = 0;
                setbdir(digestmode, currdirect, currboard->filename);
            }
            if (mode == NEWDIRECT) {
                num = last_line - screen_len + 1;
                locmem = getkeep(currdirect, num < 1 ? 1 : num, last_line);
            }
        case FULLUPDATE:
        case PARTUPDATE:
            draw_title(dotitle);
            if (last_line < locmem->top_line + screen_len) {
                num = get_num_records(currdirect, ssize);
    /* add by stiger */
    if (cmdmode != RMAIL && cmdmode != GMENU
         && (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
	    num += get_num_records(ding_direct,ssize);
    /* add end */
                if (last_line != num) {
                    last_line = num;
                    recbase = -1;
                }
            }
            if (last_line == 0) {
                prints("No Messages\n");
                entries = 0;
            } else if (recbase != locmem->top_line) {
                recbase = locmem->top_line;
                if (recbase > last_line) {
                    recbase = last_line - screen_len / 2;
                    if (recbase < 1)
                        recbase = 1;
                    locmem->top_line = recbase;
                }
    /* add by stiger */
    if (cmdmode != RMAIL && cmdmode != GMENU
         && (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
	    entries = read_get_records(currdirect,ding_direct, pnt, ssize, recbase, screen_len);
    else
    /* add end */
                entries = get_records(currdirect, pnt, ssize, recbase, screen_len);
            }
            if (locmem->crs_line > last_line)
                locmem->crs_line = last_line;

            move(3, 0);
            draw_entry(doentry, locmem, entries, ssize, pnt);
            if (TDEFINE(TDEF_SPLITSCREEN)&&cmdmode!=GMENU) {
#ifdef NINE_BUILD
            char buf[256], *t;
            struct fileheader* h;
            strcpy(buf, currdirect);
            if ((t = strrchr(buf, '/')) != NULL)
                *t = '\0';
            h = &pnt[(locmem->crs_line - locmem->top_line) * ssize];
            sprintf(genbuf, "%s/%s", buf, h->filename);
            draw_content(genbuf,h);
            update_endline();
#else
                set_alarm(0,300*1000,NULL,NULL);
		lastfile[0]=0;
#endif
	    }
            PUTCURS(locmem);
            break;

        default:
            if (TDEFINE(TDEF_SPLITSCREEN)&&cmdmode!=GMENU) /*added by bad 2002.9.2*/ {
#ifdef NINE_BUILD
                char buf[256], *t;
                struct fileheader* h;
                strcpy(buf, currdirect);
                if ((t = strrchr(buf, '/')) != NULL)
                    *t = '\0';
                h = &pnt[(locmem->crs_line - locmem->top_line) * ssize];
                sprintf(genbuf, "%s/%s", buf, h->filename);
                draw_content(genbuf,h);
                update_endline();
#else
                set_alarm(0,300*1000,NULL,NULL);
#endif
            }
            RMVCURS(locmem);
            PUTCURS(locmem);
            break;
        }
        mode = DONOTHING;
        if (entries == 0)
            break;
    }
    clear();
    free(pnt);
    pnt = NULL;
    return mode;
}


static int i_read_key(int cmdmode, struct one_key *rcmdlist, struct keeploc *locmem, int ch, int ssize, char* pnt, char* ding_direct,char *direct)
{
    int i, mode = DONOTHING;

    switch (ch) {
    case Ctrl('Z'):
        r_lastmsg();            /* Leeward 98.07.30 support msgX */
        break;

    case 'q':
    case 'e':
    case KEY_LEFT:
        if (digestmode > 0) {
            if (digestmode == 7 || digestmode == 8)
                unlink(currdirect);
            digestmode = 0;
            setbdir(digestmode, currdirect, currboard->filename);
            return NEWDIRECT;
        }
        else
            return DOQUIT;
    case Ctrl('L'):
        redoscr();
        break;
    case 'k':
    case KEY_UP:
        if (cursor_pos(locmem, locmem->crs_line - 1, screen_len - 2))
            return PARTUPDATE;
        break;
    case 'j':
    case KEY_DOWN:
        if (cursor_pos(locmem, locmem->crs_line + 1, 0))
            return PARTUPDATE;
        break;
    case 'N':
    case Ctrl('F'):
    case KEY_PGDN:
    case ' ':
        if (last_line >= locmem->top_line + screen_len) {
            locmem->top_line += screen_len - 1;
            locmem->crs_line = locmem->top_line;
            return PARTUPDATE;
        }
        RMVCURS(locmem);
        locmem->crs_line = last_line;
        PUTCURS(locmem);
        break;
    case 'P':
    case Ctrl('B'):
    case KEY_PGUP:
        if (locmem->top_line > 1) {
            locmem->top_line -= screen_len - 1;
            if (locmem->top_line <= 0)
                locmem->top_line = 1;
            locmem->crs_line = locmem->top_line;
            return PARTUPDATE;
        } else {
            RMVCURS(locmem);
            locmem->crs_line = locmem->top_line;
            PUTCURS(locmem);
        }
        break;
    case KEY_HOME:
        locmem->top_line = 1;
        locmem->crs_line = 1;
        return PARTUPDATE;
    case '$':
    case KEY_END:
        if (last_line >= locmem->top_line + screen_len) {
            locmem->top_line = last_line - screen_len + 1;
            if (locmem->top_line <= 0)
                locmem->top_line = 1;
	/*modified by stiger */
/*            locmem->crs_line = last_line; */
    if (cmdmode != RMAIL && cmdmode != GMENU
         && (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
            locmem->crs_line = last_line - get_num_records(ding_direct,ssize);
	else
            locmem->crs_line = last_line;
	/*modified by stiger end */
            return PARTUPDATE;
        }
        RMVCURS(locmem);
	/*modified by stiger */
    if (cmdmode != RMAIL && cmdmode != GMENU
         && (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD))
        locmem->crs_line = last_line - get_num_records(ding_direct,ssize);
	else
        locmem->crs_line = last_line;
	/*modified by stiger end */
		if( (locmem->crs_line - locmem->top_line) <= 0){
			locmem->top_line = last_line - screen_len + 1;
			return PARTUPDATE;
		}
        PUTCURS(locmem);
        break;
    case 'L':
    case 'l':                  /* Luzi 1997.10.31 */
        if (uinfo.mode != LOOKMSGS) {
            show_allmsgs();
            return FULLUPDATE;
            break;
        }

        else
            return DONOTHING;
    case 'w':                  /* Luzi 1997.10.31 */
        if (!HAS_PERM(currentuser, PERM_PAGE))
            break;
        s_msg();
        return FULLUPDATE;
        break;
    case 'u':                  /* Haohmaru. 99.11.29 */
        clear();
        modify_user_mode(QUERY);
        t_query(NULL);
        return FULLUPDATE;
        break;
    case 'O':
    case 'o':                  /* Luzi 1997.10.31 */
        {                       /* Leeward 98.10.26 fix a bug by saving old mode */
            int savemode = uinfo.mode;

            if (!HAS_PERM(currentuser, PERM_BASIC))
                break;
            t_friends();
            modify_user_mode(savemode);
            return FULLUPDATE;
            break;
        }
    case ',':
        if(uinfo.mode==GMENU) break;
        if (TDEFINE(TDEF_SPLITSCREEN))
        	tmpuser&=~TDEF_SPLITSCREEN;
        else
        	tmpuser|=TDEF_SPLITSCREEN;
        screen_len = t_lines - 4;
        if (TDEFINE(TDEF_SPLITSCREEN))
            screen_len = screen_len/2-1;

/*        num = last_line - screen_len + 2;
//        locmem = getkeep(currdirect, num < 1 ? 1 : num, last_line);
//        modify_locmem(locmem, last_line);*/

        return NEWSCREEN;
        break;
    case '!':                  /*Haohmaru 1998.09.24 */
        Goodbye();
        return FULLUPDATE;
        break;
    case '\n':
    case '\r':
    case KEY_RIGHT:
            ch = 'r';

        /*
         * lookup command table 
         */
    default:
        for (i = 0; rcmdlist[i].fptr != NULL; i++) {
            if (rcmdlist[i].key == ch) {
                if (cmdmode != RMAIL && cmdmode != GMENU
                     && (digestmode==DIR_MODE_NORMAL||digestmode==DIR_MODE_THREAD)) {
		    /* add by stiger */
        		if( POSTFILE_BASENAME(((fileheader *)(pnt+(locmem->crs_line-locmem->top_line)*ssize))->filename)[0]=='Z' ){
        		    if(ch=='D' || ch=='b' || ch=='B') return DONOTHING;
        		    else if(ch=='s')
                            mode = (*(rcmdlist[i].fptr)) (locmem->crs_line - get_num_records(currdirect, ssize), &pnt[(locmem->crs_line - locmem->top_line) * ssize], direct );
        		    else
                            mode = (*(rcmdlist[i].fptr)) (locmem->crs_line - get_num_records(currdirect, ssize), &pnt[(locmem->crs_line - locmem->top_line) * ssize], ding_direct );
        		}
        		else
                            mode = (*(rcmdlist[i].fptr)) (locmem->crs_line, &pnt[(locmem->crs_line - locmem->top_line) * ssize], direct);
                }
                else
                      mode = (*(rcmdlist[i].fptr)) (locmem->crs_line, &pnt[(locmem->crs_line - locmem->top_line) * ssize], direct);
                break;
            }
        }
    }
    return mode;
}

int sread(int passonly, int readfirst, int pnum, int auser, struct fileheader *ptitle)
{
}

