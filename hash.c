#include "hash.h"
#include "def.h"

HASHNODE *hash_conf[HASH_MAX]			= {NULL};     // configuration
HASHNODE *hash_pid_ip[HASH_MAX]		=	{NULL};   	// client pid to IP
HASHNODE *hash_ip_count[HASH_MAX]	=	{NULL}; 		// connections per IP
HASHNODE *hash_comm[HASH_MAX]			=	{NULL};	  	// command to command process function

/** hash function, caculate Subscript **/
unsigned int hash_fun(char *key)
{
	unsigned int index = 0;
	while(*key)
	{
		index = *key + 5*index;
		key++;
	}
	
	return index % HASH_MAX;
}

HASHNODE * hash_search(HASHNODE *hash_table[], char *key)
{
	unsigned int index;
	index = hash_fun(key);
	HASHNODE *p = NULL;
	p = hash_table[index];
	for(; p != NULL; p = p->next)
	{
		if(strcmp(p->key, key) == 0)
			return p;
	}
	
	return p;
}

/** hash function, insert **/
int hash_insert(HASHNODE *hash_table[], char *key, char *val, int way, HASH_FUN fun1)
{
	HASHNODE * p = hash_search(hash_table, key);
	int index,tempi = 0;
	char tempstr[10];
	if(p == NULL)
	{
		p = (HASHNODE *)malloc(sizeof(HASHNODE));
		if(p == NULL)
		{
			perror("malloc");
			return -1 ;
		}
		memset(p, 0, sizeof(HASHNODE));
		p->key = (char *)strdup(key);
		if(p->key == NULL)
		{
			free(p);
			return -1;
		}
		if(way ==  0) // hash_conf,hash_pid_ip
		{
			p->val =(char *)strdup(val);
			if(p->val == NULL)
			{
				free(p->key);
				free(p);
				return -1;
			}
		}
		else if(way == 1) // hash_ip_count
		{
			char str[2];
			sprintf(str, "%d", 1);
			p->val = (char *)strdup(str);
			if(p->val == NULL)
			{
				free(p->key);
				free(p);
				return -1;
			}
		}
		else if(way == 2) // hash_fun
			p->fun = fun1;  // (HASH_FUN)val

		p->next = NULL;
		p->prev = NULL;

		index = hash_fun(key);
		if(hash_table[index] == NULL)
			hash_table[index] = p;
		else
		{
			p->next = hash_table[index];      // head insert
			hash_table[index] = p;
		}
	}
	else // p is not null, ip_count + 1
	{
		tempi = atoi(p->val);
		tempi += 1;
		free(p->val);
		sprintf(tempstr, "%d", tempi);
		p->val = (char *)strdup(tempstr);
	}
	return 0;
}

int hash_del(HASHNODE *hash_table[], char *key, int way)
{
	int tempi = 0;
	char tempstr[20];
	HASHNODE *p = hash_search(hash_table, key);
	if(p == NULL)
		return -1;
	if(way == 1) // per IP connections -1
	{
		tempi = atoi(p->val);
		tempi -= 1;
		free(p->val);
		sprintf(tempstr, "%d", tempi);
		p->val = (char *)strdup(tempstr);
	}
	else
	{
		unsigned int index = hash_fun(key);
		if(p == hash_table[index]) // delete head
		{
			if(p->next == NULL)      // only one node
				hash_table[index] = NULL;
			else                     // more than one node
				hash_table[index] = p->next;
		}
		else                       // delete other node, not head node
		{
			p->prev->next = p->next; // not last node
			if(p->next == NULL)      // delete last node
				p->prev->next = NULL;
		}
		free(p->key);
		free(p->val);
		free(p);
		p = NULL;
	}
	
	return 0;
}

/** get value by key **/
void* hash_getval(HASHNODE *hash_table[], char *key, int way)
{
	HASHNODE* p = hash_search(hash_table, key);
	if(p == NULL)
		return NULL;
		
	if(way == 1)
		return (void *)p->val;
	else if(way == 2)
		return (void *)p->fun;

	return NULL;
}

/** free all hash nodes **/
void free_hash_node(HASHNODE *hash_test[])
{
	int i;
	HASHNODE *p, *t;
	for(i = 0; i < HASH_MAX; i++)
	{
		if(hash_test[i] != NULL)
		{
			p = hash_test[i];
			while(p)
			{
				t = p;
				p = p->next;
				free(t);
			}
			hash_test[i] = NULL;
		}
	}
}
