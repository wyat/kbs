#ifndef GLOBAL_VAR_H
#define GLOBAL_VAR_H

struct UTMPHEAD {
    int next[USHM_SIZE];
    int hashhead[UTMP_HASHSIZE + 1];    /* use UCACHE_HASHSIZE/32 */
    int number;
    int listhead;
    int list_prev[USHM_SIZE];   /* sorted list prev ptr */
    int list_next[USHM_SIZE];   /* sorted list next ptr */
    time_t uptime;
    int WORDBOUND, WHOLELINE, NOUPPER, INVERSE, FILENAMEONLY, SILENT, FNAME;
    int ONLYCOUNT;
};

/* global unique variable */
extern struct boardheader *bcache;
extern struct BCACHE *brdshm;
extern struct BDIRCACHE	*bdirshm;
extern struct UTMPFILE *utmpshm;
extern struct UTMPHEAD *utmphead;


typedef struct {
    struct userec *currentuser;
    struct usermemo *currentmemo;
    struct friends_info* topfriend;
    
    char fromhost[IPLEN + 1];

    struct favbrd_struct *favbrd_list=NULL;
    int *favbrd_list_count;
    struct favbrd_struct mybrd_list[FAVBOARDNUM];
    int mybrd_list_t = -1;
    int favnow = 0;
    
    int *zapbuf;
    int zapbuf_changed = 0;

    char MsgDesUid[20];
    char msgerr[255];
    
    int  num_of_matched;
    int total_line;
    char *CurrentFileName;
    
#ifdef SMS_SUPPORT
        struct sms_shm_head* head;
        void * smsbuf=NULL;
        int smsresult=0;
        struct user_info * smsuin;
#endif
} session_t;
#endif
