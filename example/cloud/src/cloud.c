#include <netinet/in.h> //pour AF_INET
#include <sys/types.h>	//pour les types de socket
#include <sys/socket.h>	//pour toutes les fct des socket
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

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
		perror("error ecoute");
		exit(EXIT_FAILURE);
	}

	printf("attente connexion...\n");


	while(!fin)
	{
		//char **args;
		char id;
		
		sock_service = accept(desc, (struct sockaddr *)&addr_env, &size);
		if(sock_service == -1)
		{
			perror("error accept");
			exit(EXIT_FAILURE);
		}
		
		//Receive the ID
		if(read(sock_service, (char*)&id, 65) < 65)
		{
			 fprintf(stderr, "ERROR: Failed to read EpidMessage. bytes read.(errno = %d)\n", errno);
       exit(EXIT_FAILURE);
		}
		printf("SUCCESS: ID received. \n");
	  printf("ID received: %s\n", id);
	  
	  /** PARSE BLOCKCHAIN **/
	  
	  /** START OPENVPN **/
	  
	  /** NOTIFY CLIENT **/
		

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
