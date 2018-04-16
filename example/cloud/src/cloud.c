#include <netinet/in.h> //pour AF_INET
#include <sys/types.h>	//pour les types de socket
#include <sys/socket.h>	//pour toutes les fct des socket
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int createfile(char* command, char* filename)
{
	FILE* w;
	fp = popen(command, "r");
		if (fp == NULL)
		{
			printf("popen() failed\n");
			return -1;
		}	
		w = fopen(filename, "rb+");

		while ((c = fgetc(fp)) != EOF)
				fputc(c, w);

		status = pclose(fp);
		if (status == -1) {
				printf("popen() failed\n");
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
	int res = 0, i;	
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
		char id;
		int fd;
		FILE *fp;
		int status;
		int c, i;
		char command[150];
		char* bc_checksum;
		char* member_checksum;
		size_t md5cs_size;
		char answer[8];
		
		sock_service = accept(desc, (struct sockaddr *)&addr_env, &size);
		if(sock_service == -1)
		{
			perror("error accept");
			exit(EXIT_FAILURE);
		}
		
		//Receive the ID
		if(read(sock_service, (char*)&id, 65) < 65)
		{
			 fprintf(stderr, "ERROR: Failed to read ID. (errno = %d)\n", errno);
       exit(EXIT_FAILURE);
		}
		printf("SUCCESS: ID received. \n");
	  printf("ID received: %s\n", id);
		
		//receive the length of the checksum
		if(read(sock_service, (int*)&md5cs_size, sizeof(int)) < (int)sizeof(int))
		{
			 fprintf(stderr, "ERROR: Failed to read the md5 checksum size. (errno = %d)\n", errno);
       exit(EXIT_FAILURE);
		}
		printf("SUCCESS: md5 size received. \n");
	  printf("md5 size: %d\n", md5cs_size);
		
		//receive the md5 checksum
		member_checksum = (char*) malloc(md5cs_size * sizeof(char));
		if(read(sock_service, (char*)member_checksum, md5cs_size) < md5cs_size)
		{
			 fprintf(stderr, "ERROR: Failed to read the md5 checksum. (errno = %d)\n", errno);
       exit(EXIT_FAILURE);
		}		
	  printf("SUCCESS: md5 received. \n");
	  printf("md5: %s\n", member_checksum);
	  
	  //Query the blockchain and put the result in a file
		strcpy(command, "multichain-cli chain2 liststreamkeys stream1 ");
		strcat(command, id);
		createfile(command, "query.txt");
		
		//Parse the query

		fp = popen("./jq-linux64 -R query.txt", "r");
		if (fp == NULL)
		{
			printf("popen() failed\n");
			exit(EXIT_FAILURE);
		}	
		bc_checksum =(char*) malloc(md5cs_size * sizeof(char));
		i=0;
		while ((c = fgetc(fp)) != EOF)
				bc_checksum[i++] = c;
				
		//Compare the two checksums
		if(strcmp(bc_checksum, member_checksum) == 0)
		{
			strcpy(answer, "success");
		}
		else
			strcpy(answer, "failure");
		
		if(write(sock_service, (char*)answer, sizeof(answer)) < (int)sizeof(answer))
		{
			 fprintf(stderr, "ERROR: Failed to write the answer. (errno = %d)\n", errno);
       exit(EXIT_FAILURE);
		}				
		
		/** NOTIFY CLIENT **/
	  
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
