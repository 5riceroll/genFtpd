#ifndef _HASH_H
#define _HASH_H
#define HASH_MAX 101

typedef struct	_sess
{
	char *com;     // command from client
	char *charm;   // parameter after command from client
	char *dir;     // current dir
}SESSION;

/*
typedef struct _d
{
  char com[10];
  char charm[100];
	char dir[100];
}SESSION;
*/
typedef void (*HASH_FUN)(SESSION nowsession);
typedef struct _hash
{
	char *key;
	char *val;
	HASH_FUN fun;
	struct _hash *next;
	struct _hash *prev;
}HASHNODE;

extern HASHNODE *hash_conf[HASH_MAX];                 // configuration
extern HASHNODE *hash_pid_ip[HASH_MAX];        				// process id of client and its corresponding IP address
extern HASHNODE *hash_ip_count[HASH_MAX];      				// per IP connections 
extern HASHNODE *hash_comm[HASH_MAX];      						// Command execution function

unsigned int hash_fun(char *key);            									
HASHNODE * hash_search(HASHNODE *hash_table[],char *key);						
int hash_insert(HASHNODE *hash_table[],char *key,char *val,int way, HASH_FUN fun1);
int hash_del(HASHNODE *hash_table[],char *key,int way);							
void* hash_getval(HASHNODE *hash_table[],char *key,int way);				
void free_hash_node(HASHNODE *hash_test[]);

#endif  /*_HASH_H*/
