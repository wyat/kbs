#ifndef __READ_H__
#define __READ_H__

typedef int (*READ_KEY_FUNC)(struct _select_def*,void*,void*);

struct key_command {                /* Used to pass commands to the readmenu */
    int key;
    READ_KEY_FUNC fptr;
    void* arg;
};

struct read_arg {
  /* save argument */
  enum BBS_DIR_MODE mode;
  char* direct;
  void (*dotitle) ();
  READ_FUNC doentry;
  struct key_command *rcmdlist;
  int ssize;

  void* data; //readed data
  int fd; //filehandle,open always

  int filecount; //the item count of file
};

int new_i_read(enum BBS_DIR_MODE cmdmode, char *direct, void (*dotitle) (), READ_FUNC doentry, struct key_command *rcmdlist, int ssize);

/* some function for fileheader */
int auth_search(struct _select_def* conf, struct fileheader *fileinfo,void* extraarg);

#endif  //__READ_H__
