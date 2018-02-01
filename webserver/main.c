#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "socket.h"


int main(/*int argc, char** argv*/){
	
	int socket_serveur = creer_serveur(8080);
		
	int socket_client ;
	
	while(1){
		socket_client = accept ( socket_serveur , NULL , NULL );  /* Bloqué jusqua connexion d'un client */

		if ( socket_client == -1){
			perror ( " accept " );
			return -1;
		/* traitement d ’ erreur */
		}

		/* On peut maintenant dialoguer avec le client */
		const char* message_bienvenue = " Bonjour , bienvenue sur mon serveur \n" ;
		write ( socket_client , message_bienvenue , strlen ( message_bienvenue ));
		write ( socket_client , message_bienvenue , strlen ( message_bienvenue ));
		write ( socket_client , message_bienvenue , strlen ( message_bienvenue ));
	}




	return 0;
}
