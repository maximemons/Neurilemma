#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "socket.h"
#include <signal.h>
#include <sys/wait.h>


/*-----------*/
#define DEF_PORT 8080
#define SIZE_BUF 4096

/*-----------*/

void traitement_signal(int sig){
	printf("Signal %d reçu\n", sig);
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

void initialiser_signaux(void){
	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		perror("signal");
	struct sigaction sa;
	sa.sa_handler = traitement_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if(sigaction(SIGCHLD,&sa ,NULL) == -1)
		perror("sigaction (SIGCHLD)");
}

char *substring(char *src, int pos, int len){
	char *rep=NULL;
	if(len>0){
		rep=(char *)malloc(len+1);
		strncat(rep,src+pos,len);
	}
	return rep;
}
void badRequest(FILE *client){
	fprintf(client, "HTTP/1.1 400 Bad Request\r\nConnection: close\r\nContent-Length: 17\r\n\r\n400 Bad Request\r\n");
	exit(0);
}
void goodRequest(FILE *client){
	const char *msg="Connection OK\n";
	fprintf(client, "HTTP1.1 200 OK\r\nContent-Length: %d\r\n\r\n%s",(int)strlen(msg),msg);
}

void verif_requete(char * msg_client, FILE *client){
	if(strncmp(msg_client,"GET",3) != 0){
		fprintf(client,"ERROR GET \n");
		badRequest(client);
	}
	if(strncmp(substring(msg_client,3,1)," ",1) != 0){
		fprintf(client, "ERROR SPACE\n" );
		badRequest(client);
	}
	if(strncmp(substring(msg_client,6,8),"HTTP/1.1",8) != 0 && strncmp(substring(msg_client, 6,8),"HTTP/1.0",8)!=0 ){
		fprintf(client, "ERROR HTTP\n" );
		badRequest(client);
	}
	goodRequest(client);
}

int main(int argc, char** argv){
	int socket_serveur, socket_client;
	const char* message_bienvenue = "\n\n\e[2m*******************************************************\n	neurilemma - noun [nu̇r-ə-ˈle-mə] : the plasma\n  membrane surrounding a Schwann cell of a myelinated\n  nerve fiber and separating layers of myelin\n*******************************************************\e[22m\n    \e[1m\e[4m\"Neurilemma ? Ne vous prenez pas la tête !\"\e[21m\e[24m\n\n\n    \e[1m=> Soyez les bienvenus sur Neurilemma ! <=\e[21m\n\n\n  Neurilemma est un serveur utilisé par les plus \ngrandes entreprises du monde tel que \e[1mGoogle\e[21m, \e[1mFacebook\e[21m,\n\e[1mTwitter\e[21m, \e[1mAmazon\e[21m, et maintenant \e[1mvous\e[21m :D\n\n  La force de ce projet est sa simplicité et son\nefficacité. En effet, Neurilimma à été pensé pour vous\nsimplifier la vie, par deux talentueux étudiants \e[1mMelvin\nBELEMBERT\e[21m et \e[1mMaxime MONS\e[21m, qui eux, pour le coup, se sont\nbien compliqués la vie pour le réaliser.\n\n\n\t\t\t\t\t\e[7mVersion 1.1 beta\e[27m\n\n\n> ";
	
	initialiser_signaux();

	if(argc > 1){
		socket_serveur = creer_serveur(atoi(argv[1]));
		printf("Sever running on port %d\n", atoi(argv[1]));
	}
	else{
		socket_serveur = creer_serveur(DEF_PORT);
		printf("Sever running on port %d\n", DEF_PORT);
	}fflush(stdout);

	while(1){
		socket_client = accept (socket_serveur, NULL, NULL);

		if(socket_client == -1){
			perror("accept");
			return -1;
		}

		if(fork() == 0){
			close(socket_serveur);
			//write(socket_client, message_bienvenue, strlen(message_bienvenue));
			char buf[SIZE_BUF];

			FILE *client = fdopen(socket_client, "w+");
			while(1){
				bzero(buf,SIZE_BUF);
				if(fgets(buf, SIZE_BUF, client) == NULL) break;

				// 1er Fgets pour récuprer la premiere ligne (la ligne GET)

				// Analyser la requetet ... sauvegarder le resultat dans un boolean 

				// while (fgets(buf)){ 
					/*if (buf != "\r\n"){
						on a trouver la ligne vide !!!
						break;
					}*/
				//}

				// Fin header


				// Envoyer réponse !!!



				//if(fprintf(client, "\033[1m<Neurilemma>:\033[21m %s\n> ", buf) < 0) break;
				verif_requete(message_bienvenue, client);

			}
			close(socket_client);	
			return 0;
		}
		close(socket_client);
	}
	return 0;
}