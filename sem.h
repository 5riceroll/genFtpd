#ifndef _SEM_H
#define _SEM_H

int sem_create(key_t key);
int sem_myopen(key_t key);
int sem_p(int semid);
int sem_v(int semid);
int sem_d(int semid);
int sem_setval(int semid,int val);
int sem_getval(int semid);

#endif  /*_SEM_H*/
