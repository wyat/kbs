/**
  *  ÐÂµÄread.c£¬Ê¹ÓÃselect½á¹¹ÖØÐ´Ô­À´µÄi_read
  * ÏÈÊµÏÖmail
  */
#include "bbs.h"


static int read_key(struct _select_def *conf, int command)
{
    struct read_arg *arg = (struct read_arg *) conf->arg;
    int i;
    int ret=SHOW_CONTINUE;
    
    for (i = 0; arg->rcmdlist[i].fptr != NULL; i++) {
        if (arg->rcmdlist[i].key == command) {
            int mode = (*(arg->rcmdlist[i].fptr)) (conf, arg->data+(conf->pos - conf->page_pos) * arg->ssize, arg->rcmdlist[i].arg);
            switch (mode) {
                case FULLUPDATE:
                case PARTUPDATE:
                    clear();
                    ret=SHOW_REFRESH;
                    break;
                case DIRCHANGED:
                case NEWDIRECT:
                    ret=SHOW_DIRCHANGE;
                    break;
                case DOQUIT:
                case CHANGEMODE:
                    ret=SHOW_QUIT;
                    break;
                case READ_NEXT:
                    if (conf->pos<conf->item_count) {
                        conf->new_pos = conf->pos + 1;
                        list_select_add_key(conf,'r'); //SEL changeµÄÏÂÒ»ÌõÖ¸ÁîÊÇread
                        ret=SHOW_SELCHANGE;
                    } else ret=SHOW_REFRESH;
                    break;
                case READ_PREV:
                    if (conf->pos>1) {
                        conf->new_pos = conf->pos - 1;
                        list_select_add_key(conf,'r'); //SEL changeµÄÏÂÒ»ÌõÖ¸ÁîÊÇread
                        ret=SHOW_SELCHANGE;
                    } else ret= SHOW_REFRESH;
                    break;
            } 
            break;
        }
    }
    return ret;
}

static int read_onselect(struct _select_def *conf)
{
    struct read_arg *arg = (struct read_arg *) conf->arg;
    return SHOW_CONTINUE;
}

static int read_getdata(struct _select_def *conf, int pos, int len)
{
    struct read_arg *arg = (struct read_arg *) conf->arg;
    int n;
    struct stat st;
    int entry=0;
    int count;

    if (arg->data==NULL)
        arg->data=calloc(BBS_PAGESIZE,arg->ssize);
    
    if (fstat(arg->fd,&st)!=-1) {
        count=st.st_size/arg->ssize;
        if (count!=conf->item_count) {
            //need refresh
            conf->item_count=count;
            arg->filecount=count;//TODO,ÖÃ¶¥µÄÊ±ºò£¬filecountºÍitem_count²»Ò»Ñù£¬ÉÙ¸öÖÃ¶¥
            return SHOW_DIRCHANGE;
        }
        
        if (lseek(arg->fd, arg->ssize * (pos - 1), SEEK_SET) != -1) {
            if ((n = read(arg->fd, arg->data, arg->ssize * len)) != -1) {
                entry=(n / arg->ssize);
            }
        }
    } else
        return SHOW_QUIT;
    return SHOW_CONTINUE;
}

static int read_title(struct _select_def *conf)
{
    struct read_arg *arg = (struct read_arg *) conf->arg;
    clear();
    (*arg->dotitle) ();
    return SHOW_CONTINUE;
}

static int read_endline(struct _select_def *conf)
{
    struct read_arg *arg = (struct read_arg *) conf->arg;

    if (!conf->tmpnum)
        update_endline();
    else if (DEFINE(currentuser, DEF_ENDLINE)) {
        extern time_t login_start_time;
        int allstay;
        char pntbuf[256], nullbuf[2] = " ";
        char lbuf[11];

        snprintf(lbuf,11,"%d",conf->tmpnum);

        allstay = (time(0) - login_start_time) / 60;
        snprintf(pntbuf, 256, "\033[33;44m×ªµ½¡Ã[\033[36m%9.9s\033[33m]" "  ºô½ÐÆ÷[ºÃÓÑ:%3s¡ÃÒ»°ã:%3s] Ê¹ÓÃÕß[\033[36m%.12s\033[33m]%sÍ£Áô[%3d:%2d]\033[m", 
            lbuf, (!(uinfo.pager & FRIEND_PAGER)) ? "NO " : "YES", (uinfo.pager & ALL_PAGER) ? "YES" : "NO ", currentuser->userid,      /*13-strlen(currentuser->userid)
                                                                                                                                                                                                                                                                                         * TODO:Õâ¸öµØ·½ÓÐÎÊÌâ£¬ËûÏë¶ÔÆë£¬µ«ÊÇ´úÂë²»¶Ô
                                                                                                                                                                                                                                                                                         * , */ nullbuf,
                 (allstay / 60) % 1000, allstay % 60);
        move(t_lines - 1, 0);
        prints(pntbuf);
        clrtoeol();
    }
    return SHOW_CONTINUE;
}

