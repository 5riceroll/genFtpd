#ifndef _PUB_H
#define _PUB_H

typedef struct con_key
{
	char *key;
	char *val;
}CON_KEY;

typedef struct _dd
{
	int stor_count;       //number of uploaded files
	int retr_count;       //number of downloaded files
	long stor_byte;       //total number of bytes uploaded
	long retr_byte;       //total number of bytes downloaded
	int link_total_count; //number of connections 
}JWLSHM;

JWLSHM *myshm;
void create_shm();
int test_max_count(int way, long nret);
extern CON_KEY  mycon[10];
extern long transfer_data_now;
double limit_speed;

void speed_limit(char *key);
void close_timer();

int read_config(char *filename);
void check_config();
int mylist(int data_conn_sock, char *path);
int mystor(int data_conn_sock,  char *storname );
int myretr(int data_conn_sock, char *retrname );

int read_loop(int fd,void* buf,int size);
int write_loop(int fd,void* buf,int size);
int read_peek(int fd,void *buf,int len);
int read_line(int fd,char *buf,int maxlen);

void set_egid_euid(char *user);
int check_pass(char *username, char *pass);

int mydaemon(int ischdir, int isclose);

#endif  /*_PUB_H*/
