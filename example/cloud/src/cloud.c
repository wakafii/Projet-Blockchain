#include <netinet/in.h> //pour AF_INET
#include <sys/types.h>	//pour les types de socket
#include <sys/socket.h>	//pour toutes les fct des socket
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "util/buffutil.h"

int write_query_file(char* command, const char* filename)
{
	FILE* w, *fp;
	int status, c;
	fp = popen(command, "r");
	if (fp == NULL)
	{
		printf("popen() write query failed\n");
		return -1;
	}	
	w = fopen(filename, "rb+");

	while ((c = fgetc(fp)) != EOF)
			fputc(c, w);

	status = pclose(fp);
	if (status == -1) {
		printf("command failed\n");
		return -1;
	}
	return 0;
}

int creaSocket(int Domaine, int Type, int Protocole, int Port)
{
	int descSocket = 0;
	struct sockaddr_in ad1;
	int res = 0;
	
	descSocket = socket(Domaine, Type, Protocole);
	if(descSocket == -1)
	{
		printf("echec creation, ligne %d", __LINE__);
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

	
	if(argc != 2)
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

	res = listen(desc, 1);
	if(res == -1)
	{
		perror("Listen: ");
		exit(EXIT_FAILURE);
	}

	printf("attente connexion...\n");


	while(!fin)
	{
		//char **args;
		char *id;
		FILE *fp, *wf;
		int c, i;
		char command[300];
		char answer[8];
		char *queried_data;
		char fn_query[13];
		char fn_hex[12];
		char fn_key[12];
		
		sock_service = accept(desc, (struct sockaddr *)&addr_env, &size);
		if(sock_service == -1)
		{
			perror("Accept: ");
			exit(EXIT_FAILURE);
		}
		
		//Receive the ID
		if(read(sock_service, (char*)id, 65) < 65)
		{
			fprintf(stderr, "ERROR: Failed to read ID. (errno = %d)\n", errno);
			exit(EXIT_FAILURE);
		}
		printf("SUCCESS: ID received. \n");
	  	printf("ID received: %s\n", id);
	  
	  	//Query the blockchain and put the result in a file
	  	strcpy(fn_query, "query");
	  	strcat(fn_query, id);
	  	strcat(fn_query, ".txt");
		write_query_file("multichain-cli chain1 liststreamitems stream1", fn_query);		
		
		//Parse the query and put it in a stream
		strcpy(command, "cat ");
		strcat(command, fn_query);
		strcat(command, " | ./jq --raw-output '(.[] | select(.key == \"");
		strcat(command, id);
		strcat(command,"\")?) | .data'");
		fp = popen(command, "r");
		if (fp == NULL)
		{
			printf("popen() jq failed\n");
			exit(EXIT_FAILURE);
		}
		//Process the result
		strcpy(fn_hex, "hex");
	  	strcat(fn_hex, id);
	  	strcat(fn_hex, ".txt");
		wf = fopen(fn_hex, "rb+");
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
	    queried_data = (char*) calloc(sizeof(char), size);
	    fseek(fp, 0, SEEK_SET);
		i=0;
		while ((c = fgetc(fp)) != EOF)
		{
			if(c == '\n')
				break;
			fputc(c, wf);
			queried_data[i++] = c;
		}
		
		fclose(wf);
		fclose(fp);
				
		//Check if the query has given something
		if(strcmp(queried_data, "") == 0)
		{
			strcpy(answer, "failure");
		}
		else
		{
			strcpy(answer, "success");
			strcpy(fn_hex, "key");
		  	strcat(fn_hex, id);
		  	strcat(fn_hex, ".txt");
			if(execlp("python", "python", "decode.py", fn_hex, fn_key, NULL) == -1)
			{
				fprintf(stderr, "ERROR: Failed to run the decode python script.(errno = %d)\n", errno);
     			exit(EXIT_FAILURE);
			}
		}
		
		//Notify the Client
		if(write(sock_service, (char*)answer, sizeof(answer)) < (int)sizeof(answer))
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
