/**
  *  新的read.c，使用select结构重写原来的i_read
  * 先实现mail
  */
#include "bbs.h"

struct read_arg {
  /* save argument */
  enum BBS_DIR_MODE mode;
  char* direct;
  void (*dotitle) ();
  READ_FUNC doentry;
  struct one_key *rcmdlist;
  int ssize;

  void* data; //readed data
  int fd; //filehandle,open always

  bool reading; //用于表示返回READ_NEXT,READ_PREV的时候直接读写
};

static int read_key(struct _select_def *conf, int command)
{
    struct read_arg *arg = (struct read_arg *) conf->arg;
    int i;
    int ret=SHOW_CONTINUE;
    
    for (i = 0; arg->rcmdlist[i].fptr != NULL; i++) {
        if (arg->rcmdlist[i].key == command) {
            int mode = (*(arg->rcmdlist[i].fptr)) (conf->pos, arg->data+(conf->pos - conf->page_pos) * arg->ssize, arg->direct);
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
                        list_select_add_key(conf,'r'); //SEL change的下一条指令是read
                        ret=SHOW_SELCHANGE;
                    } else ret=SHOW_REFRESH;
                    break;
                case READ_PREV:
                    if (conf->pos>1) {
                        conf->new_pos = conf->pos - 1;
                        list_select_add_key(conf,'r'); //SEL change的下一条指令是read
                        ret=SHOW_SELCHANGE;
                    }
                    ret= SHOW_REFRESH;
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

    if (arg.data==NULL)
        arg.data=calloc(BBS_PAGESIZE,ssize);
    
    if (fstat(arg->fd,&st)!=-1) {
        count=st.st_size/arg->ssize;
        if (count!=conf->item_count) {
            //need refresh
            conf->item_count=count;
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
        snprintf(pntbuf, 256, "\033[33;44m转到∶[\033[36m%9.9s\033[33m]" "  呼叫器[好友:%3s∶一般:%3s] 使用者[\033[36m%.12s\033[33m]%s停留[%3d:%2d]\033[m", 
            lbuf, (!(uinfo.pager & FRIEND_PAGER)) ? "NO " : "YES", (uinfo.pager & ALL_PAGER) ? "YES" : "NO ", currentuser->userid,      /*13-strlen(currentuser->userid)
                                                                                                                                                                                                                                                                                         * TODO:这个地方有问题，他想对齐，但是代码不对
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
        read_conf.item_pos = pts;
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
            prints("没有任何信件...");
            pressreturn();
            clear();

    }
}


