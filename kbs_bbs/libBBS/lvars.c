#include "bbs.h"

struct boardheader *bcache;
struct BCACHE *brdshm;
struct BDIRCACHE	*bdirshm;
struct UTMPFILE *utmpshm;
struct UTMPHEAD *utmphead;

int WORDBOUND, WHOLELINE, NOUPPER, INVERSE, FILENAMEONLY, SILENT, FNAME;
int ONLYCOUNT;

struct UCACHE *uidshm = NULL;

/*log.c*/
int disablelog = 0;
int logmsqid = -1;

struct public_data *publicshm;

#ifndef THREAD
session_t session;
#endif
void init_sessiondata(session_t * session)
{
    bzero(session,sizeof(*session));
#ifdef HAVE_BRC_CONTROL
#if USE_TMPFS==1
    session->brc_cache_entry=NULL;
#endif
    session->brc_currcache=-1;
#endif

    session->zapbuf_changed = 0;

    session->mybrd_list_t = -1;
    session->favnow = 0;
    session->favbrd_list = NULL;

    session->currentuser=NULL;
    session->currentmemo=NULL;
    session->topfriend=NULL;

    session->sigjmp_stack=NULL;

#ifdef SMS_SUPPORT    
    session->lastsmsstatus=0;
#endif

    session->utmpent=-1;
}

session_t* getSession()
{
  return &session;
}
