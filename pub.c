#include "def.h"
#include "pub.h"
#include "com.h"
#include "hash.h"
#include "net.h"
#include "sem.h"

CON_KEY mycon[10];
long transfer_data_now 	= 0;
double limit_speed 			=	0;

/** mydaemon or use system function daemon **/
int mydaemon(int ischdir, int isclose)
{
	pid_t pid = fork();    // the parent process may be the leader of process group,
	if (pid == -1)         // and you cannot create a new session.
	{
		EXIT_ERR("fork");
		return -1;
	}
	
	if (pid > 0)
		exit(1);						// parent process exit
	
	setsid();							// child process is not the leader of process group, you can create a new session by setsid.
	
	pid = fork();
	if (pid == -1)
	{
		EXIT_ERR("son fork");
		return -1;
	}
	
	if (pid > 0)					// child process exit, grandson process left
		exit(1);
	
	if (ischdir == 0)
		chdir("/"); 

	umask((mode_t)0);
	
	if (isclose == 0)
	{
		close(0);
		close(1);
		close(2);
	}
	
	return 0;
}

/** create share memory **/
void create_shm()
{
	int shmid = shmget((key_t)4444, sizeof(JWLSHM), IPC_CREAT | 0666);
	if(shmid == -1)
	{
		EXIT_ERR("shmid");
	}
	myshm = (JWLSHM *)shmat(shmid,NULL,0);
	if(myshm == (JWLSHM *)-1)
		EXIT_ERR("shm");
	myshm->stor_count				=	0;
	myshm->retr_count				=	0;
	myshm->stor_byte				=	0;
	myshm->retr_byte				=	0;
	myshm->link_total_count	=	0;
	int res	=	shmdt(myshm);
	if(res == -1)
		EXIT_ERR("shmdt");
}

/** count current connections,the number of uploaded,the number of downloaded,the bytes of uploaded,
 		the bytes of downloaded. **/
int test_max_count(int way, long nret)  
{
	int c;	
	int semid = sem_myopen((key_t)1234);
	sem_p(semid); 		
	int myshmid = shmget((key_t)4444, sizeof(JWLSHM), 0);
	if(myshmid == -1)
		EXIT_ERR("myshmid");
	JWLSHM *mymyshm = (JWLSHM *)shmat(myshmid, NULL, 0);
	if(mymyshm == (JWLSHM *)-1)
		EXIT_ERR("shm");
	if(way == 0)
		c = mymyshm->link_total_count;
	else if(way == 1)
		mymyshm->link_total_count++;
	else if(way == 2)
		mymyshm->link_total_count--;
	else if(way == 3)
	{
		mymyshm->stor_count ++;
		printf("now stor count = [%d]\n", mymyshm->stor_count);
	}
	else if(way == 4)
	{
		mymyshm->retr_count++;
		printf("now retr count = [%d]\n", mymyshm->retr_count);
	}
	else if(way == 5)
	{
		mymyshm->stor_byte += nret;
		printf("now stor_byte count = [%ld]\n", mymyshm->stor_byte);
	}
	else if(way == 6)
	{
		mymyshm->retr_byte += nret;	
		printf("now retr_byte = [%ld]\n", mymyshm->retr_byte);
	}
	
	int res = shmdt(mymyshm);
	if(res == -1)
		EXIT_ERR("shmdt");

	sem_v(semid);
	
	return c;
}

/** set limit_speed, and timer 1ms **/
void speed_limit(char *key)
{
	struct itimerval val;
	struct timeval myval;
	memset(&val, 0, sizeof(val));
	memset(&myval, 0, sizeof(myval));
	
	myval.tv_sec 		= 0;
	myval.tv_usec 	= 1000;
	val.it_interval = myval;
	memset(&myval, 0, sizeof(myval));
	myval.tv_sec 	= 0;
	myval.tv_usec = 1000;
	val.it_value 	= myval;
	
	int ret = setitimer(ITIMER_REAL, &val, NULL);   	// setitimer trigger SIGALRM
	if (ret == -1)
		EXIT_ERR("setitimer");
	
	char *p = (char *)hash_getval(hash_conf, key, 1);
	limit_speed = atof(p) * 1024;
}

