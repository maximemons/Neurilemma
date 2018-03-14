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

enum http_method{
	HTTP_GET ,
	HTTP_UNSUPPORTED ,
};
typedef struct{
	enum http_method method ;
	int major_version ;
	int minor_version ;
	char * target ;
}http_request ;

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
	if(code == 400) fprintf(client, "HTTP/1.1 400 Bad Request\r\nConnection: close\r\nContent-Length: 17\r\n\r\n400 Bad Request\r\n");
	else if(code == 404) fprintf(client, "HTTP/1.1 404 Not Found\r\nConnection: close\r\nContent-Length: 15\r\n\r\n404 Not Found\r\n");
	exit(0);
}
void goodRequest(FILE *client, const char* msg){
	fprintf(client, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n%s\r\n",(int)strlen(msg),msg);
}

char *fgets_or_exit(char *buffer, int size, FILE *stream){
	if(fgets(buffer, size, stream) == NULL) exit(0);
	return buffer;
}

int parse_http_request(char *request_line, http_request *request){
	char* token = strtok(request_line, " ");
	int i;
	for(i = 0; i < 3; i++){
		if(token == NULL) return 0;
		if(i == 0) request -> method = (strcmp(token, "GET") == 0 ? HTTP_GET : HTTP_UNSUPPORTED);
		else if(i == 1){
			request->target = (char*) malloc(sizeof(char)*(strlen(token)+1) );
			strcpy(request->target, token);
		}else
			if(strncmp(token, "HTTP/1.0", 8) == 0 || strncmp(token, "HTTP/1.1", 8) == 0){
				request -> major_version = (int)token[5];
				request -> minor_version = (int)token[7];
			}else return 0;
		token = strtok(NULL, " ");
	}return (strtok(NULL, " ") == NULL ? 1 : 0);
}

void skip_headers(char *buffer, int size, FILE *stream){
	while(fgets_or_exit(buffer, size, stream))
		if(strcmp(buffer, "\r\n") == 0) break;
}

void send_status(FILE *client, int code, char *reason_phrase){
	fprintf(client, "HTTP/1.1 %d %s\r\n", code, reason_phrase);
	fprintf(client, (code == 200 ? "Content-Length: " : "Connection: close\r\nContent-Length: "));
}

void send_response(FILE *client, int valid_request, http_request parsed_request, const char *message){
	int ok = 0;
	if(valid_request == 0){
		send_status(client, 400, "Bad Request");
		fprintf(client, "%d\r\n\r\n%s\r\n", (int)strlen("Error 400: Bad Request") + 2, "Error 400: Bad Request");
	}else if(parsed_request.method == HTTP_UNSUPPORTED){
		send_status(client, 405, "Method Not Allowed");
		fprintf(client, "%d\r\n\r\n%s\r\n", (int)strlen("Error 405: Method Not Allowed") + 2, "Error 405: Method Not Allowed");
	}else if(strcmp(parsed_request.target, "/") == 0){
		send_status(client, 200, "OK");
		fprintf(client, "%d\r\n\r\n%s\r\n", (int)strlen(message) + 2, message);
		ok = 1;
	}else{
		send_status(client, 404, "Not Found");
		fprintf(client, "%d\r\n\r\n%s\r\n", (int)strlen("Error 404: Not Found") + 2, "Error 404: Not Found");
	}if(ok == 0) exit(0);
}

char *rewrite_target(char *target){
	return (strchr(target, '?') != NULL ? strtok(target, "?") : target);
}

int main(int argc, char** argv){
	/***
	./neurilemma [ROOT] [PORT]
	***/
	int socket_serveur, socket_client;
	char *root;
	//const char* message_bienvenue = "\n\n\e[2m*******************************************************\n	neurilemma - noun [nu̇r-ə-ˈle-mə] : the plasma\n  membrane surrounding a Schwann cell of a myelinated\n  nerve fiber and separating layers of myelin\n*******************************************************\e[22m\n    \e[1m\e[4m\"Neurilemma ? Ne vous prenez pas la tête !\"\e[21m\e[24m\n\n\n    \e[1m=> Soyez les bienvenus sur Neurilemma ! <=\e[21m\n\n\n  Neurilemma est un serveur utilisé par les plus \ngrandes entreprises du monde tel que \e[1mGoogle\e[21m, \e[1mFacebook\e[21m,\n\e[1mTwitter\e[21m, \e[1mAmazon\e[21m, et maintenant \e[1mvous\e[21m :D\n\n  La force de ce projet est sa simplicité et son\nefficacité. En effet, Neurilimma à été pensé pour vous\nsimplifier la vie, par deux talentueux étudiants \e[1mMelvin\nBELEMBERT\e[21m et \e[1mMaxime MONS\e[21m, qui eux, pour le coup, se sont\nbien compliqués la vie pour le réaliser.\n\n\n\t\t\t\t\t\e[7mVersion 1.1 beta\e[27m\n\n\n> ";
	const char* message_bienvenue = "Hakuna Matata";

	initialiser_signaux();

	//On vérifie si on donne un port particulier au serveur
	socket_serveur = (argc == 2) ? creer_serveur(DEF_PORT) : creer_serveur(atoi(argv[2]));
	printf("Sever running on port %d\n", (argc == 2) ? DEF_PORT : atoi(argv[2]));
	fflush(stdout);
	root = (argc > 1) ? argv[1] : NULL;

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
				int valid_request;
				http_request parsed_request;
				//int request = 400; // 1 => OK; 400 => Bad Request; 404 => Not Found
				bzero(buf,SIZE_BUF);

				// On récupére la première ligne de l'en-tete
				fgets_or_exit(buf, SIZE_BUF, client);
				// On vérifie si la première ligne est du type GET / HTTP/1.* (* = {0;1}))
				valid_request = parse_http_request(buf, &parsed_request);

				// On lit jusqu'a la fin de l'en-tete, i.e. ligne = \r\n
				skip_headers(buf, SIZE_BUF, client);
				printf("->%d\n=>%s\n", valid_request, rewrite_target(parsed_request.target));
				fflush(stdout);

				// On envoie une réponse au client selon la nature de l'en-tete
				send_response(client, valid_request, parsed_request, message_bienvenue);
			}
			close(socket_client);	
			return 0;
		}
		close(socket_client);
	}
	return 0;
}