static int read_prekey(struct _select_def *conf, int *command)
{
    struct read_arg *arg = (struct read_arg *) conf->arg;
    return SHOW_CONTINUE;
}

static int read_show(struct _select_def *conf, int pos)
{
    struct read_arg *arg = (struct read_arg *) conf->arg;
    char foroutbuf[512];
    prints("%s",(*arg->doentry) (foroutbuf, pos, arg->data+(pos-conf->page_pos) * arg->ssize));
    clrtoeol();
    return SHOW_CONTINUE;
}

static int read_onsize(struct _select_def* conf)
{
    int i;
    struct read_arg *arg = (struct read_arg *) conf->arg;
    if (conf->item_pos!=NULL)
        free(conf->item_pos);
    conf->item_pos = (POINT *) calloc(BBS_PAGESIZE,sizeof(POINT));

    for (i = 0; i < BBS_PAGESIZE; i++) {
        conf->item_pos[i].x = 1;
        conf->item_pos[i].y = i + 3;
    };
    if (arg->data!=NULL) {
        free(arg->data);
        arg->data=NULL;
    }
    conf->item_per_page = BBS_PAGESIZE;
    return SHOW_DIRCHANGE;
}

int new_i_read(enum BBS_DIR_MODE cmdmode, char *direct, void (*dotitle) (), READ_FUNC doentry, struct one_key *rcmdlist, int ssize)
{
    struct _select_def read_conf;
    struct read_arg arg;
    int i;
    const struct key_translate ktab[]= {
            {'\n','r'},
            {'\r','r'},
            {KEY_RIGHT,'r'},
            {-1,-1}
    };


    modify_user_mode(cmdmode);

    /* save argument */
    arg.mode=cmdmode;
    arg.direct=direct;
    arg.dotitle=dotitle;
    arg.doentry=doentry;
    arg.rcmdlist=rcmdlist;
    arg.ssize=ssize;

    clear();

    if ((arg.fd = open(arg.direct, O_RDONLY, 0)) != -1) {
        bzero((char *) &read_conf, sizeof(struct _select_def));
        read_conf.item_per_page = BBS_PAGESIZE;
        read_conf.flag = LF_NUMSEL | LF_VSCROLL | LF_BELL | LF_LOOP | LF_MULTIPAGE;     /*|LF_HILIGHTSEL;*/
        read_conf.prompt = ">";
        read_conf.arg = &arg;
        read_conf.title_pos.x = 0;
        read_conf.title_pos.y = 0;
        read_conf.pos = 1; //TODO: get last position
        read_conf.page_pos = ((1-1)/BBS_PAGESIZE)*BBS_PAGESIZE+1; //TODO

        read_conf.get_data = read_getdata;

        read_conf.on_select = read_onselect;
        read_conf.show_data = read_show;
        read_conf.pre_key_command = read_prekey;
        read_conf.key_command = read_key;
        read_conf.show_title = read_title;
        read_conf.show_endline= read_endline;
        read_conf.on_size= read_onsize;
        read_conf.key_table = &(ktab[0]);

        read_getdata(&read_conf,read_conf.pos,read_conf.item_per_page);

        list_select_loop(&read_conf);
        if (arg.data!=NULL)
            free(arg.data);    
        if (read_conf.item_pos!=NULL)
            free(read_conf.item_pos);
        close(arg.fd);
    } else {
            prints("Ã»ÓÐÈÎºÎÐÅ¼þ...");
            pressreturn();
            clear();

    }
}


/* COMMAN : use mmap to speed up searching */
/* add by stiger
 * return :   2 :  DIRCHANGED
 *            1 :  FULLUPDATE
 *            0 :  DONOTHING
 */
