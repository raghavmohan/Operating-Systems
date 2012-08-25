#ifndef __REQUEST_H__

/*
typedef struct 
{
  int is_static;
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;
} requestStats;
*/



void requestHandle(server_request stats);
void requestError(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) ;
void requestReadhdrs(rio_t *rp);
int requestParseURI(char *uri, char *filename, char *cgiargs);
server_request getFileStat(int fd);
int getSize(int fd);
#endif
