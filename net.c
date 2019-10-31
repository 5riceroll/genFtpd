#include "net.h"
#include "def.h"
#include "com.h"
#include "pub.h"
#include "sem.h"
#include "hash.h"

int connfd;
int connsockpair[2];
long transfer_data_last = 0;

int MyFtpdSer(char *conf)
{	
	int semid = sem_create((key_t)1234);
	sem_setval(semid, 1);                       
	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	if(listenfd == -1)
		EXIT_ERR("socket");
	
	int opt = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = PF_INET;
	
	read_config(conf);	// read configuration into hash table 
	check_config();     // check configuration
	
	/*int len;
	len = sizeof(mycomfun)/sizeof(mycomfun[0]);             
	for (i = 0; i < 10; i++)		                          
	hash_insert(hash_comm, mycomfun[i].mycom, NULL, 2, mycomfun[i].myfun);*/
	
	com_to_hash();			// command process function into hash table
	int maxip = 0;
	char *q = (char *)hash_getval(hash_conf, "servermaxip", 1);
	if (q != NULL)
		maxip = atoi(q);
	printf(" ******test maxip = [%d]******\n", maxip);

	char *m = (char *)hash_getval(hash_conf, "serverport", 1);
	if (m != NULL)
	{
		servaddr.sin_port = htons(atoi(m));
	}

	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	socklen_t servlen = sizeof(servaddr);
	int res = bind(listenfd, (struct sockaddr*)&servaddr, servlen);
	if(res == -1)
	{
		EXIT_ERR("bind");
	}
	signal(SIGCHLD, chld_handle);
	signal(SIGALRM, alrm_handle);
	pid_t pid;
	char tem[20] = "";
	
	int ret = listen(listenfd, 5);
	if(ret == -1)
		EXIT_ERR("listen");
		
	while(1)
	{
		struct sockaddr_in cliaddr;
		memset(&cliaddr, 0, sizeof(cliaddr));
		socklen_t clilen = sizeof(cliaddr);
		connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
		if(connfd == -1)
			EXIT_ERR("accept");
		
		printf("cli ip=[%s] port=%d\n", inet_ntoa(cliaddr.sin_addr), htons(cliaddr.sin_port));
	
		int nowmaxip = 0;
		nowmaxip = test_max_count(0, 0);
		if (nowmaxip >= maxip)
		{
			sleep(4);
			write_loop(connfd,"421 Connection refused: too many sessions\r\n", strlen("421 Connection refused: too many sessions\r\n"));
			close(connfd);
			continue;
		}
		
		int permax;		
		char *p;
		p = (char*)hash_getval(hash_conf, "permaxip", 1);
		if (p != NULL)
			permax = atoi(p);
		printf("*****hash test perip = [%d]*******\n", permax);
		
		int nowcount = 0;
		char *k = (char *)hash_getval(hash_ip_count, inet_ntoa(cliaddr.sin_addr), 1);
		if (k != NULL)
			nowcount = atoi(k);
		else
			nowcount = 0;
		if (nowcount >= permax)
		{
			write_loop(connfd, "Connection refused: too many sessions for this address.\r\n", 
			strlen("Connection refused: too many sessions for this address.\r\n"));
			close(connfd);
			sleep(1);
			continue;
		}
		write_loop(connfd, "220 hello\r\n", strlen("220 hello\r\n"));

		pid = fork();
		switch(pid)
		{
			case -1:
					return -1;
			case  0:
					close(listenfd);
					clisession(connfd);
			default:
					hash_insert(hash_ip_count, inet_ntoa(cliaddr.sin_addr), NULL, 1, NULL);
					memset(tem, 0, sizeof(tem));
					sprintf(tem, "%d", pid);
					hash_insert(hash_pid_ip, tem, inet_ntoa(cliaddr.sin_addr), 0, NULL);
					test_max_count(1, 0);			// total connection +1
					printf("cli ip=[%s] pid=%d\n", inet_ntoa(cliaddr.sin_addr), pid);
					close(connfd);
					break;
		}
	}

	close(listenfd); 
  //sem_d(semid);
	free_hash_node(hash_conf);      
	free_hash_node(hash_pid_ip);
	free_hash_node(hash_ip_count);
	free_hash_node(hash_comm);
	exit(EXIT_SUCCESS);
}

/** signal SIGCHLD process function **/
void chld_handle(int signum)
{
	pid_t pid 		 = wait(0);
	char szPid[20] = "";
	int nowcount   = 0;
	sprintf(szPid, "%d", pid);
  printf("close cli pid=%d, signum=%d\n", pid, signum);
	
	char *ip = (char *)hash_getval(hash_pid_ip, szPid, 1);
	char *k = (char *)hash_getval(hash_ip_count, ip, 1);
	if (k != NULL)
		nowcount = atoi(k);
	if (nowcount > 1) 								// per IP count more than 1
		hash_del(hash_ip_count, ip, 1); // ip_count-1
	else                              // count <= 1
		hash_del(hash_ip_count, ip, 0); // del ip-count data in hash
	
	hash_del(hash_pid_ip, szPid, 0);  // hash_pid_ip delete szPid
	test_max_count(2, 0);             // total connections--
}

