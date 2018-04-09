all: client cloudServer pseudoBlockchain verifierServer
client: client.c
				gcc -o client client.c
cloudServer: cloudServer.c
				gcc -o cloudServer cloudServer.c
pseudoBlockchain: pseudoBlockchain.c
				gcc -o pseudoBlockchain pseudoBlockchain.c
verifierServer: verifierServer.c
				gcc -o verifierServer verifierServer.c
				
clean: 
			rm *.o
						
