#include "daemon.h"
/*
typedef struct daemoncmd {
	char *cmdname;
	char *usage;
	int argc;
	int (*main) ARG((FILE*,FILE*,int,char**,char*));
} daemoncmd_t;

*/

void deargify ARG((char ***));
static daemoncmd_t *dcmdp = NULL;
static char *startupmessage = NULL;
static int startupcode = 100;
static FILE *DIN, *DOUT, *DIO;
typedef int (*F) ();

void installdaemon(cmds, code, startupmsg)
daemoncmd_t *cmds;
int code;
char *startupmsg;
{
    dcmdp = cmds;
    startupcode = code;
    startupmessage = startupmsg;
}

daemoncmd_t *searchcmd(cmd)
char *cmd;
{
    daemoncmd_t *p;

    for (p = dcmdp; p->name != NULL; p++) {
#ifdef DEBUGCMD
        printf("searching name %s for cmd %s\n", p->name, cmd);
#endif
        if (!strncasecmp(p->name, cmd, 1024))
            return p;
    }
    return NULL;
}

#define MAX_ARG 32
#define MAX_ARG_SIZE 16384

int argify(line, argvp)
char *line, ***argvp;
{
    static char *argvbuffer[MAX_ARG + 2];
    char **argv = argvbuffer;
    int i;
    static char argifybuffer[MAX_ARG_SIZE];
    char *p;

    while (strchr("\t\n\r ", *line))
        line++;
    i = strlen(line);
    /*
     * p=(char*) mymalloc(i+1); 
     */
    p = argifybuffer;
    strncpy(p, line, sizeof argifybuffer);
    for (*argvp = argv, i = 0; *p && i < MAX_ARG;) {
        for (*argv++ = p; *p && !strchr("\t\r\n ", *p); p++);
        if (*p == '\0')
            break;
        for (*p++ = '\0'; strchr("\t\r\n ", *p) && *p; p++);
    }
    *argv = NULL;
    return argv - *argvp;
}

void deargify(argv)
char ***argv;
{
    return;
    /*
     * if (*argv != NULL) { if (*argv[0] != NULL){ free(*argv[0]);
     * argv[0] = NULL; } free(*argv); argv = NULL; }
     */
}

/*
 * Disabled by flyriver, 2004.2.25
 *
int daemonprintf(format)
char *format;
{
    fprintf(DOUT, format);
    fflush(DOUT);
}
*/

int daemonputs(output)
char *output;
{
    fputs(output, DOUT);
    fflush(DOUT);
}