/** signal SIGALRM preocess function **/
void alrm_handle()
{	
	struct timespec req;
	long transfer_data = transfer_data_now - transfer_data_last;	
	double speed_now;
	double temp_time;
	double mytime;
	speed_now = transfer_data * 1000;
	if (speed_now > limit_speed)
	{
		memset(&req, 0, sizeof(req));
		mytime = (double)transfer_data/limit_speed - 0.001;  	// unit second 
		temp_time = mytime;
		req.tv_sec = (long)temp_time;  												// = 0?
		req.tv_nsec = (long)((mytime - req.tv_sec) * 1000000000); 
		if (req.tv_nsec < 0)
		{
			req.tv_nsec = 0;
		}
		
		int ret = nanosleep(&req, NULL);
		if (ret == -1)
		{
			EXIT_ERR("nanosleep");
		}	
	}
	transfer_data_last = transfer_data_now;
}

void clisession(int connfd)
{
	char buf[200] = "";
	int res = socketpair(PF_UNIX, SOCK_STREAM, 0, connsockpair);
	if (res == -1)
		EXIT_ERR("socketpair");
	switch(fork())
	{
		case -1:
				EXIT_ERR("data_fork");
				break;
		case  0:
				close(connsockpair[0]);
				while (1)
				{
					memset(buf, 0, sizeof(buf));
					read_line(connsockpair[1], buf, sizeof(buf));
					if (strlen(buf) == 0)
						break;
					exec_com(buf);
				}
				exit(EXIT_SUCCESS);
		default:
				close(connsockpair[1]);
				while (1)
				{
					memset(buf, 0, sizeof(buf));
					int n = read_line(connfd, buf, sizeof(buf));
					if (n == -1)
					{
						printf("read from connfd error!\n");
						EXIT_ERR("read from connfd");
					}
					printf("recv from cli = %s\n", buf);
					if (n == 0)
					{
						printf("cli close\n");
						exit(EXIT_SUCCESS);
					}
					check_com(buf);
				}
				break;
		}	
}	

void check_com(char buf[])
{
	char tempbuf[200] = "";
	if((strncmp(buf, "LIST", 4) == 0)   || (strncmp(buf, "PORT", 4) == 0)
	  || (strncmp(buf, "PASV", 4) == 0) || (strncmp(buf, "REST", 4) == 0)
	  || (strncmp(buf, "RETR", 4) == 0) || (strncmp(buf, "STOR", 4) == 0) 
	  || (strncmp(buf, "APPE", 4) == 0))
	{
		memset(tempbuf, 0, sizeof(tempbuf));
		char nowdir[200] = "";
		getcwd(nowdir, sizeof(nowdir));
		if (strncmp(buf, "LIST", 4) == 0)
			sprintf(tempbuf, "%s %s %s\r\n", "LIST", nowconnuser,  nowdir);
		else
			sprintf(tempbuf, "%s", buf);
		write_loop(connsockpair[0], tempbuf, strlen(tempbuf));
	}
	else
		exec_com(buf);
}
	
void exec_com(char buf[])
{
	char com[100]   = "";
	char charm[200] = "";  
	char dir[200]   = "";
	SESSION nowsession;
	memset(&nowsession, 0, sizeof(nowsession));

	get_com(buf, com);
	get_charm(buf, charm);
	getcwd(dir, sizeof(dir));
	nowsession.com = com;
	nowsession.charm = charm;
	nowsession.dir = dir;
	if (strncmp(nowsession.com, "LIST", 4) == 0)
	{
		char tempuser[50] = "", tempdir[100] = "";
		get_com(nowsession.charm, tempuser);
		get_charm(nowsession.charm, tempdir);
		strcpy(nowconnuser, tempuser);
		strcpy(nowsession.dir, tempdir);
	}
	if (strncmp(nowsession.com, "PASV", 4) == 0)  // When the user logs in, the login user is set to be a valid user in the child process,
		set_egid_euid(nowconnuser);                 // and a special command such as pasv is used in the grandchild process. The valid user at this time is unknown.

	printf("exec_com=[%s] charm=[%s] dir=[%s]\n", nowsession.com, nowsession.charm, nowsession.dir);

	if (strcmp(nowsession.com, "XRMD") == 0)      // XRMD/XMKD/XCWD to RMD/XMKD/XCWD
	{
		strcpy(nowsession.com, "RMD");
	}
	
	if (strcmp(nowsession.com, "XMKD") == 0)
	{
		strcpy(nowsession.com, "MKD");
	}
	
	if (strcmp(nowsession.com, "XCWD") == 0)
	{
		strcpy(nowsession.com, "CWD");
	}
	
	/*if (strcmp(nowsession.charm, "anonymous") == 0)
	{
		set_egid_euid("root");
		chroot("/srv/ftp");
	}*/	
	
	HASH_FUN hs = (HASH_FUN)hash_getval(hash_comm, nowsession.com, 2);
	if (hs != NULL)
		hs(nowsession);
	else
		write_loop(connfd, "500 Unknown command\r\n", strlen("500 Unknown command\r\n"));
}


