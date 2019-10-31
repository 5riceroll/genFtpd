#include "def.h"
#include "pub.h"
#include "com.h"
#include "net.h"

void PrintHelpMsg()
{
	printf("genFtpd Version: 1.0.0\n"                   
		     "Usage: genFtpd [-h] [-c filename]\n"
		     "Options:\n"
		     "   -h         : this help\n"
		     "   -c filename: set server configuration file\n");
}

int main(int argc, char **argv)
{
	if(2 == argc || 3 == argc)
	{                         
		if((2 == argc) && (strcmp(argv[1], "-h") == 0)) 
			PrintHelpMsg();	
		else if((3 == argc) && (strcmp(argv[1], "-c") == 0) && (access(argv[2], F_OK) != -1))
		{
			if(strcmp(argv[1], "-c") == 0 && access(argv[2], F_OK) != -1)
			{
				/*daemon(1, 0)*/;    // set daemon
				create_shm();
				MyFtpdSer(argv[2]);	
			}
		}
		else 
			printf("genFtpd: Invalid Option, try myVsftpd -h for help\n");
	}
	else
	{
		PrintHelpMsg();
	}
	
	return EXIT_SUCCESS;
}
