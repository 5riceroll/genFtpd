#include "def.h"
#include "com.h"
#include "pub.h"
#include "net.h"

char nowconnuser[10];
char up_com[10];
int port_port;
char port_ip[50];
int pasv_connsock;
int data_conn_sock;
char *pp;
char oldpath[200] = "";
long restdata;

void do_stat(SESSION nowsession)
{
	printf("command:[%s] process\n", nowsession.com);
	int myshmid = shmget((key_t)1133, sizeof(JWLSHM), 0);
	if(myshmid == -1)
		EXIT_ERR("myshmid");
	JWLSHM *mymyshm = (JWLSHM *)shmat(myshmid, NULL, 0);
	if(mymyshm == (JWLSHM *)-1)
		EXIT_ERR("shm");
	char status[200] = "";
	sprintf(status, "%s %d %s %ld %s%s %d %s %ld %s", "stored", mymyshm->stor_count, "files", mymyshm->stor_byte, "bytes\n", "Retrieved", mymyshm->retr_count, "files", mymyshm->retr_byte, "bytes\r\n" );
	write(connfd, status, strlen(status));
	write(connfd, "211 end\r\n", strlen("211 end\r\n"));
}

void do_user(SESSION nowsession)	
{
	strcpy(nowconnuser, nowsession.charm);
	write_loop(connfd, "331 Please specify the password.\r\n", strlen("331 Please specify the password.\r\n"));
}

void do_quit(SESSION nowsession)
{
	printf("command:[%s] process\n", nowsession.com);
	write_loop(connfd, "221 quit\r\n", strlen("221 quit\r\n"));
}

void do_pass(SESSION nowsession)
{
	//set_egid_euid("root");  // passport check , only root in Ubuntu.
	int ret = check_pass(nowconnuser, nowsession.charm);
	if (ret == 1|| strcmp(nowconnuser, "anonymous") == 0)
	{
		write_loop(connfd, "230 Login Successufl\r\n", strlen("230 Login Successufl\r\n"));
		if (strcmp(nowconnuser, "anonymous") == 0)
		{
	//	set_egid_euid("root");
	//	chroot("/srv/ftp");
			strcpy(nowconnuser, "ftp");
			set_egid_euid(nowconnuser);
			chdir(getpwnam(nowconnuser)->pw_dir);
	//	set_egid_euid("root");
	//	chroot("/srv/ftp");
			set_egid_euid(nowconnuser);			
		}
	}
	
	if (ret == -1)
		write_loop(connfd, "530 Login incorrect.\r\n", strlen("530 Login incorrect.\r\n"));		
}	

void do_site(SESSION nowsession)
{
// method-1
/*
	char chpath[100] = "";
	char mode[10] = "";
	get_charm(nowsession.charm, chpath);
	get_com(chpath, mode);
	get_charm(chpath, chpath);
	int ret = strtol(mode, NULL, 8);   // client data Octal to decimal
	chmod(chpath, (mode_t)ret);
*/	

// method-2
	char chpath[100] = "";
	int mode;
	get_charm(nowsession.charm, chpath);
	sscanf(chpath, "%04o %s", &mode, chpath);  // Split data with sccanf
	chmod(chpath, (mode_t)mode);
	write_loop(connfd, "200 SITE CHMOD command ok.\r\n", strlen("200 SITE CHMOD command ok.\r\n"));	
}

void do_syst(SESSION nowsession)
{
	printf("command:[%s] process\n", nowsession.com);
	write_loop(connfd, "215 UNIX Type:L8\r\n", strlen("215 UNIX Type:L8\r\n"));
}

void do_feat(SESSION nowsession)
{	
	printf("command:[%s] process\n", nowsession.com);
	write_loop(connfd, "211-Features:\n EPRT\n EPSV\n MDTM\n PASV\n REST STREAM\n SIZE\n TVFS\n UTF8\n211 End\r\n", strlen("211-Features:\n EPRT\n EPSV\n MDTM\n PASV\n REST STREAM\n SIZE\n TVFS\n UTF8\n211 End\r\n"));
}	
		
