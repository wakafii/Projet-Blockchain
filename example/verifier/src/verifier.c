#include <netinet/in.h> //pour AF_INET
#include <sys/types.h>	//pour les types de socket
#include <sys/socket.h>	//pour toutes les fct des socket
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "util/EpidMessage.h"
#include "util/fileio.h"
#include "verifyprocess.h"

#define FN_SIZE 5

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
	int res, i;	
	int port;
	int sock_service, desc;
	int fin;
	socklen_t size;
	struct sockaddr_in addr_env;
	
	if(argc != 2)
	{
		printf("How to use: ./verifier port_number ");
		exit(EXIT_FAILURE);
	}
	res = fin = 0;
	
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
		perror("error ecoute");
		exit(EXIT_FAILURE);
	}

	printf("attente connexion...\n");


	while(!fin)
	{
		char fn_key[70];
		char fn_sig[70];
		char msg_file[85];
		char sig_file[85];
		char **args;
		EpidMessage em;
		char answer[50];
		int pid;
		
		
		sock_service = accept(desc, (struct sockaddr *)&addr_env, &size);
		if(sock_service == -1)
		{
			perror("error accept");
			exit(EXIT_FAILURE);
		}
		
		//Receive the EpidMessage
		if(read(sock_service, (EpidMessage*)&em, sizeof(em)) < (int)sizeof(em))
		{
			 fprintf(stderr, "ERROR: Failed to read EpidMessage. bytes read: %d/%lu.(errno = %d)\n", res, sizeof(EpidMessage), errno);
       exit(EXIT_FAILURE);
		}
		printf("SUCCESS: EpidMessage received. \n");
	  printf("hash received: %s\n", em.id);
		
		//Receive the Signature
		strncpy(fn_sig, em.id, FN_SIZE);
		strcat(fn_sig, ".dat");
		if(receivefile(sock_service, fn_sig, em.sig_size))
		{
			 fprintf(stderr, "ERROR: Failed to receive the Signature.(errno = %d)\n", errno);
       exit(EXIT_FAILURE);
		}
		printf("SUCCESS: Signature received. \n");
		
		//Receive the member's Public Key
		strncpy(fn_key, em.id, FN_SIZE);
		strcat(fn_key, ".txt");
		if(receivefile(sock_service, fn_key, em.msg_size))
		{
			 fprintf(stderr, "ERROR: Failed to receive the Public Key.(errno = %d)\n", errno);
       exit(EXIT_FAILURE);
		}
		printf("SUCCESS: Key received. \n");
		

		/**************************************************
		**************** traitement message ***************
		**************************************************/
		
		/*************************************************/
		
		args = (char **) malloc(3 * sizeof(char*));
		for(i=0; i<3; i++)
		{
     	args[i] = (char *)calloc(100, sizeof(char));
		}
					
		//args setup
		strcpy(sig_file, "--sig=");
		strcat(sig_file, fn_sig);
		strcpy(msg_file, "--msgfile=");
		strcat(msg_file, fn_key);
		
		strcpy(args[0], "./verifysig");
		strcpy(args[1], sig_file);
		strcpy(args[2], msg_file);
	
		
		if(verifyprocess(3, args) == EXIT_FAILURE)
		{
      strcpy(answer, "failure");
		}
		else
		{
			char fn_hex[70];
			char *hex_string;
			size_t size_hex;
			strcpy(answer, "success");
			
			//Convert the Public Key into a hex string
			strncpy(fn_hex, em.id, FN_SIZE);
			strcat(fn_hex, ".hex");
			pid = fork();
			if(pid == 0)
			{
				if(execlp("python", "python2", "encode.py", fn_key, fn_hex, NULL) == -1)
				{
					fprintf(stderr, "ERROR: Failed run the b2h python script.(errno = %d)\n", errno);
       		exit(EXIT_FAILURE);
				}
			}
			//wait for the child to finish
			wait(NULL);
			//Get the hex string from the file
			hex_string = (char*) NewBufferFromFile(fn_hex, &size_hex);
			printf("Hex string:\n%s\n", hex_string);
			//Launch a shell command to write the file in the Client Blockchain
			pid = fork();
			if(pid == 0)
			{
				execlp("multichain-cli", "multichain-cli", "chain1", "publish", "stream1", em.id, hex_string, NULL);
			}
			
		}		
		//Send the answer
		if(send(sock_service, (char*)answer, sizeof(answer)) < (int)sizeof(answer))
		{
			 fprintf(stderr, "ERROR: Failed to send Answer.(errno = %d)\n", errno);
       exit(EXIT_FAILURE);
		}
		printf("SUCCESS: Answer sent \n");		
		

		//printf("%s\n",buffer);
		close(sock_service);
		
	}
	//if(buffer) free(buffer);
	close(desc);
	return EXIT_SUCCESS;
}
