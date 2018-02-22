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
void badRequest(FILE *client, int code){
	if(code == 400)fprintf(client, "HTTP/1.1 400 Bad Request\r\nConnection: close\r\nContent-Length: 17\r\n\r\n400 Bad Request\r\n");
	if(code == 404)fprintf(client, "HTTP/1.1 400 Not Found\r\nConnection: close\r\nContent-Length: 15\r\n\r\n404 Not Found\r\n");
	exit(0);
}
void goodRequest(FILE *client, const char* msg){
	fprintf(client, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n%s\r\n",(int)strlen(msg),msg);
}

int main(int argc, char** argv){
	int socket_serveur, socket_client;
	//const char* message_bienvenue = "\n\n\e[2m*******************************************************\n	neurilemma - noun [nu̇r-ə-ˈle-mə] : the plasma\n  membrane surrounding a Schwann cell of a myelinated\n  nerve fiber and separating layers of myelin\n*******************************************************\e[22m\n    \e[1m\e[4m\"Neurilemma ? Ne vous prenez pas la tête !\"\e[21m\e[24m\n\n\n    \e[1m=> Soyez les bienvenus sur Neurilemma ! <=\e[21m\n\n\n  Neurilemma est un serveur utilisé par les plus \ngrandes entreprises du monde tel que \e[1mGoogle\e[21m, \e[1mFacebook\e[21m,\n\e[1mTwitter\e[21m, \e[1mAmazon\e[21m, et maintenant \e[1mvous\e[21m :D\n\n  La force de ce projet est sa simplicité et son\nefficacité. En effet, Neurilimma à été pensé pour vous\nsimplifier la vie, par deux talentueux étudiants \e[1mMelvin\nBELEMBERT\e[21m et \e[1mMaxime MONS\e[21m, qui eux, pour le coup, se sont\nbien compliqués la vie pour le réaliser.\n\n\n\t\t\t\t\t\e[7mVersion 1.1 beta\e[27m\n\n\n> ";
	const char* message_bienvenue = "Hakuna Matata";

	initialiser_signaux();

	//On vérifie si on donne un port particulier au serveur
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
				int request = 400; // 1 => OK; 400 => Bad Request; 404 => Not Found
				bzero(buf,SIZE_BUF);

				// On récupére la première ligne de l'en-tete
				if(fgets(buf, SIZE_BUF, client) == NULL) break;

				// On vérifie si la première ligne est du type GET / HTTP/1.* (* = {0;1})
				char* token = strtok(buf, " ");
				char *comp[4] = {"GET", "/", "HTTP/1.0\r\n", "HTTP/1.1\r\n"};
				int cpt;
				int valide = 0;
				for(cpt = 0; cpt < 4; cpt++){
					if(strcmp(token, comp[cpt]) != 0) valide++;
					if(cpt != 2) token = strtok(NULL, " ");
				}if(valide == 1) request = 1;
				else{
					token = strtok(buf, " ");
					if(strcmp(token, "GET") == 0) request = 404; 
				}

				// On lit jusqu'a la fin de l'en-tete, i.e. ligne = \r\n
				while(fgets(buf, SIZE_BUF, client))
					if(strcmp(buf, "\r\n") == 0) break;

				// On envoie une réponse au client selon la nature de l'en-tete
				if(request) goodRequest(client, message_bienvenue);
				else badRequest(client, request);
			}
			close(socket_client);	
			return 0;
		}
		close(socket_client);
	}
	return 0;
}