void do_pwd(SESSION nowsession)
{	
	char pd[100] = "";
	if ((strcmp(nowconnuser, "ftp") == 0))
	{
		if (strcmp(nowsession.dir, "/srv/ftp") == 0)  
			strcpy(nowsession.dir, "/");
		else
			strcpy(nowsession.dir, nowsession.dir + 8); 
	}
	sprintf(pd, "%s%s%s", "257 \"", nowsession.dir, "\"\r\n");
	write_loop(connfd, pd, strlen(pd));	
}

void do_cwd(SESSION nowsession)
{
	/*if ((strcmp(nowconnuser, "ftp") == 0))
	{
		set_egid_euid("root");
		chroot("/srv/ftp");
		set_egid_euid("ftp");
	}*/

	if(strcmp(nowsession.com, "CDUP") == 0)
	{
		strcpy(nowsession.charm, "../");
	}

	if((strcmp(nowconnuser, "ftp") == 0))
	{
		char reverdir[100] = "";
		sprintf(reverdir, "%s%s", "/srv/ftp", nowsession.charm); 
		puts(reverdir);
		chdir(reverdir);
	}
	else
		chdir(nowsession.charm);
	write_loop(connfd, "250 Directory successfully changed.\r\n", strlen("250 Directory successfully changed.\r\n"));
}

void do_mkd(SESSION nowsession)
{
	char dir[200] = "";	
	if ((strcmp(nowconnuser, "ftp") == 0))
		write_loop(connfd, "500 Permission denied.\r\n", strlen("500 Permission denied.\r\n"));
	else
	{
		mkdir(nowsession.charm, 0666);
		sprintf(dir, "%s%s%s\r\n", "257 \"", nowsession.charm, "\" created");
		write_loop(connfd, dir, strlen(dir));
		chdir(nowsession.charm);
	}
}

void do_rmd(SESSION nowsession)
{
	int ret = rmdir(nowsession.charm);
	if (ret == 0)
		write_loop(connfd, "250 Remove directory operation successful.\r\n", strlen("250 Remove directory operation successful.\r\n"));
	else
		write_loop(connfd, "502 Command unfinished.\r\n", strlen("502 Command unfinished.\r\n"));
}

void do_rnfr(SESSION nowsession)
{
	strcpy(oldpath, nowsession.charm);
	write_loop(connfd, "350 Ready for RNTO.\r\n", strlen("350 Ready for RNTO.\r\n"));
}

void do_rnto(SESSION nowsession)
{
	rename(oldpath, nowsession.charm);
	write_loop(connfd, "250 Rename Successful.\r\n", strlen("250 Rename Successful.\r\n"));
}

void do_noop(SESSION nowsession)
{
	printf("command:[%s] process\n", nowsession.com);
	write_loop(connfd, "200 Noop ok.\r\n", strlen("200 Noop ok.\r\n"));
}

void do_type(SESSION nowsession)
{
	if(strcmp(nowsession.charm, "A") == 0)
		write_loop(connfd, "200 Switching to ASCII mode.\r\n", strlen("200 Switching to ASCII mode.\r\n"));
	if(strcmp(nowsession.charm, "I") == 0)
		write_loop(connfd, "200 Switching to I mode.\r\n", strlen("200 Switching to I mode.\r\n"));
}

void do_port(SESSION nowsession)	
{
	write_loop(connfd, "200 port ok\r\n", strlen("200 port ok\r\n"));
	char temp[50];
	int n1, n2, n3, n4, n5, n6;
	strcpy(temp, nowsession.charm);
	sscanf(temp, "%d,%d,%d,%d,%d,%d", &n1, &n2, &n3, &n4, &n5, &n6);
	sprintf(port_ip, "%d.%d.%d.%d", n1, n2, n3, n4);
	port_port = n5*256 + n6;
	strcpy(up_com, "PORT");	
}

