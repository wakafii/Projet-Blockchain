#include <netinet/in.h> //pour AF_INET
#include <sys/types.h>	//pour les types de socket
#include <sys/socket.h>	//pour toutes les fct des socket
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include "util/EpidMessage.h"
#include "util/fileio.h"
#include "util/calchash.h"
#include "signprocess.h"
#include "util/buffutil.h"

#define FILENAME "Hello.txt"

void delay(unsigned int milliseconds)
{
  clock_t start = clock();
  while((clock() - start) * 1000 / CLOCKS_PER_SEC < milliseconds);
}

int creaSocket(int Domaine, int Type, int Protocole, int Port)
{
	int descSocket = 0;
	struct sockaddr_in ad1;
	int res = 0;
	
	descSocket = socket(Domaine, Type, Protocole);
	if(descSocket == -1)
	{
		printf("echec creation, ligne %d\n", __LINE__);
		perror("");
		return -1;
		//exit(EXIT_FAILURE); normalement exit mais ne sert a rien dans une fonction
	}

	ad1.sin_family = Domaine;
	ad1.sin_port = htons(Port);
	ad1.sin_addr.s_addr = INADDR_ANY;
	memset(ad1.sin_zero,0,8);

	res = bind(descSocket,(struct sockaddr*)&ad1,sizeof(ad1));
	if(res == -1)
	{
		printf("echec bind socket, ligne %d\n", __LINE__);
		perror("");
		return -1;
		//exit(EXIT_FAILURE); normalement exit mais ne sert a rien dans une fonction
	}
	return descSocket;
}

