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
	int i;
	EpidMessage em;
	int desc = 0;
	char* args[3];
	char msg_file[50];
	struct sockaddr_in addr_dest;
	struct hostent *hp;
	char ans[8];
	
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
	************ Communication with the Verifier ***********
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
	
	//Read the Verifier's answer
	if(read(desc,ans,8)<8)
	{
		fprintf(stderr, "ERROR: Failed to read the Verifier's answer (errno = %d)\n", errno);
		exit(EXIT_FAILURE);
	}

	if(strcmp(ans, "failure") == 0)
	{
		printf("The signature is not valid.\n");
		return EXIT_FAILURE;
	}
	else if(strcmp(ans, "success") == 0)
	{
		printf("The signature is valid.\n");
	}
	
	/*****************************************************
	************ Communication with the Cloud ************
	*****************************************************/
	
	//Send a connection request to the Cloud
	if(send(desc, em.id, sizeof(em.id)) < (int)sizeof(em.id))
	{
		fprintf(stderr, "ERROR: Failed to send the connection request to the Cloud (errno = %d)\n", errno);
		exit(EXIT_FAILURE);
	}
	
	//Receive the Cloud's answer
	bzero(ans, 8)
	if(read(desc, ans, 8)<8)
	{
		fprintf(stderr, "ERROR: Failed to read the Cloud's answer (errno = %d)\n", errno);
		exit(EXIT_FAILURE);
	}

	if(strcmp(ans, "failure") == 0)
	{
		printf("The Cloud refused the connection.\n");
		return EXIT_FAILURE;
	}
	else if(strcmp(ans, "success") == 0)
	{
		printf("The Cloud is waiting for a VPN connection.\n");
		
	}
	

	close(desc);
	return EXIT_SUCCESS;
}