/** close timer **/
void close_timer()
{
	int ret = setitimer(ITIMER_REAL, NULL, NULL);
	if (ret == -1)
	{
		EXIT_ERR("close_timer setitimer");
	}
}

/** upload process function **/
int mystor(int data_conn_sock,  char *storname)
{
	int fd;
	int n;
	char bufstor[1024] = "";
	long storbytecount = 0;
	if (restdata != 0)
	{
		fd = open(storname, O_WRONLY);
		off_t flag = lseek(fd, restdata, SEEK_SET);
		restdata 	 = 0;
		if (flag == -1)
			EXIT_ERR("lseek");
	}
	else
	{
		fd = open(storname, O_WRONLY|O_CREAT, 0666);
	}
	if (fd == -1)
		EXIT_ERR("stor open");
	speed_limit("up");	
	transfer_data_now  = 0;
	transfer_data_last = 0;
	while(1)
	{
		memset(bufstor, 0, sizeof(bufstor));
		n = read(data_conn_sock, bufstor, sizeof(bufstor));
		storbytecount += n;
		transfer_data_now = storbytecount;
		if (n == -1)
		{	
			if(errno == EINTR)
				continue;
			else
				return -1;
		}
		if (n == 0)
			break;
		write_loop(fd, bufstor, n);
	}
	close_timer();
	test_max_count(5, storbytecount);
	close(fd);
	
	return 0;
}

/** download process function **/
int myretr(int data_conn_sock, char *retrname )
{
	int fd;
	int n;
	long retrbytecount = 0;
	char bufretr[1024] = "";
	if (restdata != 0)
	{
		fd = open(retrname, O_RDONLY);
		off_t flag = lseek(fd, restdata, SEEK_SET);
		restdata = 0;
		if (flag == -1)
			EXIT_ERR("lseek");
	}
	else
		fd = open(retrname, O_RDONLY, 0666);
	if (fd == -1)
		EXIT_ERR("retr open");	
	speed_limit("down");	
	transfer_data_now = 0;
	transfer_data_last = 0;
	while(1)
	{	
		memset(bufretr, 0, sizeof(bufretr));
		n = read(fd, bufretr, sizeof(bufretr));
		retrbytecount += n;
		transfer_data_now = retrbytecount;
		if (n == -1)
		{	
			if(errno == EINTR)
				continue;
			else
				return -1;
		}
		if (n == 0)
			break;
		write_loop(data_conn_sock, bufretr, n);	
	}
	
	close_timer();
	test_max_count(6, retrbytecount);
	close(fd);
	
	return 0;
}

