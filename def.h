#ifndef _MYDEF_H
#define _MYDEF_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>	
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/time.h>
#include <crypt.h> 		// -lcrypt 
#include <sys/sysmacros.h>
#include <shadow.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <assert.h>
#include <sys/shm.h>  //if no, will warn cast to pointer from integer of a different size when call 'shmat' in 64 bit
                      //and call shmdt, segmentation fault occured                                          

#define EXIT_ERR(m) (perror(m),exit(-1))

#endif  /*_MYDEF_H*/