static int read_search_articles(struct _select_def* conf, char *query, int offset, int aflag)
{
    char ptr[STRLEN];
    int now, match = 0;
    int complete_search;
    char upper_ptr[STRLEN], upper_query[STRLEN];
    bool init;
    size_t bm_search[256];

/*	int mmap_offset,mmap_length; */
    struct fileheader *pFh, *pFh1;
    int size;
    struct read_arg *arg = (struct read_arg *) conf->arg;

    get_upper_str(upper_query, query);
    if (aflag >= 2) {
        complete_search = 1;
        aflag -= 2;
    } else {
        complete_search = 0;
    }
    if (*query == '\0') {
        return 0;
    }

    /*
     * move(t_lines-1,0);
     * clrtoeol();
     * prints("[44m[33mËÑÑ°ÖÐ£¬ÇëÉÔºò....                                                             [m");
     */
    init=false;
    now = conf->pos;

/*    refresh();*/
    match = 0;
    BBS_TRY {
        if (safe_mmapfile_handle(arg->fd, O_RDONLY, PROT_READ, MAP_SHARED, (void **) &pFh, &size, NULL) == 0)
            BBS_RETURN(0);
        if(now > arg->filecount){
	/*ÔÚÖÃ¶¥ÎÄÕÂÇ°ËÑË÷*/
            now = arg->filecount;
        }
        if (now <= arg->filecount) {
            pFh1 = pFh + now - 1;
            while (1) {
                if (offset > 0) {
                    if (++now > arg->filecount)
                        break;
                    pFh1++;
                } else {
                    if (--now < 1)
                        break;
                    pFh1--;
                }
                if (now == arg->filecount)
                    break;
                if (aflag == -1) { /*ÄÚÈÝ¼ìË÷*/
                    char p_name[256];

                    if (uinfo.mode != RMAIL)
                        setbfile(p_name, currboard->filename, pFh1->filename);
                    else
                        setmailfile(p_name, currentuser->userid, pFh1->filename);
                    if (searchpattern(p_name, query)) {
                        match = 1;
                        break;
                    } else
                        continue;
                }
                strncpy(ptr, aflag ? pFh1->owner : pFh1->title, STRLEN - 1);
                ptr[STRLEN - 1] = 0;
                if (complete_search == 1) {
                    char *ptr2 = ptr;

                    if ((*ptr == 'R' || *ptr == 'r')

                        && (*(ptr + 1) == 'E' || *(ptr + 1) == 'e') && (*(ptr + 2) == ':')
                        && (*(ptr + 3) == ' ')) {
                        ptr2 = ptr + 4;
                    }
                    if (!strcmp(ptr2, query)) {
                        match = 1;
                        break;
                    }
                } else {
                    /*
                     * Í¬×÷Õß²éÑ¯¸Ä³ÉÍêÈ«Æ¥Åä by dong, 1998.9.12 
                     */
                    if (aflag == 1) {   /* ½øÐÐÍ¬×÷Õß²éÑ¯ */
                        if (!strcasecmp(ptr, upper_query)) {
                            match = 1;
                            break;
                        }
                    }

                    else if (bm_strcasestr_rp(ptr, upper_query,bm_search,&init) != NULL) {
                        match = 1;
                        break;
                    }
                }
            }
        }
    }
    BBS_CATCH {
        match = 0;
    }
    BBS_END
    end_mmapfile((void *) pFh, size, -1);
    move(t_lines - 1, 0);
    clrtoeol();
    if(match) {
        conf->pos=now;
        return 1;
    }
    return 0;
}


int auth_search(struct _select_def* conf, struct fileheader* fh, void* extraarg)
{
    static char author[IDLEN + 1];
    char ans[IDLEN + 1], pmt[STRLEN];
    char currauth[STRLEN];
    bool up=(bool)extraarg;
    
    strncpy(currauth, fh->owner, STRLEN);
    snprintf(pmt, STRLEN, "%sµÄÎÄÕÂËÑÑ°×÷Õß [%s]: ", offset > 0 ? "ÍùºóÀ´" : "ÍùÏÈÇ°", currauth);
    move(t_lines - 1, 0);
    clrtoeol();
    getdata(t_lines - 1, 0, pmt, ans, IDLEN + 1, DOECHO, NULL, true);   /*Haohmaru.98.09.29.ÐÞÕý×÷Õß²éÕÒÖ»ÄÜ11Î»IDµÄ´íÎó */
    if (ans[0] != '\0')
        strncpy(author, ans, IDLEN);

    else
        strcpy(author, currauth);
    switch (read_search_articles(conf, author, up, 1)) {
        case 1:
            return PARTUPDATE;
        default:
            conf->show_endline();
    }
    return DONOTHING;
}