/** list files and dir **/
int mylist(int data_conn_sock, char *path)
{
	chdir(path);
	char listfile[1024] = "";
	DIR *dir = opendir(path);
	if(dir == NULL)
	{
		printf("this dir = NULL\n");
		EXIT_ERR("opendir");
	}

	struct dirent *p;
	while((p = readdir(dir)) != NULL)
	{
		if(*(p->d_name) == '.' )
			continue;	
			
		struct stat buf;
		memset(&buf, 0, sizeof(buf));
		int ret = lstat(p->d_name, &buf);
		if( ret == -1)
			continue;
			
		char mymode[11] = "----------";
		switch(buf.st_mode & S_IFMT)
		{
			case S_IFREG: mymode[0] ='-'; break;
			case S_IFBLK: mymode[0] ='b'; break;
			case S_IFDIR: mymode[0] ='d'; break;			  
			case S_IFCHR: mymode[0] ='c'; break;			  
			case S_IFIFO: mymode[0] ='p'; break;
			case S_IFLNK: mymode[0] ='l'; break;
			default     : mymode[0] ='0'; break;
		}

		mymode[1] = buf.st_mode & S_IRUSR ? 'r': '-';
		mymode[2] = buf.st_mode & S_IWUSR ? 'w': '-';
		mymode[3] = buf.st_mode & S_IXUSR ? 'x': '-';
		mymode[4] = buf.st_mode & S_IRGRP ? 'r': '-';
		mymode[5] = buf.st_mode & S_IWGRP ? 'w': '-';
		mymode[6] = buf.st_mode & S_IXGRP ? 'x': '-';
		mymode[7] = buf.st_mode & S_IROTH ? 'r': '-';
		mymode[8] = buf.st_mode & S_IWOTH ? 'w': '-';
		mymode[9] = buf.st_mode & S_IXOTH ? 'x': '-';
		
		struct passwd *ur_passwd;
		ur_passwd = getpwuid(buf.st_uid); 
		char szUid[16] = { 0 };
		sprintf(szUid, "%u", buf.st_uid);
				
		struct group *g_group;
		g_group = getgrgid(buf.st_gid);
		char szGid[16] = { 0 };
		sprintf(szGid, "%u", buf.st_gid);
			
		if((mymode[0] == '-') || (mymode[0] == 'd') || (mymode[0] == 'p') ||(mymode[0] == 'l'))
		{
			sprintf(listfile, "%s %4ld %s %s %14d", mymode, (long)buf.st_nlink, ur_passwd != NULL? ur_passwd->pw_name : szUid,  
				g_group != NULL? g_group->gr_name : szGid, (int)buf.st_size);
		}

		if((mymode[0] == 'b') || (mymode[0] == 'c'))
		{
			int primary_id;
			int secondary_id;
			primary_id = buf.st_rdev / 256;
			secondary_id = buf.st_rdev % 256;
			sprintf(listfile, "%s %4ld %s %s %4d %4d", mymode, (long)buf.st_nlink, ur_passwd != NULL? ur_passwd->pw_name : szUid,
				g_group != NULL? g_group->gr_name : szGid, primary_id, secondary_id);
		}
    
		struct tm *file_time;
		struct tm filetime;
		memset(&filetime, 0, sizeof(filetime));
		file_time = localtime_r(&buf.st_ctime, &filetime);

		int nTimeNowSecs = time((time_t *)NULL);
		nTimeNowSecs -= 15552000; // 180 days
		int nFileSecs = mktime(file_time);

		char timestr[64] = "";
		if (nFileSecs - (nTimeNowSecs - 15552000) < 0)                  // 15552000 secs = 180 days, to judge whether file time is near half a year
		{
			strftime(timestr, sizeof(timestr), "%b %d %Y", file_time);
			sprintf(listfile, "%s %s", listfile, timestr);
		}
		else
		{
			strftime(timestr, sizeof(timestr), "%b %d %H:%M", file_time);
			sprintf(listfile, "%s %s", listfile, timestr);
		}

		if( mymode[0] == 'l' )
		{
			char slink[10] = "";
			readlink(p->d_name, slink, 10);
			sprintf(listfile, "%s %s->%s", listfile, p->d_name, slink);
		}
		else
		{
			sprintf(listfile, "%s %s", listfile, p->d_name);
		}
		sprintf(listfile, "%s\r\n", listfile); 
		write_loop(data_conn_sock, listfile, strlen(listfile));
	}

	closedir(dir);
	return EXIT_SUCCESS;
}