int main(int argc, char **argv)
{
	int res = 0;	
	int port;
	//int sock_service;
	//int fin = 0;
	//int nbAleat = 0;
	//int compt = 0;
	int i;
	EpidMessage em;
	int desc = 0;
	char* args[3];
	char msg_file[50];
	/*strcpy(buffer, "GET /");
	strcat(buffer, argv[3]);
	strcat(buffer, " HTTP/1.1\n");
	strcat(buffer, "Host: ");
	strcat(buffer, argv[1]);
	strcat(buffer,":");
	strcat(buffer, argv[2]);
	strcat(buffer,"\n\n");*/
	struct sockaddr_in addr_dest;
	struct hostent *hp;
	
	if(argc != 3)
	{
		printf("How to use: ./member hostname port\n");
		exit(EXIT_FAILURE);
	}
	
	port = atoi(argv[2]);

	desc = creaSocket(AF_INET,SOCK_STREAM,0,0);
	if(desc == -1)
	{
		exit(EXIT_FAILURE);
	}



	/*******************************************************
	************ Provenance verifier ***********************
	*******************************************************/
	
	/**Message signature creation**/
	for(i=0; i<3; i++)
	{
		args[i] = (char*) malloc(sizeof(char)*65);
	}
	strcpy(msg_file, "--msgfile=");
	strcat(msg_file, FILENAME);
	strcpy(args[0], "./signmsg");
	strcpy(args[1], msg_file);
	strcpy(args[2], "--verbose");
	
	signprocess(2, args, &em);
	
	/**EpidMessage struct fill**/
	calc_sha256(FILENAME, em.id);
	//printf("%s's hash: %s\n", FILENAME, em.id);
	em.code = 0;
	em.sig_size = GetFileSize("sig.dat");
	em.msg_size = GetFileSize(FILENAME);
	/**Connection to the Provenance Verifier**/
	//printf("dedans\n");
	hp = gethostbyname(argv[1]);	//pour recuperer l'@ip de la cible
	if(hp == NULL)
	{
		perror("erreur1");
		exit(EXIT_FAILURE);
	}
	addr_dest.sin_family = AF_INET;
	addr_dest.sin_port = htons(port);
	memcpy(&addr_dest.sin_addr.s_addr, hp->h_addr, hp->h_length);
	memset(addr_dest.sin_zero,0,8);

	res = connect(desc, (struct sockaddr *)&addr_dest, sizeof(addr_dest));
	if(res == -1)
	{
		perror("Failed to connect to the Provenance Verifier");
		exit(EXIT_FAILURE);
	}
	
	//Send the EpidMessage
	if(send(desc, &em, sizeof(EpidMessage), 0) < 0)
  {
      fprintf(stderr, "ERROR: Failed to send EpidMessage. (errno = %d)\n", errno);
      exit(EXIT_FAILURE);
  }
	
	//Send the Signature
	if(sendfile(desc, "sig.dat"))
	{
		fprintf(stderr, "ERROR: Failed to send file sig.dat (errno = %d)\n", errno);
		exit(EXIT_FAILURE);
	}
	
	//Send the Public Key
	if(sendfile(desc, FILENAME))
	{
		fprintf(stderr, "ERROR: Failed to send file %s (errno = %d)\n", FILENAME, errno);
		exit(EXIT_FAILURE);
	}
	
	/*****************************************************
	************ Cloud Serveur ***************************
	*****************************************************/
	/*
	if(nbAleat == 50)
	{
		close(desc);
		desc = creaSocket(AF_INET,SOCK_STREAM,0,port);
		if(desc == -1)
		{
			exit(EXIT_FAILURE);
		}

		strcpy(buffer, "a478er58fgt54");

		hp = gethostbyname(argv[3]);
		if(hp == NULL)
		{
			perror("");
			exit(EXIT_FAILURE);
		}
		addr_dest.sin_family = AF_INET;
		addr_dest.sin_port = htons(atoi(argv[4]));
		memcpy(&addr_dest.sin_addr.s_addr, hp->h_addr,hp->h_length);

		res = connect(desc, (struct sockaddr *)&addr_dest, sizeof(addr_dest));
		if(res == -1)
		{
			perror("fail connect 2");
			exit(EXIT_FAILURE);
		}

		if(write(desc, (void*)buffer,10000)<0){
			perror("");
			exit(EXIT_FAILURE);
		}
		if(read(desc, (void*)buffer,10000)<0){
			perror("");
			exit(EXIT_FAILURE);
		}
		printf("%s\n",buffer);

		/******************************************************/

		/******************************************************
		******************* Traitement message ****************
		******************************************************/
	/*
		compt = 0;
		while(buffer[compt] != '\0')
		{
			buffer_res[compt] = buffer[compt];
			compt++;
		}
		buffer_res[compt] = '\0';

		if(strcmp(buffer_res, "echec") == 0)
		{
			printf("KOKO\n");
			nbAleat = 1;
		}
		else
		{
			nbAleat = 50;
		}

		/*****************************************************/
	/*
		if(nbAleat == 50)
		{
			close(desc);
			desc = creaSocket(AF_INET,SOCK_STREAM,0,port);
			if(desc == -1)
			{
				exit(EXIT_FAILURE);
			}
			hp = gethostbyname(argv[3]);
			if(hp == NULL)
			{
				perror("");
				exit(EXIT_FAILURE);
			}
			addr_dest.sin_family = AF_INET;
			addr_dest.sin_port = htons(atoi(argv[4]));
			memcpy(&addr_dest.sin_addr.s_addr, hp->h_addr,hp->h_length);

			res = connect(desc, (struct sockaddr *)&addr_dest, sizeof(addr_dest));
			if(res == -1)
			{
				perror("fail connect 2");
				exit(EXIT_FAILURE);
			}				
			
			strcpy(buffer, "f56fsf44fsfsf8ff1fds3fzgg");

			if(write(desc, (void*)buffer,10000)<0){
				perror("");
				exit(EXIT_FAILURE);
			}
			if(read(desc, (void*)buffer,10000)<0){
				perror("");
				exit(EXIT_FAILURE);
			}
			printf("%s\n",buffer);
		}
	}
	*/
	close(desc);
	return EXIT_SUCCESS;
}
