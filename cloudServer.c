#include <netinet/in.h> //pour AF_INET
#include <sys/types.h>	//pour les types de socket
#include <sys/socket.h>	//pour toutes les fct des socket
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

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

int clientBlockchain(int portSock, char* port, char* name, char* buffer)
{
		struct sockaddr_in addr_dest;
		struct hostent *hp;
		int res = 0;
		int desc = creaSocket(AF_INET,SOCK_STREAM,0,portSock);

		if(desc == -1)
		{
			perror("fail crea 1");
			return 0;
		}
		hp = gethostbyname(name);	//pour recuperer l'@ip de la cible
		if(hp == NULL)
		{
			perror("erreur1");
			exit(0);
		}
		addr_dest.sin_family = AF_INET;
		addr_dest.sin_port = htons(atoi(port));
		memcpy(&addr_dest.sin_addr.s_addr, hp->h_addr,hp->h_length);
		memset(addr_dest.sin_zero,0,8);

		res = connect(desc, (struct sockaddr *)&addr_dest, sizeof(addr_dest));
		if(res == -1)
		{
			perror("fail connect 1");
			return 0;
		}
		res = write(desc,(void*)buffer,10000);
		if(res == -1)
		{
			perror("fail write 1");
			return 1;
		}
		res = read(desc,buffer,10000);
		if(res == -1)
		{
			perror("fail read 1");
			return 1;
		}
		printf("%s\n",buffer);
		close(desc);
}

int main(int argc, char **argv)
{
	int res = 0;	
	int port = atoi(argv[1]);
	int sock_service;
	int desc = creaSocket(AF_INET,SOCK_STREAM,0,port);
	int fin = 0;
	int nbAleat = 0;
	int compt=0;

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
	char* buffer = (char*)calloc(10000,sizeof(char));
	char* buffer_data = (char*)calloc(10000,sizeof(char));
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

		compt = 0;
		while(buffer[compt] != '\0')
		{
			buffer_data[compt] = buffer[compt];
			compt++;
		}
		buffer_data[compt] = '\0';

		strcpy(buffer,"lecture ");
		strcat(buffer,buffer_data);
		printf("%s\n",buffer);
		clientBlockchain(htons(atoi(argv[1])+1), argv[2], argv[3], buffer);
		
		if(strcmp(buffer, "echec") == 0)
		{
			printf("KO\n");
			nbAleat = 50;
		}
		else
		{
			nbAleat = 1;
		}

		/*nbAleat = rand()%100;
		if(nbAleat <= 80)
		{
			strcpy(buffer, "reussite\0");
			nbAleat = 1;
		}
		else
		{
			strcpy(buffer, "echec\0");
		}*/
		/*************************************************/

		printf("%s\n",buffer);
		res = write(sock_service,buffer,10000);
		close(sock_service);

		if(nbAleat == 1)
		{
			sock_service = accept(desc, (struct sockaddr *)&addr_env, &longueur);
			res = read(sock_service,buffer,10000);
			printf("%s\n",buffer);

			strcpy(buffer, "OK\n");
			res = write(sock_service,buffer,10000);
			//printf("%s\n",buffer);
			close(sock_service);
		}
	}
	free(buffer);
	close(desc);
}
