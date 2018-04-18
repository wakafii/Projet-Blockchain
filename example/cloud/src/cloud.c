#include <netinet/in.h> //pour AF_INET
#include <sys/types.h>	//pour les types de socket
#include <sys/socket.h>	//pour toutes les fct des socket
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "util/buffutil.h"
#include "util/EpidMessage.h"

//#define FN_QUERY "cloudquery.txt"

int creaSocket(int Domaine, int Type, int Protocole, int Port)
{
	int descSocket = 0;
	struct sockaddr_in ad1;
	int res = 0;
	
	descSocket = socket(Domaine, Type, Protocole);
	if(descSocket == -1)
	{
		printf("echec creation, ligne %d\n", __LINE__);
		return -1;
	}

	ad1.sin_family = Domaine;
	ad1.sin_port = htons(Port);
	ad1.sin_addr.s_addr = INADDR_ANY;
	memset(ad1.sin_zero,0,8);

	res = bind(descSocket,(struct sockaddr*)&ad1,sizeof(ad1));
	if(res == -1)
	{
		printf("echec bind socket, ligne %d", __LINE__);
		return -1;
	}
	return descSocket;
}

int main(int argc, char **argv)
{
	int res = 0;	
	int port;
	int sock_service, desc;
	int fin = 0;
	socklen_t size;
	struct sockaddr_in addr_env;
	EpidMessage em;

	
	if(argc < 2)
	{
		printf("How to use: ./member port_number ");
		exit(EXIT_FAILURE);
	}
	
	size = sizeof(struct sockaddr_in);
	
	port = atoi(argv[1]);
	
	desc = creaSocket(AF_INET,SOCK_STREAM,0,port);

	if(desc == -1)
	{
		exit(EXIT_FAILURE);
	}

	res = listen(desc, 5);
	if(res == -1)
	{
		perror("Listen: ");
		exit(EXIT_FAILURE);
	}

	printf("attente connexion...\n");


	while(!fin)
	{
		//char **args;
		FILE *fp;
		int c, i;
		char command[300];
		char answer[8];
		char *queried_data;
		
		sock_service = accept(desc, (struct sockaddr *)&addr_env, &size);
		if(sock_service == -1)
		{
			perror("Accept: ");
			exit(EXIT_FAILURE);
		}
		
		//Receive the EpidMessage
		if((size_t)read(sock_service, (EpidMessage*)&em, sizeof(em)) < sizeof(em))
		{
			fprintf(stderr, "ERROR: Failed to read EpidMessage. (errno = %d)\n", errno);
			exit(EXIT_FAILURE);
		}
		printf("SUCCESS: EpidMessage received. \n");
		
		//Parse the query, convert its content, and put it in a stream
    	queried_data = (char*) malloc(sizeof(char) * 1000);
		strcpy(command, "./cloud_script ");
		strcat(command, em.id);
		fp = popen(command, "r");
		if (fp == NULL)
		{
			printf("popen() cloud_script failed\n");
			exit(EXIT_FAILURE);
		}
		i=0;
		while ((c = fgetc(fp)) != EOF)
		{
			if(c == '\n')
				break;
			queried_data[i++] = c;
		}
		
		if(pclose(fp) == -1)
		{
			printf("pclose caught an error \n");
			exit(EXIT_FAILURE);
		}
		
		printf("Queried data: %s\n", queried_data);
				
		//Check if the query has given something
		if(strcmp(queried_data, "") == 0)
		{
			strcpy(answer, "failure");
		}
		else
		{
			strcpy(answer, "success");			
		}
		
		//Notify the Client
		if((size_t)write(sock_service, (char*)answer, sizeof(answer)) < sizeof(answer))
		{
			fprintf(stderr, "ERROR: Failed to write the answer. (errno = %d)\n", errno);
			exit(EXIT_FAILURE);
		}				
		
		
	  
	  	/** START OPENVPN **/
	  

		

		/**************************************************
		**************** traitement message ***************
		**************************************************/
		
		
		
		//printf("%s\n",buffer);
		close(sock_service);
	}
	//if(buffer) free(buffer);
	close(desc);
	return EXIT_SUCCESS;
}
