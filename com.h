#ifndef _COM_H
#define _COM_H
#include "hash.h"

/** typedef void (* HASH_FUN)(SESSION nowsession) **/
typedef struct _ddd
{
  char *mycom;
  HASH_FUN  myfun;
}COM_FUN;

extern COM_FUN mycomfun[];
extern char nowconnuser[10];
extern char up_com[10];
extern int port_port;
extern char port_ip[50];
extern int pasv_connsock;
extern int data_conn_sock;
extern char *pp;
extern long restdata;
void get_com(char *line, char *con);
void get_charm(char line[], char charm[]);
void com_to_hash();

void do_site(SESSION nowsession);
void do_user(SESSION nowsession);
void do_port(SESSION nowsession);
void do_dele(SESSION nowsession);
void do_list(SESSION nowsession);
void do_stor(SESSION nowsession);
void do_retr(SESSION nowsession);
void do_pass(SESSION nowsession);
void do_syst(SESSION nowsession);
void do_feat(SESSION nowsession);
void do_pwd(SESSION nowsession);
void do_cwd(SESSION nowsession);
void do_mkd(SESSION nowsession);
void do_rmd(SESSION nowsession);
void do_rnfr(SESSION nowsession);
void do_rnto(SESSION nowsession);
void do_noop(SESSION nowsession);
void do_rest(SESSION nowsession);
void do_type(SESSION nowsession);
void do_pasv(SESSION nowsession);
void do_quit(SESSION nowsession);

int test_pasv_or_port();
void initfun(SESSION nowsession);

#endif  /*_COM_H*/