void do_pasv(SESSION nowsession)
{	
	printf("command:[%s] process\n", nowsession.com);
	struct sockaddr_in s_addr, tempaddr;
	socklen_t templen = sizeof(tempaddr);
	char buf[200], tempip[30];
	unsigned int pasv_port, port1, port2;
	char *q = (char *)hash_getval(hash_conf,"serverip",1); // read server ip from hash table
	if (q != NULL)
		strcpy(tempip, q);
				
	/*for(i = 0; i < 10; i++)  // read serverip from arry mycon
	{
		if (strcmp(mycon[i].key, "serverip") == 0)
		{
			strcpy(tempip, mycon[i].val);
			break;                       
		}
	}*/
	
	char *p = tempip;
	for(; *p != '\0'; p++)
	{
		if(*p == '.')
			*p = ',';
	}
	memset(&s_addr, 0, sizeof(s_addr));
	pasv_connsock = socket(AF_INET, SOCK_STREAM, 0);

	if (pasv_connsock == -1)
		EXIT_ERR("pasv_connsock");
	
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(0);
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int opt = 1;
	int ret = setsockopt(pasv_connsock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (ret == -1)
		EXIT_ERR("pasv setsockopt");
	
	int reb = bind(pasv_connsock, (struct sockaddr*)&s_addr, sizeof(s_addr));
	if (reb == -1)	
		EXIT_ERR("pasv bind");
		
	int rel = listen(pasv_connsock, 10);
	if (rel == -1)
		EXIT_ERR("pasv listen");
	
	int res = getsockname(pasv_connsock, (struct sockaddr*)&tempaddr, (socklen_t*)&templen);
	if (res == -1)
		EXIT_ERR("pasv getsockname");
	
	pasv_port = ntohs(tempaddr.sin_port);
	port1 = pasv_port >> 8;
	port2 = pasv_port & 0xFF;

	sprintf(buf, "227 Entering Passive Mode(%s,%d,%d)\r\n", tempip, port1, port2);
	write_loop(connfd, buf, strlen(buf));
	
	strcpy(up_com, "PASV");
}

void do_dele(SESSION nowsession)
{
	unlink(nowsession.charm);
	write_loop(connfd, "250 Delete operation successful.\r\n", strlen("250 Delete operation successful.\r\n"));	
}

void do_rest(SESSION nowsession)
{
	restdata = atoi(nowsession.charm);
	char rest[64] = "";
	sprintf(rest, "%s%s%s\r\n", "350 Restart position accepted (", nowsession.charm, ").");
	write_loop(connfd, rest, strlen(rest));
}

void do_stor(SESSION nowsession)
{
	data_conn_sock = test_pasv_or_port();
	write_loop(connfd, "150 start\r\n", strlen("150 start\r\n"));
	mystor(data_conn_sock, nowsession.charm);
	write_loop(connfd, "226 end\r\n", strlen("226 end\r\n"));
	test_max_count(3, 0);
	if (strcmp(up_com, "PASV") == 0)
	{
		close(pasv_connsock);
	}
	close(data_conn_sock);
}

void do_retr(SESSION nowsession)
{
	data_conn_sock = test_pasv_or_port();
	write_loop(connfd, "150 start\r\n", strlen("150 start\r\n"));
	myretr(data_conn_sock, nowsession.charm);
	write_loop(connfd, "226 end\r\n", strlen("226 end\r\n"));
	test_max_count(4, 0);
	if (strcmp(up_com, "PASV") == 0)
		close(pasv_connsock);
	close(data_conn_sock);
}

void do_list(SESSION nowsession)
{
	data_conn_sock = test_pasv_or_port();	
	write_loop(connfd, "150 start\r\n", strlen("150 start\r\n"));
	mylist(data_conn_sock, nowsession.dir);
	write_loop(connfd, "226 end\r\n", strlen("226 end\r\n"));
	
	if (strcmp(up_com, "PASV") == 0)
		close(pasv_connsock);
	close(data_conn_sock);
}

/** Check whether the previous command in the list is a port or a pasv,
    and perform different processing to establish a corresponding data channel. **/
int test_pasv_or_port()      
{                            
	if (strcmp(up_com, "PORT") == 0)
	{	
		int opt = 1;
		struct sockaddr_in cli_addr;
		struct sockaddr_in ser_addr;
		int sockk;
		socklen_t addrlen = sizeof(cli_addr);
		memset(&cli_addr, 0, sizeof(cli_addr));
		cli_addr.sin_family 		 = AF_INET;
		cli_addr.sin_port 			 = htons(port_port);
		cli_addr.sin_addr.s_addr = inet_addr(port_ip);
		
		memset(&ser_addr, 0, sizeof(ser_addr));
		ser_addr.sin_family      = AF_INET;
		ser_addr.sin_port 			 = htons(20);
		ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		
		set_egid_euid("root");
		
		sockk = socket(AF_INET, SOCK_STREAM, 0);
		setsockopt(sockk, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
		int reb = bind(sockk, (struct sockaddr*)&ser_addr, sizeof(ser_addr));
		if (reb == -1)
			EXIT_ERR("port bind");
		int res = connect(sockk, (struct sockaddr*)&cli_addr, addrlen);
	
		if (res == -1)
			EXIT_ERR("port data_conn_sock");
		data_conn_sock = sockk;
		
		set_egid_euid(nowconnuser);
	}
	else if (strcmp(up_com, "PASV") == 0)
	{   
	  struct sockaddr_in cliaddr;
		memset(&cliaddr, 0, sizeof(cliaddr));
		socklen_t cliaddr_llen = sizeof(cliaddr);
		data_conn_sock = accept(pasv_connsock, (struct sockaddr *)&cliaddr, &cliaddr_llen);
		if (data_conn_sock == -1)
			EXIT_ERR("pasv sock");
	}
	
	return data_conn_sock;
}

COM_FUN mycomfun[] = {
				        {"USER", do_user}, {"PORT", do_port}, {"PASS", do_pass},
								{"DELE", do_dele}, {"LIST", do_list}, {"PWD", do_pwd},
								{"SYST", do_syst}, {"FEAT", do_feat}, {"STOR", do_stor},
								{"TYPE", do_type}, {"PASV", do_pasv}, {"MKD", do_mkd},
								{"RETR", do_retr}, {"CWD", do_cwd},   {"RNTO", do_rnto}, 
								{"CDUP", do_cwd},  {"RMD", do_rmd},   {"REST", do_rest},
								{"RNFR", do_rnfr}, {"NOOP", do_noop}, {"STAT", do_stat}, 
								{"QUIT", do_quit}, {"SITE", do_site} 
							};
	
/** command processing function into hash table **/
void com_to_hash()
{
	int len, i;
	len = sizeof(mycomfun) / sizeof(mycomfun[0]);            
	for (i = 0; i < len; i++)		                      
		hash_insert(hash_comm, mycomfun[i].mycom, NULL, 2, mycomfun[i].myfun);						
}

void initfun(SESSION nowsession)                           
{
	int i;
	int len = sizeof(mycomfun) / sizeof(mycomfun[0]);
	for(i = 0; i < len; i++)
	{
		if(strcmp(mycomfun[i].mycom, nowsession.com ) == 0)
		{
			mycomfun[i].myfun(nowsession);
			break;
		}
	}
	if(i == len)
		printf("unknown com!\n");
}

void get_com(char line[], char con[])                 
{	
	char *p;
	char tem[100] = "";
	strcpy(tem, line);
	p = strchr(tem, ' ');
	if(p != NULL)
	{
		*p = '\0';
		strcpy(con, tem);	
	}
	else
	{
		p = strchr(tem, '\r');
		if(p != NULL)
		{
			*p = '\0';
			strcpy(con, tem);
		}
	}
}

void get_charm(char line[], char charm[])              
{
	char *p;
	p = strchr(line, ' ');
	if(p != NULL )
	{
		strcpy(charm , p+1);
		p = strchr(charm, '\r');
		if(p != NULL)
			*p = '\0';
	}
}