/** read configuration into hash table or arry **/
int read_config(char *filename)
{
	FILE *fp 	= NULL;	
	char *key = NULL;
	char *val = NULL;
	char *ch  = NULL;
	char *tem = NULL;
	int i 		= 0;
	char *str = NULL;
	str = (char *)malloc(64);
	memset(str, 0x0, 64);
	
	fp = fopen(filename, "r");
	if(fp == NULL)
	{
			printf("fopen error!");
			return EXIT_FAILURE;
	}

	while(fgets(str, 64, fp) != NULL)
	{
		if(*str == '#')
				continue;
		else
		{
			ch = strchr(str, '=');			
			if(ch != NULL)
			{
				*ch = '\0';
				key = str;
			  //mycon[i].key = str;
				val = ch + 2;
			  //mycon[i].val = val;
				tem = strchr(val, ')');
				if(tem != NULL)
					*tem = '\0';
			}
			if(NULL != key && NULL != val)
				hash_insert(hash_conf, key, val, 0, NULL); 								    // insert configuration to hash table only if key is not null and val is not null
			else
				EXIT_ERR("read_config, config file error!");	 
		  //hash_insert(hash_conf, mycon[i].key, mycon[i].val, 0, NULL);  // insert configuration to arry mycon
			i++;
		}
		//str = (char *)malloc(64);	
		memset(str, 0x0, 64);
	}
	
	free(str);
	str = NULL;
	return EXIT_SUCCESS;
}

void check_config()
{
	if(//NULL == (char*)hash_getval(hash_conf, "serverip", 1)   			  // ser addr set INADDR_ANY
	   NULL == (char*)hash_getval(hash_conf, "serverport", 1)
	|| NULL == (char*)hash_getval(hash_conf, "servermaxip", 1)
	|| NULL == (char*)hash_getval(hash_conf, "permaxip", 1)
	|| NULL == (char*)hash_getval(hash_conf, "down", 1)
	|| NULL == (char*)hash_getval(hash_conf, "up", 1))
		EXIT_ERR("check_config, config file error!");
}

/** read size data until finished **/
int read_loop(int fd, void *buf, int size)
{
	int ret      = -1;
	int num_read = 0;
	while (1)
	{
		ret = read(fd, buf + num_read, size);
		if (ret == -1)
		{
			if(errno == EINTR)
				continue;
			else
				return  -1;
		}
	
	if (ret == 0)
		return num_read;

	num_read += ret;
	size -= ret;
	if (size == 0)
		return num_read;
	}
}

/** write data until finished **/
int write_loop(int fd, void *buf, int size)
{
	int ret					= -1;
	int num_written = 0;
	
	while (1)
	{
		ret = write(fd, buf + num_written, size);
		if (ret == -1)
		{
			if (errno == EINTR)
				continue;
			else
				return -1;
		}
		if (ret == 0)	
			continue;
		
		num_written += ret;
		size -= ret;
		if (size == 0)
			return num_written;
	}
}

/** read_peek is different from read **/
int read_peek(int fd, void *buf, int len)
{
	int ret = 0;
	for(;;)
	{
		ret = recv(fd, buf, len, MSG_PEEK);
		if(ret == -1 && errno == EINTR)
			continue;
		else
			return ret;
	}
}

/** read data line by line **/
int read_line(int fd, char *buf, int maxlen)
{
	char *p 		= buf;
	int num_get = maxlen, i, num_read, retval;
	for(;;)
	{
		retval = read_peek(fd, p, num_get);
		if (retval <= 0)
			return retval;
		
		num_read = retval;	
		for(i = 0; i < num_read; i++)
		{
			if (p[i] == '\n')
			{
				retval = read_loop(fd, p, i+1);
				if (retval != i+1)
					return -1;
					
				p = strchr(buf, '\n');
				*(++p) = '\0';
				return retval;
			}
		}
		
		num_get -= num_read;
		retval = read_loop(fd, p, num_read);
		if (retval != num_read)
			return -1;
		p += num_read;
	}
}

void set_egid_euid(char *user)
{
	struct passwd *p = getpwnam(user);
	if(p != NULL)
	{
		setegid(p->pw_gid);
		seteuid(p->pw_uid);
	}
}

/** check username and password **/
int check_pass(char *username, char *pass)
{
	struct spwd *p = getspnam(username);
	if( p != NULL)
	{
		char s[13] = "";
		strncpy(s, p->sp_pwdp, 12);
		if(strcmp(p->sp_pwdp, (char *)crypt(pass, s)) == 0)
		{
			set_egid_euid(username);
			chdir(getpwnam(username)->pw_dir);
			return 1;
		}
		else
			return -1;
	}
	
	return -1;
}
