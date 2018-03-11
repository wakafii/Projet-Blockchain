#include <netinet/in.h> //pour AF_INET
#include <sys/types.h>	//pour les types de socket
#include <sys/socket.h>	//pour toutes les fct des socket
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int creaSocket(int Domaine, int Type, int Protocole, int Port)
{
	int descSocket = 0;
	struct sockaddr_in ad1;
	int res = 0;
	
	descSocket = socket(Domaine, Type, Protocole);
	if(descSocket == -1)
	{
		printf("echec creation, ligne %s", __LINE__);
		return -1;
		//exit(0); normalement exit mais ne sert a rien dans une fonction
	}

	ad1.sin_family = Domaine;
	ad1.sin_port = htons(Port);
	ad1.sin_addr.s_addr = INADDR_ANY;
	memset(ad1.sin_zero,0,8);

	res = bind(descSocket,(struct sockaddr*)&ad1,sizeof(ad1));
	if(res == -1)
	{
		printf("echec bind socket, ligne %s", __LINE__);
		return -1;
		//exit(0); normalement exit mais ne sert a rien dans une fonction
	}
	return descSocket;
}

int main(int argc, char **argv)
{
	int res = 0;	
	int port = atoi(argv[1]);
	int sock_service;
	int desc = creaSocket(AF_INET,SOCK_STREAM,0,port);
	int fin = 0;
	int nbAleat = 0;
	if(desc == -1)
	{
		exit(0);
	}

	res = listen(desc, 1);
	if(res == -1)
	{
		perror("error ecoute");
		exit(0);
	}

	printf("attente connexion...\n");

	struct sockaddr_in addr_env;
	int longueur = sizeof(addr_env);
	char* buffer = (char*)malloc(10000*sizeof(char));
	while(!fin)
	{
		sock_service = accept(desc, (struct sockaddr *)&addr_env, &longueur);
		if(sock_service == -1)
		{
			perror("error accept");
			exit(0);
		}

		res = read(sock_service,buffer,10000);
		printf("%s\n",buffer);

		/**************************************************
		**************** traitement message ***************
		**************************************************/
		nbAleat = rand()%100;
		if(nbAleat <= 75)
		{
			strcpy(buffer, "reussite");
			nbAleat = 1;
		}
		else
		{
			strcpy(buffer, "echec");
		}
		/*************************************************/

		res = write(sock_service,buffer,10000);

		if(nbAleat == 1)
		{
			res = read(sock_service,buffer,10000);
			printf("%s\n",buffer);


			res = write(sock_service,buffer,10000);
			printf("%s\n",buffer);
		}
	}
	free(buffer);
	close(sock_service);
	close(desc);
}
