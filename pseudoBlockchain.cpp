#include <netinet/in.h> //pour AF_INET
#include <sys/types.h>	//pour les types de socket
#include <sys/socket.h>	//pour toutes les fct des socket
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <iostream>

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
	enum action { ECRITURE, LECTURE };
	int port = atoi(argv[1]);
	int sock_service;
	int desc = creaSocket(AF_INET,SOCK_STREAM,0,port);
	int fin = 0;
	action nbAleat;
	int compt = 0;
	int compt_res = 0;
	std::vector<char *> ID;
	std::vector<char *> Keys;

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

	std::cout<<"attente connexion...\n"<<std::endl;

	struct sockaddr_in addr_env;
	int longueur = sizeof(addr_env);
	char* buffer = (char*)calloc(10000,sizeof(char));
	char* buffer_action = (char*)calloc(50,sizeof(char));
	char* buffer_signature = (char*)calloc(150,sizeof(char));
	char* buffer_data = (char*)calloc(150,sizeof(char));
	while(!fin)
	{
		sock_service = accept(desc, (struct sockaddr *)&addr_env, (socklen_t *)&longueur);
		if(sock_service == -1)
		{
			perror("error accept");
			exit(0);
		}

		res = read(sock_service,buffer,10000);
		std::cout<<buffer<<"\n"<<std::endl;

		/**************************************************
		**************** traitement message ***************
		**************************************************/
		compt = 0;
		while(buffer[compt] != ' ')
		{
			buffer_action[compt] = buffer[compt];
			compt++;
		}
		buffer_action[compt] = '\0';

		compt++;
		compt_res = 0;

		if(strcmp(buffer_action, "lecture") == 0)
		{
			nbAleat = LECTURE;
			while(buffer[compt] != '\0')
			{
				buffer_signature[compt_res] = buffer[compt];
				compt++;
				compt_res++;
			}
			buffer[compt_res] = '\0';
		}
		else if(strcmp(buffer_action, "ecriture") == 0)
		{
			nbAleat = ECRITURE;
			while(buffer[compt] != ' ')
			{
				buffer_signature[compt_res] = buffer[compt];
				compt++;
				compt_res++;
			}
			buffer_signature[compt_res] = '\0';

			compt++;
			compt_res=0;

			while(buffer[compt] != '\0')
			{
				buffer_data[compt_res] = buffer[compt];
				compt++;
				compt_res++;
			}
			buffer_data[compt_res] = '\0';
		}
		/*************************************************/

		if(nbAleat == LECTURE)
		{
			for(compt = 0;compt<ID.size();compt++)
			{
				if(strcmp(buffer_signature, Keys[compt]) == 0)
				{
					strcpy(buffer, Keys[compt]);
					break;
				}
				strcpy(buffer, "echec\n");
			}
		}
		else if(nbAleat == ECRITURE)
		{
			ID.push_back(buffer_signature);
			Keys.push_back(buffer_data);
			strcpy(buffer, "OK\n");
		}

		res = write(sock_service,buffer,10000);
		//printf("%s\n",buffer);
		std::cout<<buffer<<"\n"<<std::endl;
		close(sock_service);
	}
	free(buffer);
	close(desc);
}
