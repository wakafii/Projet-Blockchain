#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <signal.h>
#include <ctype.h>          
#include <arpa/inet.h>
#include <netdb.h>
#include "util/fileio.h"

#define LENGTH 512

int sendfile(int sockfd, char* fs_name)
{
	/* Send File to Server */

		//char* fs_name = "hello.txt";
		char sdbuf[LENGTH]; 
		printf("[Client] Sending %s to the Server... ", fs_name);
		FILE *fs = fopen(fs_name, "r");
		if(fs == NULL)
		{
			printf("ERROR: File %s not found.\n", fs_name);
			return 1;
		}

		bzero(sdbuf, LENGTH); 
		int fs_block_sz; 
		while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs)) > 0)
		{
		    if(send(sockfd, sdbuf, fs_block_sz, 0) < 0)
		    {
		        fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fs_name, errno);
		        return 1;
		    }
		    bzero(sdbuf, LENGTH);
		}
		printf("Ok File %s from Client was Sent!\n", fs_name);
		return 0;
}

int error(const char *msg)
{
	perror(msg);
	return 1;
}

int rcv_write(char* revbuf, int sockfd, FILE* fr, size_t size)
{
	int fr_block_sz = 0;
	int write_sz;
	if((fr_block_sz = recv(sockfd, revbuf, size, 0)) <= 0)
	{	
    if (errno == EAGAIN)
  	{
          printf("recv() timed out.\n");
    }
    else
    {
	      fprintf(stderr, "recv() failed due to errno = %d\n", errno);     		
    }
    return -1;
	}
  write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
	if(write_sz < fr_block_sz)
  {
     printf("File write failed on server.\n");
     return -1;
  }
	bzero(revbuf, LENGTH);
	return 0;		
}


int receivefile(int sockfd, char* id, size_t size)
{
	/* Defining Variables */
	char revbuf[LENGTH]; // Receiver buffer

	/*Receive File from Client */
	//char* fr_name = "hello.txt";
	FILE *fr = fopen(id, "a");
	if(fr == NULL)
		printf("File %s Cannot be opened file on server.\n", id);
	else
	{
		bzero(revbuf, LENGTH); 
		int i;
		int n = (int) size/LENGTH;
		int rest = size%LENGTH;		
		for(i=0; i<n; i++) 
		{
			if(rcv_write(revbuf, sockfd, fr, LENGTH) < 0)
				return EXIT_FAILURE;
		}
		
		if(rcv_write(revbuf, sockfd, fr, rest) < 0)
				return EXIT_FAILURE;		
		
		/*while((fr_block_sz = recv(sockfd, revbuf, LENGTH, 0)) > 0) 
		{
		  int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
			if(write_sz < fr_block_sz)
		  {
		  	printf("File write failed on server.\n");
     		return -1;
		  }
			bzero(revbuf, LENGTH);
			if (fr_block_sz == 0 || fr_block_sz != 512) 
			{
				break;
			}
		}
		if(fr_block_sz < 0)
		  {
		      if (errno == EAGAIN)
	      	{
	              printf("recv() timed out.\n");
	          }
	          else
	          {
	              fprintf(stderr, "recv() failed due to errno = %d\n", errno);
								return 1;
	          }
      	}
      	*/
		printf("Ok received from client!\n");
		fclose(fr); 
	}
	return EXIT_SUCCESS;
}
