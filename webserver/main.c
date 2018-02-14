#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "socket.h"
#include <signal.h>
#include <sys/wait.h>


/*-----------*/
#define SIZE_BUF 	4096

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


int main(/*int argc, char** argv*/){
	int socket_serveur, socket_client;
	const char* message_bienvenue = "\n\n\e[2m*******************************************************\n	neurilemma - noun [nu̇r-ə-ˈle-mə] : the plasma\n  membrane surrounding a Schwann cell of a myelinated\n  nerve fiber and separating layers of myelin\n*******************************************************\e[22m\n    \e[1m\e[4m\"Neurilemma ? Ne vous prenez pas la tête !\"\e[21m\e[24m\n\n\n    \e[1m=> Soyez les bienvenus sur Neurilemma ! <=\e[21m\n\n\n  Neurilemma est un serveur utilisé par les plus \ngrandes entreprises du monde tel que \e[1mGoogle\e[21m, \e[1mFacebook\e[21m,\n\e[1mTwitter\e[21m, \e[1mAmazon\e[21m, et maintenant \e[1mvous\e[21m :D\n\n  La force de ce projet est sa simplicité et son\nefficacité. En effet, Neurilimma à été pensé pour vous\nsimplifier la vie, par deux talentueux étudiants \e[1mMelvin\nBELEMBERT\e[21m et \e[1mMaxime MONS\e[21m, qui eux, pour le coup, se sont\nbien compliqués la vie pour le réaliser.\n\n\n\t\t\t\t\t\e[7mVersion 1.1 beta\e[27m\n\n";
	
	initialiser_signaux();

	socket_serveur = creer_serveur(8080);
	
	while(1){
		socket_client = accept (socket_serveur, NULL, NULL);

		if(socket_client == -1){
			perror ("accept");
			return -1;
		}

		if(fork() == 0){
			close(socket_serveur);
			write(socket_client, message_bienvenue, strlen(message_bienvenue));
			char buf[SIZE_BUF];


			while(1){
				bzero(buf,SIZE_BUF);
				if(read(socket_client, buf, SIZE_BUF) <= 0){
					perror("read");
					break;
				}else{
					if(write(socket_client, buf, strlen(buf)) <= 0){
						perror("write");
						break;
					}
				}
			}
			close(socket_client);	
			return 0;
		}
		close(socket_client);
	}
	return 0;
}