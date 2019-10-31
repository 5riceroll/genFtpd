#ifndef _NET_H
#define _NET_H

void clisession(int connfd);
int MyFtpdSer(char *conf);
void chld_handle(int signum);
void alrm_handle();
void check_com(char buf[]);
void exec_com(char buf[]);

extern long transfer_data_last;
extern int connfd;
extern int connsockpair[2];

#endif  /*_NET_H*/
