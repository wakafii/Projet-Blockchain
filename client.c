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

int main(int argc, char **argv)
{
	int res = 0;	
	int port = htons(8000);
	int sock_service;
	int fin = 0;
	int nbAleat = 0;	

	char* buffer = (char*)calloc(10000,sizeof(char));
	/*strcpy(buffer, "GET /");
	strcat(buffer, argv[3]);
	strcat(buffer, " HTTP/1.1\n");
	strcat(buffer, "Host: ");
	strcat(buffer, argv[1]);
	strcat(buffer,":");
	strcat(buffer, argv[2]);
	strcat(buffer,"\n\n");*/

	int desc = 0;

	struct sockaddr_in addr_dest;
	struct hostent *hp;

	desc = creaSocket(AF_INET,SOCK_STREAM,0,port);
	if(desc == -1)
	{
		exit(0);
	}
	nbAleat = rand()%100;
	strcpy(buffer, "a478er58fgt54 7865dfghj521edfgn87541rf");

	if(/*nbAleat <= 50*/1)
	{

		/*******************************************************
		************ Provenance verifier ***********************
		*******************************************************/
		printf("dedans");
		hp = gethostbyname(argv[1]);
		if(hp == NULL)
		{
			perror("");
			exit(0);
		}
		addr_dest.sin_family = AF_INET;
		addr_dest.sin_port = htons(atoi(argv[2]));
		memcpy(&addr_dest.sin_addr.s_addr, hp->h_addr,hp->h_length);
		memset(addr_dest.sin_zero,0,8);

		res = connect(desc, (struct sockaddr *)&addr_dest, sizeof(addr_dest));
		if(res == -1)
		{
			perror("fail connect 1");
			exit(0);
		}

		if(write(desc, (void*)buffer,10000)<0){
			perror("");
			exit(0);
		}
		if(read(desc, (void*)buffer,10000)<0){
			perror("");
			exit(0);
		}

		/******************************************************/


		/******************************************************
		******************* Traitement message ****************
		******************************************************/

		if(buffer == "echec"){nbAleat = 1;}
		else{nbAleat = 50;}

		/*****************************************************/


		/*****************************************************
		************ Cloud Serveur ***************************
		*****************************************************/
		if(nbAleat == 50)
		{
			close(desc);
			desc = creaSocket(AF_INET,SOCK_STREAM,0,port);
			if(desc == -1)
			{
				exit(0);
			}

			strcpy(buffer, "a478er58fgt54");

			hp = gethostbyname(argv[3]);
			if(hp == NULL)
			{
				perror("");
				exit(0);
			}
			addr_dest.sin_family = AF_INET;
			addr_dest.sin_port = htons(atoi(argv[4]));
			memcpy(&addr_dest.sin_addr.s_addr, hp->h_addr,hp->h_length);

			res = connect(desc, (struct sockaddr *)&addr_dest, sizeof(addr_dest));
			if(res == -1)
			{
				perror("fail connect 2");
				exit(0);
			}

			if(write(desc, (void*)buffer,10000)<0){
				perror("");
				exit(0);
			}
			if(read(desc, (void*)buffer,10000)<0){
				perror("");
				exit(0);
			}

			/******************************************************/

			/******************************************************
			******************* Traitement message ****************
			******************************************************/

			if(buffer == "echec"){nbAleat = 1;}

			/*****************************************************/

			if(nbAleat == 50)
			{

				strcpy(buffer, "f56fsf44fsfsf8ff1fds3fzgg");

				if(write(desc, (void*)buffer,10000)<0){
					perror("");
					exit(0);
				}
				if(read(desc, (void*)buffer,10000)<0){
					perror("");
					exit(0);
				}
			}
		}

		printf("%s\n",buffer);
	}

	close(desc);
}
