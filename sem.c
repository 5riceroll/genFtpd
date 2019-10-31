#include "def.h"
#include "sem.h"

union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short *arry;
	struct seminfo *__buf;
};

/** create semaphore **/
int sem_create(key_t key)                 	
{
	int semid = semget(key, 1, 0666|IPC_CREAT);
	if (semid == -1)
		EXIT_ERR("sem_create semget");
		
	return semid;
}

/** open semaphore, named sem_open will be conflict **/
int sem_myopen(key_t key)                  
{
	int semid = semget(key, 0, 0);
	if (semid == -1)
		EXIT_ERR("sem_open semget");
	
	return semid;
}

/** semaphore semid-1 **/
int sem_p(int semid)                     	
{
	struct sembuf sb = {0, -1, SEM_UNDO};
	int ret = semop(semid, &sb, 1);
	if (ret == -1)
		EXIT_ERR("sem_p semop");
		
	return ret;
}

/** semaphore semid+1 **/
int sem_v(int semid)                  
{
	struct sembuf sb = {0, 1, SEM_UNDO};   
	int ret = semop(semid, &sb, 1);
	if (ret == -1)
		EXIT_ERR("sem_v semop");
		
	return ret;
}

/** semaphore delete **/
int sem_d(int semid)                     
{
	int ret = semctl(semid, 0, IPC_RMID, 0);
	if (ret == -1)
		EXIT_ERR("sem_d semctl");
	return ret;
}

/** set value of semaphore **/
int sem_setval(int semid, int val)      
{
	union semun sem;
	sem.val = val;
	int ret = semctl(semid, 0, SETVAL, sem);
	if (ret == -1)
		EXIT_ERR("sem_setval semctl");
	
	return ret;
}

/** get value by semid **/
int sem_getval(int semid)              
{
	int ret = semctl(semid, 0, GETVAL, 0);
	if (ret == -1)
		EXIT_ERR("sem_getval semctl");
		
	return ret;
}
